#pragma once
#include <optional>
#include "../member.h"

namespace ast {
   class string_member : public member {
      public:
         static constexpr const integral_type char_type = integral_type::u8;

      public:
         virtual constexpr const std::string as_c_type_specifier() const override;
         virtual constexpr const std::string as_c_array_declarator_extents() const override;
         virtual constexpr std::size_t compute_single_element_bitcount() const override;
      protected:
         virtual constexpr std::vector<std::string> _get_all_used_constants_impl() const override;
         virtual constexpr size_t _get_alignment_impl() const override;
         virtual constexpr size_t _compute_single_element_unpacked_bytecount() const override;

      public:
         size_constant max_length;
         bool only_early_terminator = false; // if true, string is `max_length` bytes, not `max_length + 1` bytes
   };
}

#include "./string_member.inl"