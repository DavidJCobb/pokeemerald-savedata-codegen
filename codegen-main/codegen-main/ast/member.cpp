#include "./member.h"
#include <cassert>
#include "../registry.h"

namespace ast {
   /*virtual*/ std::size_t integral_member::compute_serialization_bitcount() const /*override*/ {
      assert(this->value_type.has_value());

      if (this->serialization_bitcount.has_value())
         return this->serialization_bitcount.value();

      if (!this->min.has_value() && !this->max.has_value()) {
         if (this->c_bitfield.has_value())
            return this->c_bitfield.value();
         return ast::bitcount_of(this->value_type.value());
      }

      std::intmax_t min;
      std::intmax_t max;
      if (this->min.has_value()) {
         min = this->min.value();
      } else {
         min = ast::minimum_of(this->value_type.value());
      }
      if (this->max.has_value()) {
         max = this->max.value();
      } else {
         max = ast::maximum_of(this->value_type.value());
      }
      return std::bit_width((std::uintmax_t)(max - min));
   }

   /*virtual*/ std::size_t string_member::compute_serialization_bitcount() const /*override*/ {
      assert(this->char_type.has_value());
      return ast::bitcount_of(this->char_type.value());
   }

   /*virtual*/ std::size_t struct_member::compute_serialization_bitcount() const /*override*/ {
      return registry::get().bitcount_of_struct(this->type_name);
   }

   /*virtual*/ std::size_t inlined_union_member::compute_serialization_bitcount() const /*override*/ {
      return this->get_member_to_serialize().compute_serialization_bitcount();
   }
}
