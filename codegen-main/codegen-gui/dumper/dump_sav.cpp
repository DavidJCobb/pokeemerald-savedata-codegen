#include "./dump_sav.h"
#include <array>
#include <cstdint>
#include <vector>
#include "../registry.h"
#include "../struct_bitwalker.h"
#include "../struct_bytewalker.h"

using namespace std::literals::string_literals;

namespace {
   constexpr const size_t sector_full_size   = 4096;
   constexpr const size_t sector_footer_size = 128;
   constexpr const size_t sector_data_size   = sector_full_size - sector_footer_size;

   constexpr const uint32_t valid_sector_signature = 0x08012025;

   constexpr const size_t slots_per_save = 2;

   using sector_group = registry::sector_group;
   
   struct sector_footer {
      uint8_t  unused[sector_footer_size - 12];
      uint16_t id;
      uint16_t checksum;
      uint32_t signature;
      uint32_t counter;
   };

   struct stitched_sector_group {
      const sector_group* definition = nullptr;
      //
      uint8_t* buffer = nullptr;
      uint16_t pieces = 0; // mask

      ~stitched_sector_group() {
         if (buffer)
            delete buffer;
      }

      constexpr bool is_complete() const {
         auto mask = (1 << this->definition->max_sector_count) - 1;
         return (pieces & mask) == mask;
      }

      constexpr size_t size_if_complete() const {
         return sector_data_size * this->definition->max_sector_count;
      }

      void stitch(const void* src, uint8_t piece) {
         if (pieces & (1 << piece)) { // we already loaded this piece?
            return;
         }
         //assert((pieces & (1 << piece)) == 0);
         if (!buffer)
            buffer = (uint8_t*)malloc(size_if_complete());

         memcpy(buffer + (piece * sector_data_size), src, sector_data_size);
         pieces |= (1 << piece);
      }
   };

   struct slot {
      std::vector<stitched_sector_group> loaded_groups;
   };
}

static std::string _dump_structs(const void* data, size_t max_size, bool bitpacked, const std::vector<const ast::structure*>& structures) {
   if (bitpacked) {
      struct_bitwalker walker;
      walker.set_buffer(data, max_size);
      walker.walk(structures);
      return walker.output;
   } else {
      struct_bytewalker walker;
      walker.set_buffer(data, max_size);
      walker.walk(structures);
      return walker.output;
   }
}

extern std::vector<std::string> dump_sav(const void* sav, size_t sav_size, bool bitpacked) {
   auto&  reg         = registry::get();
   auto&  all_groups  = reg.all_sector_groups();
   size_t group_count = all_groups.size();

   std::string seen_ids;
   std::array<slot, slots_per_save> slots;
   {
      for (auto& slot : slots) {
         slot.loaded_groups.resize(group_count);
         for (size_t i = 0; i < group_count; ++i) {
            slot.loaded_groups[i].definition = &all_groups[i];
         }
      }
   }

   size_t sector_count = sav_size / sector_full_size;
   for (size_t i = 0; i < sector_count; ++i) {
      const void* sector = (uint8_t*)sav + (i * sector_full_size);
      const auto* foot   = (const sector_footer*)((uint8_t*)sector + sector_data_size);
      if (foot->signature != valid_sector_signature)
         continue;

      seen_ids += "Sector ";
      seen_ids += lu::strings::from_integer(i);
      seen_ids += " has ID ";
      seen_ids += lu::strings::from_integer(foot->id);
      seen_ids += ".\n";

      if (i > 14 * 2) { // special sector; does not cycle/rotate; is not tied to any one slot
         // TODO: If we want, dump HoF, Trainer Hill, and Recorded Battle here.
         continue;
      }
      uint16_t slot_local_id = foot->id;
      uint16_t slot_index    = 0;
      if (i >= 14) {
         slot_local_id %= 14;
         slot_index     = 1;
      }

      auto& target_slot = slots[slot_index];
      for (auto& group : target_slot.loaded_groups) {
         size_t start = group.definition->sector_base_id;
         size_t end   = start + group.definition->max_sector_count;

         if (slot_local_id >= start && slot_local_id < end) {
            group.stitch(sector, slot_local_id - start);
            continue;
         }
      }
   }

   std::vector<std::string> out;
   if (!seen_ids.empty()) {
      out.push_back(seen_ids);
   }
   for (size_t i = 0; i < slots_per_save; ++i) {
      std::string header = std::string("# Slot ") + lu::strings::from_integer(i + 1) + "\n\n";

      auto& slot = slots[i];
      for (auto& group : slot.loaded_groups) {
         if (group.is_complete()) {
            std::vector<const ast::structure*> to_dump;
            for (const auto& entry : group.definition->top_level_structs) {
               to_dump.push_back(entry.definition);
            }

            try {
               out.push_back("## "s + group.definition->name + "\n\n" + _dump_structs(
                  group.buffer,
                  group.size_if_complete(),
                  bitpacked,
                  to_dump
               ));
            } catch (lu::bitstreams::exceptions::read_exception& e) {
               std::string entry = "## "s + group.definition->name + "\n\n";
               entry += "Bitstream read exception occurred: \n";
               entry += "Position: ";
               entry += lu::strings::from_integer(e.position.bytes);
               entry += " bytes + ";
               entry += lu::strings::from_integer(e.position.bits);
               entry += " bits\nTrace:\n";
               for (auto& detail : e.where_were_we) {
                  entry += "* ";
                  entry += detail.type_name;
                  if (detail.index != (size_t)-1) {
                     entry += '[';
                     entry += lu::strings::from_integer(detail.index);
                     entry += ']';
                  }
                  entry += '\n';
               }
               entry += '\n';
            }
         } else {
            out.push_back("## "s + group.definition->name + "\n\nIncomplete/missing!");
         }
      }
   }
   return out;
}