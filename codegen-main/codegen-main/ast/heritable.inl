#pragma once
#include "./heritable.h"

namespace ast {
   constexpr bool heritable::is_integral() const {
      if (this->integral_type.has_value())
         return true;
      if (this->tagname == "number")
         return true;
      return false;
   }
   constexpr bool heritable::is_string() const {
      if (this->tagname == "string")
         return true;
      return false;
   }
}