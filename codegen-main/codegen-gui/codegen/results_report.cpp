#include "./results_report.h"
#include <map>
#include <unordered_map>
#include "./sector_generator.h"
#include "../registry.h"

namespace codegen {
   void results_report::generate_overall_stats(const std::vector<registry::sector_group>& all_groups) {
      size_t sector_count = 0;
      for (auto& info : all_groups)
         sector_count += info.max_sector_count;

      size_t total_packed_bits = 0;
      this->struct_table += '\n';
      this->struct_table += "| Name | Bytes in RAM | Packed bits | Packed bytes | Savings |\n";
      this->struct_table += "| - | -: | -: | -: | -: |\n";
      for (const auto& group : all_groups) {
         for (const auto& entry : group.top_level_structs) {
            const auto* definition = entry.definition;
            
            size_t bytes_in_ram = definition->compute_unpacked_bytecount();
            size_t packed_bits  = definition->compute_total_bitcount();
            size_t packed_bytes = packed_bits / 8 + ((packed_bits % 8) ? 1 : 0);
            this->struct_table += "| ";
            this->struct_table += definition->name;
            this->struct_table += " | ";
            this->struct_table += lu::strings::from_integer(bytes_in_ram);
            this->struct_table += " | ";
            this->struct_table += lu::strings::from_integer(packed_bits);
            this->struct_table += " |";
            this->struct_table += lu::strings::from_integer(packed_bytes);
            this->struct_table += " | ";
            this->struct_table += lu::strings::from_integer(bytes_in_ram - packed_bytes);
            this->struct_table += " (";
            {
               float perc = (float)packed_bytes / (float)bytes_in_ram;
               this->struct_table += lu::strings::from_integer((int64_t)(100.0 - perc * 100.0));
            }
            this->struct_table += "%)";
            this->struct_table += " |\n";
            this->total_bytes_in_ram += bytes_in_ram;
            total_packed_bits  += packed_bits;
         }
      }

      this->struct_table += "| **Total** | ";
      this->struct_table += lu::strings::from_integer(this->total_bytes_in_ram);
      this->struct_table += " | ";
      this->struct_table += lu::strings::from_integer(total_packed_bits);
      this->struct_table += " |";
      {
         size_t total_packed_bytes = total_packed_bits / 8 + ((total_packed_bits % 8) ? 1 : 0);
         this->struct_table += lu::strings::from_integer(total_packed_bytes);
         this->struct_table += " | ";
         this->struct_table += lu::strings::from_integer(this->total_bytes_in_ram - total_packed_bytes);
         this->struct_table += " (";
         {
            float perc = (float)total_packed_bytes / (float)this->total_bytes_in_ram;
            this->struct_table += lu::strings::from_integer((int64_t)(100.0 - perc * 100.0));
         }
         this->struct_table += "%)";
         this->struct_table += " |\n";
      }
      this->overall_stats += lu::strings::from_integer(this->total_bytes_in_ram);
      this->overall_stats += " bytes in RAM / ";
      this->overall_stats += lu::strings::from_integer(codegen::sector_generator::max_bytes_per_sector * sector_count);
      this->overall_stats += " bytes available across ";
      this->overall_stats += lu::strings::from_integer(sector_count);
      this->overall_stats += " sectors (";
      {
         float perc = (float)this->total_bytes_in_ram / (float)(codegen::sector_generator::max_bytes_per_sector * sector_count);
         this->overall_stats += lu::strings::from_integer((int)(perc * 100.0));
      }
      this->overall_stats += "% space usage)  \n";
      this->overall_stats += lu::strings::from_integer(total_packed_bits);
      this->overall_stats += " packed bits = ";
      {
         size_t bytespan = (total_packed_bits / 8) + ((total_packed_bits % 8) ? 1 : 0);
         this->overall_stats += lu::strings::from_integer(bytespan);
         this->overall_stats += " packed bytes (";
         {
            float perc = (float)bytespan / (float)(codegen::sector_generator::max_bytes_per_sector * sector_count);
            this->overall_stats += lu::strings::from_integer((int)(perc * 100.0));
         }
      }
      this->overall_stats += "% space usage)  \n";
   }

