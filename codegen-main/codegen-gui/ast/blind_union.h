#pragma once
#include <string>

namespace ast {
   class blind_union {
      public:
         std::string name;
         size_t      size_in_bytes = 0;
         struct {
            bool is_defined_via_typedef = false; // if true, you never use the `union` keyword to prefix types of variables
         } c_type_info;
   };
}
