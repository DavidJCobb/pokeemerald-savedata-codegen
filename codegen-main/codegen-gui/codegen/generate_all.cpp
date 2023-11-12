#include "./generate_all.h"
#include <fstream>
#include "./sector_generator.h"
#include "./struct_body_serializer.h"
#include "./struct_serialization_function_generator.h"

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
      };

      std::filesystem::path struct_members;

      rel_set struct_serialize;
      rel_set sector_serialize;
      rel_set savegame_integration;
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

   static void _generate_sector_group_code(const computed_paths& paths) {
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

         if (output.sector_count > group.max_sector_count) {
            throw std::runtime_error(
               "Exceeded sector count for sector group "s
               + group.name
               + "; number of sectors is "s
               + lu::strings::from_integer(output.sector_count)
               + ". Code files saved anyway, for you to review output."
            );
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
      std::filesystem::path rel_save_integrations
   ) {
      computed_paths paths = {
         .struct_members = base_h / rel_struct_members,

         .struct_serialize     = computed_paths::rel_set(base_h, base_c, rel_struct_serialize),
         .sector_serialize     = computed_paths::rel_set(base_h, base_c, rel_sector_serialize),
         .savegame_integration = computed_paths::rel_set(base_h, base_c, rel_save_integrations),
      };

      auto& reg         = registry::get();
      auto& all_structs = reg.all_structs();
      auto& all_groups  = reg.all_sector_groups();

      _generate_struct_code(paths);
      _generate_sector_group_code(paths);

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
   }
}
