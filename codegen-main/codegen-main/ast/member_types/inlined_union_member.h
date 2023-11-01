#pragma once
#include <string>
#include <vector>
#include "../member.h"

namespace ast {
   class inlined_union_member : public member {
      public:
         virtual constexpr const std::string as_c_type_specifier() const override;
         virtual constexpr std::size_t compute_single_element_bitcount() const override;
      protected:
         virtual constexpr std::vector<std::string> _get_all_used_constants_impl() const override;
         virtual constexpr size_t _get_alignment_impl() const override { return 4; }
         virtual constexpr size_t _compute_single_element_unpacked_bytecount() const override;

      public:
         std::vector<std::unique_ptr<member>> members;
         std::string member_to_serialize;

         constexpr member& get_member_to_serialize() const;

         constexpr std::vector<std::string> get_all_direct_constant_dependencies() const;
         constexpr std::vector<std::string> get_all_direct_struct_dependencies() const;
         constexpr bool has_any_string_members() const;
   };
}

#include "./inlined_union_member.inl"