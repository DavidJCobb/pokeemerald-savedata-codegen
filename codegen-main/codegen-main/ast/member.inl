#pragma once
#include "./member.h"
#include "lu/strings/from_integer.h"
#include "./heritable.h"

namespace ast {
   /*virtual*/ constexpr const std::string member::as_c_array_declarator_extents() const {
      if (this->array_extents.empty())
         return {};

      std::string out;
      for (const auto& rank : this->array_extents) {
         out += '[';
         if (!rank.extent_expr.empty()) {
            out += rank.extent_expr;
         } else {
            out += lu::strings::from_integer(rank.extent);
         }
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
         out += lu::strings::from_integer(this->c_alignment.value());
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

   /*virtual*/ constexpr const std::string string_member::as_c_array_declarator_extents() const /*override*/ {
      std::string out = member::as_c_array_declarator_extents();
      out += '[';
      out += lu::strings::from_integer(this->max_length + 1); // include `EOS` terminator byte
      out += ']';
      return out;
   }

   /*virtual*/ constexpr const std::string integral_member::as_c_type_specifier() const /*override*/ {
      return std::string(integral_type_to_string(this->value_type.value()));
   }
   /*virtual*/ constexpr const std::string integral_member::as_c_bitfield_specifier() const /*override*/ {
      if (this->c_bitfield.has_value()) {
         std::string out = " : ";
         out += lu::strings::from_integer(this->c_bitfield.value());
         return out;
      }
      return {};
   }

   /*virtual*/ constexpr const std::string string_member::as_c_type_specifier() const /*override*/ {
      return std::string(integral_type_to_string(this->char_type.value()));
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