   void results_report::add_to_sector_group_report(
      const registry::sector_group& group,
      const codegen::sector_generator& generator
   ) {
      auto& sector_report = this->by_sector.emplace_back();
      sector_report = "## Sector group: " + group.name + "\n";
      bool this_sector_group_failed = generator.items_by_sector.size() > group.max_sector_count;
      if (this_sector_group_failed) {
         this->any_sector_group_overflowed = true;
         sector_report += "**Status:** Failed; overflowed max sector count, which was ";
         sector_report += lu::strings::from_integer(group.max_sector_count);
         sector_report += ".\n\n";
      } else {
         sector_report += "**Status:** Successfully packed into ";
         sector_report += lu::strings::from_integer(group.max_sector_count);
         sector_report += " sectors.\n\n";
      }
      sector_report += '\n';
      sector_report += "| Sector # | Bits used | Bits available | % |\n";
      sector_report += "| - | -: | -: |\n";
      //
      size_t total_packed_bits = 0;
      for (size_t i = 0; i < generator.items_by_sector.size(); ++i) {
         const auto& sector_items = generator.items_by_sector[i];
         size_t bits_per_sector = 0;
         for (const auto* item : sector_items) {
            bits_per_sector += item->bitcount();
         }
         total_packed_bits += bits_per_sector;
         sector_report += "| ";
         sector_report += lu::strings::from_integer(i);
         sector_report += " | ";
         sector_report += lu::strings::from_integer(bits_per_sector);
         sector_report += " | ";
         {
            size_t available = codegen::sector_generator::max_bytes_per_sector * 8;
            sector_report += lu::strings::from_integer(available);
            sector_report += " | ";
            {
               auto perc = (float)bits_per_sector / (float)available;
               sector_report += lu::strings::from_integer((int)(perc * 100.0));
               sector_report += '%';
            }
         }
         sector_report += " |\n";
      }
      sector_report += "| Total | ";
      sector_report += lu::strings::from_integer(total_packed_bits);
      sector_report += " | ";
      {
         auto total_available = codegen::sector_generator::max_bytes_per_sector * 8 * generator.items_by_sector.size();
         sector_report += lu::strings::from_integer(total_available);
         sector_report += " | ";
         {
            auto perc = (float)total_packed_bits / (float)total_available;
            sector_report += lu::strings::from_integer((int)(perc * 100.0));
            sector_report += '%';
         }
      }
      sector_report += " |\n\n";
      sector_report += "### Member info\n";
      auto serialization_item_to_field_name = [](const codegen::serialization_item& item) {
         std::string out = item.accessor;
         if (auto* member = item.member_definition) {
            auto& indices = item.array_indices;
            auto& extents = member->array_extents;
            //
            for (auto index : indices) {
               out += '[';
               out += lu::strings::from_integer(index);
               out += ']';
            }
            for (size_t i = indices.size(); i < extents.size(); ++i) {
               out += "[*]";
            }
         }
         return out;
      };
      for (size_t i = 0; i < generator.items_by_sector.size(); ++i) {
         const auto& sector_items = generator.items_by_sector[i];
         sector_report += "* Last field serialized to sector ";
         sector_report += lu::strings::from_integer(i);
         sector_report += ": ";
         if (sector_items.empty()) {
            sector_report += "<none>";
         } else {
            auto* item = sector_items.back();
            sector_report += '`';
            sector_report += serialization_item_to_field_name(*item);
            sector_report += '`';
         }
         sector_report += "\n";
      }
      if (this_sector_group_failed) {
         sector_report += "\n";
         for (size_t i = 0; i < generator.items_by_sector.size(); ++i) {
            sector_report += "#### All fields in sector ";
            sector_report += lu::strings::from_integer(i);
            sector_report += "\n";
            for (auto* item : generator.items_by_sector[i]) {
               sector_report += "* `";
               sector_report += serialization_item_to_field_name(*item);
               sector_report += "`\n";
            }
            sector_report += '\n';
         }
      }
   }

