#pragma once
#include "./pointer_member.h"

namespace ast {
   /*virtual*/ constexpr const std::string pointer_member::as_c_type_specifier() const /*override*/ {
      std::string out;
      if (auto* casted = std::get_if<const ast::structure*>(&this->pointed_to_type)) {
         auto& dfn = **casted;
         if (!dfn.c_type_info.is_defined_via_typedef)
            out = "struct ";
         out += dfn.name;
      } else if (auto* casted = std::get_if<const ast::blind_union*>(&this->pointed_to_type)) {
         auto& dfn = **casted;
         if (!dfn.c_type_info.is_defined_via_typedef)
            out = "struct ";
         out += dfn.name;
      } else if (auto* casted = std::get_if<integral_type>(&this->pointed_to_type)) {
         out = std::string(integral_type_to_string(*casted));
      } else {
         out = "?";
      }
      return out + "*";
   }
   /*virtual*/ constexpr std::size_t pointer_member::compute_single_element_bitcount() const /*override*/ {
      return 32;
   }
}
