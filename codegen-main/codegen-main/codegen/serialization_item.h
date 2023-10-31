#pragma once
#include <string>
#include <vector>

namespace ast {
   class structure;
   class member;
}

namespace codegen {
   class serialization_item {
      public:
         using item_list = std::vector<serialization_item*>;

      public:
         const ast::structure* struct_definition = nullptr; // containing struct
         const ast::member*    member_definition = nullptr; // this member

         std::string         accessor;      // field to access, as a C expression but not including array ranks
         std::vector<size_t> array_indices; // e.g. 3,4,5 if this is a call to serialize p_Struct.foo[3][4][5]

         constexpr size_t bitcount() const;
         constexpr item_list expand() const;

         static constexpr serialization_item for_top_level_struct(const ast::structure&);

         constexpr bool is_array() const;
   };
}

#include "./serialization_item.inl"