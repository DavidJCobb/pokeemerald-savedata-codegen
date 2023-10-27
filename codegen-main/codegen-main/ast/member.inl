#pragma once
#include "./member.h"
#include "./heritable.h"

namespace ast {
   constexpr void member::inherit_from(const heritable& src) {
      auto& src_sem = src.semantic_info;
      auto& dst_sem = this->semantic_info;

      if (std::holds_alternative<semantic::number_info>(src_sem)) {
         auto& src_num = std::get<semantic::number_info>(src_sem);
         if (std::holds_alternative<std::monostate>(dst_sem)) {
            dst_sem = src_num;
         } else if (std::holds_alternative<semantic::number_info>(dst_sem)) {
            auto& dst_num = std::get<semantic::number_info>(dst_sem);

            if (!dst_num.min.has_value())
               dst_num.min = src_num.min;
            if (!dst_num.max.has_value())
               dst_num.max = src_num.max;
         }

         if (src.number_c_type.has_value()) {
            auto& dst_cti = this->c_type_info;
            if (std::holds_alternative<std::monostate>(dst_cti.name)) {
               dst_cti.name = src.number_c_type.value();
            }
         }
      } else if (std::holds_alternative<semantic::string_info>(src_sem)) {
         auto& src_casted = std::get<semantic::string_info>(src_sem);
         if (std::holds_alternative<std::monostate>(dst_sem)) {
            dst_sem = src_casted;
         } else if (std::holds_alternative<semantic::string_info>(dst_sem)) {
            auto& dst_casted = std::get<semantic::string_info>(dst_sem);

            if (!dst_casted.max_length)
               dst_casted.max_length = src_casted.max_length;
            if (!dst_casted.char_type.has_value())
               dst_casted.char_type = src_casted.char_type;
         }
      }
   }

   constexpr void member::resolve_all_specifications() {
      auto& cti = c_type_info;
         
      if (std::holds_alternative<semantic::string_info>(semantic_info)) {
         auto& si = std::get<semantic::string_info>(semantic_info);
         if (si.char_type.has_value())
            cti.name = si.char_type.value();

         cti.array_extents.push_back(c_type::array_rank{
            .extent = si.max_length + 1
         });
      }
   }
}
