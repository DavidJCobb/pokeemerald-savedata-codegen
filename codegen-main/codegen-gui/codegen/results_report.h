#pragma once
#include <string>
#include <vector>
#include "../registry.h"

namespace codegen {
   class sector_generator;
}

namespace codegen {
   struct results_report {
      std::string overall_stats;
      std::string struct_table;
      std::vector<std::string> by_sector;
      std::string every_struct;
      std::string every_heritable;

      size_t total_bytes_in_ram = 0;
      bool   any_sector_group_overflowed = false;

      void generate_overall_stats(const std::vector<registry::sector_group>&);

      void add_to_sector_group_report(
         const registry::sector_group&,
         const codegen::sector_generator&
      );

      void generate_stats_for_every_struct(const std::vector<registry::sector_group>&);

      void generate_stats_for_heritables(const std::vector<registry::sector_group>&);
   };
}
