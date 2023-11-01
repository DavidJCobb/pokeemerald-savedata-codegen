#pragma once
#include "./member.h"
#include <algorithm>
#include "lu/strings/from_integer.h"
#include "./heritable.h"

namespace ast {
   /*virtual*/ constexpr const std::string member::as_c_array_declarator_extents() const {
      if (this->array_extents.empty())
         return {};

      std::string out;
      for (const auto& rank : this->array_extents) {
         out += '[';
         out += rank.as_c_expression();
         out += ']';
      }
      return out;
   }
   constexpr const std::string member::as_c_declaration() const {
      std::string out;
      if (is_const)
         out += "const ";

      out += this->as_c_type_specifier();
      out += ' ';
      if (this->c_alignment.has_value()) {
         out += "ALIGNED(";
         out += this->c_alignment.value().as_c_expression();
         out += ") ";
      }
      out += this->name;
      out += this->as_c_array_declarator_extents();
      out += this->as_c_bitfield_specifier();
      out += ';';

      if (!this->c_line_comment.empty()) {
         out += " // ";
         out += this->c_line_comment;
      }

      return out;
   }
   constexpr std::vector<std::string> member::get_all_used_constants() const {
      std::vector<std::string> out = this->_get_all_used_constants_impl();

      for (const auto& rank : this->array_extents)
         if (!rank.extent.preprocessor_name.empty())
            out.push_back(rank.extent.preprocessor_name);

      if (this->c_alignment.has_value()) {
         auto& v = this->c_alignment.value();
         if (!v.preprocessor_name.empty())
            out.push_back(v.preprocessor_name);
      }

      auto it = std::unique(out.begin(), out.end());
      out.erase(it, out.end());
      return out;
   }
   constexpr size_t member::compute_total_bitcount() const {
      if (this->skip_when_serializing)
         return 0;
      size_t count = this->compute_single_element_bitcount();
      for (const auto& rank : this->array_extents) {
         count *= rank.extent.value;
      }
      return count;
   }
   constexpr size_t member::compute_total_unpacked_bytecount() const {
      size_t single = this->_compute_single_element_unpacked_bytecount();
      size_t count  = 1;
      for (const auto& rank : this->array_extents) {
         count *= rank.extent.value;
      }
      size_t bytes = single * count;
      if (count > 1) {
         size_t align    = this->compute_unpacked_alignment();
         size_t misalign = single % align;
         if (misalign) {
            bytes += (align - misalign) * (count - 1);
         }
      }
      return bytes;
   }
   constexpr size_t member::compute_unpacked_alignment() const {
      if (this->c_alignment.has_value())
         return this->c_alignment.value().value;
      return this->_get_alignment_impl();
   }
}
