#pragma once
#include <string>
#include <vector>
#include "./serialization_item.h"

namespace ast {
   class structure;
}

namespace codegen {
   class sector_generator {
      public:
         struct output {
            std::string header;
            std::string implementation;
            size_t      sector_count = 0;
         };

         static constexpr const size_t sector_size_including_footer = 4096;
         static constexpr const size_t sector_footer                = 128; // only 12 bytes are actually used lol
         static constexpr const size_t max_bytes_per_sector = sector_size_including_footer - sector_footer;

         static constexpr const bool enable_debug_prints = true;
         static constexpr const bool enable_array_extent_expressions = true;

      public:
         constexpr ~sector_generator();

         std::string function_name_fragment;
         std::string sector_serialize_header_folder;
         std::string whole_struct_serialize_header_folder;

         std::vector<const ast::structure*> top_level_structs;
         std::vector<serialization_item::item_list> items_by_sector; // owned
         size_t out_sector_count = 0;

      protected:
         static constexpr const size_t bytespan_of(size_t bitcount) {
            return (bitcount / 8) + (bitcount % 8 ? 1 : 0);
         }

         constexpr std::vector<serialization_item::item_list> perform_expansions_and_splits(serialization_item::item_list) const;

      public:
         constexpr void prepare(const std::vector<const ast::structure*>& structures);

         constexpr output generate() const;
   };
}

#include "./sector_generator.inl"