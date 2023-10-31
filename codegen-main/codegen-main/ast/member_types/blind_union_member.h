#pragma once
#include <optional>
#include <string>
#include "../member.h"

namespace ast {
   class blind_union;
}

namespace ast {
   class blind_union_member : public member {
      public:
         enum class decl {
            c_union,
            blank,
         };
         
      public:
         virtual constexpr const std::string as_c_type_specifier() const override;
         virtual constexpr std::size_t compute_single_element_bitcount() const override;
      protected:
         virtual constexpr std::vector<std::string> _get_all_used_constants_impl() const override { return {}; }

      public:
         std::string        type_name;
         const blind_union* type_def = nullptr;
         //
         std::optional<decl> decl; // defaults to blank
   };
}

#include "./blind_union_member.inl"