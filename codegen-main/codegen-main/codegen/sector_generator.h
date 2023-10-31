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
         };

      public:
         std::string function_name_fragment;
         std::vector<ast::structure*> top_level_structs;
         std::string sector_serialize_header_folder;
         std::string whole_struct_serialize_header_folder;

         size_t sector_max_bytes = 3968; // 4KiB sectors with 128B burned on the footer, of which 12B are actually used

      protected:
         static constexpr const size_t bytespan_of(size_t bitcount) {
            return (bitcount / 8) + (bitcount % 8 ? 1 : 0);
         }

         constexpr std::vector<serialization_item::item_list> perform_expansions_and_splits(serialization_item::item_list) const;

      public:
         constexpr output generate(const std::vector<const ast::structure*>& structures) const;
   };
}

#include "./sector_generator.inl"