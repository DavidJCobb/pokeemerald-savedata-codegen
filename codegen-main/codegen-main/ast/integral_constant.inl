#pragma once
#include "./integral_constant.h"

#include "lu/strings/from_integer.h"

namespace ast {
   template<typename UnderlyingType>
   constexpr const std::string integral_constant<UnderlyingType>::to_string() const {
      if (std::holds_alternative<std::string>(value))
         return std::get<std::string>(value);

      return lu::strings::from_integer(std::get<UnderlyingType>(value));
   }
}