#pragma once
#include <memory>
#include <string>
#include <vector>

namespace ast {
   class structure;
   class member;
}

namespace codegen {
   class serialization_item {
      public:
         using item_list = std::vector<std::unique_ptr<serialization_item>>;

      protected:
         // Given a "template" serialization item for a non-array member, this adds a copy of the template to `dst`.
         // 
         // Given a "template" serialization item for an array member, this adds an instance of the template to `dst` 
         // for every array element; for example, an array like my_values[3][2] would add six serialization items to 
         // the `dst` array, with indices filled in for each.
         //
         static constexpr void _expand_array_ranks(const serialization_item& item_template, item_list& dst);

      public:
         const ast::structure* struct_definition = nullptr; // containing struct
         const ast::member*    member_definition = nullptr; // this member

         std::string         accessor;      // field to access, as a C expression but not including array ranks
         std::vector<size_t> array_indices; // e.g. 3,4,5 if this is a call to serialize p_Struct.foo[3][4][5]

         constexpr item_list expand();

         static constexpr item_list for_top_level_struct(const ast::structure&);
   };
}

#include "./serialization_item.inl"