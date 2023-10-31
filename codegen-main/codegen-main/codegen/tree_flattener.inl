#pragma once
#include "./tree_flattener.h"
#include "../ast/member.h"
#include "../ast/structure.h"

namespace codegen {
   using namespace std::literals::string_literals;

   /*static*/ constexpr void serialization_item::_generate_array_ranks(const serialization_item& item_template, std::vector<std::unique_ptr<serialization_item>>& dst) {
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

   constexpr void serialization_item::expand() {
      if (!children.empty())
         return;

      const ast::structure* my_type = nullptr;
      {
         auto* casted = dynamic_cast<const ast::struct_member*>(this->member_definition);
         if (!casted)
            return;
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

         serialization_item item_template;
         item_template.struct_definition = my_type;
         item_template.member_definition = member;
         item_template.parent            = this;
         item_template.accessor          = member_accessor;
         
         _generate_array_ranks(item_template, this->children);
      }
   }

   /*static*/ constexpr std::vector<std::unique_ptr<serialization_item>> serialization_item::for_top_level_struct(const ast::structure& def) {
      std::vector<std::unique_ptr<serialization_item>> out;

      for (const auto& member_ptr : def.members) {
         const auto* member = member_ptr.get();
         std::string member_accessor;
         while (auto* casted = dynamic_cast<const ast::inlined_union_member*>(member)) {
            member_accessor += member->name;
            member_accessor += '.';
            member = &(casted->get_member_to_serialize());
         }
         member_accessor += member->name;

         serialization_item item_template;
         item_template.struct_definition = &def;
         item_template.member_definition = member;
         item_template.parent            = nullptr;
         item_template.accessor          = member_accessor;

         _generate_array_ranks(item_template, out);
      }

      return out;
   }

   constexpr void tree_flattener::_walk_struct(std::string accessor, const ast::structure& structure) {
      for (const auto& member_ptr : structure.members) {
         std::string member_accessor = accessor;
         const auto* member          = member_ptr.get();
         while (auto* casted = dynamic_cast<const ast::inlined_union_member*>(member)) {
            member_accessor += '.';
            member_accessor += member->name;
            member = &(casted->get_member_to_serialize());
         }
         member_accessor += '.';
         member_accessor += member->name;

         const auto elem_bitcount = member->compute_single_element_bitcount();

         single_call call_template;
         call_template.member_definition    = member;
         call_template.bitcount_per_element = elem_bitcount;
         call_template.target               = member_accessor;
         call_template.function_names.read  = "lu_BitstreamRead_";
         call_template.function_names.write = "lu_BitstreamWrite_";

         if (auto* casted = dynamic_cast<const ast::integral_member*>(member)) {
            call_template.reading_uses_assign = true;

            std::string type_suffix;
            if (elem_bitcount == 1)
               type_suffix = "bool";
            else if (elem_bitcount <= 8)
               type_suffix = "u8";
            else if (elem_bitcount <= 16)
               type_suffix = "u16";
            else
               type_suffix = "u32";

            call_template.function_names.read  += type_suffix;
            call_template.function_names.write += type_suffix;
         } else if (auto* casted = dynamic_cast<const ast::string_member*>(member)) {
            call_template.function_names.read  += "string";
            call_template.function_names.write += "string";
         } else if (auto* casted = dynamic_cast<const ast::struct_member*>(member)) {
            call_template.function_names.read  += casted->type_def->name;
            call_template.function_names.write += casted->type_def->name;
         } else {
            throw;
         }

         if (member->array_extents.empty()) {
            this->calls.push_back(call_template);
         } else {
            // given extents 3, 4, 5, produce as output:
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
            size_t total_elements = 1;
            for (const auto& rank : member->array_extents) {
               total_elements *= rank.extent.value;
            }

            for (size_t i = 0; i < total_elements; ++i) {
               single_call call = call_template;

               size_t index = i;
               for (auto it = member->array_extents.rbegin(); it != member->array_extents.rend(); ++it) {
                  auto div = it->extent.value;
                  call.array_extents.push_back(index % div);
                  index /= div;
               }

               this->calls.push_back(call);
            }
         }
      }
   }

   constexpr void tree_flattener::flatten_top_level_struct(std::string top_level_struct_typename, const ast::structure& dfn) {
      std::string accessor = "p_"s + dfn.name;
      this->_walk_struct(accessor, dfn);
   }
}