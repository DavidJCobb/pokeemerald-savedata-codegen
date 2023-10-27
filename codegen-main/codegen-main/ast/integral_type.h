#pragma once
#include <cstdint>
#include <optional>
#include <string_view>

namespace ast {
   enum class integral_type {
      u8,
      u16,
      u32,
      s8,
      s16,
      s32,
      bool8,
   };

   static constexpr std::size_t bitcount_of(integral_type);
   static constexpr std::intmax_t  minimum_of(integral_type);
   static constexpr std::uintmax_t maximum_of(integral_type);
   static constexpr std::string_view integral_type_to_string(integral_type);
   static constexpr std::optional<integral_type> integral_type_from_string(std::string_view);
}

#include "./integral_type.inl"