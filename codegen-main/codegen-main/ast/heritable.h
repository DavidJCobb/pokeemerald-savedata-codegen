#pragma once
#include <variant>
#include "./semantic_type_info.h"

namespace ast {
   class heritable {
      public:
         std::string name;
         std::variant<
            semantic::number_info,
            semantic::string_info
         > semantic_info;

         std::optional<c_type::primitive> number_c_type;
   };
}
