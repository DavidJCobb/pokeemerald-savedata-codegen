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

bool registry::generate_all_files() {
   bool failed = false;

   struct {
      std::string overall_stats;
      std::string struct_table;
      std::vector<std::string> by_sector;
      std::string every_struct;
      std::string every_heritable;
   } report;
   report.by_sector.resize(this->sector_groups.size());

   #pragma region Split across sectors, and generate report text
   std::vector<const ast::structure*> top_level_structures;
   size_t total_bytes_in_ram = 0;

   #pragma region Report tables: overall stats; per-struct stats
   {
      size_t sector_count = 0;
      for (auto& info : this->sector_groups) {
         sector_count += info.max_sector_count;
         //
         for (auto& s_info : info.top_level_structs)
            top_level_structures.push_back(s_info.definition);
      }

      size_t total_packed_bits  = 0;

      report.struct_table += '\n';
      report.struct_table += "| Name | Bytes in RAM | Packed bits | Packed bytes | Savings |\n";
      report.struct_table += "| - | -: | -: | -: | -: |\n";
      for (const auto* s : top_level_structures) {
         size_t bytes_in_ram = s->compute_unpacked_bytecount();
         size_t packed_bits  = s->compute_total_bitcount();
         size_t packed_bytes = packed_bits / 8 + ((packed_bits % 8) ? 1 : 0);

         report.struct_table += "| ";
         report.struct_table += s->name;
         report.struct_table += " | ";
         report.struct_table += lu::strings::from_integer(bytes_in_ram);
         report.struct_table += " | ";
         report.struct_table += lu::strings::from_integer(packed_bits);
         report.struct_table += " |";
         report.struct_table += lu::strings::from_integer(packed_bytes);
         report.struct_table += " | ";
         report.struct_table += lu::strings::from_integer(bytes_in_ram - packed_bytes);
         report.struct_table += " (";
         {
            float perc = (float)packed_bytes / (float)bytes_in_ram;
            report.struct_table += lu::strings::from_integer((int64_t)(100.0 - perc * 100.0));
         }
         report.struct_table += "%)";
         report.struct_table += " |\n";

         total_bytes_in_ram += bytes_in_ram;
         total_packed_bits  += packed_bits;
      }
      report.struct_table += "| **Total** | ";
      report.struct_table += lu::strings::from_integer(total_bytes_in_ram);
      report.struct_table += " | ";
      report.struct_table += lu::strings::from_integer(total_packed_bits);
      report.struct_table += " |";
      {
         size_t total_packed_bytes = total_packed_bits / 8 + ((total_packed_bits % 8) ? 1 : 0);
         report.struct_table += lu::strings::from_integer(total_packed_bytes);
         report.struct_table += " | ";
         report.struct_table += lu::strings::from_integer(total_bytes_in_ram - total_packed_bytes);
         report.struct_table += " (";
         {
            float perc = (float)total_packed_bytes / (float)total_bytes_in_ram;
            report.struct_table += lu::strings::from_integer((int64_t)(100.0 - perc * 100.0));
         }
         report.struct_table += "%)";
         report.struct_table += " |\n";
      }

      report.overall_stats += lu::strings::from_integer(total_bytes_in_ram);
      report.overall_stats += " bytes in RAM / ";
      report.overall_stats += lu::strings::from_integer(codegen::sector_generator::max_bytes_per_sector * sector_count);
      report.overall_stats += " bytes available across ";
      report.overall_stats += lu::strings::from_integer(sector_count);
      report.overall_stats += " sectors (";
      {
         float perc = (float)total_bytes_in_ram / (float)(codegen::sector_generator::max_bytes_per_sector * sector_count);
         report.overall_stats += lu::strings::from_integer((int)(perc * 100.0));
      }
      report.overall_stats += "% space usage)  \n";

      report.overall_stats += lu::strings::from_integer(total_packed_bits);
      report.overall_stats += " packed bits = ";
      {
         size_t bytespan = (total_packed_bits / 8) + ((total_packed_bits % 8) ? 1 : 0);
         report.overall_stats += lu::strings::from_integer(bytespan);
         report.overall_stats += " packed bytes (";
         {
            float perc = (float)bytespan / (float)(codegen::sector_generator::max_bytes_per_sector * sector_count);
            report.overall_stats += lu::strings::from_integer((int)(perc * 100.0));
         }
      }
      report.overall_stats += "% space usage)  \n";
   }
   #pragma endregion

   std::vector<codegen::sector_generator> generators;
   generators.resize(this->sector_groups.size());
   for (size_t i = 0; i < this->sector_groups.size(); ++i) {
      auto& info = this->sector_groups[i];
      auto& gen  = generators[i];

      gen.function_name_fragment               = info.name;
      gen.sector_serialize_header_folder       = this->paths.output_paths.sector_serialize.generic_string();
      gen.whole_struct_serialize_header_folder = this->paths.output_paths.struct_serialize.generic_string();
      
      std::vector<const ast::structure*> structures;
      for (auto& entry : info.top_level_structs) {
         structures.push_back(entry.definition);
      }
      gen.prepare(structures);

      auto& sector_report = report.by_sector[i];
      sector_report = "## Sector group: " + info.name + "\n";

      bool this_sector_group_failed = gen.items_by_sector.size() > info.max_sector_count;

      if (this_sector_group_failed) {
         failed = true;
         sector_report += "**Status:** Failed; overflowed max sector count, which was ";
         sector_report += lu::strings::from_integer(info.max_sector_count);
         sector_report += ".\n\n";
      } else {
         sector_report += "**Status:** Successfully packed into ";
         sector_report += lu::strings::from_integer(info.max_sector_count);
         sector_report += " sectors.\n\n";
      }

      sector_report += '\n';
      sector_report += "| Sector # | Bits used | Bits available | % |\n";
      sector_report += "| - | -: | -: |\n";
      //
      size_t total_packed_bits = 0;
      for (size_t i = 0; i < gen.items_by_sector.size(); ++i) {
         const auto& sector_items = gen.items_by_sector[i];

         size_t bits_per_sector = 0;
         for (const auto* item : sector_items) {
            bits_per_sector += item->bitcount();
         }
         total_packed_bits += bits_per_sector;

         sector_report += "| ";
         sector_report += lu::strings::from_integer(i);
         sector_report += " | ";
         sector_report += lu::strings::from_integer(bits_per_sector);
         sector_report += " | ";
         {
            size_t available = codegen::sector_generator::max_bytes_per_sector * 8;
            sector_report += lu::strings::from_integer(available);
            sector_report += " | ";
            {
               auto perc = (float)bits_per_sector / (float)available;
               sector_report += lu::strings::from_integer((int)(perc * 100.0));
               sector_report += '%';
            }
         }
         sector_report += " |\n";
      }
      sector_report += "| Total | ";
      sector_report += lu::strings::from_integer(total_packed_bits);
      sector_report += " | ";
      {
         auto total_available = codegen::sector_generator::max_bytes_per_sector * 8 * gen.items_by_sector.size();
         sector_report += lu::strings::from_integer(total_available);
         sector_report += " | ";
         {
            auto perc = (float)total_packed_bits / (float)total_available;
            sector_report += lu::strings::from_integer((int)(perc * 100.0));
            sector_report += '%';
         }
      }
      sector_report += " |\n\n";

      sector_report += "### Member info\n";

      auto serialization_item_to_field_name = [](const codegen::serialization_item& item) {
         std::string out = item.accessor;
         if (auto* member = item.member_definition) {
            auto& indices = item.array_indices;
            auto& extents = member->array_extents;
            //
            for (auto index : indices) {
               out += '[';
               out += lu::strings::from_integer(index);
               out += ']';
            }
            for (size_t i = indices.size(); i < extents.size(); ++i) {
               out += "[*]";
            }
         }
         return out;
      };

      for (size_t i = 0; i < gen.items_by_sector.size(); ++i) {
         const auto& sector_items = gen.items_by_sector[i];

         sector_report += "* Last field serialized to sector ";
         sector_report += lu::strings::from_integer(i);
         sector_report += ": ";
         if (sector_items.empty()) {
            sector_report += "<none>";
         } else {
            auto* item = sector_items.back();
            sector_report += '`';
            sector_report += serialization_item_to_field_name(*item);
            sector_report += '`';
         }
         sector_report += "\n";
      }

      if (this_sector_group_failed) {
         sector_report += "\n";
         for (size_t i = 0; i < gen.items_by_sector.size(); ++i) {
            sector_report += "#### All fields in sector ";
            sector_report += lu::strings::from_integer(i);
            sector_report += "\n";
            for (auto* item : gen.items_by_sector[i]) {
               sector_report += "* `";
               sector_report += serialization_item_to_field_name(*item);
               sector_report += "`\n";
            }
            sector_report += '\n';
         }
      }
   }

   #pragma region Report table: every struct
   {
      struct struct_info {
         size_t count       = 0;
         size_t ram_bytes   = 0;
         size_t packed_bits = 0;
      };
      std::map<std::string, struct_info> all_structs;

      auto _walk_struct = [&all_structs](this auto&& recurse, const ast::structure& s, const ast::member* m, size_t containing_array_count) -> void {
         auto& entry = all_structs[s.name];

         size_t total_count = 1;
         if (m) {
            for (auto& rank : m->array_extents)
               total_count *= rank.extent.value;
         }
         entry.count += containing_array_count * total_count;

         if (!entry.ram_bytes) {
            entry.ram_bytes = s.compute_unpacked_bytecount();
         }
         if (!entry.packed_bits) {
            entry.packed_bits = s.compute_total_bitcount();
         }

         for (auto& member_ptr : s.members) {
            auto* member = member_ptr.get();
            while (auto* casted = dynamic_cast<ast::inlined_union_member*>(member)) {
               member = &casted->get_member_to_serialize();
            }
            if (auto* casted = dynamic_cast<ast::struct_member*>(member)) {
               recurse(*casted->type_def, casted, containing_array_count * total_count);
            }
         }
      };
      for (auto* s : top_level_structures) {
         _walk_struct(*s, nullptr, 1);
      }
      report.every_struct += '\n';
      report.every_struct += "| Name | Bytes in RAM | Packed bits | Packed bytes | Savings per | Count | Savings total |\n";
      report.every_struct += "| - | -: | -: | -: | -: | -: | -: |\n";
      for (const auto& pair : all_structs) {
         auto& name  = pair.first;
         auto& stats = pair.second;

         size_t packed_bytes = stats.packed_bits / 8 + (stats.packed_bits % 8 ? 1 : 0);

         report.every_struct += "| ";
         report.every_struct += name;
         report.every_struct += " | ";
         report.every_struct += lu::strings::from_integer(stats.ram_bytes);
         report.every_struct += " | ";
         report.every_struct += lu::strings::from_integer(stats.packed_bits);
         report.every_struct += " | ";
         report.every_struct += lu::strings::from_integer(packed_bytes);
         report.every_struct += " | ";
         {  // Savings per
            report.every_struct += lu::strings::from_integer(stats.ram_bytes - packed_bytes);
            report.every_struct += " (";
            {
               float perc = (float)packed_bytes / (float)stats.ram_bytes;
               report.every_struct += lu::strings::from_integer((int64_t)(100.0 - perc * 100.0));
            }
            report.every_struct += "%)";
         }
         report.every_struct += " | ";
         report.every_struct += lu::strings::from_integer(stats.count);
         report.every_struct += " | ";
         {  // Savings total
            size_t numer = packed_bytes * stats.count;

            report.every_struct += lu::strings::from_integer((stats.ram_bytes * stats.count) - numer);
            report.every_struct += " (";
            {
               float perc = (float)numer / (float)total_bytes_in_ram;
               report.every_struct += lu::strings::from_integer((int64_t)(perc * 100.0));
            }
            report.every_struct += "%)";
         }
         report.every_struct += " |\n";
      }
      report.every_struct += '\n';
   }
   #pragma endregion

   #pragma region Report table: heritables
   {
      struct heritable_count_set {
         std::vector<size_t> by_sector_group;
      };
      std::unordered_map<const ast::heritable*, heritable_count_set> heritable_counts;
      
      auto _walk_struct = [&heritable_counts](this auto&& recurse, size_t sector_group_idx, const ast::structure& s, const ast::member* m, size_t containing_array_count) -> void {
         size_t total_count = 1;
         if (m) {
            for (auto& rank : m->array_extents)
               total_count *= rank.extent.value;
         }
         total_count *= containing_array_count;

         for (auto& member_ptr : s.members) {
            auto* member = member_ptr.get();
            while (auto* casted = dynamic_cast<ast::inlined_union_member*>(member)) {
               member = &casted->get_member_to_serialize();
            }
            if (member->inherited_from) {
               auto& list = heritable_counts[member->inherited_from].by_sector_group;
               if (list.size() < sector_group_idx + 1)
                  list.resize(sector_group_idx + 1);
               list[sector_group_idx] += total_count;
            }
            if (auto* casted = dynamic_cast<ast::struct_member*>(member)) {
               recurse(sector_group_idx, *casted->type_def, casted, total_count);
            }
         }
      };
      for (size_t i = 0; i < this->sector_groups.size(); ++i) {
         const auto& group = this->sector_groups[i];
         for (const auto& entry : group.top_level_structs) {
            _walk_struct(i, *entry.definition, nullptr, 1);
         }
      }

      report.every_heritable += "\n"; // GitHub Markdown tables require a blank line above them.
      report.every_heritable += "| Name";
      for (auto& group : this->sector_groups) {
         report.every_heritable += " | Uses in ";
         report.every_heritable += group.name;
      }
      report.every_heritable += " | Total uses |\n";
      //
      report.every_heritable += "| -";
      for (auto& group : this->sector_groups) {
         report.every_heritable += " | -: ";
      }
      report.every_heritable += " | -: |\n";
      //
      for (const auto& pair : this->heritables) {
         const auto* heritable = pair.second.get();

         heritable_count_set counts;
         {
            auto it = heritable_counts.find(heritable);
            if (it != heritable_counts.end())
               counts = it->second;
         }
         counts.by_sector_group.resize(this->sector_groups.size());
         //
         report.every_heritable += "| ";
         report.every_heritable += heritable->name;
         size_t total = 0;
         for (auto count : counts.by_sector_group) {
            report.every_heritable += " | ";
            report.every_heritable += lu::strings::from_integer(count);
            total += count;
         }
         report.every_heritable += " | ";
         report.every_heritable += lu::strings::from_integer(total);
         report.every_heritable += " |\n";
      }
      report.every_heritable += '\n';
   }
   #pragma endregion
   #pragma endregion

   if (failed) {
      auto out_path = this->paths.output_paths.codegen_report;
      if (out_path.empty()) {
         out_path = this->paths.output_paths.h / this->paths.output_paths.sector_serialize / "README.md";
      }

      {
         auto out_folder = out_path.remove_filename();
         std::error_code error;
         if (!std::filesystem::create_directories(out_folder, error))
            assert(!error);
      }

      std::ofstream stream(out_path);
      assert(!!stream);
      stream << "# Last failed codegen\n";
      stream << "This report describes the last failed attempt at code generation.\n\n"; // TODO: include date
      stream << "## Overall stats\n";
      stream << report.overall_stats;
      stream << "\n\n";
      stream << "## Struct stats";
      stream << report.struct_table;
      stream << "\n\n";
      for (const auto& s : report.by_sector)
         stream << s;
      stream << "\n\n## Every struct\n";
      stream << "The \"Savings\" columns are measured in bytes. The percentage in the \"Savings per\" column indicates by "
                "how much the struct itself has been made smaller thanks to bitpacking. The percentage in the \"Savings "
                "total\" column indicates by how much the total size of all serialized data has been made smaller as a "
                "result of bitpacking that row's specific struct type.\n";
      stream << report.every_struct;
      stream << "\n\n## Every heritable data definition\n";
      stream << report.every_heritable;
   } else {
      {  // Create all paths
         std::error_code error;
         if (!std::filesystem::create_directories(this->paths.output_paths.h, error)) {
            assert(!error);
         }
         if (!std::filesystem::create_directories(this->paths.output_paths.c, error)) {
            assert(!error);
         }
         if (!std::filesystem::create_directories(this->paths.output_paths.h / this->paths.output_paths.sector_serialize, error)) {
            assert(!error);
         }
         if (!std::filesystem::create_directories(this->paths.output_paths.c / this->paths.output_paths.sector_serialize, error)) {
            assert(!error);
         }
         if (!std::filesystem::create_directories(this->paths.output_paths.h / this->paths.output_paths.struct_serialize, error)) {
            assert(!error);
         }
         if (!std::filesystem::create_directories(this->paths.output_paths.c / this->paths.output_paths.struct_serialize, error)) {
            assert(!error);
         }
         // Note: next needs only an H subfolder
         if (!std::filesystem::create_directories(this->paths.output_paths.h / this->paths.output_paths.struct_members, error)) {
            assert(!error);
         }
         //
         if (!std::filesystem::create_directories(this->paths.output_paths.h / this->paths.output_paths.save_functors, error)) {
            assert(!error);
         }
         if (!std::filesystem::create_directories(this->paths.output_paths.c / this->paths.output_paths.save_functors, error)) {
            assert(!error);
         }
      }

      {  // Save report
         auto out_path = this->paths.output_paths.codegen_report;
         if (out_path.empty()) {
            out_path = this->paths.output_paths.h / this->paths.output_paths.sector_serialize / "README.md";
         }

         std::ofstream stream(out_path);
         assert(!!stream);
         stream << "# Codegen for bitpacked savedata\n";
         stream << "This report describes the last successful attempt at code generation.\n\n"; // TODO: include date
         stream << "In vanilla `pokeemerald`, the `SaveBlock1` (world state) and `SaveBlock2` (character state) structs consume "
                   "99% of the space allotted to them in flash memory (savedata). This is because they are blindly `memcpy`'d "
                   "from RAM. A bitpacked format would consume substantially less space; however, maintaining the code to bitpack "
                   "these structs would be prohibitively difficult by hand because savedata is split into ~4KiB strips (\"sectors\"). "
                   "Instead, we use custom code generation to produce the serialization code, minding sector boundaries.\n\n";
         stream << "## Overall stats\n";
         stream << report.overall_stats;
         stream << "\n\n";
         stream << "## Struct stats";
         stream << report.struct_table;
         stream << "\n\n";
         for (const auto& s : report.by_sector)
            stream << s;
         stream << "\n\n## Every struct\n";
         stream << "The \"Savings\" columns are measured in bytes. The percentage in the \"Savings per\" column indicates by "
                   "how much the struct itself has been made smaller thanks to bitpacking. The percentage in the \"Savings "
                   "total\" column indicates by how much the total size of all serialized data has been made smaller as a "
                   "result of bitpacking that row's specific struct type.\n";
         stream << report.every_struct;
         stream << "\n\n## Every heritable data definition\n";
         stream << report.every_heritable;
      }
      this->generate_all_struct_body_files();
      this->generate_whole_struct_serialization_code();
      {
         auto out_folder_h = this->paths.output_paths.h / this->paths.output_paths.sector_serialize;
         auto out_folder_c = this->paths.output_paths.c / this->paths.output_paths.sector_serialize;
         for (size_t i = 0; i < sector_groups.size(); ++i) {
            auto& info = sector_groups[i];
            auto& gen  = generators[i];

            auto output = gen.generate();

            {
               std::ofstream stream(out_folder_h / std::filesystem::path(info.name + ".h"));
               assert(!!stream);
               stream << output.header;
            }
            {
               std::ofstream stream(out_folder_c / std::filesystem::path(info.name + ".c"));
               assert(!!stream);
               stream << output.implementation;
            }
         }
      }
      this->generate_save_functors();
   }

   return !failed;
}

