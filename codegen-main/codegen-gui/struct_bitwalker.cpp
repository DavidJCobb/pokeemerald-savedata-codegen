#include "./struct_bitwalker.h"
#include <cassert>
#include "lu/strings/from_integer.h"
#include "ast/member.h"
#include "ast/structure.h"

#include "ast/member_types/_all.h"

std::string struct_bitwalker::_stringify_pos() const {
   std::string out;
   if constexpr (!show_offsets) {
      return out;
   }
   out = "/* 0x";
   {
      auto to_digit = [](uint8_t c) {
         if (c < 10)
            return '0' + c;
         return 'A' + (c - 10);
      };

      size_t  bytes = this->reader.get_bytepos();
      uint8_t a = (bytes >> 12) & 0xF;
      uint8_t b = (bytes >>  8) & 0xF;
      uint8_t c = (bytes >>  4) & 0xF;
      uint8_t d = bytes & 0xF;

      out += to_digit(a);
      out += to_digit(b);
      out += to_digit(c);
      out += to_digit(d);
   }
   out += "+";
   out += lu::strings::from_integer(this->reader.get_bitshift());
   out += "b */ ";
   return out;
}

std::string struct_bitwalker::_pull_value(const std::string& accessor, const ast::member& m_def) {
   if (m_def.skip_when_serializing) {
      return "<skipped when serializing>";
   }

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

      std::string out;
      size_t buffer_len = casted->max_length.value;
      for (size_t i = 0; i < buffer_len; ++i) {
         auto ch = this->reader.stream_bits(ast::bitcount_of(char_type));
         if (ch >= 0xBB && ch <= 0xD4) {
            out += '\'';
            out += ((char)(ch - 0xBB) + 'A');
            out += '\'';
         } else if (ch >= 0xD5 && ch <= 0xEE) {
            out += '\'';
            out += ((char)(ch - 0xD5) + 'a');
            out += '\'';
         } else {
            out += lu::strings::from_integer(ch);
         }
         out += ' ';
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
         line += _stringify_pos();
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

         line += _stringify_pos();
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
            _walk_struct(computed_accessor + indices, *as_struct_member->type_def);
         }
      }
   }
}

void struct_bitwalker::set_buffer(const void* data, size_t size) {
   this->reader.set_buffer((lu::bitstreams::reader::buffer_type)data, size);
}
void struct_bitwalker::walk(std::vector<const ast::structure*> top_level_structs) {
   for (const auto* s_def : top_level_structs) {
      std::string accessor = s_def->name;
      _walk_struct(accessor, *s_def);
   }
}