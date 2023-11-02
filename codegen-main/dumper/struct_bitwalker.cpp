#include "./struct_bitwalker.h"
#include <cassert>
#include "codegen-main/lu/strings/from_integer.h"
#include "codegen-main/ast/member.h"
#include "codegen-main/ast/structure.h"

#include "codegen-main/ast/member_types/_all.h"

std::string struct_bitwalker::_pull_value(const std::string& accessor, const ast::member& m_def) {
   if (auto* casted = dynamic_cast<const ast::blind_union_member*>(&m_def)) {
      for (size_t i = 0; i < casted->type_def->size_in_bytes; ++i) {
         uint8_t dummy;
         this->reader.stream(dummy);
      }
      return "<blind union>";
   }
   if (auto* casted = dynamic_cast<const ast::integral_member*>(&m_def)) {
      const auto value_type = casted->value_type.value();

      auto bitcount = casted->compute_single_element_bitcount();
      std::int64_t value = this->reader.stream_bits(bitcount);
      
      if (ast::is_signed(value_type) && !casted->min.has_value()) {
         bool sign = value >> (bitcount - 1);
         if (sign) {
            int64_t sign_extend = ~((1 << bitcount) - 1);
            value |= sign_extend;
         }
      }
      if (casted->min.has_value()) {
         value += casted->min.value();
      }

      return lu::strings::from_integer(value);
   }
   if (auto* casted = dynamic_cast<const ast::pointer_member*>(&m_def)) {
      uint32_t v = this->reader.stream_bits(32);
      return lu::strings::from_integer(v);
   }
   if (auto* casted = dynamic_cast<const ast::string_member*>(&m_def)) { // NOTE: this'll be in GF encoding for now
      auto char_type = casted->char_type.value_or(ast::integral_type::u8);

      size_t buffer_len = casted->max_length.value;
      if (!casted->only_early_terminator)
         ++buffer_len;

      std::string out;
      for (size_t i = 0; i < buffer_len; ++i) {
         auto ch = this->reader.stream_bits(ast::bitcount_of(char_type));
         out += lu::strings::from_integer(ch);

         if (i + 1 < buffer_len) {
            if (!casted->only_early_terminator && i + 1 == casted->max_length.value) {
               out += " | ";
            } else {
               out += ' ';
            }
         }
      }
      return out;
   }

   return {};
}

void struct_bitwalker::_walk_struct(const std::string& accessor, const ast::structure& s_def) {
   bool in_packed = s_def.c_type_info.is_packed;

   for (const auto& member_ptr : s_def.members) {
      std::string computed_accessor = accessor;
      //
      const ast::member* member = member_ptr.get();
      while (auto* casted = dynamic_cast<const ast::inlined_union_member*>(member)) {
         computed_accessor += '.';
         computed_accessor += member->name;
         member = &casted->get_member_to_serialize();
      }
      computed_accessor += '.';
      computed_accessor += member->name;

      auto* as_struct_member = dynamic_cast<const ast::struct_member*>(member);

      size_t consumed = 0;
      if (member->array_extents.empty()) {
         std::string line;
         line += computed_accessor;
         line += "\t\t\t";
         if (as_struct_member) {
            line += "...";
         } else {
            line += _pull_value(computed_accessor, *member);
         }
         line += "\n";
         this->output += line;
         if (as_struct_member) {
            _walk_struct(computed_accessor, *as_struct_member->type_def);
         }
         continue;
      }

      size_t total_elements = 1;
      for (auto& extent : member->array_extents) {
         total_elements *= extent.extent.value;
      }

      for (size_t i = 0; i < total_elements; ++i) {
         std::string line;
         std::string indices;

         size_t index = i;
         for (auto it = member->array_extents.rbegin(); it != member->array_extents.rend(); ++it) {
            std::string here = "[";
            here += lu::strings::from_integer(index % it->extent.value);
            here += ']';

            indices = here + indices;
            index /= it->extent.value;
         }

         line += computed_accessor;
         line += indices;
         line += "\t\t\t";
         if (as_struct_member) {
            line += "...";
         } else {
            line += _pull_value(computed_accessor, *member);
         }
         line += "\n";
         this->output += line;

         if (as_struct_member) {
            _walk_struct(computed_accessor, *as_struct_member->type_def);
         }
      }
   }
}

void struct_bitwalker::set_buffer(const void* data, size_t size) {
   this->reader.set_buffer((cobb::bitstreams::reader::buffer_type)data, size);
}
void struct_bitwalker::walk(std::vector<const ast::structure*> top_level_structs) {
   for (const auto* s_def : top_level_structs) {
      std::string accessor = s_def->name;
      _walk_struct(accessor, *s_def);
   }
}