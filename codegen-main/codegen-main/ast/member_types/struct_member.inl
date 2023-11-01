#pragma once
#include "./struct_member.h"
#include "../structure.h"

namespace ast {
   /*virtual*/ constexpr const std::string struct_member::as_c_type_specifier() const /*override*/ {
      std::string out;
      switch (this->decl.value_or(decl::c_struct)) {
         case decl::c_struct:
            out = "struct ";
            break;
         case decl::c_union:
            out = "union ";
            break;
         case decl::blank:
            break;
      }
      out += this->type_name;
      return out;
   }
   /*virtual*/ constexpr std::vector<std::string> struct_member::_get_all_used_constants_impl() const /*override*/ {
      return {};
   }
   /*virtual*/ constexpr size_t struct_member::_get_alignment_impl() const /*override*/ {
      if (this->type_def->c_type_info.is_packed)
         return 1;
      return 4; // default struct align
   }
   /*virtual*/ constexpr size_t struct_member::_compute_single_element_unpacked_bytecount() const /*override*/ {
      return this->type_def->compute_unpacked_bytecount();
   }
   /*virtual*/ constexpr std::size_t struct_member::compute_single_element_bitcount() const /*override*/ {
      if (std::is_constant_evaluated()) {
         if (!this->type_def)
            throw;
      }
      return this->type_def->compute_total_bitcount();
   }
}
