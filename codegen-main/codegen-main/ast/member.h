#pragma once
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "./integral_constant.h"
#include "./integral_type.h"

namespace ast {
   class heritable;
   class structure;
}

namespace ast {
   class member {
      public:
         struct array_rank {
            size_constant extent;
            std::string   extent_expr;

            constexpr std::string as_c_expression() const {
               if (!extent_expr.empty())
                  return extent_expr;
               return extent.as_c_expression();
            }
         };

      public:
         virtual ~member() {}
         virtual constexpr const std::string as_c_type_specifier() const = 0;
         virtual constexpr const std::string as_c_array_declarator_extents() const;
         virtual constexpr const std::string as_c_bitfield_specifier() const { return {}; }
         virtual constexpr std::size_t compute_single_element_bitcount() const = 0; // computes the bitcount of a single item, i.e. don't account for array ranks
      protected:
         virtual constexpr std::vector<std::string> _get_all_used_constants_impl() const = 0;

      public:
         constexpr const std::string as_c_declaration() const;
         constexpr std::vector<std::string> get_all_used_constants() const;

         constexpr size_t compute_total_bitcount() const;

      public:
         std::string name;

         std::vector<array_rank> array_extents;
         bool is_const = false;
         bool skip_when_serializing = false;

         std::optional<size_constant> c_alignment;
         std::string c_line_comment;
   };

   class integral_member;

   class string_member;

   class struct_member;

   class inlined_union_member;
}

#include "./member.inl"