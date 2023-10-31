#pragma once
#include "./integral_member.h"
#include <bit>

namespace ast {
   /*virtual*/ constexpr const std::string integral_member::as_c_type_specifier() const /*override*/ {
      return std::string(integral_type_to_string(this->value_type.value()));
   }
   /*virtual*/ constexpr const std::string integral_member::as_c_bitfield_specifier() const /*override*/ {
      if (this->c_bitfield.has_value()) {
         std::string out = " : ";
         out += this->c_bitfield.value().as_c_expression();
         return out;
      }
      return {};
   }
   /*virtual*/ constexpr std::size_t integral_member::compute_single_element_bitcount() const /*override*/ {
      if (std::is_constant_evaluated()) {
         if (!this->value_type.has_value())
            throw;
      }

      if (this->serialization_bitcount.has_value())
         return this->serialization_bitcount.value().value;

      if (!this->min.has_value() && !this->max.has_value()) {
         if (this->c_bitfield.has_value())
            return this->c_bitfield.value().value;
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
   /*virtual*/ constexpr std::vector<std::string> integral_member::_get_all_used_constants_impl() const /*override*/ {
      std::vector<std::string> out;

      if (c_bitfield.has_value())
         if (!c_bitfield.value().preprocessor_name.empty())
            out.push_back(c_bitfield.value().preprocessor_name);

      if (serialization_bitcount.has_value())
         if (!serialization_bitcount.value().preprocessor_name.empty())
            out.push_back(serialization_bitcount.value().preprocessor_name);

      return out;
   }
   //
   constexpr bool integral_member::is_signed() const {
      return ::ast::is_signed(this->value_type.value());
   }
}
