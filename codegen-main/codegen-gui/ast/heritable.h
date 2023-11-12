#pragma once
#include <variant>
#include "./integral_constant.h"
#include "./integral_type.h"

namespace ast {
   class heritable {
      public:
         enum class fundamental_type {
            integral,
            string,
            structure,
         };

      public:
         std::string name;
         
         std::string tagname;
         struct {
            std::optional<bool> do_not_serialize;
            std::optional<bool> is_const;
            std::optional<size_constant> c_alignment;

            std::string c_type;

            // attributes for numbers
            std::optional<std::intmax_t>  min;
            std::optional<std::uintmax_t> max;
            std::optional<size_constant>  c_bitfield;
            std::optional<size_constant>  serialization_bitcount;
            std::optional<bool> is_checksum;

            // attributes for strings
            std::optional<size_constant> length;
            std::optional<bool> only_early_terminator;
         } attributes;
         std::optional<integral_type> integral_type;

         constexpr bool is_integral() const;
         constexpr bool is_string() const;
   };
}

#include "./heritable.inl"