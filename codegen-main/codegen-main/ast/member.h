#pragma once
#include <optional>
#include <string>
#include "./c_type.h"
#include "./integral_constant.h"
#include "./semantic_type_info.h"

namespace ast {
   class heritable;
}

namespace ast {
   class member {
      public:
         struct union_info {
            std::string member_to_serialize;
            std::vector<member> members;
         };

      public:
         std::string name;
         std::optional<size_t> explicit_bitcount;
         bool skip_when_serializing = false;
      
         std::variant<
            std::monostate,
            semantic::number_info,
            semantic::string_info
         > semantic_info;
      
         c_type c_type_info;
         std::string c_line_comment;

         std::optional<union_info> c_union;

         constexpr void inherit_from(const heritable&);
      
         constexpr void resolve_all_specifications();
      
         // must have run resolve_all_specifications() first
         constexpr const std::string as_c_declaration() const {
            if (c_union.has_value()) {
               std::string out = "union {\n";
               for (const auto& m : c_union.value().members) {
                  out += "      " + m.as_c_declaration() + '\n';
               }
               out += "} " + this->name + ";";
               return out;
            }

            auto& cti = c_type_info;
            return cti.as_c_member_prefix() + ' ' + name + cti.as_c_member_postfix();
         }
   };
}

#include "./member.inl"