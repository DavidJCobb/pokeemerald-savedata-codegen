#include "./registry.h"
#include <bit>
#include <fstream>
#include <stdexcept>
#include <string_view>
#include "RapidXml/rapidxml.hpp"
#include "lu/rapidxml_helpers/for_each_attribute.h"
#include "lu/rapidxml_helpers/for_each_child_element.h"
#include "lu/rapidxml_helpers/get_sole_child_element_of_name.h"
#include "lu/rapidxml_helpers/get_attribute.h"
#include "lu/rapidxml_helpers/get_element_text_content.h"
#include "lu/strings/to_integer.h"

#include "./ast/constant_definition.h"
#include "./ast/heritable.h"
#include "./ast/member.h"
#include "./ast/structure.h"

using namespace std::literals::string_literals;

std::filesystem::path registry::_normalize_path(const std::filesystem::path& p) {
   std::filesystem::path out = p;
   if (out.is_relative()) {
      out = this->path / out;
   }
   out = std::filesystem::weakly_canonical(out);
   out = out.make_preferred();
   return out;
}
std::optional<size_t> registry::_size_or_constant_name_to_size(const std::string_view& text) {
   bool   ok;
   size_t out = lu::strings::to_integer<std::size_t>(text.data(), text.size(), &ok);
   if (ok)
      return out;
   
   auto ugh = std::string(text);
   if (this->constants.contains(ugh)) {
      return this->constants[ugh]->value;
   }
   return {};
}
std::optional<std::intmax_t> registry::_int_or_constant_name_to_int(const std::string_view& text) {
   bool ok;
   auto out = lu::strings::to_integer<std::intmax_t>(text.data(), text.size(), &ok);
   if (ok)
      return out;

   auto ugh = std::string(text);
   if (this->constants.contains(ugh)) {
      return this->constants[ugh]->value;
   }
   return {};
}

void registry::_parse_and_handle_dependencies(parse_wrapper& scaffold, rapidxml::xml_node<>& base_node) {
   using namespace rapidxml;

   lu::rapidxml_helpers::for_each_child_element(
      base_node,
      "xml",
      [this, &scaffold](xml_node<>& node) {
         auto* attr = lu::rapidxml_helpers::get_attribute(node, "path");
         if (!attr) {
            scaffold.error("Dependency is missing its `path` attribute!", node);
         }

         auto specified = std::string_view(attr->value(), attr->value_size());
         std::filesystem::path resolved = _normalize_path(specified);
         for (const auto& seen : this->seen_files) {
            if (resolved == seen) {
               return;
            }
         }

         //
         // This dependency is not yet parsed.
         //
         std::ifstream file_in(resolved);
         if (!file_in) {
            scaffold.error(std::string("Failed to open dependency: ") + std::string(specified) + " (tried to load from " + resolved.string().c_str() + ")", node);
         }

         parse_wrapper nested_scaffold;
         nested_scaffold.set_path(resolved);
         std::string xml;
         {
            file_in.seekg(0, std::ios::end);
            xml.reserve(file_in.tellg());
            file_in.seekg(0, std::ios::beg);
            xml.assign(std::istreambuf_iterator<char>(file_in), std::istreambuf_iterator<char>());
         }
         nested_scaffold.set_data(xml);

         this->seen_files.push_back(resolved);
         this->parse_xml(nested_scaffold);
      },
      [&scaffold](const std::string_view& tagname, xml_node<>& node) {
         scaffold.warn(std::string("Unrecognized child node (tagname ") + std::string(tagname) + ")", node); // can't wait for P2591...
      }
   );
}

