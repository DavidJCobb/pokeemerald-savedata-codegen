#pragma once
#include "./serialization_item.h"
#include "../ast/member.h"
#include "../ast/structure.h"

namespace codegen {
   using namespace std::literals::string_literals;
   
   /*static*/ constexpr void serialization_item::_expand_array_ranks(const serialization_item& item_template, std::vector<std::unique_ptr<serialization_item>>& dst) {
      const auto& extents = item_template.member_definition->array_extents;
      if (extents.empty()) {
         dst.push_back(std::make_unique<serialization_item>(item_template));
         return;
      }
      //
      // given extents 3, 4, 5, produce as output:
      // 
      //  - [0][0][0]
      //  - [0][0][1]
      //  - [0][0][2]
      //  - [0][0][3]
      //  - [0][0][4]
      //  - [0][1][0]
      //  - [0][1][1]
      //  - [0][1][2]
      //  - [0][1][3]
      //  - [0][1][4]
      //  - ...
      //  - [2][3][4]
      //
      size_t total_elements = 1;
      for (const auto& rank : extents) {
         total_elements *= rank.extent.value;
      }

      for (size_t i = 0; i < total_elements; ++i) {
         auto item = std::make_unique<serialization_item>();
         *item = item_template;
         item->array_indices.reserve(extents.size());

         size_t index = i;
         for (auto it = extents.rbegin(); it != extents.rend(); ++it) {
            auto div = it->extent.value;
            item->array_indices.push_back(index % div);
            index /= div;
         }

         dst.push_back(item);
      }
   }

   constexpr serialization_item::item_list serialization_item::expand() {
      serialization_item::item_list out;

      if (this->array_indices.size() < this->member_definition->array_extents.size()) {
         //
         // Expand next array rank, e.g. foo[2] -> foo[2][0], foo[2][1], foo[2][2], foo[2][3]
         //
         size_t next_rank = this->array_indices.size();
         size_t extent = this->member_definition->array_extents[next_rank].extent.value;
         for (size_t i = 0; i < extent; ++i) {
            auto next = std::make_unique<serialization_item>(*this);
            next->array_indices.push_back(i);
            out.push_back(std::move(next));
         }
         return out;
      }

      const ast::structure* my_type = nullptr;
      {
         auto* casted = dynamic_cast<const ast::struct_member*>(this->member_definition);
         if (!casted)
            return out;
         my_type = casted->type_def;
      }

      for (const auto& member_ptr : my_type->members) {
         std::string member_accessor;
         const auto* member          = member_ptr.get();
         while (auto* casted = dynamic_cast<const ast::inlined_union_member*>(member)) {
            member_accessor += member->name;
            member_accessor += '.';
            member = &(casted->get_member_to_serialize());
         }
         member_accessor += member->name;

         auto item = std::make_unique<serialization_item>();
         item->struct_definition = my_type;
         item->member_definition = member;
         item->accessor          = member_accessor;
         out.push_back(std::move(item));
      }

      return out;
   }
}
