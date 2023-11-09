#pragma once
#include "./structure.h"
#include <algorithm>
#include "lu/type_traits/function_traits.h"
#include "./member_types/inlined_union_member.h"
#include "./member_types/integral_member.h"
#include "./member_types/struct_member.h"

namespace ast {
   constexpr const std::string structure::to_string() const {
      std::string out;
      out += "struct " + name;
      if (c_type_info.is_packed) {
         out += " __attribute((packed))";
      }
      out += " {\n";
      out += this->as_c_member_declaration_list();
      out += '}';
      out += ';';
         
      return out;
   };

   constexpr const std::string structure::as_c_member_declaration_list() const {
      std::string out;
      for (const auto& member : members) {
         out += "   ";
         out += member->as_c_declaration();
         out += '\n';
      }
      return out;
   };

   constexpr std::vector<std::string> structure::get_all_direct_constant_dependencies() const {
      std::vector<std::string> out;

      for (const auto& member : members) {
         auto list = member->get_all_used_constants();
         for (auto& item : list)
            out.push_back(std::move(item));
      }

      auto it = std::unique(out.begin(), out.end());
      out.erase(it, out.end());
      return out;
   }
   constexpr std::vector<std::string> structure::get_all_direct_struct_dependencies() const {
      std::vector<std::string> out;
      for (const auto& member : members) {
         if (auto* casted = dynamic_cast<const struct_member*>(member.get())) {
            out.push_back(casted->type_name);
            continue;
         }
         if (auto* casted = dynamic_cast<const inlined_union_member*>(member.get())) {
            auto list = casted->get_all_direct_struct_dependencies();
            for (const auto& item : list) {
               {
                  bool already = false;
                  for (auto& prior : out) {
                     if (prior == item) {
                        already = true;
                        break;
                     }
                  }
                  if (already)
                     continue;
               }
               out.push_back(item);
            }
         }
      }
      return out;
   }
   constexpr bool structure::has_any_string_members() const {
      for (const auto& member : members) {
         if (dynamic_cast<const string_member*>(member.get()))
            return true;
         if (auto* casted = dynamic_cast<const inlined_union_member*>(member.get()))
            if (casted->has_any_string_members())
               return true;
      }
      return false;
   }

   constexpr size_t structure::compute_total_bitcount() const {
      size_t bitcount = 0;
      for (const auto& member_ptr : this->members) {
         bitcount += member_ptr->compute_total_bitcount();
      }
      return bitcount;
   }

   constexpr size_t structure::compute_unpacked_bytecount() const {
      return this->count_unpacked_bytecounts([](const member*, size_t, size_t, size_t, size_t) {});
   }

   template<typename Functor>
   constexpr size_t structure::count_unpacked_bytecounts(Functor&& functor) const requires impl::_structure::is_valid_bytecounter<Functor> {
      constexpr const bool track_accessor = impl::_structure::is_bytecounter_wants_accessor<Functor>;
      //
      using accessor_tracker_t = std::conditional_t<
         impl::_structure::is_bytecounter_wants_accessor<Functor>,
         std::string,
         int
      >;

      size_t offset        = 0;
      size_t bitfield_bits = 0;
      for (const auto& member_ptr : this->members) {
         accessor_tracker_t accessor = {};
         if constexpr (track_accessor) {
            const auto* member = member_ptr.get();
            while (auto* casted = dynamic_cast<const inlined_union_member*>(member)) {
               accessor += member->name;
               accessor += '.';
               member = &casted->get_member_to_serialize();
            }
            accessor += member->name;
         }

         size_t this_bitfield = 0;
         if (auto* casted = dynamic_cast<const integral_member*>(member_ptr.get())) {
            if (casted->c_bitfield.has_value()) {
               this_bitfield = casted->c_bitfield.value().value;
            }
         }
         if (this_bitfield) {
            //
            // C doesn't support bitfield arrays or array bitfields, so no need to handle array ranks/extents here.
            //
            if (bitfield_bits > 8) {
               offset += (bitfield_bits / 8);
               bitfield_bits %= 8;
            }
            if constexpr (track_accessor) {
               functor(accessor, member_ptr.get(), offset, bitfield_bits, 0, this_bitfield);
            } else {
               functor(member_ptr.get(), offset, bitfield_bits, 0, this_bitfield);
            }
            bitfield_bits += this_bitfield;
            continue;
         } else {
            if (bitfield_bits)
               offset += (bitfield_bits / 8) + ((bitfield_bits % 8) ? 1 : 0);
            bitfield_bits = 0;
         }

         if (!this->c_type_info.is_packed) {
            size_t align    = member_ptr->compute_unpacked_alignment();
            size_t misalign = offset % align;
            if (misalign) {
               offset += (align - misalign);
            }
         }

         size_t bytecount = member_ptr->compute_total_unpacked_bytecount();
         if constexpr (track_accessor) {
            functor(accessor, member_ptr.get(), offset, 0, bytecount, 0);
         } else {
            functor(member_ptr.get(), offset, 0, bytecount, 0);
         }
         offset += bytecount;
      }

      if (bitfield_bits)
         offset += (bitfield_bits / 8) + ((bitfield_bits % 8) ? 1 : 0);

      if (!this->c_type_info.is_packed) {
         size_t padding = offset % 4;
         if (padding) {
            offset += 4 - padding;
         }
      }

      functor(nullptr, offset, 0, 0, 0);

      return offset;
   }
}