void registry::_parse_constants(parse_wrapper& scaffold, rapidxml::xml_node<>& base_node) {
   using namespace rapidxml;

   std::string current_header;

   auto _parse_single_constant = [this, &scaffold, &current_header]<bool HeaderAlreadyKnown = false>(xml_node<>& node) {
      auto item = std::make_unique<ast::constant_definition>();

      lu::rapidxml_helpers::for_each_attribute(node, [&scaffold, &item](std::string_view attr_name, std::string_view attr_value, xml_attribute<>& attr) {
         if (attr_name == "name") {
            item->name = attr_value;
         } else if (attr_name == "value") {
            bool ok;
            item->value = lu::strings::to_integer<std::intmax_t>(attr_value.data(), attr_value.size(), &ok);
            if (!ok) {
               scaffold.error("Constant defined with invalid value.", attr);
            }
         } else if (attr_name == "header-path") {
            if constexpr (HeaderAlreadyKnown) {
               scaffold.error("Constant inside of HEADER tag trying to specify its own header path.", attr);
            } else {
               item->header = attr_value;
            }
         }
      });

      if (item->name.empty()) {
         scaffold.error("Constant defined with empty or missing name", node);
      }

      this->constants[item->name] = std::move(item);
   };

   for (xml_node<>* child = base_node.first_node(); child; child = child->next_sibling()) {
      if (child->type() != node_type::node_element)
         continue;

      auto child_tagname = std::string(child->name(), child->name_size());
      if (child_tagname == "constant") {
         _parse_single_constant(*child);
      } else if (child_tagname == "header") {
         std::string header_path;
         if (auto* attr = lu::rapidxml_helpers::get_attribute(*child, "path")) {
            header_path = std::string(attr->value(), attr->value_size());
            if (header_path.empty()) {
               scaffold.error("Constant header defined with empty path", *child);
            }
         } else {
            scaffold.error("Constant header is missing its path attribute.", *child);
         }

         lu::rapidxml_helpers::for_each_child_element(
            *child,
            "constant",
            [&_parse_single_constant](xml_node<>& node) {
               _parse_single_constant.operator()<true>(node);
            },
            [&scaffold](const std::string_view& tagname, xml_node<>& node) {
               scaffold.warn(std::string("Unrecognized child node (tagname ") + std::string(tagname) + ")", node); // can't wait for P2591...
            }
         );
      } else {
         scaffold.warn(std::string("Unrecognized child node (tagname ") + child_tagname + ")", *child);
      }
   }
}
void registry::_parse_heritables(parse_wrapper& scaffold, rapidxml::xml_node<>& base_node) {
   using namespace rapidxml;

   lu::rapidxml_helpers::for_each_child_element(base_node, [this, &scaffold](std::string_view tagname, xml_node<>& node) {
      auto  heritable = std::make_unique<ast::heritable>();
      auto& semantic  = heritable->semantic_info;

      if (tagname == "string") {
         semantic.emplace<ast::semantic::string_info>();
      } else if (tagname == "number") {
         semantic.emplace<ast::semantic::number_info>();
      } else {
         auto pn = ast::c_type::primitive_from_string(tagname);
         if (pn.has_value()) {
            heritable->number_c_type = pn.value();
            semantic.emplace<ast::semantic::number_info>();
         } else {
            scaffold.warn(std::string("Unrecognized child node (tagname ") + std::string(tagname) + ")", node); // can't wait for P2591...
            return;
         }
      }

      if (std::holds_alternative<ast::semantic::number_info>(semantic)) {
         auto& casted = std::get<ast::semantic::number_info>(semantic);
         lu::rapidxml_helpers::for_each_attribute(node, [this, &scaffold, &heritable, &casted](std::string_view attr_name, std::string_view attr_value, xml_attribute<>& attr) {
            if (attr_name == "name") {
               heritable->name = attr_value;
            } else if (attr_name == "min") {
               auto value = _int_or_constant_name_to_int(attr_value);
               if (!value.has_value()) {
                  scaffold.error(std::string("Heritable: Number min value is an unrecognized constant or otherwise not a valid integral (seen: ") + std::string(attr_value) + ")", attr); // can't wait for P2591...
               }
               casted.min = value.value();
            } else if (attr_name == "max") {
               auto value = _int_or_constant_name_to_int(attr_value);
               if (!value.has_value()) {
                  scaffold.error(std::string("Heritable: Number max value is an unrecognized constant or otherwise not a valid integral (seen: ") + std::string(attr_value) + ")", attr); // can't wait for P2591...
               }
               casted.max = value.value();
            }
         });
      } else if (std::holds_alternative<ast::semantic::string_info>(semantic)) {
         auto& casted = std::get<ast::semantic::string_info>(semantic);
         lu::rapidxml_helpers::for_each_attribute(node, [this, &scaffold, &heritable, &casted](std::string_view attr_name, std::string_view attr_value, xml_attribute<>& attr) {
            if (attr_name == "name") {
               heritable->name = attr_value;
            } else if (attr_name == "char-type") {
               auto prim = ast::c_type::primitive_from_string(attr_value);
               if (!prim.has_value()) {
                  scaffold.error(std::string("Heritable: String char type is unrecognized (seen: ") + std::string(attr_value) + ")", attr); // can't wait for P2591...
               }
               casted.char_type = prim.value();
            } else if (attr_name == "length") {
               auto value = _size_or_constant_name_to_size(attr_value);
               if (!value.has_value()) {
                  scaffold.error(std::string("Heritable: String max length is an unrecognized constant or otherwise not a valid integral (seen: ") + std::string(attr_value) + ")", attr); // can't wait for P2591...
               }
               casted.max_length = value.value();
            }
         });
      }

      if (heritable->name.empty()) {
         scaffold.error("Heritable defined with empty or missing name", node);
      }
      if (this->heritables.contains(heritable->name)) {
         scaffold.error("Heritable: A heritable named \"" + heritable->name + "\" already exists.", node);
      }
      this->heritables[heritable->name] = std::move(heritable);
   });
}
std::unique_ptr<ast::member> registry::_parse_member(parse_wrapper& scaffold, rapidxml::xml_node<>& node, bool& out_is_checksum) {
   using namespace rapidxml;

   out_is_checksum = false;
   const auto tagname = std::string_view(node.name(), node.name_size());

   if (tagname == "union") {
      auto  field      = std::make_unique<ast::member>();
      auto& union_info = field->c_union.emplace();

      if (auto* attr = lu::rapidxml_helpers::get_attribute(node, "name")) {
         field->name = std::string_view(attr->value(), attr->value_size());
      } else {
         scaffold.error("Field: Anonymous unions are not allowed in GCC C90.", node);
      }
      if (auto* attr = lu::rapidxml_helpers::get_attribute(node, "member-to-serialize")) {
         union_info.member_to_serialize = std::string_view(attr->value(), attr->value_size());
      } else {
         scaffold.error("Field: You must specify a union member to serialize.", node);
      }

      lu::rapidxml_helpers::for_each_child_element(node, [this, &scaffold, &union_info](std::string_view tagname, xml_node<>& node) {
         bool is_checksum;
         auto field = this->_parse_member(scaffold, node, is_checksum);
         if (is_checksum) {
            scaffold.error("Field: Checksums nested in unions are not supported.", node);
         }
         union_info.members.push_back(*field);
      });
      {
         bool found = false;
         for (const auto& m : union_info.members) {
            if (m.name == union_info.member_to_serialize) {
               found = true;
               break;
            }
         }
         if (!found) {
            scaffold.error(std::string("Field: You must specify a union member to serialize (member \"") + union_info.member_to_serialize + "\") was not found.", node);
         }
      }

      return field;
   }

   auto field = std::make_unique<ast::member>();
   {
      auto prim = ast::c_type::primitive_from_string(tagname);
      if (prim.has_value()) {
         if (prim.value() != ast::c_type::primitive::bool8)
            field->semantic_info.emplace<ast::semantic::number_info>();
         field->c_type_info.name = prim.value();
      } else {
         if (tagname != "field") {
            scaffold.warn(std::string("Unrecognized child node (tagname ") + std::string(tagname) + ")", node); // can't wait for P2591...
            return nullptr;
         }
      }
   }

   ast::heritable* inherit_from = nullptr;

   lu::rapidxml_helpers::for_each_attribute(node, [this, &scaffold, &field, &inherit_from, &out_is_checksum](std::string_view attr_name, std::string_view attr_value, xml_attribute<>& attr) {
      if (attr_name == "name") {
         field->name = attr_value;
      } else if (attr_name == "c-bitfield") {
         auto value = _size_or_constant_name_to_size(attr_value);
         if (!value.has_value()) {
            scaffold.error(std::string("Field: Bitfield bitcount is an unrecognized constant or otherwise not a valid integral (seen: ") + std::string(attr_value) + ")", attr); // can't wait for P2591...
         }
         field->c_type_info.bitfield_size = value.value();
      } else if (attr_name == "c-type") {
         field->c_type_info.set_type_name(attr_value);
      } else if (attr_name == "c-type-decl") {
         if (attr_value == "struct") {
            field->c_type_info.is_struct = true;
         } else if (attr_value == "union") {
            field->c_type_info.is_union = true;
         } else {
            scaffold.error(std::string("Field: Unrecognized c-type-decl (seen: ") + std::string(attr_value) + ")", attr); // can't wait for P2591...
         }
      } else if (attr_name == "do-not-serialize") {
         if (attr_value == "true") {
            field->skip_when_serializing = true;
         } else if (attr_value == "false") {
            field->skip_when_serializing = false;
         } else {
            scaffold.warn(std::string("Field: Unrecognized attribute value for `do-not-serialize` (seen: ") + std::string(attr_value) + ")", attr); // can't wait for P2591...
         }
      } else if (attr_name == "inherit") {
         auto av = std::string(attr_value);
         if (this->heritables.contains(av)) {
            inherit_from = this->heritables[av].get();
         } else {
            scaffold.warn(std::string("Field: Failed to find heritable (seen: ") + std::string(attr_value) + ")", attr); // can't wait for P2591...
         }
      } else if (attr_name == "is-checksum") {
         if (attr_value == "true") {
            out_is_checksum = true;
         }
      } else if (attr_name == "min") {
         auto value = _int_or_constant_name_to_int(attr_value);
         if (!value.has_value()) {
            scaffold.error(std::string("Field: Number min value is an unrecognized constant or otherwise not a valid integral (seen: ") + std::string(attr_value) + ")", attr); // can't wait for P2591...
         }

         auto& si = field->semantic_info;
         if (std::holds_alternative<std::monostate>(si)) {
            si.emplace<ast::semantic::number_info>();
         }
         if (!std::holds_alternative<ast::semantic::number_info>(si)) {
            scaffold.warn(std::string("Field: Unrecognized attribute (name: ") + std::string(attr_name) + ") (field is not a number)", attr); // can't wait for P2591...
            return;
         }

         std::get<ast::semantic::number_info>(si).min = value.value();
      } else if (attr_name == "max") {
         auto value = _int_or_constant_name_to_int(attr_value);
         if (!value.has_value()) {
            scaffold.error(std::string("Field: Number max value is an unrecognized constant or otherwise not a valid integral (seen: ") + std::string(attr_value) + ")", attr); // can't wait for P2591...
         }

         auto& si = field->semantic_info;
         if (std::holds_alternative<std::monostate>(si)) {
            si.emplace<ast::semantic::number_info>();
         }
         if (!std::holds_alternative<ast::semantic::number_info>(si)) {
            scaffold.warn(std::string("Field: Unrecognized attribute (name: ") + std::string(attr_name) + ") (field is not a number)", attr); // can't wait for P2591...
            return;
         }

         std::get<ast::semantic::number_info>(si).max = value.value();
      } else {
         scaffold.warn(std::string("Field: Unrecognized attribute (name: ") + std::string(attr_name) + ")", attr); // can't wait for P2591...
      }
   });
   if (field->name.empty()) {
      scaffold.error("Field defined with an empty or missing name", node);
   }
   if (inherit_from) {
      field->inherit_from(*inherit_from);
   }

   lu::rapidxml_helpers::for_each_child_element(node, [this, &scaffold, &field](std::string_view tagname, xml_node<>& node) {
      if (tagname == "array-rank") {
         ast::c_type::array_rank rank;

         lu::rapidxml_helpers::for_each_attribute(node, [this, &scaffold, &rank](std::string_view attr_name, std::string_view attr_value, xml_attribute<>& attr) {
            if (attr_name == "extent") {
               auto value = _size_or_constant_name_to_size(attr_value);
               if (!value.has_value()) {
                  scaffold.error(std::string("Field: Array rank is an unrecognized constant or otherwise not a valid integral (seen: ") + std::string(attr_value) + ")", attr); // can't wait for P2591...
               }
               rank.extent = value.value();
            } else if (attr_name == "extent-expr") {
               rank.extent_expr = attr_value;
            } else {
               scaffold.warn(std::string("Field: Unrecognized attribute (name: ") + std::string(attr_name) + ")", attr); // can't wait for P2591...
            }
         });

         field->c_type_info.array_extents.push_back(rank);
      } else if (tagname == "checksum-of") {
         //
         // This data is unused for now.
         //
      } else if (tagname == "line-comment") {
         try {
            field->c_line_comment = lu::rapidxml_helpers::get_element_text_content<false>(node);
         } catch (std::runtime_error) {
            scaffold.error("Field `line-comment` node contains element children", node);
         }
      } else {
         scaffold.warn(std::string("Field: Unrecognized child node (tagname: ") + std::string(tagname) + ")", node); // can't wait for P2591...
      }
   });

   field->resolve_all_specifications();
   return field;
}
void registry::_parse_types(parse_wrapper& scaffold, rapidxml::xml_node<>& base_node) {
   using namespace rapidxml;

   for (xml_node<>* node = base_node.first_node(); node; node = node->next_sibling()) {
      auto tagname = std::string(node->name(), node->name_size());
      if (tagname != "struct") {
         scaffold.warn(std::string("Unrecognized child node (tagname ") + tagname + ")", *node);
         continue;
      }

      //
      // Handle structure attributes
      //
      auto structure = std::make_unique<ast::structure>();
      lu::rapidxml_helpers::for_each_attribute(*node, [&scaffold, &structure](std::string_view attr_name, std::string_view attr_value, xml_attribute<>& attr) {
         if (attr_name == "name") {
            structure->name = attr_value;
         } else if (attr_name == "header") {
            structure->header = attr_value;
         } else {
            scaffold.warn("Unrecognized attribute (name "s + std::string(attr_name) + ")", attr); // can't wait for P2591...
         }
      });
      if (structure->name.empty()) {
         scaffold.error("Struct defined with empty or missing name", *node);
      }

      //
      // Find fields node
      //
      xml_node<>* fields_node;
      {
         size_t seen = 0;
         fields_node = lu::rapidxml_helpers::get_sole_child_element_of_name(*node, "fields", &seen);
         if (seen > 1) {
            scaffold.error("Struct defined with multiple field container nodes", *node);
         } else if (seen == 0) {
            scaffold.error("Struct defined without a field container node", *node);
         }
      }

      //
      // Parse fields
      //
      lu::rapidxml_helpers::for_each_child_element(*fields_node, [this, &scaffold, &structure](std::string_view tagname, xml_node<>& node) {
         bool is_checksum;
         auto field = this->_parse_member(scaffold, node, is_checksum);
         structure->members.push_back(*field);
         if (is_checksum) {
            if (structure->checksum_member_index.has_value()) {
               scaffold.error(
                  std::string("We only support one checksum member per struct. (Existing member: ")
                     + structure->members[structure->checksum_member_index.value()].name
                     + "; new member: "
                     + field->name
                     + ")",
                  node
               );
            }
            structure->checksum_member_index = structure->members.size() - 1;
         }
      });

      //
      // Done
      //
      this->structs[structure->name] = std::move(structure);
   }
}

