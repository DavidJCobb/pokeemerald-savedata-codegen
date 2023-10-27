#pragma once
#include "./integral_type.h"

namespace ast {
   /*static*/ constexpr std::size_t bitcount_of(integral_type t) {
      switch (t) {
         case integral_type::bool8:
            return 1;
         case integral_type::u8:
         case integral_type::s8:
            return 8;
         case integral_type::u16:
         case integral_type::s16:
            return 16;
         case integral_type::u32:
         case integral_type::s32:
            return 32;
      }
   }
   /*static*/ constexpr std::intmax_t minimum_of(integral_type v) {
      switch (v) {
         case integral_type::bool8:
         case integral_type::u8:
         case integral_type::u16:
         case integral_type::u32:
            return 0;
      }
      switch (v) {
         case integral_type::s8:
         case integral_type::s16:
         case integral_type::s32:
            return -(std::intmax_t{ 1 } << (bitcount_of(v) - 1));
      }
      return 0;
   }
   /*static*/ constexpr std::uintmax_t maximum_of(integral_type v) {
      auto bc  = bitcount_of(v) - 1;
      auto max = std::uintmax_t{ 1 } << bc;
      switch (v) {
         case integral_type::s8:
         case integral_type::s16:
         case integral_type::s32:
            --max;
            break;
      }
      return max;
   }
   /*static*/ constexpr std::string_view integral_type_to_string(integral_type v) {
      switch (v) {
         case integral_type::u8:    return "u8";  break;
         case integral_type::u16:   return "u16"; break;
         case integral_type::u32:   return "u32"; break;
         case integral_type::s8:    return "s8";  break;
         case integral_type::s16:   return "s16"; break;
         case integral_type::s32:   return "s32"; break;
         case integral_type::bool8: return "bool8"; break;
      }
      return {};
   }
   /*static*/ constexpr std::optional<integral_type> integral_type_from_string(std::string_view v) {
      if (v == "u8")
         return integral_type::u8;
      if (v == "u16")
         return integral_type::u16;
      if (v == "u32")
         return integral_type::u32;
      if (v == "s8")
         return integral_type::s8;
      if (v == "s16")
         return integral_type::s16;
      if (v == "s32")
         return integral_type::s32;
      if (v == "bool8")
         return integral_type::bool8;
      return {};
   }
}
