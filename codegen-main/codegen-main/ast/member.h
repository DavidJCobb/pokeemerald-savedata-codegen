#pragma once
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "./integral_constant.h"
#include "./integral_type.h"

namespace ast {
   class heritable;
}

namespace ast {
   class member {
      public:
         struct array_rank {
            size_t extent;
            std::string extent_expr;
         };

      public:
         virtual ~member() {}
         virtual constexpr const std::string as_c_type_specifier() const = 0;
         virtual constexpr const std::string as_c_array_declarator_extents() const;
         virtual constexpr const std::string as_c_bitfield_specifier() const { return {}; }
         virtual std::size_t compute_serialization_bitcount() const = 0; // computes the bitcount of a single item, i.e. not the whole array, and not the whole string

         constexpr const std::string as_c_declaration() const;

      public:
         std::string name;

         std::vector<array_rank> array_extents;
         bool is_const = false;
         bool skip_when_serializing = false;

         std::optional<size_t> c_alignment;
         std::string c_line_comment;
   };
   class integral_member : public member {
      public:
         virtual constexpr const std::string as_c_type_specifier() const override;
         virtual constexpr const std::string as_c_bitfield_specifier() const override;
         virtual std::size_t compute_serialization_bitcount() const override;

      public:
         std::optional<integral_type> value_type;
         std::optional<std::intmax_t> min;
         std::optional<std::intmax_t> max;
         std::optional<std::size_t>   c_bitfield;
         std::optional<std::size_t>   serialization_bitcount;
   };
   class string_member : public member {
      public:
         virtual constexpr const std::string as_c_type_specifier() const override;
         virtual constexpr const std::string as_c_array_declarator_extents() const override;
         virtual std::size_t compute_serialization_bitcount() const override;

      public:
         std::optional<integral_type> char_type;
         std::size_t max_length = 0;
   };
   class struct_member : public member {
      public:
         enum class decl {
            c_struct,
            c_union,
         };
         
      public:
         virtual constexpr const std::string as_c_type_specifier() const override;
         virtual std::size_t compute_serialization_bitcount() const override;

      public:
         std::string type_name;
         std::optional<decl> decl; // defaults to c_struct
   };
   class inlined_union_member : public member {
      public:
         virtual constexpr const std::string as_c_type_specifier() const override;
         virtual std::size_t compute_serialization_bitcount() const override;

      public:
         std::vector<std::unique_ptr<member>> members;
         std::string member_to_serialize;

         constexpr member& get_member_to_serialize() const;

         constexpr std::vector<std::string> get_all_direct_struct_dependencies() const;
         constexpr bool has_any_string_members() const;
   };
}

#include "./member.inl"