void registry::generate_all_struct_body_files() {
   auto base_path = this->paths.output_paths.h / this->paths.output_paths.struct_members;
   for (auto& pair : this->structs) {
      std::ofstream stream(base_path / std::filesystem::path(pair.first + ".members.inl"));
      assert(!!stream);

      stream << pair.second->as_c_member_declaration_list();
   }
}

void registry::generate_whole_struct_serialization_code() {
   auto out_folder_h = this->paths.output_paths.h / this->paths.output_paths.struct_serialize;
   auto out_folder_c = this->paths.output_paths.c / this->paths.output_paths.struct_serialize;
   for (auto& pair : this->structs) {
      const auto* s_def = pair.second.get();

      {  // h
         std::ofstream stream(out_folder_h / std::filesystem::path("serialize_"s + pair.first + ".h"));
         assert(!!stream);

         stream << "#ifndef GUARD_LU_SERIALIZE_" << pair.first << "\n";
         stream << "#define GUARD_LU_SERIALIZE_" << pair.first << "\n";
         stream << '\n';
         stream << "struct lu_BitstreamState;\n"; // must declare struct for it to be globally scoped
         if (s_def->c_type_info.is_defined_via_typedef) {
            //
            // Cannot forward-declare structs defined as `typedef struct { ... } name`.
            //
            if (!s_def->header.empty()) {
               if (s_def->header != "global.h") {
                  stream << "#include \"global.h\"\n"; // lots of pokeemerald stuff breaks if this isn't included first/at all
               }
               stream << "#include \"" << s_def->header << "\"\n";
            }
         } else {
            stream << "struct " << pair.first << ";\n"; // must declare struct for it to be globally scoped
         }
         stream << '\n';

         stream << "void lu_BitstreamRead_" << pair.first << "(struct lu_BitstreamState*, ";
         if (!s_def->c_type_info.is_defined_via_typedef) {
            stream << "struct ";
         }
         stream << pair.first << "* dst);\n";

         stream << "void lu_BitstreamWrite_" << pair.first << "(struct lu_BitstreamState*, const ";
         if (!s_def->c_type_info.is_defined_via_typedef) {
            stream << "struct ";
         }
         stream << pair.first << "* src);\n";

         stream << '\n';
         stream << "#endif";
      }
      {  // c
         std::ofstream stream(out_folder_c / std::filesystem::path("serialize_"s + pair.first + ".c"));
         assert(!!stream);

         stream << "#include \"" << this->paths.output_paths.struct_serialize.generic_string() << "/serialize_" << pair.first << ".h\"\n\n";

         stream << "#include \"global.h\"\n"; // lots of pokeemerald stuff breaks if this isn't included first/at all
         if (!s_def->header.empty() && s_def->header != "global.h") {
            // this should also take care of including all preprocessor constants referenced by the struct's members
            stream << "#include \"" << s_def->header << "\" // struct definition\n";
         }
         stream << '\n';

         stream << "#include \"lu/bitstreams.h\"\n\n";

         std::vector<std::string> dependencies = s_def->get_all_direct_struct_dependencies();
         bool has_any_strings = s_def->has_any_string_members();

         if (dependencies.size()) {
            stream << "// dependencies\n";
            for (const auto& name : dependencies) {
               stream << "#include \"";
               stream << this->paths.output_paths.struct_serialize.generic_string();
               stream << "/serialize_" << name << ".h\"\n";
            }
            stream << '\n';
         }
         if (has_any_strings) {
            stream << "#include \"string_util.h\" // gflib; for StringLength\n\n";
         }

         {
            auto constants = s_def->get_all_direct_constant_dependencies();
            if (!constants.empty()) {
               stream << "// check constants:\n";
               for (const auto& item : constants) {
                  const auto& entry = this->constants[item];

                  stream << "#if " << item << " != " << entry->value << "\n";
                  stream << "   #error Constant `" << item << "` changed in C, but XML not updated or codegen not re-run!\n";
                  stream << "#endif\n";
               }
               stream << "\n";
            }
         }

         std::string code_read;
         std::string code_write;

         {
            code_read   = "void lu_BitstreamRead_";
            code_write  = "void lu_BitstreamWrite_";
            code_read  += pair.first;
            code_write += pair.first;
            code_read  += "(struct lu_BitstreamState* state, ";
            code_write += "(struct lu_BitstreamState* state, const ";
            if (!s_def->c_type_info.is_defined_via_typedef) {
               code_read  += "struct ";
               code_write += "struct ";
            }
            code_read  += pair.first;
            code_write += pair.first;
            code_read  += "* v) {\n";
            code_write += "* v) {\n";

            {
               auto item = codegen::serialization_item::for_top_level_struct(*s_def);
               item.accessor = "v"; // override the default, which is p_StructName

               codegen::serialization_function_body body;
               body.item_list = item.expand();
               //
               body.state_is_argument      = true;
               body.writes_can_debug_print = false;

               body.generate();

               code_read  += body.out.read;
               code_write += body.out.write;

               for (auto* item : body.item_list)
                  delete item;
            }

            code_read  += "}\n";
            code_write += "}\n";
         }

         stream << code_read;
         stream << '\n';
         stream << code_write;
      }
   }
}

