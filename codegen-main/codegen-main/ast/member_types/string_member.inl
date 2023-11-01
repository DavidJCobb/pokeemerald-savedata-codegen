#pragma once
#include "./string_member.h"
#include <bit>

namespace ast{
   /*virtual*/ constexpr const std::string string_member::as_c_type_specifier() const /*override*/ {
      return std::string(integral_type_to_string(this->char_type.value()));
   }
   /*virtual*/ constexpr const std::string string_member::as_c_array_declarator_extents() const /*override*/ {
      std::string out = member::as_c_array_declarator_extents();
      out += '[';
      {
         auto& ml = this->max_length;
         if (ml.preprocessor_name.empty()) {
            out += lu::strings::from_integer(ml.value + 1); // include `EOS` terminator byte
         } else {
            out += ml.preprocessor_name;
            out += " + 1";
         }
      }
      out += ']';
      return out;
   }
   /*virtual*/ constexpr std::vector<std::string> string_member::_get_all_used_constants_impl() const /*override*/ {
      std::vector<std::string> out;

      if (!max_length.preprocessor_name.empty())
         out.push_back(max_length.preprocessor_name);

      return out;
   }
   /*virtual*/ constexpr size_t string_member::_get_alignment_impl() const /*override*/ {
      auto bc = bitcount_of(this->char_type.value());
      if (bc < 8)
         return 8;
      return bc / 8 + ((bc % 8) ? 1 : 0);
   }
   /*virtual*/ constexpr size_t string_member::_compute_single_element_unpacked_bytecount() const /*override*/ {
      auto bc = bitcount_of(this->char_type.value());
      if (bc < 8)
         bc = 8;

      size_t buffer = this->max_length.value;
      if (!this->only_early_terminator)
         buffer += 1;

      return (bc / 8 + ((bc % 8) ? 1 : 0)) * buffer;
   }
   /*virtual*/ constexpr std::size_t string_member::compute_single_element_bitcount() const /*override*/ {
      if (std::is_constant_evaluated()) {
         if (!this->char_type.has_value())
            throw;
      }
      return ast::bitcount_of(this->char_type.value()) * this->max_length.value + std::bit_width(this->max_length.value);
   }
}