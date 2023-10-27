#pragma once
#include "./structure.h"
#include <algorithm>

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
}