void registry::parse_xml(lu::rapidxml_helpers::parsing_scaffold& scaffold) {
   using namespace rapidxml;

   constexpr const auto parse_flags = parse_validate_closing_tags | parse_trim_whitespace;

   xml_document<> doc;
   try {
      doc.parse<parse_flags>(scaffold.get_data());
   } catch (rapidxml::parse_error& e) {
      scaffold.error(e);
   }

   xml_node<>* doc_root = doc.first_node("data");
   if (!doc_root) {
      scaffold.error("No root node with tagname `data` present, or multiple root nodes and first is not `data`.", doc);
   }

   for (xml_node<>* child = doc_root->first_node(); child; child = child->next_sibling()) {
      auto child_tagname = std::string(child->name(), child->name_size());
      if (child_tagname == "dependencies") {
         this->_parse_and_handle_dependencies(scaffold, *child);
      } else if (child_tagname == "constants") {
         this->_parse_constants(scaffold, *child);
      } else if (child_tagname == "heritables") {
         this->_parse_heritables(scaffold, *child);
      } else if (child_tagname == "types") {
         this->_parse_types(scaffold, *child);
      }
   }
}

void registry::parse_all_xml_files() {
   for (const auto& entry : std::filesystem::directory_iterator(this->path)) {
      if (entry.is_directory())
         continue;

      auto current_path = _normalize_path(entry.path());
      if (!current_path.string().ends_with(".xml"))
         continue;

      {
         bool already = false;
         for (const auto& seen : this->seen_files) {
            if (current_path == seen) {
               already = true;
               break;
            }
         }
         if (already)
            continue;
      }

      parse_wrapper scaffold;
      scaffold.set_path(current_path);

      std::ifstream file_in(current_path);
      if (!file_in) {
         scaffold.file_error(std::string("Failed to open file: ") + current_path.string().c_str());
      }

      std::string xml;
      {
         file_in.seekg(0, std::ios::end);
         xml.reserve(file_in.tellg());
         file_in.seekg(0, std::ios::beg);
         xml.assign(std::istreambuf_iterator<char>(file_in), std::istreambuf_iterator<char>());
      }
      scaffold.set_data(xml);

      this->seen_files.push_back(current_path);
      this->parse_xml(scaffold);
   }
}

