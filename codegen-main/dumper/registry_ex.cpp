#include "./registry_ex.h"
#include "codegen-main/lu/strings/from_integer.h"
#include "codegen-main/ast/member_types/_all.h"
#include "codegen-main/ast/blind_union.h"
#include "codegen-main/ast/integral_constant.h"
#include "codegen-main/ast/integral_type.h"
#include "codegen-main/ast/structure.h"

#include "./struct_bytewalker.h"
#include "./struct_bitwalker.h"

registry_ex* registry_ex::instance = nullptr;

std::string registry_ex::dump_unpacked(const void* data, size_t max_size, const std::vector<std::string>& structures) {
   struct_bytewalker walker;
   walker.set_buffer(data, max_size);

   std::vector<const ast::structure*> s_defs;
   for (const auto& name : structures) {
      s_defs.push_back(this->structs[name].get());
   }

   walker.walk(s_defs);

   return walker.output;
}
std::string registry_ex::dump_packed(const void* data, size_t max_size, const std::vector<std::string>& structures) {
   struct_bitwalker walker;
   walker.set_buffer(data, max_size);

   std::vector<const ast::structure*> s_defs;
   for (const auto& name : structures) {
      s_defs.push_back(this->structs[name].get());
   }

   walker.walk(s_defs);

   return walker.output;
}

std::string registry_ex::dump(bool bitpacked, const void* data, size_t max_size, const std::vector<std::string>& structures) {
   if (bitpacked) {
      return dump_packed(data, max_size, structures);
   } else {
      return dump_unpacked(data, max_size, structures);
   }
}

std::vector<std::string> registry_ex::_dump_from_sav(const void* sav, size_t sav_size, bool packed) {
   constexpr const size_t sector_full_size   = 4096;
   constexpr const size_t sector_footer_size = 128;
   constexpr const size_t sector_data_size   = sector_full_size - sector_footer_size;

   constexpr const uint32_t valid_sector_signature = 0x08012025;

   constexpr const size_t slots_per_save = 2;

   struct sector_footer {
      uint8_t  unused[sector_footer_size - 12];
      uint16_t id;
      uint16_t checksum;
      uint32_t signature;
      uint32_t counter;
   };

   struct {
      std::string character_data;
      std::string world_data;
      std::string pokemon_storage; // not yet implemented, since we don't pack it yet
   } output;

   struct stitching {
      uint8_t*  buffer = nullptr;
      uint16_t  pieces = 0; // mask
      //
      uint8_t   piece_count = 0;

      stitching(uint8_t pc) : piece_count(pc) {}
      ~stitching() {
         if (buffer)
            delete buffer;
      }

      constexpr bool is_complete() const {
         auto mask = (1 << piece_count) - 1;
         return (pieces & mask) == mask;
      }

      constexpr size_t size_if_complete() const {
         return sector_data_size * piece_count;
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
      std::string character_data;
      struct {
         stitching world_data      = stitching(4);
         stitching pokemon_storage = stitching(9); // not yet implemented, since we don't pack it yet
      } stitchings;
   };

   std::string seen_ids;
   slot slots[slots_per_save];

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

      if (slot_local_id == 0) {
         slots[slot_index].character_data = dump(packed, sector, sector_data_size, { "SaveBlock2" });
         continue;
      }
      if (slot_local_id >= 1 && slot_local_id <= 4) {
         slots[slot_index].stitchings.world_data.stitch(sector, slot_local_id - 1);
         continue;
      }
      if (slot_local_id >= 5 && slot_local_id <= 13) {
         slots[slot_index].stitchings.pokemon_storage.stitch(sector, slot_local_id - 5);
         continue;
      }
   }

   std::vector<std::string> out;
   if (!seen_ids.empty()) {
      out.push_back(seen_ids);
   }
   for (size_t i = 0; i < slots_per_save; ++i) {
      std::string header = std::string("# Slot ") + lu::strings::from_integer(i + 1) + "\n\n";

      if (slots[i].character_data.empty()) {
         out.push_back(header + "## Character data\n\nIncomplete/missing!");
      } else {
         out.push_back(header + "## Character data\n\n" + std::move(slots[i].character_data));
      }

      auto& pokemon_storage = slots[i].stitchings.pokemon_storage;
      auto& world_data      = slots[i].stitchings.world_data;

      if (pokemon_storage.is_complete()) {
         out.push_back("## Pokemon Storage\n\nDump not implemented!");
      } else {
         out.push_back("## Pokemon Storage\n\nIncomplete/missing!");
      }

      if (world_data.is_complete()) {
         out.push_back("## World data\n\n" + dump(
            packed,
            world_data.buffer,
            world_data.size_if_complete(),
            { "SaveBlock1" }
         ));
      } else {
         out.push_back("## World data\n\nIncomplete/missing!");
      }
   }
   return out;
}

std::vector<std::string> registry_ex::dump_unpacked_from_sav(const void* sav, size_t sav_size) {
   return this->_dump_from_sav(sav, sav_size, false);
}

std::vector<std::string> registry_ex::dump_packed_from_sav(const void* sav, size_t sav_size) {
   return this->_dump_from_sav(sav, sav_size, true);
}