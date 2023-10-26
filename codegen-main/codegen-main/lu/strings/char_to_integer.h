// SPDX-License-Identifier: CC0-1.0

#pragma once
#include <type_traits>

namespace lu::strings {
   template<typename Integer = int, size_t Base = 10> requires (std::is_arithmetic_v<Integer> && (Base <= 36))
   constexpr Integer char_to_integer(char c, bool& ok) {
      Integer digit = c - '0';
      if constexpr (Base <= 10) {
         if (digit >= Base) {
            ok = false;
            return {};
         }
      }
      if (digit < Base) {
         return digit;
      }
      if constexpr (Base > 10) {
         //
         // For bases above 10, move on to using letters.
         //
         if (c >= 'a' && c <= 'z')
            c -= 0x20;
         digit  = c - 'a';
         if (digit < 26) {
            digit += 10;
            if (digit >= Base) {
               ok = false;
               return {};
            }
            return digit;
         }
         //
         // Not a letter. Fall through.
         //
      }
      //
      // Unrecognized character:
      //
      ok = false;
      return {};
   }
}