#pragma once
#include <optional>
#include "./c_type.h"

namespace ast::semantic {
   struct number_info {
      std::optional<intmax_t> min;
      std::optional<intmax_t> max;
   };

   struct string_info {
      std::optional<c_type::primitive> char_type; // defaults to u8 if unspecified
      size_t max_length = 0; // NOT including terminator
   };
}