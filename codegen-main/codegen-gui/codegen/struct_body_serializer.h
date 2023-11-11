#pragma once
#include <string>
#include "../ast/structure.h"

namespace codegen {
   class struct_body_serializer {
      public:
         const ast::structure& definition;

      public:
         constexpr std::string generate();
   };
}

#include "./struct_body_serializer.inl"