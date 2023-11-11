#include "./registry.h"
#include <bit>
#include <fstream>
#include <map>
#include <stdexcept>
#include <string_view>
#include "RapidXml/rapidxml.hpp"
#include "lu/rapidxml_helpers/for_each_attribute.h"
#include "lu/rapidxml_helpers/for_each_child_element.h"
#include "lu/rapidxml_helpers/get_sole_child_element_of_name.h"
#include "lu/rapidxml_helpers/get_attribute.h"
#include "lu/rapidxml_helpers/get_element_text_content.h"
#include "lu/strings/to_integer.h"

#include "./ast/member_types/_all.h"
#include "./ast/constant_definition.h"
#include "./ast/heritable.h"
#include "./ast/structure.h"

#include "./codegen/sector_generator.h"
#include "./codegen/serialization_function_body.h"

#include "./struct_bitwalker.h"
#include "./struct_bytewalker.h"

using namespace std::literals::string_literals;

std::filesystem::path registry::_normalize_xml_path(const std::filesystem::path& p) {
   std::filesystem::path out = p;
   if (out.is_relative()) {
      out = this->paths.input_paths.xml / out;
   }
   out = std::filesystem::weakly_canonical(out);
   out = out.make_preferred();
   return out;
}

void registry::set_paths(const path_set& v) {
   this->paths = v;

   auto fixup = [](std::filesystem::path& p) {
      p = std::filesystem::weakly_canonical(p).make_preferred();
   };

   fixup(this->paths.input_paths.xml);
   fixup(this->paths.output_paths.h);
   fixup(this->paths.output_paths.c);
}

