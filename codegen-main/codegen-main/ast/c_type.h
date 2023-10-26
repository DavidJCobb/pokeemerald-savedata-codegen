#pragma once
#include <optional>
#include <string_view>
#include <vector>
#include "./integral_constant.h"

namespace ast {
   struct c_type {
      public:
         enum class primitive {
            u8,
            u16,
            u32,
            s8,
            s16,
            s32,
            bool8,
         };
         static constexpr size_t bitcount_of(primitive);
         static constexpr std::string_view primitive_to_string(primitive);
         static constexpr std::optional<primitive> primitive_from_string(std::string_view);

         struct array_rank {
            size_t extent;
            std::string extent_expr;
         };
   
      public:
         bool is_const  = false;
         bool is_struct = false;
         bool is_union  = false;
         std::optional<size_t> explicit_alignment;
         std::optional<size_t> bitfield_size;
         std::variant<std::monostate, primitive, std::string> name;
         std::vector<array_rank> array_extents;

         constexpr void set_type_name(std::string_view);
   
         constexpr const std::string as_c_member_prefix() const;
         constexpr const std::string as_c_member_postfix() const;
   };
}

#include "./c_type.inl"