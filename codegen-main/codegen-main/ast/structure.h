#pragma once
#include <string>
#include <vector>
#include "./member.h"

namespace ast {
   class structure {
      public:
         std::string name;
         std::string header;
         std::vector<member> members;
         std::optional<size_t> checksum_member_index;
         struct {
            bool is_packed = false;
         } c_type_info;
   
         constexpr const std::string to_string() const;

         constexpr const std::string as_c_member_declaration_list() const;
   };
}

#include "./structure.inl"