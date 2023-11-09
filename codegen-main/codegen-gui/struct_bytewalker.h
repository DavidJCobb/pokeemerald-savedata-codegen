#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "ast/integral_type.h"

namespace ast {
   class member;
   class structure;
}

class struct_bytewalker {
   protected:
      const void* data = nullptr;
      size_t      size = 0;

      struct {
         size_t bytes = 0;
         size_t bits  = 0;
      } pos;
      size_t depth = 0;

   public:
      std::string output;

   protected:
      void _advance_pos_by_bytes(size_t);
      void _advance_pos_by_bits(size_t);
      void _advance_pos_to_alignment(size_t);

      uint8_t _consume_byte_for_read(uint8_t bitcount, uint8_t& consumed_bitcount);
      uint32_t _read_bits(size_t bitcount);

      int64_t _read(ast::integral_type);

      std::string _pull_value(const std::string& accessor, const ast::member&, bool in_packed);
      void _walk_struct(const std::string& accessor, const ast::structure&);

   public:
      void set_buffer(const void* data, size_t size);
      void walk(std::vector<const ast::structure*> top_level_structs);
};