std::optional<ast::size_constant> registry::interpret_size_constant(const std::string_view& text) {
   bool   ok;
   size_t out = lu::strings::to_integer<std::size_t>(text.data(), text.size(), &ok);
   if (ok) {
      return ast::size_constant{
         .value = out
      };
   }
   
   auto ugh = std::string(text);
   if (this->constants.contains(ugh)) {
      return ast::size_constant{
         .preprocessor_name = ugh,
         .value = (size_t)(this->constants[ugh]->value)
      };
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

void registry::_enter_union(const ast::inlined_union_member& m) {
   auto& list = this->load_state.unions;
   if (!list.empty()) {
      list.back().in_member_to_serialize = (list.back().target_union->member_to_serialize == m.name);
   }
   list.push_back({ .target_union = &m });
}
void registry::_exit_union(const ast::inlined_union_member& m) {
   auto& list = this->load_state.unions;
   assert(!list.empty());
   assert(list.back().target_union == &m);
   list.pop_back();
   if (!list.empty()) {
      list.back().in_member_to_serialize = false;
   }
}
bool registry::_in_inactive_union_member(const std::string& member_name) const {
   auto& list = this->load_state.unions;
   if (list.empty())
      return false;
   auto size = list.size();
   for (size_t i = 0; i < size - 1; ++i) {
      auto& item = list[i];
      if (!item.in_member_to_serialize)
         return true;
   }
   return (list.back().target_union->member_to_serialize != member_name);
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
         std::filesystem::path resolved = _normalize_xml_path(specified);
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
      auto heritable = std::make_unique<ast::heritable>();
      heritable->tagname = tagname;
      
      lu::rapidxml_helpers::for_each_attribute(node, [this, &scaffold, &heritable](std::string_view attr_name, std::string_view attr_value, xml_attribute<>& attr) {
         if (attr_name == "name") {
            heritable->name = attr_value;
            return;
         }
         if (attr_name == "do-not-serialize") {
            if (attr_value == "true") {
               heritable->attributes.do_not_serialize = true;
            } else if (attr_value == "false") {
               heritable->attributes.do_not_serialize = false;
            } else {
               scaffold.warn("Heritable: Unrecognized attribute value for `do-not-serialize` (seen: "s + std::string(attr_value) + "); ignoring...", attr); // can't wait for P2591...
            }
            return;
         }
         if (attr_name == "c-alignment") {
            auto constant = interpret_size_constant(attr_value);
            if (!constant.has_value()) {
               scaffold.error("Heritable: `c-alignment` attribute value is an unrecognized constant or otherwise not a valid unsigned integral (seen: "s + std::string(attr_value) + ").", attr); // can't wait for P2591...
            }
            if (constant.value().value == 0) {
               scaffold.error("Heritable: `c-alignment` attribute value cannot be zero (seen: "s + std::string(attr_value) + ").", attr); // can't wait for P2591...
            }
            heritable->attributes.c_alignment = constant.value();
            return;
         }

         if (attr_name == "c-type") {
            heritable->attributes.c_type = attr_value;
            return;
         }
         if (attr_name == "c-type-decl") {
            heritable->attributes.c_type_decl = attr_value;
            return;
         }

         if (attr_name == "min") {
            auto value = _int_or_constant_name_to_int(attr_value);
            if (!value.has_value()) {
               scaffold.error("Heritable: Number `min` value is an unrecognized constant or otherwise not a valid integral (seen: "s + std::string(attr_value) + ")", attr); // can't wait for P2591...
            }
            heritable->attributes.min = value.value();
            return;
         }
         if (attr_name == "max") {
            auto value = _int_or_constant_name_to_int(attr_value);
            if (!value.has_value()) {
               scaffold.error("Heritable: Number `max` value is an unrecognized constant or otherwise not a valid integral (seen: "s + std::string(attr_value) + ")", attr); // can't wait for P2591...
            }
            heritable->attributes.max = value.value();
            return;
         }
         if (attr_name == "serialization-bitcount") {
            auto constant = interpret_size_constant(attr_value);
            if (!constant.has_value()) {
               scaffold.error("Heritable: `serialization-bitcount` attribute value is an unrecognized constant or otherwise not a valid integral (seen: "s + std::string(attr_value) + ").", attr); // can't wait for P2591...
            }
            if (constant.value().value < 0) {
               scaffold.error("Heritable: `serialization-bitcount` attribute value cannot be a negative number (seen: "s + std::string(attr_value) + ").", attr); // can't wait for P2591...
            }
            if (constant.value().value == 0) {
               scaffold.error("Heritable: `serialization-bitcount` attribute value cannot be zero (to prevent serialization, use `do-not-serialize=\"true\"` instead) (seen: "s + std::string(attr_value) + ").", attr); // can't wait for P2591...
            }
            heritable->attributes.serialization_bitcount = constant.value();
            return;
         }
         if (attr_name == "c-bitfield") {
            auto constant = interpret_size_constant(attr_value);
            if (!constant.has_value()) {
               scaffold.error("Heritable: `c-bitfield` attribute value is an unrecognized constant or otherwise not a valid integral (seen: "s + std::string(attr_value) + ").", attr); // can't wait for P2591...
            }
            if (constant.value().value < 0) {
               scaffold.error("Heritable: `c-bitfield` attribute value cannot be a negative number (seen: "s + std::string(attr_value) + ").", attr); // can't wait for P2591...
            }
            if (constant.value().value == 0) {
               scaffold.error("Heritable: `c-bitfield` attribute value cannot be zero (seen: "s + std::string(attr_value) + ").", attr); // can't wait for P2591...
            }
            heritable->attributes.c_bitfield = constant.value();
            return;
         }
         if (attr_name == "is-checksum") {
            if (attr_value == "true") {
               heritable->attributes.is_checksum = true;
            } else if (attr_value == "false") {
               heritable->attributes.is_checksum = false;
            } else {
               scaffold.warn("Heritable: Unrecognized attribute value for `is-checksum` (seen: "s + std::string(attr_value) + "); ignoring...", attr); // can't wait for P2591...
            }
            return;
         }

         if (attr_name == "char-type") {
            auto integral = ast::integral_type_from_string(attr_value);
            if (!integral.has_value()) {
               scaffold.error("Heritable: `char-type` attribute value is not a valid integral type (seen: "s + std::string(attr_value) + ")", attr); // can't wait for P2591...
            }
            heritable->attributes.char_type = integral.value();
            return;
         }
         if (attr_name == "length") {
            auto constant = interpret_size_constant(attr_value);
            if (!constant.has_value()) {
               scaffold.error("Heritable: `length` attribute value is an unrecognized constant or otherwise not a valid unsigned integral (seen: "s + std::string(attr_value) + ")", attr); // can't wait for P2591...
            }
            heritable->attributes.length = constant.value();
            return;
         }
      });

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
      auto field = std::make_unique<ast::inlined_union_member>();

      if (auto* attr = lu::rapidxml_helpers::get_attribute(node, "name")) {
         field->name = std::string_view(attr->value(), attr->value_size());
      } else {
         scaffold.error("Field: Anonymous unions are not allowed in GCC C90.", node);
      }
      if (auto* attr = lu::rapidxml_helpers::get_attribute(node, "member-to-serialize")) {
         field->member_to_serialize = std::string_view(attr->value(), attr->value_size());
      } else {
         if (!_in_inactive_union_member(field->name)) {
            scaffold.error("Field: You must specify a union member to serialize.", node);
         }
      }

      this->_enter_union(*field.get());

      lu::rapidxml_helpers::for_each_child_element(node, [this, &scaffold, &field](std::string_view tagname, xml_node<>& node) {
         bool is_checksum;
         auto nested = this->_parse_member(scaffold, node, is_checksum);
         if (is_checksum) {
            scaffold.error("Field: Checksums nested in unions are not supported.", node);
         }
         field->members.push_back(std::move(nested));
      });

      this->_exit_union(*field.get());

      if (!_in_inactive_union_member(field->name)) {
         bool found = false;
         for (const auto& m : field->members) {
            if (m->name == field->member_to_serialize) {
               found = true;
               break;
            }
         }
         if (!found) {
            scaffold.error("Field: You must specify a union member to serialize (member \""s + field->member_to_serialize + "\") was not found.", node);
         }
      } else {
         if (!field->member_to_serialize.empty()) {
            scaffold.error("Field: Specified a union member to serialize (member \""s + field->member_to_serialize + "\") when this union exists inside of a non-serialized union member.", node);
         }
      }

      return field;
   }

   struct {
      std::string name;
      std::optional<bool> do_not_serialize;
      std::optional<ast::size_constant> c_alignment;
      std::optional<bool> is_const;

      std::string c_type;
      std::optional<std::string> c_type_decl;
      std::string inherit;

      // attributes for numbers
      std::optional<std::intmax_t>      min;
      std::optional<std::uintmax_t>     max;
      std::optional<ast::size_constant> c_bitfield;
      std::optional<ast::size_constant> serialization_bitcount;
      std::optional<bool> is_checksum;

      // attributes for strings
      std::string char_type;
      std::optional<ast::size_constant> length;
      std::optional<bool> only_early_terminator;

      // attributes for inlined unions (we handle them in the special-case above, but 
      // we should still grab their attributes for more robust user warnings).
      std::string member_to_serialize;
   } attributes;

   lu::rapidxml_helpers::for_each_attribute(node, [this, &scaffold, &attributes](std::string_view attr_name, std::string_view attr_value, xml_attribute<>& attr) {
      if (attr_name == "name") {
         attributes.name = attr_value;
         return;
      }
      if (attr_name == "const") {
         if (attr_value == "true") {
            attributes.is_const = true;
         } else if (attr_value == "false") {
            attributes.is_const = false;
         } else {
            scaffold.warn("Field: Unrecognized attribute value for `const` (seen: "s + std::string(attr_value) + "); ignoring...", attr); // can't wait for P2591...
         }
         return;
      }
      if (attr_name == "do-not-serialize") {
         if (attr_value == "true") {
            attributes.do_not_serialize = true;
         } else if (attr_value == "false") {
            attributes.do_not_serialize = false;
         } else {
            scaffold.warn("Field: Unrecognized attribute value for `do-not-serialize` (seen: "s + std::string(attr_value) + "); ignoring...", attr); // can't wait for P2591...
         }
         return;
      }
      if (attr_name == "c-alignment") {
         auto constant = interpret_size_constant(attr_value);
         if (!constant.has_value()) {
            scaffold.error("Field: `c-alignment` attribute value is an unrecognized constant or otherwise not a valid unsigned integral (seen: "s + std::string(attr_value) + ").", attr); // can't wait for P2591...
         }
         if (constant.value().value == 0) {
            scaffold.error("Field: `c-alignment` attribute value cannot be zero (seen: "s + std::string(attr_value) + ").", attr); // can't wait for P2591...
         }
         attributes.c_alignment = constant.value();
         return;
      }

      if (attr_name == "c-type") {
         attributes.c_type = attr_value;
         return;
      }
      if (attr_name == "c-type-decl") {
         attributes.c_type_decl = attr_value;
         return;
      }
      if (attr_name == "inherit") {
         attributes.inherit = attr_value;
         return;
      }

      if (attr_name == "min") {
         auto value = _int_or_constant_name_to_int(attr_value);
         if (!value.has_value()) {
            scaffold.error("Field: `min` attribute value is an unrecognized constant or otherwise not a valid integral (seen: "s + std::string(attr_value) + ")", attr); // can't wait for P2591...
         }
         attributes.min = value.value();
         return;
      }
      if (attr_name == "max") {
         auto value = _int_or_constant_name_to_int(attr_value);
         if (!value.has_value()) {
            scaffold.error("Field: `max` attribute value is an unrecognized constant or otherwise not a valid integral (seen: "s + std::string(attr_value) + ")", attr); // can't wait for P2591...
         }
         attributes.max = value.value();
         return;
      }
      if (attr_name == "serialization-bitcount") {
         auto constant = interpret_size_constant(attr_value);
         if (!constant.has_value()) {
            scaffold.error("Field: `serialization-bitcount` attribute value is an unrecognized constant or otherwise not a valid integral (seen: "s + std::string(attr_value) + ").", attr); // can't wait for P2591...
         }
         if (constant.value().value < 0) {
            scaffold.error("Field: `serialization-bitcount` attribute value cannot be a negative number (seen: "s + std::string(attr_value) + ").", attr); // can't wait for P2591...
         }
         if (constant.value().value == 0) {
            scaffold.error("Field: `serialization-bitcount` attribute value cannot be zero (to prevent serialization, use `do-not-serialize=\"true\"` instead) (seen: "s + std::string(attr_value) + ").", attr); // can't wait for P2591...
         }
         attributes.serialization_bitcount = constant.value();
         return;
      }
      if (attr_name == "c-bitfield") {
         auto constant = interpret_size_constant(attr_value);
         if (!constant.has_value()) {
            scaffold.error("Field: `c-bitfield` attribute value is an unrecognized constant or otherwise not a valid unsigned integral (seen: "s + std::string(attr_value) + ").", attr); // can't wait for P2591...
         }
         if (constant.value().value == 0) {
            scaffold.error("Field: `c-bitfield` attribute value cannot be zero (seen: "s + std::string(attr_value) + ").", attr); // can't wait for P2591...
         }
         attributes.c_bitfield = constant.value();
         return;
      }
      if (attr_name == "is-checksum") {
         if (attr_value == "true") {
            attributes.is_checksum = true;
         } else if (attr_value == "false") {
            attributes.is_checksum = false;
         } else {
            scaffold.warn("Field: Unrecognized attribute value for `is-checksum` (seen: "s + std::string(attr_value) + "); ignoring...", attr); // can't wait for P2591...
         }
         return;
      }

      if (attr_name == "char-type") {
         attributes.char_type = attr_value;
         return;
      }
      if (attr_name == "length") {
         auto constant = interpret_size_constant(attr_value);
         if (!constant.has_value()) {
            scaffold.error("Field: `length` attribute value is an unrecognized constant or otherwise not a valid unsigned integral (seen: "s + std::string(attr_value) + ")", attr); // can't wait for P2591...
         }
         attributes.length = constant.value();
         return;
      }
      if (attr_name == "only-early-terminator") {
         if (attr_value == "true") {
            attributes.only_early_terminator = true;
         } else if (attr_value == "false") {
            attributes.only_early_terminator = false;
         } else {
            scaffold.warn("Field: Unrecognized attribute value for `only-early-terminator` (seen: "s + std::string(attr_value) + "); ignoring...", attr); // can't wait for P2591...
         }
         return;
      }

      if (attr_name == "member-to-serialize") {
         attributes.member_to_serialize = attr_value;
         return;
      }

      scaffold.warn("Field: Unrecognized attribute (name: "s + std::string(attr_name) + ")", attr); // can't wait for P2591...
   });

   const ast::heritable* inherit_from = nullptr;
   if (!attributes.inherit.empty()) {
      if (this->heritables.contains(attributes.inherit)) {
         inherit_from = this->heritables[attributes.inherit].get();

         //
         // Note: Grabbing the heritable values as below simplifies the code for loading different 
         // member types, but makes error reporting inaccurate: the heritable's attributes will be 
         // reported as if they were attributes on the member node.
         //

         if (!attributes.c_alignment.has_value())
            attributes.c_alignment = inherit_from->attributes.c_alignment;

         if (attributes.c_type.empty())
            attributes.c_type = inherit_from->attributes.c_type;
         if (!attributes.c_type_decl.has_value())
            attributes.c_type_decl = inherit_from->attributes.c_type_decl;

         if (!attributes.min.has_value())
            attributes.min = inherit_from->attributes.min;
         if (!attributes.max.has_value())
            attributes.max = inherit_from->attributes.max;
         if (!attributes.c_bitfield.has_value())
            attributes.c_bitfield = inherit_from->attributes.c_bitfield;
         if (!attributes.serialization_bitcount.has_value())
            attributes.serialization_bitcount = inherit_from->attributes.serialization_bitcount;
         if (!attributes.is_checksum.has_value())
            attributes.is_checksum = inherit_from->attributes.is_checksum;

         if (!attributes.length.has_value())
            attributes.length = inherit_from->attributes.length;

      } else {
         scaffold.error("Field: `inherit` attribute did not refer to a known heritable (seen: "s + attributes.inherit + ").", node);
      }
   }

   std::unique_ptr<ast::member> field;
   {
      auto tagname_integral = ast::integral_type_from_string(tagname);
      auto attrval_integral = ast::integral_type_from_string(attributes.c_type);
      if (tagname == "pointer") {
         field = std::make_unique<ast::pointer_member>();
         if (attrval_integral.has_value()) {
            ((ast::pointer_member*)field.get())->pointed_to_type = attrval_integral.value();
         }
      } else if (tagname_integral.has_value() || attrval_integral.has_value()) {
         field = std::make_unique<ast::integral_member>();

         auto* casted = (ast::integral_member*)field.get();
         if (tagname_integral.has_value()) {
            casted->value_type = tagname_integral.value();
         } else {
            casted->value_type = attrval_integral.value();
         }
         if (tagname_integral.has_value() && attrval_integral.has_value() && tagname_integral != attrval_integral) {
            scaffold.error(
               "Field: Inconsistent c-type (between tagname `"s
                  + std::string(ast::integral_type_to_string(tagname_integral.value())) // can't wait for P2591...
                  + "` and attribute value `"
                  + std::string(ast::integral_type_to_string(attrval_integral.value())) // can't wait for P2591...
                  + "`).",
               node
            );
         }
      } else if (tagname == "string") {
         field = std::make_unique<ast::string_member>();
      } else if (tagname == "union") {
         field = std::make_unique<ast::inlined_union_member>();
      } else if (tagname == "field") {
         if (inherit_from) {
            if (inherit_from->is_integral()) {
               field = std::make_unique<ast::integral_member>();
            } else if (inherit_from->is_string()) {
               field = std::make_unique<ast::string_member>();
            } else {
               field = std::make_unique<ast::struct_member>();
            }
         } else if (!attributes.c_type.empty()) {
            //
            // We don't need to handle `c-type` values that refer to `ast::integral_type` names here, 
            // because we already checked for that above.
            //
            ast::structure*   def_struct = nullptr;
            ast::blind_union* def_union  = nullptr;
            if (this->blind_unions.contains(attributes.c_type)) {
               field = std::make_unique<ast::blind_union_member>();

               auto* casted = (ast::blind_union_member*)field.get();

               casted->type_name = attributes.c_type;
               if (attributes.c_type_decl.has_value()) {
                  if (attributes.c_type_decl == "union") {
                     casted->decl = ast::blind_union_member::decl::c_union;
                  } else if (attributes.c_type_decl == "") {
                     casted->decl = ast::blind_union_member::decl::blank;
                  } else {
                     scaffold.warn("Field: Unrecognized value for attribute `c-type-decl` (seen: "s + attributes.c_type_decl.value() + "); ignoring...", node);
                  }
               }
            } else if (this->structs.contains(attributes.c_type) || this->_in_inactive_union_member(attributes.name)) {
               field = std::make_unique<ast::struct_member>();

               auto* casted = (ast::struct_member*)field.get();

               casted->type_name = attributes.c_type;
               if (attributes.c_type_decl.has_value()) {
                  if (attributes.c_type_decl == "struct") {
                     casted->decl = ast::struct_member::decl::c_struct;
                  } else if (attributes.c_type_decl == "union") {
                     casted->decl = ast::struct_member::decl::c_union;
                  } else if (attributes.c_type_decl == "") {
                     casted->decl = ast::struct_member::decl::blank;
                  } else {
                     scaffold.warn("Field: Unrecognized value for attribute `c-type-decl` (seen: "s + attributes.c_type_decl.value() + "); ignoring...", node);
                  }
               }
            } else {
               scaffold.error("Field: Unable to identify fundamental type (integral? string? struct? inlined union?).", node);
            }
         } else {
            scaffold.error("Field: Unable to identify fundamental type (integral? string? struct? inlined union?).", node);
         }
      } else {
         scaffold.warn("Unrecognized child node (tagname "s + std::string(tagname) + ")", node); // can't wait for P2591...
         return nullptr;
      }
   }

   //
   // Field is constructed with the appropriate fundamental type; now, populate it with the 
   // parsed data.
   //

   if (inherit_from)
      field->inherited_from = inherit_from;

   field->name = attributes.name;
   if (attributes.do_not_serialize.has_value()) {
      field->skip_when_serializing = attributes.do_not_serialize.value();
   }
   if (attributes.c_alignment.has_value()) {
      field->c_alignment = attributes.c_alignment.value();
   }
   if (attributes.is_const.has_value()) {
      field->is_const = attributes.is_const.value();
   }
   if (auto* casted = dynamic_cast<ast::integral_member*>(field.get())) {
      casted->min = attributes.min;
      casted->max = attributes.max;
      casted->c_bitfield = attributes.c_bitfield;
      casted->serialization_bitcount = attributes.serialization_bitcount;

      if (attributes.is_checksum.value_or(false))
         out_is_checksum = true;

      if (attributes.c_type_decl.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: c-type-decl) (attribute is not valid for integral fields).", node);
      if (!attributes.char_type.empty())
         scaffold.warn("Field: Unrecognized attribute (name: char-type) (attribute is not valid for integral fields).", node);
      if (attributes.length.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: length) (attribute is not valid for integral fields).", node);
      if (!attributes.member_to_serialize.empty())
         scaffold.warn("Field: Unrecognized attribute (name: member-to-serialize) (attribute is not valid for integral fields).", node);
      if (attributes.only_early_terminator.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: only-early-terminator) (attribute is not valid for integral fields).", node);

      if (inherit_from && inherit_from->is_integral()) {
         if (!casted->value_type.has_value()) {
            casted->value_type = inherit_from->integral_type;
         }
      }

      if (!casted->value_type.has_value()) {
         scaffold.error("Field: Unable to identify integral value type (specify it via `c-type` or in a heritable).", node);
      }
   } else if (auto* casted = dynamic_cast<ast::string_member*>(field.get())) {
      if (!attributes.char_type.empty()) {
         auto type = ast::integral_type_from_string(attributes.char_type);
         if (!type.has_value()) {
            scaffold.error("Field: unrecognized `char-type` (seen: "s + attributes.char_type + ").", node);
         }
         casted->char_type = type.value();
      }
      if (inherit_from) {
         if (!casted->char_type.has_value()) {
            casted->char_type = inherit_from->attributes.char_type;
         }
      }
      if (attributes.length.has_value()) {
         auto v = attributes.length.value();
         if (v.value < 0) {
            scaffold.warn("Field: The string max length cannot be negative.", node);
         }
         if (v.value == 0) {
            scaffold.warn("Field: The string max length cannot be zero.", node);
         }
         casted->max_length = v;
      }
      if (attributes.only_early_terminator.has_value()) {
         casted->only_early_terminator = attributes.only_early_terminator.value();
      }

      if (!attributes.c_type.empty())
         scaffold.warn("Field: Unrecognized attribute (name: c-type) (attribute is not valid for string fields; did you mean `char-type`?).", node);
      if (attributes.c_type_decl.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: c-type-decl) (attribute is not valid for string fields).", node);
      if (attributes.min.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: min) (attribute is not valid for string fields).", node);
      if (attributes.max.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: max) (attribute is not valid for string fields).", node);
      if (attributes.c_bitfield.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: c-bitfield) (attribute is not valid for string fields).", node);
      if (attributes.serialization_bitcount.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: serialization-bitcount) (attribute is not valid for string fields).", node);
      if (attributes.is_checksum.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: is-checksum) (attribute is not valid for string fields).", node);
      if (!attributes.member_to_serialize.empty())
         scaffold.warn("Field: Unrecognized attribute (name: member-to-serialize) (attribute is not valid for string fields).", node);

      if (!casted->char_type.has_value()) {
         scaffold.error("Field: Unable to identify character type (specify it via `char-type` or in a heritable).", node);
      }
   } else if (auto* casted = dynamic_cast<ast::struct_member*>(field.get())) {
      casted->type_name = attributes.c_type;
      if (attributes.c_type_decl.has_value()) {
         if (attributes.c_type_decl == "struct") {
            casted->decl = ast::struct_member::decl::c_struct;
         } else if (attributes.c_type_decl == "union") {
            casted->decl = ast::struct_member::decl::c_union;
         } else if (attributes.c_type_decl == "") {
            casted->decl = ast::struct_member::decl::blank;
         } else {
            scaffold.warn("Field: Unrecognized value for attribute `c-type-decl` (seen: "s + attributes.c_type_decl.value() + "); ignoring (default is `struct`)...", node);
         }
      }

      if (attributes.min.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: min) (attribute is not valid for struct fields).", node);
      if (attributes.max.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: max) (attribute is not valid for struct fields).", node);
      if (attributes.c_bitfield.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: c-bitfield) (attribute is not valid for struct fields).", node);
      if (attributes.serialization_bitcount.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: serialization-bitcount) (attribute is not valid for struct fields).", node);
      if (attributes.is_checksum.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: is-checksum) (attribute is not valid for struct fields).", node);
      if (!attributes.char_type.empty())
         scaffold.warn("Field: Unrecognized attribute (name: char-type) (attribute is not valid for struct fields).", node);
      if (attributes.length.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: length) (attribute is not valid for struct fields).", node);
      if (!attributes.member_to_serialize.empty())
         scaffold.warn("Field: Unrecognized attribute (name: member-to-serialize) (attribute is not valid for struct fields).", node);
      if (attributes.only_early_terminator.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: only-early-terminator) (attribute is not valid for struct fields).", node);

      if (casted->type_name.empty()) {
         scaffold.error("Field: field seems to be a named struct/union, but no name was found.", node);
      }
      if (!this->_in_inactive_union_member(field->name)) {
         if (this->structs.contains(casted->type_name)) {
            casted->type_def = this->structs[casted->type_name].get();
         } else {
            scaffold.error("Field: struct typename is not known/defined yet (seen: "s + casted->type_name + ").", node);
         }
      }
   } else if (auto* casted = dynamic_cast<ast::blind_union_member*>(field.get())) {
      casted->type_name = attributes.c_type;
      if (attributes.c_type_decl.has_value()) {
         if (attributes.c_type_decl == "union") {
            casted->decl = ast::blind_union_member::decl::c_union;
         } else if (attributes.c_type_decl == "") {
            casted->decl = ast::blind_union_member::decl::blank;
         } else {
            scaffold.warn("Field: Unrecognized value for attribute `c-type-decl` (seen: "s + attributes.c_type_decl.value() + "); ignoring (default is `struct`)...", node);
         }
      }

      if (attributes.min.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: min) (attribute is not valid for union fields).", node);
      if (attributes.max.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: max) (attribute is not valid for union fields).", node);
      if (attributes.c_bitfield.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: c-bitfield) (attribute is not valid for union fields).", node);
      if (attributes.serialization_bitcount.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: serialization-bitcount) (attribute is not valid for union fields).", node);
      if (attributes.is_checksum.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: is-checksum) (attribute is not valid for union fields).", node);
      if (!attributes.char_type.empty())
         scaffold.warn("Field: Unrecognized attribute (name: char-type) (attribute is not valid for union fields).", node);
      if (attributes.length.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: length) (attribute is not valid for union fields).", node);
      if (!attributes.member_to_serialize.empty())
         scaffold.warn("Field: Unrecognized attribute (name: member-to-serialize) (attribute is not valid for union fields).", node);
      if (attributes.only_early_terminator.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: only-early-terminator) (attribute is not valid for union fields).", node);

      if (casted->type_name.empty()) {
         scaffold.error("Field: field seems to be a named union, but no name was found.", node);
      }
      if (!this->_in_inactive_union_member(field->name)) {
         if (this->blind_unions.contains(casted->type_name)) {
            casted->type_def = this->blind_unions[casted->type_name].get();
         } else {
            scaffold.error("Field: union typename is not known/defined yet (seen: "s + casted->type_name + ").", node);
         }
      }
   } else if (auto* casted = dynamic_cast<ast::pointer_member*>(field.get())) {
      if (attributes.c_type_decl.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: c-type-decl) (attribute is not valid for pointer fields).", node);
      if (!attributes.char_type.empty())
         scaffold.warn("Field: Unrecognized attribute (name: char-type) (attribute is not valid for pointer fields).", node);
      if (attributes.min.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: min) (attribute is not valid for pointer fields).", node);
      if (attributes.max.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: max) (attribute is not valid for pointer fields).", node);
      if (attributes.c_bitfield.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: c-bitfield) (attribute is not valid for pointer fields).", node);
      if (attributes.length.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: length) (attribute is not valid for pointer fields).", node);
      if (attributes.serialization_bitcount.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: serialization-bitcount) (attribute is not valid for pointer fields).", node);
      if (attributes.is_checksum.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: is-checksum) (attribute is not valid for pointer fields).", node);
      if (!attributes.member_to_serialize.empty())
         scaffold.warn("Field: Unrecognized attribute (name: member-to-serialize) (attribute is not valid for pointer fields).", node);
      if (attributes.only_early_terminator.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: only-early-terminator) (attribute is not valid for pointer fields).", node);

      if (!casted->pointed_to_type.has_value()) {
         scaffold.error("Field: Unable to identify pointed-to value type (specify it via `c-type` or in a heritable).", node);
      }
   }

   //
   // Field is now mostly loaded, except for child elements in the XML.
   //

   if (field->name.empty()) {
      scaffold.error("Field defined with an empty or missing name", node);
   }

   bool is_bitfield = false;
   if (auto* casted = dynamic_cast<const ast::integral_member*>(field.get())) {
      if (casted->c_bitfield.has_value())
         is_bitfield = true;
   }

   lu::rapidxml_helpers::for_each_child_element(node, [this, &scaffold, &field, is_bitfield](std::string_view tagname, xml_node<>& node) {
      if (tagname == "array-rank") {
         if (is_bitfield) {
            scaffold.error("Field: C does not support bitfield arrays or array bitfields.", node);
         }
         ast::member::array_rank rank;

         lu::rapidxml_helpers::for_each_attribute(node, [this, &scaffold, &rank](std::string_view attr_name, std::string_view attr_value, xml_attribute<>& attr) {
            if (attr_name == "extent") {
               auto constant = interpret_size_constant(attr_value);
               if (!constant.has_value()) {
                  scaffold.error("Field: Array rank is an unrecognized constant or otherwise not a valid integral (seen: "s + std::string(attr_value) + ")", attr); // can't wait for P2591...
               }
               rank.extent = constant.value();
            } else if (attr_name == "extent-expr") {
               rank.extent_expr = attr_value;
            } else {
               scaffold.warn("Field: Unrecognized attribute (name: "s + std::string(attr_name) + ")", attr); // can't wait for P2591...
            }
         });

         field->array_extents.push_back(rank);
         return;
      }
      if (tagname == "checksum-of") {
         //
         // This data is unused for now.
         //
         return;
      }
      if (tagname == "line-comment") {
         try {
            field->c_line_comment = lu::rapidxml_helpers::get_element_text_content<false>(node);
         } catch (std::runtime_error) {
            scaffold.error("Field `line-comment` node contains element children", node);
         }
         return;
      }
      scaffold.warn("Field: Unrecognized child node (tagname: "s + std::string(tagname) + ")", node); // can't wait for P2591...
   });

   return field;
}
void registry::_parse_types(parse_wrapper& scaffold, rapidxml::xml_node<>& base_node) {
   using namespace rapidxml;

   for (xml_node<>* node = base_node.first_node(); node; node = node->next_sibling()) {
      auto tagname = std::string(node->name(), node->name_size());
      if (tagname != "struct" && tagname != "union") {
         scaffold.warn(std::string("Unrecognized child node (tagname ") + tagname + ")", *node);
         continue;
      }

      if (tagname == "union") {
         {
            auto* attr = lu::rapidxml_helpers::get_attribute(*node, "do-not-codegen");
            if (!attr || std::string_view(attr->value(), attr->value_size()) != "true") {
               scaffold.error("Fully-defined unions are not yet implemented.", *node);
            }
         }

         std::string_view name;
         if (auto* attr = lu::rapidxml_helpers::get_attribute(*node, "name")) {
            name = std::string_view(attr->value(), attr->value_size());
         }
         if (name.empty()) {
            scaffold.error("Unions defined with empty or missing name", *node);
         }

         size_t size_in_bytes = 0;
         if (auto* attr = lu::rapidxml_helpers::get_attribute(*node, "sizeof")) {
            bool ok;
            size_in_bytes = lu::strings::to_integer<size_t>(attr->value(), attr->value_size(), &ok);
            if (!ok) {
               scaffold.error("Blind union: `sizeof` attribute is not a valid unsigned integral.", *node);
            }
            if (size_in_bytes <= 0) {
               scaffold.error("Blind union: `sizeof` attribute value cannot be zero or a negative number", *node);
            }
         } else {
            scaffold.error("A blind union must provide its size in bytes via the `sizeof` attribute", *node);
         }

         auto blind_union = std::make_unique<ast::blind_union>();
         blind_union->name = name;
         blind_union->size_in_bytes = size_in_bytes;
         this->blind_unions[blind_union->name] = std::move(blind_union);
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
         } else if (attr_name == "is-packed") {
            if (attr_value == "true") {
               structure->c_type_info.is_packed = true;
            } else if (attr_value == "false") {
               structure->c_type_info.is_packed = false;
            } else {
               scaffold.warn("Unrecognized value for `is-packed` attribute (seen: "s + std::string(attr_value) + ")", attr); // can't wait for P2591...
            }
         } else if (attr_name == "defined-via-typedef") {
            if (attr_value == "true") {
               structure->c_type_info.is_defined_via_typedef = true;
            } else if (attr_value == "false") {
               structure->c_type_info.is_defined_via_typedef = false;
            } else {
               scaffold.warn("Unrecognized value for `defined-via-typedef` attribute (seen: "s + std::string(attr_value) + ")", attr); // can't wait for P2591...
            }
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
         if (is_checksum) {
            if (structure->checksum_member_index.has_value()) {
               scaffold.error(
                  std::string("We only support one checksum member per struct. (Existing member: ")
                     + structure->members[structure->checksum_member_index.value()]->name
                     + "; new member: "
                     + field->name
                     + ")",
                  node
               );
            }
         }
         structure->members.push_back(std::move(field));
         if (is_checksum) {
            structure->checksum_member_index = structure->members.size() - 1;
         }
      });

      //
      // Done
      //
      this->structs[structure->name] = std::move(structure);
   }
}
//
void registry::_parse_sector_group(parse_wrapper& scaffold, rapidxml::xml_node<>& node) {
   using namespace rapidxml;

   sector_group dst;
   //
   lu::rapidxml_helpers::for_each_attribute(node, [&scaffold, &dst](std::string_view attr_name, std::string_view attr_value, xml_attribute<>& attr) {
      if (attr_name == "name") {
         dst.name = attr_value;
         return;
      }
      if (attr_name == "sector-count") {
         bool ok;
         dst.max_sector_count = lu::strings::to_integer<size_t>(attr_value.data(), attr_value.size(), &ok);
         if (!ok) {
            scaffold.error("Value of `sector-count` attribute is not convertible to an unsigned integral (seen: "s + std::string(attr_value) + ")", attr);
         }
         return;
      }
      if (attr_name == "sector-base-id") {
         bool ok;
         dst.sector_base_id = lu::strings::to_integer<size_t>(attr_value.data(), attr_value.size(), &ok);
         if (!ok) {
            scaffold.error("Value of `sector-base-id` attribute is not convertible to an unsigned integral (seen: "s + std::string(attr_value) + ")", attr);
         }
         return;
      }

      scaffold.warn("Unrecognized attribute `"s + std::string(attr_name) + "` on a sector group element", attr);
   });
   //
   if (dst.name.empty()) {
      scaffold.error("Sector group has no name.", node);
   }
   if (dst.sector_base_id == (size_t)-1) {
      scaffold.error("Sector group has no base ID.", node);
   }
   if (dst.max_sector_count == 0) {
      scaffold.error("Sector group has no max sector count or a max sector count of zero.", node);
   }

   lu::rapidxml_helpers::for_each_child_element(node, [&scaffold, &dst, this](std::string_view tagname, xml_node<>& node) {
      if (tagname != "struct") {
         scaffold.warn("Unrecognized child element `"s + std::string(tagname) + "` in sector group `" + dst.name + "`; skipping...", node);
         return;
      }

      auto& entry = dst.top_level_structs.emplace_back();
      lu::rapidxml_helpers::for_each_attribute(node, [&scaffold, &entry, this](std::string_view attr_name, std::string_view attr_value, xml_attribute<>& attr) {
         if (attr_name == "name") {
            std::string name_s = std::string(attr_value);
            if (this->structs.contains(name_s)) {
               entry.definition = this->structs[name_s].get();
            } else {
               scaffold.error("No definition for struct `"s + name_s + "` has been loaded yet.", attr);
            }
         }
         if (attr_name == "ptr") {
            if (!entry.c_variable.name.empty()) {
               scaffold.warn("Overriding variable name for a struct reference in a sector group element", attr);
            }
            entry.c_variable.name       = attr_value;
            entry.c_variable.is_pointer = true;
            return;
         }
         if (attr_name == "var") {
            if (!entry.c_variable.name.empty()) {
               scaffold.warn("Overriding variable name for a struct reference in a sector group element", attr);
            }
            entry.c_variable.name       = attr_value;
            entry.c_variable.is_pointer = false;
            return;
         }
         if (attr_name == "var-header" || attr_name == "ptr-header") {
            entry.c_variable.header = attr_value;
            return;
         }
         scaffold.warn("Unrecognized attribute `"s + std::string(attr_name) + "` on a struct reference in a sector group element", attr);
      });
      if (!entry.definition) {
         scaffold.error("Struct reference in a sector group element didn't actually specify a struct typename.", node);
      }
      if (entry.c_variable.name.empty()) {
         scaffold.error("Struct reference in a sector group element didn't specify a global variable name to serialize.", node);
      }
      if (entry.c_variable.header.empty()) {
         scaffold.error("Struct reference in a sector group element didn't specify what header declares the global variable it wants to serialize.", node);
      }
   });

   this->sector_groups.push_back(std::move(dst));
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

   lu::rapidxml_helpers::for_each_child_element(*doc_root, [this, &scaffold](std::string_view tagname, xml_node<>& node) {
      if (tagname == "dependencies") {
         this->_parse_and_handle_dependencies(scaffold, node);
      } else if (tagname == "constants") {
         this->_parse_constants(scaffold, node);
      } else if (tagname == "heritables") {
         this->_parse_heritables(scaffold, node);
      } else if (tagname == "types") {
         this->_parse_types(scaffold, node);
      } else if (tagname == "sector-groups") {
         lu::rapidxml_helpers::for_each_child_element(node, [this, &scaffold](std::string_view tagname, xml_node<>& node) {
            if (tagname == "group") {
               this->_parse_sector_group(scaffold, node);
               return;
            }
            scaffold.warn("Unrecognized child element `"s + std::string(tagname) + "` in sector group set; skipping...", node);
         });
      }
   });
}

void registry::parse_all_xml_files() {
   for (const auto& entry : std::filesystem::directory_iterator(this->paths.input_paths.xml)) {
      if (entry.is_directory())
         continue;

      auto current_path = _normalize_xml_path(entry.path());
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

void registry::clear() {
   this->sector_groups.clear();
   this->blind_unions.clear();
   this->structs.clear();
   this->constants.clear();
   this->heritables.clear();
   this->seen_files.clear();
   this->load_state.unions.clear();
}