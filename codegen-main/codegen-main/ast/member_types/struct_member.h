#pragma once
#include <optional>
#include <string>
#include "../member.h"

namespace ast {
   class structure;
}

namespace ast {
   class struct_member : public member {
      public:
         enum class decl {
            c_struct,
            c_union,
            blank,
         };
         
      public:
         virtual constexpr const std::string as_c_type_specifier() const override;
         virtual constexpr std::size_t compute_single_element_bitcount() const override;
      protected:
         virtual constexpr std::vector<std::string> _get_all_used_constants_impl() const override;

      public:
         std::string      type_name;
         const structure* type_def = nullptr;
         //
         std::optional<decl> decl; // defaults to c_struct
   };
}

#include "./struct_member.inl"