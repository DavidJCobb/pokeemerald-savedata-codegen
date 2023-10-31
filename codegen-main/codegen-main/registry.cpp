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
         scaffold.error("Field: You must specify a union member to serialize.", node);
      }

      lu::rapidxml_helpers::for_each_child_element(node, [this, &scaffold, &field](std::string_view tagname, xml_node<>& node) {
         bool is_checksum;
         auto nested = this->_parse_member(scaffold, node, is_checksum);
         if (is_checksum) {
            scaffold.error("Field: Checksums nested in unions are not supported.", node);
         }
         field->members.push_back(std::move(nested));
      });
      {
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
      }

      return field;
   }

   struct {
      std::string name;
      std::optional<bool> do_not_serialize;
      std::optional<ast::size_constant> c_alignment;

      std::string c_type;
      std::string c_type_decl;
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

      // attributes for inlined unions (we handle them in the special-case above, but 
      // we should still grab their attributes for more robust user warnings).
      std::string member_to_serialize;
   } attributes;

   lu::rapidxml_helpers::for_each_attribute(node, [this, &scaffold, &attributes](std::string_view attr_name, std::string_view attr_value, xml_attribute<>& attr) {
      if (attr_name == "name") {
         attributes.name = attr_value;
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
         if (attributes.c_type_decl.empty())
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
      if (tagname_integral.has_value() || attrval_integral.has_value()) {
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
            field = std::make_unique<ast::struct_member>();

            auto* casted = (ast::struct_member*)field.get();

            casted->type_name = attributes.c_type;
            if (!attributes.c_type_decl.empty()) {
               if (attributes.c_type_decl == "struct") {
                  casted->decl = ast::struct_member::decl::c_struct;
               } else if (attributes.c_type_decl == "union") {
                  casted->decl = ast::struct_member::decl::c_union;
               } else {
                  scaffold.warn("Field: Unrecognized value for attribute `c-type-decl` (seen: "s + attributes.c_type_decl + "); ignoring...", node);
               }
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

   field->name = attributes.name;
   if (attributes.do_not_serialize.has_value()) {
      field->skip_when_serializing = attributes.do_not_serialize.value();
   }
   if (attributes.c_alignment.has_value()) {
      field->c_alignment = attributes.c_alignment.value();
   }
   if (auto* casted = dynamic_cast<ast::integral_member*>(field.get())) {
      casted->min = attributes.min;
      casted->max = attributes.max;
      casted->c_bitfield = attributes.c_bitfield;
      casted->serialization_bitcount = attributes.serialization_bitcount;

      if (attributes.is_checksum.value_or(false))
         out_is_checksum = true;

      if (!attributes.c_type_decl.empty())
         scaffold.warn("Field: Unrecognized attribute (name: c-type-decl) (attribute is not valid for integral fields).", node);
      if (!attributes.char_type.empty())
         scaffold.warn("Field: Unrecognized attribute (name: char-type) (attribute is not valid for integral fields).", node);
      if (attributes.length.has_value())
         scaffold.warn("Field: Unrecognized attribute (name: length) (attribute is not valid for integral fields).", node);
      if (!attributes.member_to_serialize.empty())
         scaffold.warn("Field: Unrecognized attribute (name: member-to-serialize) (attribute is not valid for integral fields).", node);

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

      if (!attributes.c_type.empty())
         scaffold.warn("Field: Unrecognized attribute (name: c-type) (attribute is not valid for string fields; did you mean `char-type`?).", node);
      if (!attributes.c_type_decl.empty())
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
      if (!attributes.c_type_decl.empty()) {
         if (attributes.c_type_decl == "struct") {
            casted->decl = ast::struct_member::decl::c_struct;
         } else if (attributes.c_type_decl == "union") {
            casted->decl = ast::struct_member::decl::c_union;
         } else {
            scaffold.warn("Field: Unrecognized value for attribute `c-type-decl` (seen: "s + attributes.c_type_decl + "); ignoring...", node);
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

      if (casted->type_name.empty()) {
         scaffold.error("Field: field seems to be a named struct/union, but no name was found.", node);
      }
      if (this->structs.contains(casted->type_name)) {
         casted->type_def = this->structs[casted->type_name].get();
      } else {
         scaffold.error("Field: struct typename is not known/defined yet (seen: "s + casted->type_name + ").", node);
      }
   }

   //
   // Field is now mostly loaded, except for child elements in the XML.
   //

   if (field->name.empty()) {
      scaffold.error("Field defined with an empty or missing name", node);
   }

   lu::rapidxml_helpers::for_each_child_element(node, [this, &scaffold, &field](std::string_view tagname, xml_node<>& node) {
      if (tagname == "array-rank") {
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
         structure->members.push_back(std::move(field));
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
      bitcount += member->compute_total_bitcount();
   }
   return bitcount;
}

void registry::generate_all_struct_body_files(std::filesystem::path out_folder) {
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
         stream << "void lu_BitstreamWrite_" << pair.first << "(struct lu_BitstreamState*, const struct " << pair.first << "* src);";
         stream << '\n';
         stream << "#endif";
      }
      {  // cpp
         std::ofstream stream(out_folder / std::filesystem::path("serialize_"s + pair.first + ".cpp"));
         assert(!!stream);

         stream << "#include \"" << out_folder.string() << "/serialize_" << pair.first << ".h\"\n\n";

         if (!pair.second->header.empty()) {
            // this should also take care of including all preprocessor constants referenced by the struct's members
            stream << "#include \"" << pair.second->header << "\" // struct definition\n\n";
         }

         std::vector<std::string> dependencies = pair.second->get_all_direct_struct_dependencies();
         bool has_any_strings = pair.second->has_any_string_members();

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

         {
            auto constants = pair.second->get_all_direct_constant_dependencies();
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

         //
         // TODO: Do the BitstreamRead function, too. Zero-fill the target struct before reading.
         //
         stream << "// TODO:\n";
         stream << "// void lu_BitstreamRead_" << pair.first << "(struct lu_BitstreamState* state, struct " << pair.first << "* dst);\n";

         stream << '\n';

         stream << "void lu_BitstreamWrite_" << pair.first << "(struct lu_BitstreamState* state, const struct " << pair.first << "* src) {\n";
         for (const auto& m : pair.second->members) {
            std::string member_access = m->name;

            const ast::member* member = m.get();
            while (auto* casted = dynamic_cast<const ast::inlined_union_member*>(member)) {
               member = &(casted->get_member_to_serialize());

               member_access += '.';
               member_access += member->name;
            }

            auto&  extents = member->array_extents;
            size_t rank    = extents.size();

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
                  stream << indent << "      for (" << var << " = 0; " << var << " < " << extents[i].as_c_expression() << "; ++" << var << ") { \n";
                  indent += "   ";
               }
               stream << "      ";
            }

            if (auto* casted = dynamic_cast<const ast::struct_member*>(member)) {
               stream << indent << "   lu_BitstreamWrite_" << casted->type_name << "(state, ";
               stream << "&src." << member_access;
               _serialize_indices();
               stream << ");\n";
            } else if (auto* casted = dynamic_cast<const ast::string_member*>(member)) {
               assert(casted->max_length.value > 0);

               stream << indent << "   lu_BitstreamWrite_string(";
               stream << "state";
               stream << ", ";
               stream << "src." << member_access;
               _serialize_indices();
               stream << ", ";
               stream << casted->max_length.as_c_expression();
               stream << ", ";
               stream << std::bit_width(casted->max_length.value);
               stream << ");\n";
            } else if (auto* casted = dynamic_cast<const ast::integral_member*>(member)) {
               size_t bitcount = member->compute_single_element_bitcount();
               if (bitcount == 1) {
                  stream << indent << "   lu_BitstreamWrite_bool(state, src." << member_access;
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
               stream << "(state, src." << member_access;
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

const ast::structure* registry::lookup_struct_definition(const std::string& name) const {
   if (this->structs.contains(name))
      return this->structs.at(name).get();
   return nullptr;
}