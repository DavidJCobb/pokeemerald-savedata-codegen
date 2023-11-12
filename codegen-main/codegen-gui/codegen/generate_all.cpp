#include "./generate_all.h"
#include <fstream>
#include "./sector_generator.h"
#include "./struct_body_serializer.h"
#include "./struct_serialization_function_generator.h"
#include "./results_report.h"

#include "../registry.h"

using namespace std::literals::string_literals;

namespace {
   struct computed_paths {
      struct rel_set {
         constexpr rel_set() {}
         rel_set(const std::filesystem::path& base_h, const std::filesystem::path& base_c, const std::filesystem::path& rel) {
            this->rel = rel;
            this->h   = base_h / rel;
            this->c   = base_c / rel;
         }

         std::filesystem::path rel;
         std::filesystem::path h;
         std::filesystem::path c;

         void force_create_paths() {
            std::error_code error;
            if (!std::filesystem::create_directories(this->h, error))
               assert(!error);
            if (!std::filesystem::create_directories(this->c, error))
               assert(!error);
         }
      };

      std::filesystem::path struct_members;

      rel_set struct_serialize;
      rel_set sector_serialize;
      rel_set savegame_integration;

      void force_create_paths() {
         std::error_code error;
         if (!std::filesystem::create_directories(this->struct_members, error))
            assert(!error);
         this->struct_serialize.force_create_paths();
         this->sector_serialize.force_create_paths();
         this->savegame_integration.force_create_paths();
      }
   };

   static void _generate_struct_code(const computed_paths& paths) {
      auto& reg         = registry::get();
      auto& all_structs = reg.all_structs();
      //
      for (const auto& pair : all_structs) {
         const auto& definition = *(pair.second.get());

         {  // Struct member lists
            codegen::struct_body_serializer generator(definition);

            std::ofstream stream(paths.struct_members / std::filesystem::path(definition.name + ".members.h"));
            assert(!!stream);
            stream << generator.generate();
         }
         {  // Whole-struct serialization functions
            codegen::struct_serialization_function_generator generator(definition);
            generator.destination_folder = paths.struct_serialize.rel.generic_string();
            {
               std::ofstream stream(paths.struct_serialize.h / std::filesystem::path("serialize_"s + definition.name + ".h"));
               assert(!!stream);
               stream << generator.generate_h();
            }
            {
               std::ofstream stream(paths.struct_serialize.c / std::filesystem::path("serialize_"s + definition.name + ".c"));
               assert(!!stream);
               stream << generator.generate_c();
            }
         }
      }
   }

   static void _generate_sector_group_code(const computed_paths& paths, codegen::results_report& report) {
      auto& reg        = registry::get();
      auto& all_groups = reg.all_sector_groups();
      //
      for (auto& group : all_groups) {
         codegen::sector_generator gen;
         gen.function_name_fragment = group.name;
         gen.sector_serialize_header_folder       = paths.sector_serialize.rel.generic_string();
         gen.whole_struct_serialize_header_folder = paths.struct_serialize.rel.generic_string();
         {
            std::vector<const ast::structure*> structures;
            for (auto& entry : group.top_level_structs) {
               structures.push_back(entry.definition);
            }
            gen.prepare(structures);
         }
         auto output = gen.generate();

         report.add_to_sector_group_report(group, gen);

         {
            std::ofstream stream(paths.sector_serialize.h / std::filesystem::path(group.name + ".h"));
            assert(!!stream);
            stream << output.header;
         }
         {
            std::ofstream stream(paths.sector_serialize.c / std::filesystem::path(group.name + ".c"));
            assert(!!stream);
            stream << output.implementation;
         }
      }
   }

   static void _generate_sector_group_savegame_functors(const computed_paths& paths) {
      auto& reg        = registry::get();
      auto& all_groups = reg.all_sector_groups();
      
      {  // Header
         std::ofstream stream(paths.savegame_integration.h / "save_functors.h");
         assert(!!stream);
         stream << "#include \"global.h\"\n\n";

         for (auto& group : all_groups) {
            stream << "extern bool8 ReadSector_";
            stream << group.name;
            stream << "(u8 sliceNum, const u8* src);\n";
            stream << "extern bool8 WriteSector_";
            stream << group.name;
            stream << "(u8 sliceNum, u8* dst);\n";
         }
      }
      {  // Implementation
         std::ofstream stream(paths.savegame_integration.c / "save_functors.c");
         assert(!!stream);
         stream << "#include \"global.h\"\n\n";

         stream << "// sector-group serialize functions: \n";
         for (auto& group : all_groups) {
            auto str = paths.sector_serialize.rel.generic_string();

            stream << "#include \"";
            stream << str;
            if (!str.empty() && str.back() != '/')
               stream << "/";
            stream << group.name;
            stream << ".h\"\n";
         }
         stream << '\n';

         {
            std::vector<std::string> headers;
            for (auto& group : all_groups) {
               for (auto& entry : group.top_level_structs) {
                  if (entry.c_variable.header != "global.h")
                     headers.push_back(entry.c_variable.header);
               }
            }
            if (!headers.empty()) {
               headers.erase(std::unique(headers.begin(), headers.end()), headers.end());
               stream << "// globals to serialize:\n";
               for (auto& header : headers) {
                  stream << "#include \"";
                  stream << header;
                  stream << "\"\n";
               }
               stream << '\n';
            }
         }

         for (auto& group : all_groups) {
            codegen::serialization_function_body::function_pair pair;

            pair += "extern bool8 ";
            pair.read  += "Read";
            pair.write += "Write";
            pair += "Sector_";
            pair += group.name;
            pair += "(u8 sliceNum, ";
            pair.read  += "const u8* src";
            pair.write += "u8* dst";
            pair += ") {\n";
         
            {
               bool first = true;
               for (size_t i = 0; i < group.max_sector_count; ++i) {
                  if (!first) {
                     pair += "   } else ";
                  } else {
                     pair  += "   ";
                     first  = false;
                  }
                  pair += "if (sliceNum == ";
                  pair += lu::strings::from_integer(i);
                  pair += ") {\n";
                  pair += "      lu_";
                  pair.read  += "Read";
                  pair.write += "Write";
                  pair += "SaveSector_";
                  pair += group.name;
                  if (i < 10) {
                     pair += '0';
                  }
                  pair += lu::strings::from_integer(i);
                  pair += "(";
                  pair.read  += "src, ";
                  pair.write += "dst, ";
                  for (size_t j = 0; j < group.top_level_structs.size(); ++j) {
                     auto& entry = group.top_level_structs[j];
                     if (!entry.c_variable.is_pointer) {
                        pair += '&';
                     }
                     pair += entry.c_variable.name;
                     if (j + 1 < group.top_level_structs.size()) {
                        pair += ", ";
                     }
                  }
                  pair += ");\n";
               }
               pair += "   } else {\n";
               pair += "      return FALSE;\n";
               pair += "   }\n";
               pair += "   return TRUE;\n";
            }
            pair += "}\n";

            stream << pair.read;
            stream << pair.write;
            stream << '\n';
         }
      }
   }
}

