#include <iostream>
#include <string>
#include "./registry.h"

int main(int argcount, char* args[]) {
   std::string base_xml_path;
   std::string struct_output_path;
   std::string serialization_output_path;

   #if _DEBUG
   for (int i = 1; i < argcount; ++i) {
      std::cout << "arg: " << args[i] << '\n';
   }
   std::cout << '\n';
   #endif
   for (int i = 1; i < argcount; ++i) {
      std::string arg = args[i];
      if (arg == "--xml-path" && i + 1 < argcount) {
         ++i;
         base_xml_path = args[i];
         continue;
      }
      if (arg == "--structs-path" && i + 1 < argcount) {
         ++i;
         struct_output_path = args[i];
         continue;
      }
      if (arg == "--serialization-path" && i + 1 < argcount) {
         ++i;
         serialization_output_path = args[i];
         continue;
      }
   }

   bool fail = false;
   if (base_xml_path.empty()) {
      fail = true;
      std::cout << "Error: XML file path not specified.\n";
   }
   if (struct_output_path.empty()) {
      fail = true;
      std::cout << "Error: Struct output folder not specified.\n";
   }
   if (serialization_output_path.empty()) {
      fail = true;
      std::cout << "Error: Serialization output folder not specified.\n";
   }
   if (fail) {
      return 1;
   }

   auto& reg = registry::get_or_create();
   reg.set_base_xml_path(base_xml_path);
   try {
      reg.parse_all_xml_files();
   } catch (std::runtime_error& e) {
      std::cout << '\n';
      std::cout << "Exception thrown and not caught: std::runtime_error: " << e.what() << '\n';
      std::cout << "Terminating.";
      return 1;
   }

   reg.generate_all_struct_body_files(struct_output_path);

   return 0;
}