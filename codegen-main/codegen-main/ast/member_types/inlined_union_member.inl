#pragma once
#include "./inlined_union_member.h"
#include "./string_member.h"
#include "./struct_member.h"

namespace ast {
   /*virtual*/ constexpr const std::string inlined_union_member::as_c_type_specifier() const /*override*/ {
      std::string out;
      out = "union {\n";
      for (const auto& nested : this->members) {
         out += "   ";
         out += nested->as_c_declaration();
         out += "\n";
      }
      out += "}";
      return out;
   }
   /*virtual*/ constexpr std::vector<std::string> inlined_union_member::_get_all_used_constants_impl() const /*override*/ {
      std::vector<std::string> out;

      for (const auto& member : this->members) {
         auto list = member->get_all_used_constants();
         for (auto& item : list)
            out.push_back(std::move(item));
      }

      return out;
   }
   /*virtual*/ constexpr size_t inlined_union_member::_compute_single_element_unpacked_bytecount() const /*override*/ {
      return this->get_member_to_serialize().compute_total_unpacked_bytecount();
   }
   /*virtual*/ constexpr std::size_t inlined_union_member::compute_single_element_bitcount() const /*override*/ {
      return this->get_member_to_serialize().compute_single_element_bitcount();
   }
   

   constexpr member& inlined_union_member::get_member_to_serialize() const {
      for (const auto& member : members) {
         if (member->name == this->member_to_serialize)
            return *member.get();
      }
      throw;
   }

   constexpr std::vector<std::string> inlined_union_member::get_all_direct_struct_dependencies() const {
      std::vector<std::string> out;

      auto* member = &this->get_member_to_serialize();
      if (auto* casted = dynamic_cast<const struct_member*>(member)) {
         out.push_back(casted->type_name);
         return out;
      }
      if (auto* casted = dynamic_cast<const inlined_union_member*>(member)) {
         return casted->get_all_direct_struct_dependencies();
      }

      return {};
   }
   constexpr bool inlined_union_member::has_any_string_members() const {
      for (const auto& member : members) {
         if (dynamic_cast<const string_member*>(member.get()))
            return true;
         if (auto* casted = dynamic_cast<const inlined_union_member*>(member.get()))
            if (casted->has_any_string_members())
               return true;
      }
      return false;
   }
}
