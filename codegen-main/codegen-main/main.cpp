#include <iostream>
#include <string>
#include "./registry.h"

int main(int argcount, char* args[]) {
   std::string base_path_h;
   std::string base_path_c;

   std::string base_xml_path;
   std::string out_path_structs;
   std::string out_path_struct_serialize;
   std::string out_path_sectors;

   #if _DEBUG
   for (int i = 1; i < argcount; ++i) {
      std::cout << "arg: " << args[i] << '\n';
   }
   std::cout << '\n';
   #endif
   for (int i = 1; i < argcount; ++i) {
      std::string arg = args[i];
      bool room_for_param = i + 1 < argcount;

      if (arg == "--path-h" && room_for_param) {
         ++i;
         base_path_h = args[i];
         continue;
      }
      if (arg == "--path-c" && room_for_param) {
         ++i;
         base_path_c = args[i];
         continue;
      }

      if (arg == "--xml-path" && room_for_param) {
         ++i;
         base_xml_path = args[i];
         continue;
      }
      if (arg == "--out-path-struct" && room_for_param) {
         ++i;
         out_path_structs = args[i];
         continue;
      }
      if (arg == "--out-path-struct-serialize" && room_for_param) {
         ++i;
         out_path_struct_serialize = args[i];
         continue;
      }
      if (arg == "--out-path-sector" && room_for_param) {
         ++i;
         out_path_sectors = args[i];
         continue;
      }
   }

   bool fail = false;
   if (base_path_h.empty()) {
      fail = true;
      std::cout << "Error: output path for generated headers not specified.\n";
   }
   if (base_path_c.empty()) {
      fail = true;
      std::cout << "Error: output path for generated implementations not specified.\n";
   }
   if (base_xml_path.empty()) {
      fail = true;
      std::cout << "Error: XML file path not specified.\n";
   }
   if (out_path_structs.empty()) {
      fail = true;
      std::cout << "Error: relative output path for struct member declarations not specified.\n";
   }
   if (out_path_struct_serialize.empty()) {
      fail = true;
      std::cout << "Error: relative output path for whole-struct serialization code not specified.\n";
   }
   if (out_path_sectors.empty()) {
      fail = true;
      std::cout << "Error: relative output path for sector serialization code not specified.\n";
   }
   if (fail) {
      return 1;
   }

   std::filesystem::path path_h(base_path_h);
   std::filesystem::path path_c(base_path_c);

   auto& reg = registry::get_or_create();
   reg.set_paths({
      .input_paths = {
         .xml = base_xml_path,
      },
      .output_paths = {
         .h = path_h,
         .c = path_c,
         .struct_members   = out_path_structs,
         .struct_serialize = out_path_struct_serialize,
         .sector_serialize = out_path_sectors,
      }
   });
   try {
      reg.parse_all_xml_files();
   } catch (std::runtime_error& e) {
      std::cout << '\n';
      std::cout << "Exception thrown and not caught: std::runtime_error: " << e.what() << '\n';
      std::cout << "Terminating.";
      return 1;
   }

   reg.generate_all_struct_body_files();
   reg.generate_whole_struct_serialization_code();

   // for testing:
   {
      std::vector<registry::sector_info> list;
      list.push_back({
         .function_name_fragment = "CharacterData",
         .top_level_struct_names = { "SaveBlock2" },
      });
      list.push_back({
         .function_name_fragment = "WorldData",
         .top_level_struct_names = { "SaveBlock1" },
      });
      reg.generate_sector_code(list);
   }

   return 0;
}