namespace codegen {
   extern void generate_all(
      std::filesystem::path base_h,
      std::filesystem::path base_c,
      std::filesystem::path rel_struct_members,
      std::filesystem::path rel_struct_serialize,
      std::filesystem::path rel_sector_serialize,
      std::filesystem::path rel_save_integrations,
      std::filesystem::path results_report_path
   ) {
      computed_paths paths = {
         .struct_members = base_h / rel_struct_members,

         .struct_serialize     = computed_paths::rel_set(base_h, base_c, rel_struct_serialize),
         .sector_serialize     = computed_paths::rel_set(base_h, base_c, rel_sector_serialize),
         .savegame_integration = computed_paths::rel_set(base_h, base_c, rel_save_integrations),
      };
      paths.force_create_paths();

      auto& reg         = registry::get();
      auto& all_structs = reg.all_structs();
      auto& all_groups  = reg.all_sector_groups();

      results_report report;
      report.generate_overall_stats(all_groups);

      _generate_struct_code(paths);
      _generate_sector_group_code(paths, report);

      {  // Savegame slotted sectors table
         std::ofstream stream(paths.savegame_integration.h / "save_functor_table.h");
         assert(!!stream);
         stream << "// generated code\n\n";

         size_t sector = 0;
         for (auto& group : all_groups) {
            for (size_t slice = 0; slice < group.max_sector_count; ++sector, ++slice) {
               stream << "   SAVEBLOCK_LAYOUT_ITEM(";
               stream << slice;
               stream << ", ";
               stream << group.name;
               stream << "),";
               switch (sector) {
                  case 0: stream << " // SECTOR_ID_SAVEBLOCK2"; break;
                  case 1: stream << " // SECTOR_ID_SAVEBLOCK1_START"; break;
                  case 4: stream << " // SECTOR_ID_SAVEBLOCK1_END"; break;
                  case 5: stream << " // SECTOR_ID_PKMN_STORAGE_START"; break;
                  case 13: stream << " // SECTOR_ID_PKMN_STORAGE_END"; break;
               }
               stream << '\n';
            }
         }
      }

      _generate_sector_group_savegame_functors(paths);

      report.generate_stats_for_every_struct(all_groups);
      report.generate_stats_for_heritables(all_groups);
      //
      {
         auto out_path = results_report_path;
         if (out_path.empty()) {
            out_path = paths.sector_serialize.h / "README.md";
         }
         {
            auto out_folder = out_path;
            out_folder.remove_filename();
            std::error_code error;
            if (!std::filesystem::create_directories(out_folder, error))
               assert(!error);
         }

         std::ofstream stream(out_path);
         assert(!!stream);
         
         if (report.any_sector_group_overflowed) {
            stream << "# Last failed codegen\n";
            stream << "This report describes the last failed attempt at code generation.\n\n"; // TODO: include date
         } else {
            stream << "# Codegen for bitpacked savedata\n";
            stream << "This report describes the last successful attempt at code generation.\n\n"; // TODO: include date
            stream << "In vanilla `pokeemerald`, the `SaveBlock1` (world state) and `SaveBlock2` (character state) structs consume "
               "99% of the space allotted to them in flash memory (savedata). This is because they are blindly `memcpy`'d "
               "from RAM. A bitpacked format would consume substantially less space; however, maintaining the code to bitpack "
               "these structs would be prohibitively difficult by hand because savedata is split into ~4KiB strips (\"sectors\"). "
               "Instead, we use custom code generation to produce the serialization code, minding sector boundaries.\n\n";
         }

         stream << "## Overall stats\n";
         stream << report.overall_stats;
         stream << "\n\n";
         stream << "## Struct stats";
         stream << report.struct_table;
         stream << "\n\n";
         for (const auto& s : report.by_sector)
            stream << s;
         stream << "\n\n## Every struct\n";
         stream << "The \"Savings\" columns are measured in bytes. The percentage in the \"Savings per\" column indicates by "
            "how much the struct itself has been made smaller thanks to bitpacking. The percentage in the \"Savings "
            "total\" column indicates by how much the total size of all serialized data has been made smaller as a "
            "result of bitpacking that row's specific struct type.\n";
         stream << report.every_struct;
         stream << "\n\n## Every heritable data definition\n";
         stream << report.every_heritable;
      }
   }
}
