#pragma once
#include <string>

namespace ast {
   class blind_union {
      public:
         std::string name;
         size_t      size_in_bytes = 0;
   };
}
