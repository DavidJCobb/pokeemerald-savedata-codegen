#include "./member.h"
#include <cassert>
#include "../registry.h"

namespace ast {
   size_t member::compute_bitcount() const {
      if (this->c_union.has_value()) {
         size_t bitcount = 0;
         for (auto& m : this->c_union.value().members) {
            size_t bc = m.compute_bitcount();
            if (bc > bitcount)
               bitcount = bc;
         }
         return bitcount;
      }

      assert(!std::holds_alternative<semantic::string_info>(semantic_info));

      if (std::holds_alternative<semantic::number_info>(semantic_info)) {
         const auto& info = std::get<semantic::number_info>(semantic_info);
         std::intmax_t min;
         std::intmax_t max;

         if (info.min.has_value()) {
            min = info.min.value();
         } else {
            assert(std::holds_alternative<c_type::primitive>(c_type_info.name));
            min = c_type::minimum_of(std::get<c_type::primitive>(c_type_info.name));
         }
         if (info.max.has_value()) {
            max = info.max.value();
         } else {
            assert(std::holds_alternative<c_type::primitive>(c_type_info.name));
            max = c_type::maximum_of(std::get<c_type::primitive>(c_type_info.name));
         }

         return std::bit_width((std::uintmax_t)(max - min));
      }

      if (std::holds_alternative<c_type::primitive>(c_type_info.name)) {
         return c_type::bitcount_of(std::get<c_type::primitive>(c_type_info.name));
      } else if (std::holds_alternative<std::string>(c_type_info.name)) {
         //
         // TODO: We need some kind of guard against infinite recursion, OR we need to compute the bitcount 
         // from the registry itself rather than as a member of, uh, `member`.
         //
         return registry::get().bitcount_of_struct(std::get<std::string>(c_type_info.name));
      }

      return 0;
   }
}
