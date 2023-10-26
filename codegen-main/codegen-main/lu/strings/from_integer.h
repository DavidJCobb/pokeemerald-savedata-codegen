// SPDX-License-Identifier: CC0-1.0

#pragma once
#include <string>
#include <type_traits>

namespace lu::strings {
   template<typename Integer>
   constexpr const std::string from_integer(Integer i) {
      if (i == 0) {
         return "0";
      }

      std::string out;

      bool is_signed = false;
      if constexpr (std::is_signed_v<Integer>)
         is_signed = (i < 0);

      while (i > 0) {
         int digit = i % 10;
         if constexpr (std::is_signed_v<Integer>) {
            //
            // Negating the whole number ahead of type could lead to overflow, e.g. -(-128) for an int8_t.
            //
            if (digit < 0)
               digit = -digit;
         }
         out += ('0' + digit);

         i /= 10;
      }
      if constexpr (std::is_signed_v<Integer>) {
         if (is_signed) {
            out += '-';
         }
      }
      if (out.size() > 1)
         std::reverse(out.begin(), out.end());

      return out;
   }
}