void registry::generate_sector_code() {
   auto out_folder_h = this->paths.output_paths.h / this->paths.output_paths.sector_serialize;
   auto out_folder_c = this->paths.output_paths.c / this->paths.output_paths.sector_serialize;
   for (auto& info : this->sector_groups) {
      codegen::sector_generator gen;
      gen.function_name_fragment = info.name;
      gen.sector_serialize_header_folder       = this->paths.output_paths.sector_serialize.generic_string();
      gen.whole_struct_serialize_header_folder = this->paths.output_paths.struct_serialize.generic_string();

      std::vector<const ast::structure*> structures;
      for (auto& entry : info.top_level_structs) {
         structures.push_back(entry.definition);
      }
      gen.prepare(structures);
      auto output = gen.generate();

      {
         std::ofstream stream(out_folder_h / std::filesystem::path(info.name + ".h"));
         assert(!!stream);
         stream << output.header;
      }
      {
         std::ofstream stream(out_folder_c / std::filesystem::path(info.name + ".c"));
         assert(!!stream);
         stream << output.implementation;
      }

      if (output.sector_count > info.max_sector_count) {
         throw std::runtime_error(
            "Exceeded sector count for sector group "s
            + info.name
            + "; number of sectors is "s
            + lu::strings::from_integer(output.sector_count)
            + ". Code files saved anyway, for you to review output."
         );
      }
   }
}

