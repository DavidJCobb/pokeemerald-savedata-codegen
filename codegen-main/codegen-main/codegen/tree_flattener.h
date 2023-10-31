#pragma once
#include <memory>
#include <string>
#include <vector>
#include "../ast/integral_constant.h"

namespace ast {
   class structure;
   class member;
}

namespace codegen {

   // Represents a single item to serialize, i.e. a non-array member or a single element in the deepest rank of a member array
   struct serialization_item {
      const ast::structure* struct_definition = nullptr; // containing struct
      const ast::member*    member_definition = nullptr; // this member

      std::string         accessor;      // field to access, as a C expression but not including array ranks
      std::vector<size_t> array_indices; // e.g. 3,4,5 if this is a call to serialize p_Struct.foo[3][4][5]

      serialization_item* parent = nullptr;
      std::vector<std::unique_ptr<serialization_item>> children;

      // Given a "template" serialization item for a non-array member, this adds a copy of the template to `dst`.
      // 
      // Given a "template" serialization item for an array member, this adds an instance of the template to `dst` 
      // for every array element; for example, an array like my_values[3][2] would add six serialization items to 
      // the `dst` array, with indices filled in for each.
      //
      static constexpr void _generate_array_ranks(const serialization_item& item_template, std::vector<std::unique_ptr<serialization_item>>& dst);

      constexpr void expand();

      static constexpr std::vector<std::unique_ptr<serialization_item>> for_top_level_struct(const ast::structure&);
   };


   // Flattens arrays; does not flatten nested structs.
   class tree_flattener {
      public:
         struct single_call {
            const ast::member* member_definition = nullptr; // used to pull constant names for array extents
            size_t bitcount_per_element = 0;

            std::string target; // field to access, as a C expression but not including array ranks
            std::vector<size_t> array_extents;
            struct {
               std::string read;
               std::string write;
            } function_names;
            bool reading_uses_assign = false;
         };

         std::vector<single_call> calls;

         constexpr void _walk_struct(std::string accessor, const ast::structure&);
      public:
         constexpr void flatten_top_level_struct(std::string top_level_struct_typename, const ast::structure&);
   };
}

#include "./tree_flattener.inl"