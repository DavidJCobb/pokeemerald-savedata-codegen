#pragma once
#include "./structure.h"

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
         out += member.as_c_declaration();
         out += '\n';
      }
      return out;
   };
}
