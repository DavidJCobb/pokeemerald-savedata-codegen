#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include "../member.h"

namespace ast {
   class integral_member : public member {
      public:
         virtual constexpr const std::string as_c_type_specifier() const override;
         virtual constexpr const std::string as_c_bitfield_specifier() const override;
         virtual constexpr std::size_t compute_single_element_bitcount() const override;
      protected:
         virtual constexpr std::vector<std::string> _get_all_used_constants_impl() const override;
         virtual constexpr size_t _get_alignment_impl() const override;
         virtual constexpr size_t _compute_single_element_unpacked_bytecount() const override;

      public:
         std::optional<integral_type> value_type;
         std::optional<std::intmax_t> min;
         std::optional<std::intmax_t> max;
         std::optional<size_constant> c_bitfield;
         std::optional<size_constant> serialization_bitcount;

         constexpr bool is_signed() const; // requires `value_type` to be filled
   };
}

#include "./integral_member.inl"