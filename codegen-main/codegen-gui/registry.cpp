#include "./registry.h"
#include <bit>
#include <fstream>
#include <map>
#include <optional>
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

namespace {
   std::optional<bool> _handle_bool_attribute(registry::parse_wrapper& scaffold, rapidxml::xml_attribute<>& attr) {
      std::string_view value(attr.value(), attr.value_size());
      if (value == "true")
         return true;
      else if (value == "false")
         return false;

      std::string name(attr.name(), attr.name_size());
      scaffold.warn("Field: Unrecognized value for attribute `" + name + "` (seen: "s + std::string(value) + "); ignoring...", attr);
      return {};
   };
}

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
            auto v = _handle_bool_attribute(scaffold, attr);
            if (v.has_value())
               heritable->attributes.do_not_serialize = v;
            else
               scaffold.warn("Heritable: Unrecognized attribute value for `do-not-serialize` (seen: "s + std::string(attr_value) + "); ignoring...", attr); // can't wait for P2591...
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

         if (attr_name == "length") {
            auto constant = interpret_size_constant(attr_value);
            if (!constant.has_value()) {
               scaffold.error("Heritable: `length` attribute value is an unrecognized constant or otherwise not a valid unsigned integral (seen: "s + std::string(attr_value) + ")", attr); // can't wait for P2591...
            }
            heritable->attributes.length = constant.value();
            return;
         }
         if (attr_name == "only-early-terminator") {
            auto v = _handle_bool_attribute(scaffold, attr);
            if (v.has_value())
               heritable->attributes.only_early_terminator = v;
            else
               scaffold.warn("Heritable: Unrecognized attribute value for `only-early-terminator` (seen: "s + std::string(attr_value) + "); ignoring...", attr); // can't wait for P2591...
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

   enum class fundamental_member_type {
      number,
      string,
      pointer,
      structure,
      union_blind,
      union_bespoke,
   };
   std::optional<fundamental_member_type> fundamental_type;

   std::optional<ast::integral_type> tagname_integral = ast::integral_type_from_string(tagname);
   std::optional<ast::integral_type> attrval_integral;
   std::string_view c_type_attrval;
   if (auto* attr = lu::rapidxml_helpers::get_attribute(node, "c-type")) {
      c_type_attrval   = std::string_view(attr->value(), attr->value_size());
      attrval_integral = ast::integral_type_from_string(c_type_attrval);
   }

   if (tagname == "string") {
      fundamental_type = fundamental_member_type::string;
   } else if (tagname == "pointer") {
      fundamental_type = fundamental_member_type::pointer;
   } else if (tagname_integral.has_value() || attrval_integral.has_value()) {
      fundamental_type = fundamental_member_type::number;
   } else if (!c_type_attrval.empty()) {
      auto name_s = std::string(c_type_attrval);
      auto it_s   = this->structs.find(name_s);
      auto it_u   = this->blind_unions.find(name_s);
      if (it_s != this->structs.end()) {
         assert(it_u == this->blind_unions.end());
         fundamental_type = fundamental_member_type::structure;
      } else if (it_u != this->blind_unions.end()) {
         assert(it_s == this->blind_unions.end());
         fundamental_type = fundamental_member_type::union_blind;
      }
   }

   if (!fundamental_type.has_value()) {
      scaffold.error("Field: Unable to identify fundamental type (integral? string? struct? inlined union?).", node);
   }

   std::unique_ptr<ast::member> field;
   switch (fundamental_type.value()) {
      case fundamental_member_type::number:
         field = std::make_unique<ast::integral_member>();
         break;
      case fundamental_member_type::pointer:
         field = std::make_unique<ast::pointer_member>();
         break;
      case fundamental_member_type::string:
         field = std::make_unique<ast::string_member>();
         break;
      case fundamental_member_type::structure:
         field = std::make_unique<ast::struct_member>();
         break;
      case fundamental_member_type::union_blind:
         field = std::make_unique<ast::blind_union_member>();
         break;
      case fundamental_member_type::union_bespoke:
         field = std::make_unique<ast::inlined_union_member>();
         break;
   }

   const ast::heritable* inherit_from = nullptr;
   if (auto* attr = lu::rapidxml_helpers::get_attribute(node, "inherit")) {
      std::string value = std::string(attr->value(), attr->value_size());
      if (!this->heritables.contains(value)) {
         scaffold.error("Field: `inherit` attribute did not refer to a known heritable (seen: "s + value + ").", node);
      }
      inherit_from = this->heritables[value].get();
   }

   if (inherit_from) {
      if (auto& v = inherit_from->attributes.do_not_serialize; v.has_value())
         field->skip_when_serializing = v.value();
      if (auto& v = inherit_from->attributes.c_alignment; v.has_value())
         field->c_alignment = v.value();
      if (auto& v = inherit_from->attributes.is_const; v.has_value())
         field->is_const = v.value();
   }

   auto _handle_size_t_attribute = [this, &scaffold](xml_attribute<>& attr) -> ast::size_constant {
      std::string_view name  = std::string(attr.name(), attr.name_size());
      std::string_view value = std::string(attr.value(), attr.value_size());

      auto constant = interpret_size_constant(value);
      if (!constant.has_value()) {
         scaffold.error("Field: `"s + std::string(name) + "` attribute value is an unrecognized constant or otherwise not a valid integral(seen: "s + std::string(value) + ").", attr); // can't wait for P2591...
      }
      if (constant.value().value < 0) {
         scaffold.error("Field: `"s + std::string(name) + "` attribute value cannot be a negative number (seen: "s + std::string(value) + ").", attr); // can't wait for P2591...
      }
      if (constant.value().value == 0) {
         scaffold.error("Field: `"s + std::string(name) + "` attribute value cannot be zero (to prevent serialization, use `do-not-serialize=\"true\"` instead) (seen: "s + std::string(value) + ").", attr); // can't wait for P2591...
      }
      return constant.value();
   };

   constexpr const auto attributes_common = std::array{
      "name",
      "do-not-serialize",
      "c-alignment",
      "const"
   };

   // Attributes common to all members
   {
      if (auto* attr = lu::rapidxml_helpers::get_attribute(node, "name")) {
         std::string_view name(attr->name(), attr->name_size());
         field->name = name;
         if (name.empty())
            scaffold.error("Field has a blank name.", node);
      } else {
         scaffold.error("Field is missing a name.", node);
      }
      //
      if (auto* attr = lu::rapidxml_helpers::get_attribute(node, "do-not-serialize")) {
         auto resolved = _handle_bool_attribute(scaffold, *attr);
         if (resolved.has_value())
            field->skip_when_serializing = resolved.value();
      }
      //
      if (auto* attr = lu::rapidxml_helpers::get_attribute(node, "c-alignment")) {
         field->c_alignment = _handle_size_t_attribute(*attr);
      }
      //
      if (auto* attr = lu::rapidxml_helpers::get_attribute(node, "const")) {
         auto resolved = _handle_bool_attribute(scaffold, *attr);
         if (resolved.has_value())
            field->is_const = resolved.value();
      }
   }

   bool is_checksum = false;

   auto mt = fundamental_type.value();
   if (mt == fundamental_member_type::number) {
      auto& casted = *(ast::integral_member*)field.get();

      if (inherit_from) {
         casted.value_type = inherit_from->integral_type;
         casted.c_bitfield = inherit_from->attributes.c_bitfield;
         casted.serialization_bitcount = inherit_from->attributes.serialization_bitcount;
         casted.min = inherit_from->attributes.min;
         casted.max = inherit_from->attributes.max;
         //
         if (inherit_from->attributes.is_checksum.has_value()) {
            is_checksum = inherit_from->attributes.is_checksum.value();
         }
      }

      if (!c_type_attrval.empty()) {
         if (!attrval_integral.has_value())
            scaffold.error("Field: `c-type` attribute value is not a valid integral type (seen: "s + std::string(c_type_attrval) + ")", node);
         if (tagname_integral.has_value() && attrval_integral.has_value()) {
            if (tagname_integral.value() != attrval_integral.value()) {
               scaffold.error("Field: This numeric field specifies different types via its tagname (seen: "s + std::string(tagname) + ") and `c-type` attribute (seen: " + std::string(c_type_attrval) + ").", node);
            }
         }
      } else {
         if (!casted.value_type.has_value())
            scaffold.error("Field: This numeric member's C language value type is unknown. Specify it via the `c-type` attribute.", node);
      }

      lu::rapidxml_helpers::for_each_attribute(node, [this, &scaffold, &casted, &is_checksum, &_handle_size_t_attribute](std::string_view attr_name, std::string_view attr_value, xml_attribute<>& attr) {
         if (attr_name == "min") {
            auto value = _int_or_constant_name_to_int(attr_value);
            if (!value.has_value()) {
               scaffold.error("Field: `min` attribute value is an unrecognized constant or otherwise not a valid integral (seen: "s + std::string(attr_value) + ")", attr); // can't wait for P2591...
            }
            casted.min = value.value();
            return;
         }
         if (attr_name == "max") {
            auto value = _int_or_constant_name_to_int(attr_value);
            if (!value.has_value()) {
               scaffold.error("Field: `max` attribute value is an unrecognized constant or otherwise not a valid integral (seen: "s + std::string(attr_value) + ")", attr); // can't wait for P2591...
            }
            casted.max = value.value();
            return;
         }
         if (attr_name == "serialization-bitcount") {
            casted.serialization_bitcount = _handle_size_t_attribute(attr);
            return;
         }
         if (attr_name == "c-bitfield") {
            casted.c_bitfield = _handle_size_t_attribute(attr);
            return;
         }
         if (attr_name == "is-checksum") {
            auto resolved = _handle_bool_attribute(scaffold, attr);
            if (resolved.has_value()) {
               is_checksum = resolved.value();
            }
            return;
         }
         if (attr_name == "c-type") {
            return;
         }
         for (auto* permitted_name : attributes_common)
            if (attr_name == permitted_name)
               return;
         scaffold.warn("Field: Attribute `"s + std::string(attr_name) + "` is not relevant for numeric fields.", attr); // can't wait for P2591...
      });
   }
   if (mt == fundamental_member_type::string) {
      auto& casted = *(ast::string_member*)field.get();

      if (inherit_from) {
         if (auto& v = inherit_from->attributes.length; v.has_value())
            casted.max_length = v.value();
         if (auto& v = inherit_from->attributes.only_early_terminator; v.has_value())
            casted.only_early_terminator = v.value();
      }

      lu::rapidxml_helpers::for_each_attribute(node, [this, &scaffold, &casted, &_handle_size_t_attribute](std::string_view attr_name, std::string_view attr_value, xml_attribute<>& attr) {
         if (attr_name == "length") {
            casted.max_length = _handle_size_t_attribute(attr);
            return;
         }
         if (attr_name == "only-early-terminator") {
            auto resolved = _handle_bool_attribute(scaffold, attr);
            if (resolved.has_value()) {
               casted.only_early_terminator = resolved.value();
            }
            return;
         }
         for (auto* permitted_name : attributes_common)
            if (attr_name == permitted_name)
               return;
         scaffold.warn("Field: Attribute `"s + std::string(attr_name) + "` is not relevant for string fields.", attr); // can't wait for P2591...
      });
   }
   if (mt == fundamental_member_type::pointer) {
      auto& casted = *(ast::pointer_member*)field.get();

      std::string value_type;
      if (inherit_from)
         value_type = inherit_from->attributes.c_type;
      //
      if (value_type.empty())
         value_type = c_type_attrval;
      //
      if (value_type.empty())
         scaffold.error("Field: Pointer's `c-type` attribute is missing or empty.", node);
      else if (attrval_integral.has_value()) {
         casted.pointed_to_type = attrval_integral.value();
      } else {
         auto name_s = std::string(value_type);
         auto it_s   = this->structs.find(name_s);
         auto it_u   = this->blind_unions.find(name_s);
         if (it_s != this->structs.end()) {
            assert(it_u == this->blind_unions.end());
            casted.pointed_to_type = it_s->second.get();
            fundamental_type = fundamental_member_type::structure;
         } else if (it_u != this->blind_unions.end()) {
            assert(it_s == this->blind_unions.end());
            casted.pointed_to_type = it_u->second.get();
         }
      }

      lu::rapidxml_helpers::for_each_attribute(node, [this, &scaffold](std::string_view attr_name, std::string_view attr_value, xml_attribute<>& attr) {
         if (attr_name == "c-type")
            return;
         for (auto* permitted_name : attributes_common)
            if (attr_name == permitted_name)
               return;
         scaffold.warn("Field: Attribute `"s + std::string(attr_name) + "` is not relevant for pointer fields.", attr); // can't wait for P2591...
      });
   }
   if (mt == fundamental_member_type::structure) {
      auto& casted = *(ast::struct_member*)field.get();

      std::string type_name;
      //
      if (inherit_from) {
         type_name = inherit_from->attributes.c_type;
      }
      if (type_name.empty())
         type_name = c_type_attrval;

      if (type_name.empty())
         scaffold.error("Field: Struct-type member's `c-type` attribute is missing or empty.", node);
      assert(!attrval_integral.has_value());

      if (!this->_in_inactive_union_member(field->name)) {
         if (this->structs.contains(casted.type_name)) {
            casted.type_def = this->structs[casted.type_name].get();
         } else {
            scaffold.error("Field: struct typename is not known/defined yet (seen: "s + casted.type_name + ").", node);
         }
      }

      lu::rapidxml_helpers::for_each_attribute(node, [this, &scaffold, &casted, &_handle_size_t_attribute](std::string_view attr_name, std::string_view attr_value, xml_attribute<>& attr) {
         if (attr_name == "c-type") {
            return;
         }
         for (auto* permitted_name : attributes_common)
            if (attr_name == permitted_name)
               return;
         scaffold.warn("Field: Attribute `"s + std::string(attr_name) + "` is not relevant for struct-type fields.", attr); // can't wait for P2591...
      });
   }
   if (mt == fundamental_member_type::union_blind) {
      auto& casted = *(ast::blind_union_member*)field.get();
      
      if (c_type_attrval.empty())
         scaffold.error("Field: Union-type member's `c-type` attribute is missing or empty.", node);
      assert(!attrval_integral.has_value());

      if (!this->_in_inactive_union_member(field->name)) {
         if (this->blind_unions.contains(casted.type_name)) {
            casted.type_def = this->blind_unions[casted.type_name].get();
         } else {
            scaffold.error("Field: blind-union-type member's typename is not known/defined yet (seen: "s + casted.type_name + ").", node);
         }
      }

      lu::rapidxml_helpers::for_each_attribute(node, [this, &scaffold, &casted, &_handle_size_t_attribute](std::string_view attr_name, std::string_view attr_value, xml_attribute<>& attr) {
         if (attr_name == "c-type") {
            return;
         }
         for (auto* permitted_name : attributes_common)
            if (attr_name == permitted_name)
               return;
         scaffold.warn("Field: Attribute `"s + std::string(attr_name) + "` is not relevant for union-type fields.", attr); // can't wait for P2591...
      });
   }
   if (mt == fundamental_member_type::union_bespoke) {
      auto& casted = *(ast::inlined_union_member*)field.get();
      assert(false); // this is handled as a special case above.
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
         if (auto* attr = lu::rapidxml_helpers::get_attribute(*node, "defined-via-typedef")) {
            std::string_view attr_value(attr->value(), attr->value_size());
            if (attr_value == "true") {
               blind_union->c_type_info.is_defined_via_typedef = true;
            } else if (attr_value == "false") {
               blind_union->c_type_info.is_defined_via_typedef = false;
            } else {
               scaffold.warn("Unrecognized value for `defined-via-typedef` attribute (seen: "s + std::string(attr_value) + ")", *attr); // can't wait for P2591...
            }
         }
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

const ast::constant_definition* registry::constant_by_name(const std::string_view name) const {
   std::string name_s(name);
   auto it = this->constants.find(name_s);
   if (it == this->constants.end())
      return nullptr;
   return it->second.get();
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