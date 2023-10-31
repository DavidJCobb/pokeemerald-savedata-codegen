#pragma once
#include <optional>
#include "../member.h"

namespace ast {
   class string_member : public member {
      public:
         virtual constexpr const std::string as_c_type_specifier() const override;
         virtual constexpr const std::string as_c_array_declarator_extents() const override;
         virtual constexpr std::size_t compute_single_element_bitcount() const override;
      protected:
         virtual constexpr std::vector<std::string> _get_all_used_constants_impl() const override;

      public:
         std::optional<integral_type> char_type;
         size_constant max_length;
   };
}

#include "./string_member.inl"