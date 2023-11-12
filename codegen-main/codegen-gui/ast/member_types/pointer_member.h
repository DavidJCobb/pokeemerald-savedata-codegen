#pragma once
#include <cstdint>
#include <variant>
#include "../member.h"

namespace ast {
   class blind_union;
   class structure;
}

namespace ast {
   // Savedata may include pointers within ROM.
   class pointer_member : public member {
      public:
         virtual constexpr const std::string as_c_type_specifier() const override;
         virtual constexpr std::size_t compute_single_element_bitcount() const override;
      protected:
         virtual constexpr std::vector<std::string> _get_all_used_constants_impl() const override { return {}; }
         virtual constexpr size_t _get_alignment_impl() const override { return 4; }
         virtual constexpr size_t _compute_single_element_unpacked_bytecount() const override { return 4; }

      public:
         std::variant<
            std::monostate,
            integral_type,
            const ast::structure*,
            const ast::blind_union*
         > pointed_to_type; // integral type or struct/union typename

         constexpr std::string pointed_to_type_to_string() const;
   };
}

#include "./pointer_member.inl"