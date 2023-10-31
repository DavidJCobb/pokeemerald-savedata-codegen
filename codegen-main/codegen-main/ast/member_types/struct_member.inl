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
      }
      out += this->type_name;
      return out;
   }
   /*virtual*/ constexpr std::vector<std::string> struct_member::_get_all_used_constants_impl() const /*override*/ {
      return {};
   }
   /*virtual*/ constexpr std::size_t struct_member::compute_single_element_bitcount() const /*override*/ {
      if (std::is_constant_evaluated()) {
         if (!this->type_def)
            throw;
      }

      size_t bitcount = 0;
      for (const auto& member_ptr : this->type_def->members) {
         bitcount += member_ptr->compute_total_bitcount();
      }
      return bitcount;
   }
}