void registry::generate_save_functors() {
   auto out_folder_h = this->paths.output_paths.h / this->paths.output_paths.save_functors;
   auto out_folder_c = this->paths.output_paths.c / this->paths.output_paths.save_functors;

   {
      std::ofstream stream(out_folder_h / "save_functor_table.inl");
      assert(!!stream);

      size_t sector = 0;
      for (auto& group : this->sector_groups) {
         for (size_t slice = 0; slice < group.max_sector_count; ++sector, ++slice) {
            stream << "   SAVEBLOCK_LAYOUT_ITEM(";
            stream << slice;
            stream << ", ";
            stream << group.name;
            stream << "),";
            switch (sector) {
               case 0: stream << " // SECTOR_ID_SAVEBLOCK2"; break;
               case 1: stream << " // SECTOR_ID_SAVEBLOCK1_START"; break;
               case 4: stream << " // SECTOR_ID_SAVEBLOCK1_END"; break;
               case 5: stream << " // SECTOR_ID_PKMN_STORAGE_START"; break;
               case 13: stream << " // SECTOR_ID_PKMN_STORAGE_END"; break;
            }
            stream << '\n';
         }
      }
   }

   {
      std::ofstream stream(out_folder_h / "save_functors.h");
      assert(!!stream);
      stream << "#include \"global.h\"\n\n";

      for (auto& group : this->sector_groups) {
         stream << "extern bool8 ReadSector_";
         stream << group.name;
         stream << "(u8 sliceNum, const u8* src);\n";
         stream << "extern bool8 WriteSector_";
         stream << group.name;
         stream << "(u8 sliceNum, u8* dst);\n";
      }
   }
   {
      std::ofstream stream(out_folder_c / "save_functors.c");
      assert(!!stream);
      stream << "#include \"global.h\"\n\n";

      stream << "// sector-group serialize functions: \n";
      for (auto& group : this->sector_groups) {
         auto str = this->paths.output_paths.sector_serialize.generic_string();

         stream << "#include \"";
         stream << str;
         if (!str.empty() && str.back() != '/')
            stream << "/";
         stream << group.name;
         stream << ".h\"\n";
      }
      stream << '\n';

      {
         std::vector<std::string> headers;
         for (auto& group : this->sector_groups) {
            for (auto& entry : group.top_level_structs) {
               if (entry.c_variable.header != "global.h")
                  headers.push_back(entry.c_variable.header);
            }
         }
         if (!headers.empty()) {
            headers.erase(std::unique(headers.begin(), headers.end()), headers.end());
            stream << "// globals to serialize:\n";
            for (auto& header : headers) {
               stream << "#include \"";
               stream << header;
               stream << "\"\n";
            }
            stream << '\n';
         }
      }

      for (auto& group : this->sector_groups) {
         codegen::serialization_function_body::function_pair pair;

         pair += "extern bool8 ";
         pair.read  += "Read";
         pair.write += "Write";
         pair += "Sector_";
         pair += group.name;
         pair += "(u8 sliceNum, ";
         pair.read  += "const u8* src";
         pair.write += "u8* dst";
         pair += ") {\n";
         
         {
            bool first = true;
            for (size_t i = 0; i < group.max_sector_count; ++i) {
               if (!first) {
                  pair += "   } else ";
               } else {
                  pair  += "   ";
                  first  = false;
               }
               pair += "if (sliceNum == ";
               pair += lu::strings::from_integer(i);
               pair += ") {\n";
               pair += "      lu_";
               pair.read  += "Read";
               pair.write += "Write";
               pair += "SaveSector_";
               pair += group.name;
               if (i < 10) {
                  pair += '0';
               }
               pair += lu::strings::from_integer(i);
               pair += "(";
               pair.read  += "src, ";
               pair.write += "dst, ";
               for (size_t j = 0; j < group.top_level_structs.size(); ++j) {
                  auto& entry = group.top_level_structs[j];
                  if (!entry.c_variable.is_pointer) {
                     pair += '&';
                  }
                  pair += entry.c_variable.name;
                  if (j + 1 < group.top_level_structs.size()) {
                     pair += ", ";
                  }
               }
               pair += ");\n";
            }
            pair += "   } else {\n";
            pair += "      return FALSE;\n";
            pair += "   }\n";
            pair += "   return TRUE;\n";
         }
         pair += "}\n";

         stream << pair.read;
         stream << pair.write;
         stream << '\n';
      }
   }
}






