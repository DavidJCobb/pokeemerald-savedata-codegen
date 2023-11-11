#pragma once
#include <string>

namespace ast {
   class structure;
}

namespace codegen {
   class struct_serialization_function_generator {
      public:
         const ast::structure& definition;
         std::string destination_folder; // relative to your `include` and `src` directories

         std::string generate_h() const;
         std::string generate_c() const;
   };
}