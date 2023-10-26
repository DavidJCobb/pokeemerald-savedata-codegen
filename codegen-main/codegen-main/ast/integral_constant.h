#pragma once
#include <string>
#include <variant>

namespace ast {
   template<typename UnderlyingType>
   class integral_constant {
      public:
         using underlying_type = UnderlyingType;

      public:
         std::variant<underlying_type, std::string> value;
         
         constexpr const std::string to_string() const;
   };

   using size_constant = integral_constant<size_t>;
}

#include "./integral_constant.inl"