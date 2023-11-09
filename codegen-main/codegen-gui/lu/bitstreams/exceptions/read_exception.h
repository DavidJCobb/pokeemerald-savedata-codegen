#pragma once
#include "./any.h"
#include "../position.h"

namespace lu::bitstreams::exceptions {
   class read_exception : public any {
      public:
         using position_type = typename position;
      public:
         read_exception(const position_type& p) : position(p) {}

         const position_type position;
   };
}