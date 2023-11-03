#pragma once
#include <string>
#include "codegen-main/cobb/bitstreams/reader.h"

namespace ast {
   class member;
   class structure;
}

class struct_bitwalker {
   public:
      static constexpr const bool show_offsets = true;

   protected:
      cobb::bitstreams::reader reader;

   public:
      std::string output;

   protected:
      std::string _stringify_pos() const;
      std::string _pull_value(const std::string& accessor, const ast::member&);
      void _walk_struct(const std::string& accessor, const ast::structure&);

   public:
      void set_buffer(const void* data, size_t size);
      void walk(std::vector<const ast::structure*> top_level_structs);
};