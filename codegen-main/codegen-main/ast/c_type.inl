#pragma once
#include "./c_type.h"
#include "lu/strings/from_integer.h"

namespace ast {
   /*static*/ constexpr size_t c_type::bitcount_of(primitive t) {
      switch (t) {
         case primitive::bool8:
            return 1;
         case primitive::u8:
         case primitive::s8:
            return 8;
         case primitive::u16:
         case primitive::s16:
            return 16;
         case primitive::u32:
         case primitive::s32:
            return 32;
      }
   }
   /*static*/ constexpr std::string_view c_type::primitive_to_string(primitive v) {
      switch (v) {
         case primitive::u8:    return "u8";  break;
         case primitive::u16:   return "u16"; break;
         case primitive::u32:   return "u32"; break;
         case primitive::s8:    return "s8";  break;
         case primitive::s16:   return "s16"; break;
         case primitive::s32:   return "s32"; break;
         case primitive::bool8: return "bool8"; break;
      }
      return {};
   }
   /*static*/ constexpr std::optional<c_type::primitive> c_type::primitive_from_string(std::string_view v) {
      if (v == "u8")
         return primitive::u8;
      if (v == "u16")
         return primitive::u16;
      if (v == "u32")
         return primitive::u32;
      if (v == "s8")
         return primitive::s8;
      if (v == "s16")
         return primitive::s16;
      if (v == "s32")
         return primitive::s32;
      if (v == "bool8")
         return primitive::bool8;
      return {};
   }

   constexpr void c_type::set_type_name(std::string_view v) {
      auto pn = primitive_from_string(v);
      if (pn.has_value()) {
         this->name = pn.value();
         return;
      }
      this->name = std::string(v);
   }

   constexpr const std::string c_type::as_c_member_prefix() const {
      if (std::holds_alternative<std::monostate>(name)) {
         throw;
      }
      
      std::string out;
      
      if (is_const)
         out = "const ";
      
      if (is_struct)
         out += "struct ";
      else if (is_union)
         out += "union ";
      
      if (std::holds_alternative<primitive>(name)) {
         out += primitive_to_string(std::get<primitive>(name));
      } else if (std::holds_alternative<std::string>(name)) {
         out += std::get<std::string>(name);
      }
      
      if (explicit_alignment.has_value()) {
         out += " ALIGNED(";
         out += lu::strings::from_integer(explicit_alignment.value());
         out += ')';
      }
      
      return out;
   }
   
   constexpr const std::string c_type::as_c_member_postfix() const {
      std::string out;
      
      if (!array_extents.empty()) {
         for(const auto& rank : array_extents) {
            out += '[';
            if (rank.extent_expr.empty())
               out += lu::strings::from_integer(rank.extent);
            else
               out += rank.extent_expr;
            out += ']';
         }
      }
      
      if (bitfield_size.has_value()) {
         out += " : ";
         out += lu::strings::from_integer(bitfield_size.value());
      }
      
      out += ';';
      return out;
   }
}