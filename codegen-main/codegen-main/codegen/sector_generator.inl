#pragma once
#include "./sector_generator.h"
#include <algorithm>
#include <bit>
#include "lu/strings/from_integer.h"
#include "./ast/member_types/_all.h"

#include "./serialization_function_body.h"

namespace codegen {
   constexpr sector_generator::~sector_generator() {
      for (auto& sector_items : this->items_by_sector) {
         for (auto* item : sector_items)
            delete item;
         sector_items.clear();
      }
      this->items_by_sector.clear();
   }

   constexpr std::vector<serialization_item::item_list> sector_generator::perform_expansions_and_splits(serialization_item::item_list src) const {
      std::vector<serialization_item::item_list> out;
      out.push_back({});

      size_t current_bits = 0;
      for (size_t i = 0; i < src.size(); ++i) {
         auto* item = src[i];

         auto bitcount = item->bitcount();
         if (bytespan_of(current_bits + bitcount) < max_bytes_per_sector) {
            current_bits += bitcount;
            out.back().push_back(item);
            continue;
         }

         auto expanded = item->expand();
         if (!expanded.empty()) {
            src.insert(src.erase(src.begin() + i), expanded.begin(), expanded.end());
            delete item;

            // Repeat this iteration.
            --i;
            continue;
         }

         // Expansion failed, i.e. this item cannot be expanded. Move to the next sector and try again.
         current_bits = 0;
         out.push_back({});
         --i;
      }

      return out;
   }

   constexpr void sector_generator::prepare(const std::vector<const ast::structure*>& structures) {
      this->items_by_sector.clear();
      this->top_level_structs = structures;
      
      serialization_item::item_list base_list;
      for (auto* s : structures) {
         auto item = serialization_item::for_top_level_struct(*s);
         base_list.push_back(new serialization_item(item));
      }
      this->items_by_sector = std::move(perform_expansions_and_splits(base_list));
   }

   constexpr sector_generator::output sector_generator::generate() const {
      output out;
      out.sector_count = items_by_sector.size();

      auto sector_id_to_function_decl = [this](size_t i, bool is_read) -> std::string {
         std::string out = "void lu_";
         out += (is_read) ? "Read" : "Write";
         out += "SaveSector_";
         out += this->function_name_fragment;

         std::string id = lu::strings::from_integer(i);
         while (id.size() < 2)
            id.insert(id.begin(), '0');
         out += id;

         out += "(";
         if (is_read) {
            out += "const ";
         }
         out += "u8* ";
         if (is_read) {
            out += "src";
         } else {
            out += "dst";
         }
         out += ", ";
         //
         // TODO: Only take args that are actually used within this sector.
         //
         for (size_t j = 0; j < this->top_level_structs.size(); ++j) {
            const auto* s = this->top_level_structs[j];
            if (!is_read) {
               out += "const ";
            }
            if (!s->c_type_info.is_defined_via_typedef) {
               out += "struct ";
            }
            out += s->name;
            out += "* p_";
            out += s->name;
            if (j + 1 < this->top_level_structs.size()) {
               out += ", ";
            }
         }
         out += ")";
         return out;
      };

      {
         out.header += "#ifndef GUARD_LU_SERIALIZE_SECTOR_" + this->function_name_fragment;
         out.header += "\n#define GUARD_LU_SERIALIZE_SECTOR_" + this->function_name_fragment;
         out.header += "\n\n";

         out.header += "#include \"lu/bitstreams.h\"\n\n";

         // need to forward-declare structs outside of parameter lists
         bool included_global = false;
         for (auto* s_def : this->top_level_structs) {
            if (s_def->c_type_info.is_defined_via_typedef) {
               //
               // Cannot forward-declare structs defined as `typedef struct { ... } name`.
               //
               if (!s_def->header.empty()) {
                  if (!included_global && s_def->header != "global.h") {
                     out.header += "#include \"global.h\"\n"; // lots of pokeemerald stuff breaks if this isn't included first/at all
                  }
                  out.header += "#include \"";
                  out.header += s_def->header;
                  out.header += "\"\n";
               }
               continue;
            }
            out.header += "struct ";
            out.header += s_def->name;
            out.header += ";\n";
         }
         out.header += "\n";

         for (size_t i = 0; i < items_by_sector.size(); ++i) {
            out.header += sector_id_to_function_decl(i, true); // read
            out.header += ";\n";
            out.header += sector_id_to_function_decl(i, false); // write
            out.header += ";\n";
         }

         out.header += "\n#endif";
      }

      out.implementation += "#include \"";
      out.implementation += this->sector_serialize_header_folder;
      if (!this->sector_serialize_header_folder.empty()) {
         if (this->sector_serialize_header_folder.back() != '/')
            out.implementation += '/';
      }
      out.implementation += this->function_name_fragment;
      out.implementation += ".h\"\n\n";
      {
         std::vector<const ast::structure*> dependencies;
         std::string code_read;
         std::string code_write;

         for (size_t i = 0; i < items_by_sector.size(); ++i) {
            auto& item_list = items_by_sector[i];

            code_read  += sector_id_to_function_decl(i, true);
            code_write += sector_id_to_function_decl(i, false);
            code_read  += " {\n";
            code_write += " {\n";

            serialization_function_body body;
            body.item_list         = item_list;
            body.state_is_argument = false;
            body.generate();
            //
            code_read  += body.out.read;
            code_write += body.out.write;

            code_read  += "};\n\n";
            code_write += "};\n\n";

            for (auto* dep : body.results.direct_dependencies) {
               dependencies.push_back(dep);
            }
         }

         if (!dependencies.empty()) {
            auto it = std::unique(dependencies.begin(), dependencies.end());
            dependencies.erase(it, dependencies.end());

            out.implementation += "// whole-struct serialize funcs:\n";
            for (const auto* structure : dependencies) {
               out.implementation += "#include \"";
               {
                  const auto& folder = this->whole_struct_serialize_header_folder;
                  if (!folder.empty()) {
                     out.implementation += folder;
                     if (folder.back() != '/')
                        out.implementation += '/';
                  }
               }
               out.implementation += "serialize_";
               out.implementation += structure->name;
               out.implementation += ".h\"\n";
            }
            out.implementation += '\n';
         }
         out.implementation += code_read;
         out.implementation += code_write;
      }

      return out;
   }
}