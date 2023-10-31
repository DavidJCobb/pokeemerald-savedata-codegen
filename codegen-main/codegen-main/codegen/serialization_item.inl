#pragma once
#include "./serialization_item.h"
#include "../ast/member_types/_all.h"
#include "../ast/structure.h"

namespace codegen {
   using namespace std::literals::string_literals;
   
   constexpr size_t serialization_item::bitcount() const {
      if (!this->member_definition) {
         size_t count = 0;
         for (const auto& member_ptr : this->struct_definition->members) {
            const auto* member = member_ptr.get();
            while (auto* casted = dynamic_cast<const ast::inlined_union_member*>(member)) {
               member = &(casted->get_member_to_serialize());
            }

            count += member->compute_total_bitcount();
         }
         return count;
      }

      auto&  extents = this->member_definition->array_extents;
      size_t element = this->member_definition->compute_single_element_bitcount();
      for (size_t i = this->array_indices.size(); i < extents.size(); ++i) {
         element *= extents[i].extent.value;
      }
      return element;
   }
   constexpr serialization_item::item_list serialization_item::expand() const {
      serialization_item::item_list out;

      if (this->member_definition) {
         if (this->array_indices.size() < this->member_definition->array_extents.size()) {
            //
            // Expand next array rank, e.g. foo[2] -> foo[2][0], foo[2][1], foo[2][2], foo[2][3]
            //
            size_t next_rank = this->array_indices.size();
            size_t extent = this->member_definition->array_extents[next_rank].extent.value;
            for (size_t i = 0; i < extent; ++i) {
               auto next = new serialization_item(*this);
               next->array_indices.push_back(i);
               out.push_back(next);
            }
            return out;
         }
      }

      const ast::structure* my_type = nullptr;
      if (this->member_definition) {
         auto* casted = dynamic_cast<const ast::struct_member*>(this->member_definition);
         if (!casted)
            return out;
         my_type = casted->type_def;
      } else {
         //
         // We may be missing a member definition if we represent a top-level struct.
         //
         my_type = this->struct_definition;
      }

      for (const auto& member_ptr : my_type->members) {
         std::string member_accessor = this->accessor;
         const auto* member          = member_ptr.get();
         {
            bool is_top_level = (this->member_definition == nullptr);
            bool is_outermost = true;
            
            while (auto* casted = dynamic_cast<const ast::inlined_union_member*>(member)) {
               if (is_outermost) {
                  for (auto idx : this->array_indices) {
                     member_accessor += '[';
                     member_accessor += lu::strings::from_integer(idx);
                     member_accessor += ']';
                  }
                  is_outermost = false;
               }
               if (is_top_level) {
                  member_accessor += "->";
                  is_top_level = false;
               } else {
                  member_accessor += '.';
               }
               member_accessor += member->name;
               member = &(casted->get_member_to_serialize());
            }

            if (is_outermost) {
               for (auto idx : this->array_indices) {
                  member_accessor += '[';
                  member_accessor += lu::strings::from_integer(idx);
                  member_accessor += ']';
               }
               is_outermost = false;
            }
            if (is_top_level) {
               member_accessor += "->";
               is_top_level = false;
            } else {
               member_accessor += '.';
            }
            member_accessor += member->name;
         }

         auto item = new serialization_item;
         item->struct_definition = my_type;
         item->member_definition = member;
         item->accessor          = member_accessor;
         out.push_back(item);
      }

      return out;
   }

   /*static*/ constexpr serialization_item serialization_item::for_top_level_struct(const ast::structure& def) {
      serialization_item out;
      out.struct_definition = &def;
      out.member_definition = nullptr;
      out.accessor          = "p_"s + def.name;
      return out;
   }

   constexpr bool serialization_item::is_array() const {
      if (!this->member_definition)
         return false;
      const auto& extents = this->member_definition->array_extents;
      return !extents.empty();
   }
}