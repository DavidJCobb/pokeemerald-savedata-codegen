#pragma once
#include <string>
#include <vector>
#include "./serialization_item.h"

namespace codegen {
   class serialization_item;
}

namespace codegen {
   class serialization_function_body {
      public:
         struct function_pair {
            std::string read;
            std::string write;

            template<typename T> requires (requires(std::string& a, T&& b) { { a += b }; })
            constexpr function_pair& operator+=(T&& v) {
               read  += v;
               write += v;
               return *this;
            }
         };

      protected:
         struct array_rank_info {
            size_t deepest_rank = 0;
            //
            bool all_indices_are_u8       = true;
            bool all_indices_are_numbered = false;

            constexpr array_rank_info(const serialization_item::item_list&);

            constexpr std::string generate_forward_declarations() const;
            constexpr std::string rank_to_index_variable(size_t) const;
         };

      public:
         serialization_item::item_list item_list; // unowned
         bool enable_array_extent_expressions = true;
         bool writes_can_debug_print          = true;  // recommended for sectors; not recommended for whole-struct/single-struct calls
         bool state_is_argument               = false; // if `true`, the bitstream state is a pointer argument; if false, it's a local variable; in both cases it's named `state`
         //
         function_pair out;
         struct {
            std::vector<const ast::structure*> direct_dependencies;
         } results;

      protected:
         constexpr std::string _generate_loop_start(const std::string& var, size_t start, const std::string& stop);
         constexpr std::string _generate_loop_start(const std::string& var, size_t start, size_t stop);
         constexpr std::string _generate_loop_start(const std::string& var, size_t start, const ast::member::array_rank& stop);

      public:
         constexpr void generate();
   };

}

#include "./serialization_function_body.inl"