#pragma once
#include <string>
#include <vector>
#include "./ast/member.h" // can't forward-declare; need access to `array_rank` member type

namespace ast {
   class structure;
}

struct block_generator {
   std::string data;
   size_t indent = 0;
};
// TODO: use above struct. everything we're genning within the body of a function is either a call or a loop

class treewalker {
   protected:
      struct depth_frame {
         ast::structure* type         = nullptr;
         size_t          member_index = 0;
      };

      struct generated_sector_code {
         std::vector<std::string> top_level_types_touched;
         struct {
            std::string read;
            std::string write;
         } bodies;
         size_t bits_used = 0;
      };

      size_t bitcount         = 0;
      size_t bytes_per_sector = 0;
      size_t current_sector   = 0;
      size_t max_sectors      = 0;
      //
      std::string function_name_base;
      std::vector<std::string> top_level_structs;
      std::vector<generated_sector_code> generated_funcs;

      std::string header;

      std::string code_to_read;
      std::string code_to_write;

      size_t indent = 0;

   public:
      struct serialization_params {
         std::string function_name_base;
         size_t sector_count     = 1;
         size_t bytes_per_sector = 0;

         std::vector<std::string> top_level_structs;
      };

   protected:
      void _write_indent(std::string& dst);

      void _enter_extent_loops(std::string& dst, const std::vector<ast::member::array_rank>&); // writes an anonymous block, variable declarations, and loop starts; manages indentation
      void _exit_extent_loops(std::string& dst, const std::vector<ast::member::array_rank>&); // writes closing braces for loops and anonymous block; manages indentation
      std::string _indices_as_string(const std::vector<ast::member::array_rank>&);

      template<typename T>
      void _write_arg(std::string& dst, T&& arg, bool is_final); // support func for fold expressions in `write_call` and `write_assign`

      template<typename... T>
      void _write_call(std::string& dst, const std::string_view function_name, T&&... args); // writes "foo(bar, baz);"

      template<typename... T>
      void _write_assign(std::string& dst, const std::string_view assign_to, const std::string_view function_name, T&&... args); // writes "foo = bar(baz, bap);"

      void _serialize_whole_member(std::string accessor, const ast::member&);
      void _serialize_single_element(std::string accessor, const ast::member&); // does not advance "bits used"

      void _walk_array_rank(std::string top_level_struct_name, std::string accessor, const ast::member&, size_t rank);
      void _walk_struct(std::string top_level_struct_name, std::string accessor, const ast::structure&);

   public:
      void generate(const serialization_params&);

      const std::string& get_generated_header() const;
      const std::string& get_generated_read_code() const;
      const std::string& get_generated_write_code() const;
};

#include "treewalker.inl"