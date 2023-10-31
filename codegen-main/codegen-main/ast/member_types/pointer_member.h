#pragma once
#include <cstdint>
#include <optional>
#include "../member.h"

namespace ast {
   // Savedata may include pointers within ROM.
   class pointer_member : public member {
      public:
         virtual constexpr const std::string as_c_type_specifier() const override;
         virtual constexpr std::size_t compute_single_element_bitcount() const override;
      protected:
         virtual constexpr std::vector<std::string> _get_all_used_constants_impl() const override { return {}; }

      public:
         std::optional<integral_type> pointed_to_type;
   };
}

#include "./pointer_member.inl"