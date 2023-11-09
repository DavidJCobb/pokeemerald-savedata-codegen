#pragma once
#include <string>

namespace ast {
   class constant_definition {
      public:
         std::string   name;
         std::string   header;
         std::intmax_t value = 0;
   };
}