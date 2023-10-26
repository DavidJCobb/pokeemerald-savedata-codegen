// SPDX-License-Identifier: CC0-1.0

#pragma once
#include <type_traits>
#include "./char_to_integer.h"

namespace lu::strings {
   template<typename Integer, size_t Base = 10> requires (std::is_arithmetic_v<Integer> && (Base <= 36))
   constexpr Integer to_integer(const char* str, bool* ok) {
      Integer out = {};
      bool    sgn = false;
      if constexpr (std::is_signed_v<Integer>) {
         if (*str == '-') {
            sgn = true;
            ++str;
         }
      }
      for (; *str; ++str) {
         char c = *str;

         bool c_ok;
         auto digit = char_to_integer(c, c_ok);
         if (!c_ok) {
            if (ok)
               *ok = false;
            return {};
         }
         out *= Base;
         out += digit;
      }
      if constexpr (std::is_signed_v<Integer>) {
         if (sgn)
            out *= -1;
      }
      if (ok)
         *ok = true;
      return out;
   }
   
   template<typename Integer, size_t Base = 10> requires (std::is_arithmetic_v<Integer> && (Base <= 36))
   constexpr Integer to_integer(const char* str, size_t len, bool* ok) {
      Integer out = {};
      bool    sgn = false;
      size_t  i   = 0;
      if constexpr (std::is_signed_v<Integer>) {
         if (len && str[0] == '-') {
            sgn = true;
            ++i;
         }
      }
      for (; i < len; ++i) {
         char c = str[i];
         
         bool c_ok;
         auto digit = char_to_integer(c, c_ok);
         if (!c_ok) {
            if (ok)
               *ok = false;
            return {};
         }
         out *= Base;
         out += digit;
      }
      if constexpr (std::is_signed_v<Integer>) {
         if (sgn)
            out *= -1;
      }
      if (ok)
         *ok = true;
      return out;
   }
}