size_t registry::bitcount_of_struct(std::string name) const {
   if (!this->structs.contains(name))
      return 0;
   const auto& structure = this->structs.at(name);

   size_t bitcount = 0;
   for (const auto& member : structure->members) {
      bitcount += member.compute_bitcount();
   }
   return bitcount;
}

void registry::generate_all_struct_body_files(std::filesystem::path out_folder) {
   //
   // TODO: We need to remember all constants seen when parsing a struct, so we can 
   // emit preprocessor directives to assert that they have the expected values. We 
   // DO NOT need to pull in the headers for these constants (and in fact, can't, 
   // since we're generating code for the innards of struct definitions) since the 
   // constants should already be included/defined at the time the structs themselves 
   // are defined.
   //
   for (auto& pair : this->structs) {
      std::ofstream stream(out_folder / std::filesystem::path(pair.first + ".members.inl"));
      assert(!!stream);

      stream << pair.second->as_c_member_declaration_list();
   }
}

void registry::generate_serialization_code(std::filesystem::path out_folder) {
   for (auto& pair : this->structs) {
      {  // h
         std::ofstream stream(out_folder / std::filesystem::path("serialize_"s + pair.first + ".h"));
         assert(!!stream);

         stream << "#ifndef GUARD_LU_SERIALIZE_" << pair.first << "\n";
         stream << "#define GUARD_LU_SERIALIZE_" << pair.first << "\n";
         stream << '\n';
         stream << "void lu_BitstreamRead_" << pair.first << "(struct lu_BitstreamState*, struct " << pair.first << "* dst);";
         stream << "void lu_BitstreamWrite_" << pair.first << "(struct lu_BitstreamState*, struct " << pair.first << "* src);";
         stream << '\n';
         stream << "#endif";
      }
      {  // cpp
         std::ofstream stream(out_folder / std::filesystem::path("serialize_"s + pair.first + ".cpp"));
         assert(!!stream);

         stream << "#include \"" << out_folder.string() << "/serialize_" << pair.first << ".h\"\n\n";

         std::vector<std::string> dependencies;
         bool has_any_strings = false;
         for (const auto& m : pair.second->members) {
            if (std::holds_alternative<std::string>(m.c_type_info.name)) {
               const auto& name = std::get<std::string>(m.c_type_info.name);

               bool already = false;
               for (auto& prior : dependencies) {
                  if (prior == name) {
                     already = true;
                     break;
                  }
               }
               if (already)
                  continue;

               dependencies.push_back(name);
            }
            if (!has_any_strings) {
               if (std::holds_alternative<ast::semantic::string_info>(m.semantic_info)) {
                  has_any_strings = true;
               }
            }
         }

         if (dependencies.size()) {
            stream << "// dependencies\n";
            for (const auto& name : dependencies) {
               stream << "#include \"./serialize_" << name << ".h\"\n";
            }
            stream << '\n';
         }
         if (has_any_strings) {
            stream << "#include \"string_util.h\" // gflib; for StringLength\n\n";
         }
         if (!pair.second->header.empty()) {
            stream << "#include \"" << pair.second->header << "\"\n\n";
         }

         //
         // TODO: Do the BitstreamRead function, too. Zero-fill the target struct before reading.
         //
         stream << "// TODO:\n";
         stream << "// void lu_BitstreamRead_" << pair.first << "(struct lu_BitstreamState* state, struct " << pair.first << "* dst);\n";

         stream << '\n';

         stream << "void lu_BitstreamWrite_" << pair.first << "(struct lu_BitstreamState* state, struct " << pair.first << "* src) {\n";
         for (const auto& m : pair.second->members) {
            auto&  extents = m.c_type_info.array_extents;
            size_t rank    = extents.size();
            if (std::holds_alternative<ast::semantic::string_info>(m.semantic_info)) {
               // we appended the strlen as an extent
               // (maybe we shouldn't do that? makes it easier to do the struct definitions, but...)
               --rank;
            }

            constexpr const char first_array_index_name = 'i';
            bool array_indices_are_numbered = rank > ('z' - 'i');

            auto _serialize_indices = [&stream, rank, array_indices_are_numbered]() {
               for (size_t i = 0; i < rank; ++i) {
                  stream << '[';
                  if (array_indices_are_numbered) {
                     stream << "index_" << i;
                  } else {
                     stream << (char)(first_array_index_name + (char)i);
                  }
                  stream << ']';
               }
            };

            std::string indent;
            if (rank > 0) {
               stream << "   {\n";
               stream << "      u16 ";
               for (size_t i = 0; i < rank; ++i) {
                  if (array_indices_are_numbered) {
                     stream << "index_" << i;
                  } else {
                     stream << (char)(first_array_index_name + (char)i);
                  }
                  if (i + 1 < rank) {
                     stream << ", ";
                  }
               }
               stream << ";\n";
               for (size_t i = 0; i < rank; ++i) {
                  char var = ('i' + i);
                  stream << indent << "      for (" << var << " = 0; " << var << " < " << extents[i].extent << "; ++" << var << ") { \n";
                  indent += "   ";
               }
               stream << "      ";
            }

            if (std::holds_alternative<std::string>(m.c_type_info.name)) {
               stream << indent << "   lu_BitstreamWrite_" << std::get<std::string>(m.c_type_info.name) << "(";
               stream << "&src." << m.name;
               _serialize_indices();
               stream << ");\n";
            } else if (std::holds_alternative<ast::semantic::string_info>(m.semantic_info)) {
               const auto& casted = std::get<ast::semantic::string_info>(m.semantic_info);
               assert(casted.max_length > 0);

               stream << indent << "   lu_BitstreamWrite_string(";
               stream << "state";
               stream << ", ";
               stream << "src." << m.name;
               _serialize_indices();
               stream << ", ";
               stream << casted.max_length;
               stream << ", ";
               stream << std::bit_width(casted.max_length);
               stream << ");\n";
            } else {
               size_t bitcount = m.compute_bitcount();
               if (bitcount == 1) {
                  stream << indent << "   lu_BitstreamWrite_bool(state, src." << m.name;
                  _serialize_indices();
                  stream << ");\n";
                  continue;
               }
               stream << indent << "   lu_BitstreamWrite_u";
               if (bitcount > 16)
                  stream << "32";
               else if (bitcount > 8)
                  stream << "16";
               else
                  stream << "8";
               stream << "(state, src." << m.name;
               _serialize_indices();
               stream << ", " << bitcount << ");\n";
            }

            if (rank > 0) {
               for (size_t i = 0; i < rank; ++i) {
                  stream << indent << "   }\n"; // close (nested) for-loop
                  indent = indent.substr(3);
               }
               stream << "   }\n"; // close anonymous block containing loop iterator var
            }

         }
         stream << "}";
      }
   }
}