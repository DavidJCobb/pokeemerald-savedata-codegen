#pragma once
#include "./blind_union_member.h"
#include "../blind_union.h"

namespace ast {
   /*virtual*/ constexpr const std::string blind_union_member::as_c_type_specifier() const /*override*/ {
      std::string out;
      if (!this->type_def->c_type_info.is_defined_via_typedef) {
         out = "union ";
      }
      out += this->type_name;
      return out;
   }
   /*virtual*/ constexpr std::size_t blind_union_member::compute_single_element_bitcount() const /*override*/ {
      if (std::is_constant_evaluated()) {
         if (!this->type_def)
            throw;
      }
      return this->type_def->size_in_bytes * 8;
   }
   /*virtual*/ constexpr size_t blind_union_member::_compute_single_element_unpacked_bytecount() const /*override*/ {
      return this->type_def->size_in_bytes;
   }
}
