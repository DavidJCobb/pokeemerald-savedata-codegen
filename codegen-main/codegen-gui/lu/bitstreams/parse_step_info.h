#pragma once
#include <string>

namespace lu::bitstreams {
   struct parse_step_info {
      std::string type_name;
      size_t      index = (size_t)-1;
   };
}