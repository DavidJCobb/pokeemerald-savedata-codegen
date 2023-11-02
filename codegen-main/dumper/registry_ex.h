#pragma once
#include "codegen-main/registry.h"

class registry_ex;
class registry_ex : protected registry {
   protected:
      static registry_ex* instance;
   public:
      static registry_ex& get_or_create() {
         static registry_ex instance;
         return instance;
      }
      static registry_ex& get() {
         return *registry_ex::instance;
      }

      using registry::set_paths;
      using registry::parse_all_xml_files;

   public:
      std::string dump_unpacked(const void* data, size_t max_size, const std::vector<std::string>& structures);
      std::string dump_packed(const void* data, size_t max_size, const std::vector<std::string>& structures);

      std::string dump(bool bitpacked, const void* data, size_t max_size, const std::vector<std::string>& structures);

   protected:
      std::vector<std::string> _dump_from_sav(const void* sav, size_t sav_size, bool packed);

   public:
      std::vector<std::string> dump_unpacked_from_sav(const void* sav, size_t sav_size);

      std::vector<std::string> dump_packed_from_sav(const void* sav, size_t sav_size);
};
