#include <array>
#include <fstream>
#include <iostream>
#include "./registry_ex.h"

//
// This program is designed to help debug *.sav files pulled from an emulator, to 
// investigate bugs in our savegame code. Just use a hex editor to pull bytes out 
// and stuff them into the `sector` array. (Be sure to prefix the octets with 0x; 
// JavaScript is good for that.)
// 

//
// If this program fails to build due to linker errors, then it's probably because 
// you're referencing files in the `codegen-main` project. In a good build system, 
// you'd be able to just include the headers and whatnot. MSBuild is not a good 
// build system.
// 
// You have to right-click the `dumper` project, choose Add Existing Item, and 
// manually grab every single header and CPP file from `codegen-main` to use any 
// of them.
//

constexpr const bool dump_bitpacked = true;

int main() {
   auto& reg = registry_ex::get_or_create();
   reg.set_paths({
      .input_paths = {
         .xml = "../codegen-main/_usage/xml",
      },
   });
   try {
      reg.parse_all_xml_files();
   } catch (std::runtime_error& e) {
      std::cout << '\n';
      std::cout << "Exception thrown and not caught: std::runtime_error: " << e.what() << '\n';
      std::cout << "Terminating.";
      return 1;
   }

   /*//
   auto out = reg.dump_unpacked(sector.data(), sector.size(), {
      "SaveBlock2",
   });

   std::cout << out;

   std::ofstream stream("last-dump.txt");
   assert(!!stream);
   stream << out;
   //*/

   {
      std::ifstream stream("pokeemerald.sav", std::ios::binary);
      if (!!stream) {
         auto sav_size = std::filesystem::file_size("pokeemerald.sav");

         std::vector<uint8_t> buffer;
         buffer.resize(sav_size);
         if (stream.read((char*)buffer.data(), sav_size)) {
            std::vector<std::string> sectioned_dump;
            if (dump_bitpacked) {
               sectioned_dump = reg.dump_packed_from_sav(buffer.data(), buffer.size());
            } else {
               sectioned_dump = reg.dump_unpacked_from_sav(buffer.data(), buffer.size());
            }

            std::ofstream out_stream("last-sectioned-dump.txt");
            for (const auto& item : sectioned_dump) {
               std::cout << item << "\n\n";
               if (out_stream)
                  out_stream << item << "\n\n";
            }
         }
      }
   }

   return 0;
};