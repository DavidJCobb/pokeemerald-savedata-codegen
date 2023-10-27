#pragma once
#include "./member.h"
#include <algorithm>
#include "lu/strings/from_integer.h"
#include "./heritable.h"

namespace ast {
   /*virtual*/ constexpr const std::string member::as_c_array_declarator_extents() const {
      if (this->array_extents.empty())
         return {};

      std::string out;
      for (const auto& rank : this->array_extents) {
         out += '[';
         out += rank.as_c_expression();
         out += ']';
      }
      return out;
   }
   constexpr const std::string member::as_c_declaration() const {
      std::string out;
      if (is_const)
         out += "const ";

      out += this->as_c_type_specifier();
      out += ' ';
      if (this->c_alignment.has_value()) {
         out += "ALIGNED(";
         out += this->c_alignment.value().as_c_expression();
         out += ") ";
      }
      out += this->name;
      out += this->as_c_array_declarator_extents();
      out += this->as_c_bitfield_specifier();
      out += ';';

      if (!this->c_line_comment.empty()) {
         out += " // ";
         out += this->c_line_comment;
      }

      return out;
   }
   constexpr std::vector<std::string> member::get_all_used_constants() const {
      std::vector<std::string> out = this->_get_all_used_constants_impl();

      for (const auto& rank : this->array_extents)
         if (!rank.extent.preprocessor_name.empty())
            out.push_back(rank.extent.preprocessor_name);

      if (this->c_alignment.has_value()) {
         auto& v = this->c_alignment.value();
         if (!v.preprocessor_name.empty())
            out.push_back(v.preprocessor_name);
      }

      auto it = std::unique(out.begin(), out.end());
      out.erase(it, out.end());
      return out;
   }

   /*virtual*/ constexpr const std::string integral_member::as_c_type_specifier() const /*override*/ {
      return std::string(integral_type_to_string(this->value_type.value()));
   }
   /*virtual*/ constexpr const std::string integral_member::as_c_bitfield_specifier() const /*override*/ {
      if (this->c_bitfield.has_value()) {
         std::string out = " : ";
         out += this->c_bitfield.value().as_c_expression();
         return out;
      }
      return {};
   }
   /*virtual*/ constexpr std::vector<std::string> integral_member::_get_all_used_constants_impl() const /*override*/ {
      std::vector<std::string> out;

      if (c_bitfield.has_value())
         if (!c_bitfield.value().preprocessor_name.empty())
            out.push_back(c_bitfield.value().preprocessor_name);

      if (serialization_bitcount.has_value())
         if (!serialization_bitcount.value().preprocessor_name.empty())
            out.push_back(serialization_bitcount.value().preprocessor_name);

      return out;
   }
   
   /*virtual*/ constexpr const std::string string_member::as_c_type_specifier() const /*override*/ {
      return std::string(integral_type_to_string(this->char_type.value()));
   }
   /*virtual*/ constexpr const std::string string_member::as_c_array_declarator_extents() const /*override*/ {
      std::string out = member::as_c_array_declarator_extents();
      out += '[';
      {
         auto& ml = this->max_length;
         if (ml.preprocessor_name.empty()) {
            out += lu::strings::from_integer(ml.value + 1); // include `EOS` terminator byte
         } else {
            out += ml.preprocessor_name;
            out += " + 1";
         }
      }
      out += ']';
      return out;
   }
   /*virtual*/ constexpr std::vector<std::string> string_member::_get_all_used_constants_impl() const /*override*/ {
      std::vector<std::string> out;

      if (!max_length.preprocessor_name.empty())
         out.push_back(max_length.preprocessor_name);

      return out;
   }

   /*virtual*/ constexpr const std::string struct_member::as_c_type_specifier() const /*override*/ {
      std::string out;
      switch (this->decl.value_or(decl::c_struct)) {
         case decl::c_struct:
            out = "struct ";
            break;
         case decl::c_union:
            out = "union ";
            break;
      }
      out += this->type_name;
      return out;
   }
   /*virtual*/ constexpr std::vector<std::string> struct_member::_get_all_used_constants_impl() const /*override*/ {
      return {};
   }

   /*virtual*/ constexpr const std::string inlined_union_member::as_c_type_specifier() const /*override*/ {
      std::string out;
      out = "union {\n";
      for (const auto& nested : this->members) {
         out += "   ";
         out += nested->as_c_declaration();
         out += "\n";
      }
      out += "}";
      return out;
   }
   /*virtual*/ constexpr std::vector<std::string> inlined_union_member::_get_all_used_constants_impl() const /*override*/ {
      std::vector<std::string> out;

      for (const auto& member : this->members) {
         auto list = member->get_all_used_constants();
         for (auto& item : list)
            out.push_back(std::move(item));
      }

      return out;
   }
   

   constexpr member& inlined_union_member::get_member_to_serialize() const {
      for (const auto& member : members) {
         if (member->name == this->member_to_serialize)
            return *member.get();
      }
      throw;
   }

   constexpr std::vector<std::string> inlined_union_member::get_all_direct_struct_dependencies() const {
      std::vector<std::string> out;
      for (const auto& member : members) {
         if (auto* casted = dynamic_cast<const struct_member*>(member.get())) {
            out.push_back(casted->type_name);
            continue;
         }
         if (auto* casted = dynamic_cast<const inlined_union_member*>(member.get())) {
            auto list = casted->get_all_direct_struct_dependencies();
            for (const auto& item : list) {
               {
                  bool already = false;
                  for (auto& prior : out) {
                     if (prior == item) {
                        already = true;
                        break;
                     }
                  }
                  if (already)
                     continue;
               }
               out.push_back(item);
            }
         }
      }
      return out;
   }
   constexpr bool inlined_union_member::has_any_string_members() const {
      for (const auto& member : members) {
         if (dynamic_cast<const string_member*>(member.get()))
            return true;
         if (auto* casted = dynamic_cast<const inlined_union_member*>(member.get()))
            if (casted->has_any_string_members())
               return true;
      }
      return false;
   }
}
