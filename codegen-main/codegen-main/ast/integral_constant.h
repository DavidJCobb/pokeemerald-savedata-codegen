#pragma once
#include <optional>
#include <string>
#include "lu/strings/from_integer.h"

namespace ast {
   template<typename UnderlyingType>
   class integral_constant {
      public:
         using underlying_type = UnderlyingType;

      public:
         std::string     preprocessor_name; // optional
         underlying_type value = 0; // required

         constexpr std::string as_c_expression() const {
            if (!preprocessor_name.empty())
               return preprocessor_name;
            return lu::strings::from_integer(value);
         }
   };

   using size_constant = integral_constant<size_t>;
}