   void results_report::generate_stats_for_every_struct(const std::vector<registry::sector_group>& all_groups) {
      struct struct_info {
         size_t count       = 0;
         size_t ram_bytes   = 0;
         size_t packed_bits = 0;
      };
      std::map<std::string, struct_info> all_structs;
      auto _walk_struct = [&all_structs](this auto&& recurse, const ast::structure& s, const ast::member* m, size_t containing_array_count) -> void {
         auto& entry = all_structs[s.name];
         size_t total_count = 1;
         if (m) {
            for (auto& rank : m->array_extents)
               total_count *= rank.extent.value;
         }
         entry.count += containing_array_count * total_count;
         if (!entry.ram_bytes) {
            entry.ram_bytes = s.compute_unpacked_bytecount();
         }
         if (!entry.packed_bits) {
            entry.packed_bits = s.compute_total_bitcount();
         }
         for (auto& member_ptr : s.members) {
            auto* member = member_ptr.get();
            while (auto* casted = dynamic_cast<ast::inlined_union_member*>(member)) {
               member = &casted->get_member_to_serialize();
            }
            if (auto* casted = dynamic_cast<ast::struct_member*>(member)) {
               recurse(*casted->type_def, casted, containing_array_count * total_count);
            }
         }
      };
      for (const auto& group : all_groups) {
         for (const auto& entry : group.top_level_structs) {
            _walk_struct(*entry.definition, nullptr, 1);
         }
      }
      this->every_struct += '\n';
      this->every_struct += "| Name | Bytes in RAM | Packed bits | Packed bytes | Savings per | Count | Savings total |\n";
      this->every_struct += "| - | -: | -: | -: | -: | -: | -: |\n";
      for (const auto& pair : all_structs) {
         auto& name  = pair.first;
         auto& stats = pair.second;
         size_t packed_bytes = stats.packed_bits / 8 + (stats.packed_bits % 8 ? 1 : 0);
         this->every_struct += "| ";
         this->every_struct += name;
         this->every_struct += " | ";
         this->every_struct += lu::strings::from_integer(stats.ram_bytes);
         this->every_struct += " | ";
         this->every_struct += lu::strings::from_integer(stats.packed_bits);
         this->every_struct += " | ";
         this->every_struct += lu::strings::from_integer(packed_bytes);
         this->every_struct += " | ";
         {  // Savings per
            this->every_struct += lu::strings::from_integer(stats.ram_bytes - packed_bytes);
            this->every_struct += " (";
            {
               float perc = (float)packed_bytes / (float)stats.ram_bytes;
               this->every_struct += lu::strings::from_integer((int64_t)(100.0 - perc * 100.0));
            }
            this->every_struct += "%)";
         }
         this->every_struct += " | ";
         this->every_struct += lu::strings::from_integer(stats.count);
         this->every_struct += " | ";
         {  // Savings total
            size_t numer = packed_bytes * stats.count;
            this->every_struct += lu::strings::from_integer((stats.ram_bytes * stats.count) - numer);
            this->every_struct += " (";
            {
               float perc = (float)numer / (float)total_bytes_in_ram;
               this->every_struct += lu::strings::from_integer((int64_t)(perc * 100.0));
            }
            this->every_struct += "%)";
         }
         this->every_struct += " |\n";
      }
      this->every_struct += '\n';
   }

   void results_report::generate_stats_for_heritables(const std::vector<registry::sector_group>& all_groups) {
      auto& reg = registry::get();

      struct heritable_count_set {
         std::vector<size_t> by_sector_group;
      };
      std::unordered_map<const ast::heritable*, heritable_count_set> heritable_counts;
      
      auto _walk_struct = [&heritable_counts](this auto&& recurse, size_t sector_group_idx, const ast::structure& s, const ast::member* m, size_t containing_array_count) -> void {
         size_t total_count = 1;
         if (m) {
            for (auto& rank : m->array_extents)
               total_count *= rank.extent.value;
         }
         total_count *= containing_array_count;
         for (auto& member_ptr : s.members) {
            auto* member = member_ptr.get();
            while (auto* casted = dynamic_cast<ast::inlined_union_member*>(member)) {
               member = &casted->get_member_to_serialize();
            }
            if (member->inherited_from) {
               auto& list = heritable_counts[member->inherited_from].by_sector_group;
               if (list.size() < sector_group_idx + 1)
                  list.resize(sector_group_idx + 1);
               list[sector_group_idx] += total_count;
            }
            if (auto* casted = dynamic_cast<ast::struct_member*>(member)) {
               recurse(sector_group_idx, *casted->type_def, casted, total_count);
            }
         }
      };
      for (size_t i = 0; i < all_groups.size(); ++i) {
         const auto& group = all_groups[i];
         for (const auto& entry : group.top_level_structs) {
            _walk_struct(i, *entry.definition, nullptr, 1);
         }
      }
      this->every_heritable += "\n"; // GitHub Markdown tables require a blank line above them.
      this->every_heritable += "| Name";
      for (auto& group : all_groups) {
         this->every_heritable += " | Uses in ";
         this->every_heritable += group.name;
      }
      this->every_heritable += " | Total uses |\n";
      //
      this->every_heritable += "| -";
      for (auto& group : all_groups) {
         this->every_heritable += " | -: ";
      }
      this->every_heritable += " | -: |\n";
      //
      for (const auto& pair : reg.all_heritables()) {
         const auto* heritable = pair.second.get();
         heritable_count_set counts;
         {
            auto it = heritable_counts.find(heritable);
            if (it != heritable_counts.end())
               counts = it->second;
         }
         counts.by_sector_group.resize(all_groups.size());
         //
         this->every_heritable += "| ";
         this->every_heritable += heritable->name;
         size_t total = 0;
         for (auto count : counts.by_sector_group) {
            this->every_heritable += " | ";
            this->every_heritable += lu::strings::from_integer(count);
            total += count;
         }
         this->every_heritable += " | ";
         this->every_heritable += lu::strings::from_integer(total);
         this->every_heritable += " |\n";
      }
      this->every_heritable += '\n';
   }
}
