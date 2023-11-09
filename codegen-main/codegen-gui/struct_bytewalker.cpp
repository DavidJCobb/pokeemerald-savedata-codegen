#include "./struct_bytewalker.h"
#include <cassert>
#include "lu/strings/from_integer.h"
#include "ast/member.h"
#include "ast/structure.h"

#include "ast/member_types/_all.h"

void struct_bytewalker::_advance_pos_by_bytes(size_t b) {
   this->pos.bytes += b;
}
void struct_bytewalker::_advance_pos_by_bits(size_t b) {
   if (b >= 8) {
      this->pos.bytes += b / 8;
      b %= 8;
   }
   if (!b)
      return;
   this->pos.bits += b;
   if (this->pos.bits >= 8) {
      this->pos.bytes += this->pos.bits / 8;
      this->pos.bits  %= 8;
   }
}
void struct_bytewalker::_advance_pos_to_alignment(size_t align) {
   if (this->pos.bits) {
      this->pos.bytes++;
      this->pos.bits = 0;
   }
   size_t misalign = this->pos.bytes % align;
   if (misalign)
      this->pos.bytes += align - misalign;
}

uint8_t struct_bytewalker::_consume_byte_for_read(uint8_t bitcount, uint8_t& consumed_bitcount) {
   uint8_t raw  = *((const uint8_t*)this->data + this->pos.bytes) & (0xFF >> this->pos.bits);
   uint8_t read = 8 - this->pos.bits;
   if (bitcount < read) {
      raw = raw >> (read - bitcount);
      _advance_pos_by_bits(bitcount);
      consumed_bitcount = bitcount;
   } else {
      _advance_pos_by_bits(read);
      consumed_bitcount = read;
   }
   return raw;
}
uint32_t struct_bytewalker::_read_bits(size_t bitcount) {
   uint8_t  consumed;
   uint8_t  raw       = _consume_byte_for_read(bitcount, consumed);
   uint32_t result    = raw;
   uint8_t  remaining = bitcount - consumed;
   while (remaining) {
      raw        = _consume_byte_for_read(remaining, consumed);
      result     = (result << consumed) | raw;
      remaining -= consumed;
   }
   return result;
}

int64_t struct_bytewalker::_read(ast::integral_type value_type) {
   size_t bitcount  = ast::bitcount_of(value_type);
   if (bitcount < 8)
      bitcount = 8;
   size_t bytecount = bitcount / 8;

   this->_advance_pos_to_alignment(bytecount);
   //
   int64_t value = 0;
   switch (bytecount) {
      case 1:
         value = *((uint8_t*)this->data + this->pos.bytes);
         break;
      case 2:
         value = *(uint16_t*)((uint8_t*)this->data + this->pos.bytes);
         break;
      case 4:
         value = *(uint32_t*)((uint8_t*)this->data + this->pos.bytes);
         break;
      default:
         assert(false && "impossible bytecount");
   }
   //
   this->_advance_pos_by_bytes(bytecount);
   
   if (ast::is_signed(value_type)) {
      bool sign = (value >> (bitcount - 1)) & 1;
      if (sign) {
         int64_t extend = ~((1 << bitcount) - 1);
         value |= extend;
      }
   }

   return value;
}

std::string struct_bytewalker::_pull_value(const std::string& accessor, const ast::member& m_def, bool in_packed) {
   size_t alignment = m_def.compute_unpacked_alignment();
   if (in_packed)
      alignment = 1;

   if (auto* casted = dynamic_cast<const ast::blind_union_member*>(&m_def)) {
      this->_advance_pos_to_alignment(alignment);
      this->_advance_pos_by_bytes(casted->type_def->size_in_bytes);
      return "<blind union>";
   }
   if (auto* casted = dynamic_cast<const ast::integral_member*>(&m_def)) {
      const auto value_type = casted->value_type.value();

      if (casted->c_bitfield.has_value()) {
         size_t bitfield_size = casted->c_bitfield.value().value;

         int64_t raw       = this->_read_bits(bitfield_size);
         int64_t truncated = raw;
         {
            auto bc = ast::bitcount_of(value_type);
            if (bc < 32 && bc < casted->c_bitfield.value().value) {
               truncated &= (1 << bc) - 1;
            }
         }
         if (ast::is_signed(value_type) && !casted->min.has_value()) {
            bool sign = raw >> (bitfield_size - 1);
            if (sign) {
               int64_t sign_extend = ~((1 << bitfield_size) - 1);
               raw       |= sign_extend;
               truncated |= sign_extend;
            }
         }
         int64_t offset       = raw;
         int64_t offset_trunc = truncated;
         if (casted->min.has_value()) {
            offset       -= casted->min.value();
            offset_trunc -= casted->min.value();
         }

         std::string out = lu::strings::from_integer(offset_trunc);
         if (offset != offset_trunc || raw != truncated) {
            out += " (truncated; ";
            out += lu::strings::from_integer(raw);
            out += " -> ";
            out += lu::strings::from_integer(truncated);
            out += " -> ";
            out += lu::strings::from_integer(offset_trunc);
            out += ")";
         }
         return out;
      }

      auto value = this->_read(value_type);
      if (casted->min.has_value()) {
         value += casted->min.value();
      }
      return lu::strings::from_integer(value);
   }
   if (auto* casted = dynamic_cast<const ast::pointer_member*>(&m_def)) {
      this->_advance_pos_to_alignment(alignment);
      uint32_t v = *(uint32_t*)((uint8_t*)this->data + this->pos.bytes);
      this->_advance_pos_by_bytes(4);
      return lu::strings::from_integer(v);
   }
   if (auto* casted = dynamic_cast<const ast::string_member*>(&m_def)) { // NOTE: this'll be in GF encoding for now
      auto char_type = casted->char_type.value_or(ast::integral_type::u8);

      size_t buffer_len = casted->max_length.value;
      if (!casted->only_early_terminator)
         ++buffer_len;

      std::string out;
      for (size_t i = 0; i < buffer_len; ++i) {
         auto ch = this->_read(char_type);
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

void struct_bytewalker::_walk_struct(const std::string& accessor, const ast::structure& s_def) {
   bool in_packed = s_def.c_type_info.is_packed;

   this->_advance_pos_to_alignment(in_packed ? 1 : 4);

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
            line += _pull_value(computed_accessor, *member, in_packed);
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
            line += _pull_value(computed_accessor, *member, in_packed);
         }
         line += "\n";
         this->output += line;

         if (as_struct_member) {
            _walk_struct(computed_accessor, *as_struct_member->type_def);
         }
      }
   }
}

void struct_bytewalker::set_buffer(const void* data, size_t size) {
   this->data = data;
   this->size = size;
   this->pos  = {};
}
void struct_bytewalker::walk(std::vector<const ast::structure*> top_level_structs) {
   for (const auto* s_def : top_level_structs) {
      std::string accessor = s_def->name;
      _walk_struct(accessor, *s_def);
   }
}