std::string registry::_dump_unpacked(const void* data, size_t max_size, const std::vector<std::string>& structures) {
   struct_bytewalker walker;
   walker.set_buffer(data, max_size);

   std::vector<const ast::structure*> s_defs;
   for (const auto& name : structures) {
      s_defs.push_back(this->structs[name].get());
   }

   walker.walk(s_defs);

   return walker.output;
}
std::string registry::_dump_packed(const void* data, size_t max_size, const std::vector<std::string>& structures) {
   struct_bitwalker walker;
   walker.set_buffer(data, max_size);

   std::vector<const ast::structure*> s_defs;
   for (const auto& name : structures) {
      s_defs.push_back(this->structs[name].get());
   }

   walker.walk(s_defs);

   return walker.output;
}

std::string registry::_dump(bool bitpacked, const void* data, size_t max_size, const std::vector<std::string>& structures) {
   if (bitpacked) {
      return _dump_packed(data, max_size, structures);
   } else {
      return _dump_unpacked(data, max_size, structures);
   }
}





std::vector<std::string> registry::_dump_from_sav(const void* sav, size_t sav_size, bool packed) {
   constexpr const size_t sector_full_size   = 4096;
   constexpr const size_t sector_footer_size = 128;
   constexpr const size_t sector_data_size   = sector_full_size - sector_footer_size;

   constexpr const uint32_t valid_sector_signature = 0x08012025;

   constexpr const size_t slots_per_save = 2;

   struct sector_footer {
      uint8_t  unused[sector_footer_size - 12];
      uint16_t id;
      uint16_t checksum;
      uint32_t signature;
      uint32_t counter;
   };

   struct stitched_sector_group {
      const sector_group* definition = nullptr;
      //
      uint8_t* buffer = nullptr;
      uint16_t pieces = 0; // mask

      ~stitched_sector_group() {
         if (buffer)
            delete buffer;
      }

      constexpr bool is_complete() const {
         auto mask = (1 << this->definition->max_sector_count) - 1;
         return (pieces & mask) == mask;
      }

      constexpr size_t size_if_complete() const {
         return sector_data_size * this->definition->max_sector_count;
      }

      void stitch(const void* src, uint8_t piece) {
         if (pieces & (1 << piece)) { // we already loaded this piece?
            return;
         }
         //assert((pieces & (1 << piece)) == 0);
         if (!buffer)
            buffer = (uint8_t*)malloc(size_if_complete());

         memcpy(buffer + (piece * sector_data_size), src, sector_data_size);
         pieces |= (1 << piece);
      }
   };

   struct slot {
      std::vector<stitched_sector_group> loaded_groups;
   };

   std::string seen_ids;
   std::array<slot, slots_per_save> slots;
   {
      size_t group_count = this->sector_groups.size();
      for (auto& slot : slots) {
         slot.loaded_groups.resize(group_count);
         for (size_t i = 0; i < group_count; ++i) {
            slot.loaded_groups[i].definition = &this->sector_groups[i];
         }
      }
   }

   size_t sector_count = sav_size / sector_full_size;
   for (size_t i = 0; i < sector_count; ++i) {
      const void* sector = (uint8_t*)sav + (i * sector_full_size);
      const auto* foot   = (const sector_footer*)((uint8_t*)sector + sector_data_size);
      if (foot->signature != valid_sector_signature)
         continue;

      seen_ids += "Sector ";
      seen_ids += lu::strings::from_integer(i);
      seen_ids += " has ID ";
      seen_ids += lu::strings::from_integer(foot->id);
      seen_ids += ".\n";

      if (i > 14 * 2) { // special sector; does not cycle/rotate; is not tied to any one slot
         // TODO: If we want, dump HoF, Trainer Hill, and Recorded Battle here.
         continue;
      }
      uint16_t slot_local_id = foot->id;
      uint16_t slot_index    = 0;
      if (i >= 14) {
         slot_local_id %= 14;
         slot_index     = 1;
      }

      auto& target_slot = slots[slot_index];
      for (auto& group : target_slot.loaded_groups) {
         size_t start = group.definition->sector_base_id;
         size_t end   = start + group.definition->max_sector_count;

         if (slot_local_id >= start && slot_local_id < end) {
            group.stitch(sector, slot_local_id - start);
            continue;
         }
      }
   }

   std::vector<std::string> out;
   if (!seen_ids.empty()) {
      out.push_back(seen_ids);
   }
   for (size_t i = 0; i < slots_per_save; ++i) {
      std::string header = std::string("# Slot ") + lu::strings::from_integer(i + 1) + "\n\n";

      auto& slot = slots[i];
      for (auto& group : slot.loaded_groups) {
         if (group.is_complete()) {
            std::vector<std::string> to_dump;
            for (const auto& entry : group.definition->top_level_structs) {
               to_dump.push_back(entry.definition->name);
            }

            try {
               out.push_back("## "s + group.definition->name + "\n\n" + _dump(
                  packed,
                  group.buffer,
                  group.size_if_complete(),
                  to_dump
               ));
            } catch (lu::bitstreams::exceptions::read_exception& e) {
               std::string entry = "## "s + group.definition->name + "\n\n";
               entry += "Bitstream read exception occurred: \n";
               entry += "Position: ";
               entry += lu::strings::from_integer(e.position.bytes);
               entry += " bytes + ";
               entry += lu::strings::from_integer(e.position.bits);
               entry += " bits\nTrace:\n";
               for (auto& detail : e.where_were_we) {
                  entry += "* ";
                  entry += detail.type_name;
                  if (detail.index != (size_t)-1) {
                     entry += '[';
                     entry += lu::strings::from_integer(detail.index);
                     entry += ']';
                  }
                  entry += '\n';
               }
               entry += '\n';
            }
         } else {
            out.push_back("## "s + group.definition->name + "\n\nIncomplete/missing!");
         }
      }
   }
   return out;
}

std::vector<std::string> registry::dump_unpacked_from_sav(const void* sav, size_t sav_size) {
   return this->_dump_from_sav(sav, sav_size, false);
}

std::vector<std::string> registry::dump_packed_from_sav(const void* sav, size_t sav_size) {
   return this->_dump_from_sav(sav, sav_size, true);
}