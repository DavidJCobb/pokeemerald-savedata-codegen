#pragma once
#include "./pointer_member.h"

namespace ast {
   /*virtual*/ constexpr const std::string pointer_member::as_c_type_specifier() const /*override*/ {
      return std::string(integral_type_to_string(this->pointed_to_type.value())) + "*";
   }
   /*virtual*/ constexpr std::size_t pointer_member::compute_single_element_bitcount() const /*override*/ {
      return 32;
   }
}
