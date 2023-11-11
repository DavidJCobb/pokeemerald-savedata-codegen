#pragma once
#include <filesystem>

namespace codegen {
   extern void generate_all(
      std::filesystem::path base_h,
      std::filesystem::path base_c,
      std::filesystem::path rel_struct_members,
      std::filesystem::path rel_struct_serialize,
      std::filesystem::path rel_sector_serialize,
      std::filesystem::path rel_save_integrations
   );
}
