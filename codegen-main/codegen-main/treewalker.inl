#pragma once
#include "treewalker.h"

template<typename T>
void treewalker::_write_arg(std::string& dst, T&& arg, bool is_final) {
   if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>) {
      dst += arg;
   } else if constexpr (std::is_integral_v<T>) {
      dst += lu::strings::from_integer(arg);
   } else if constexpr (ast::is_integral_constant<T>) {
      dst += arg.as_c_expression();
   }
   if (!is_final)
      dst += ", ";
}

template<typename... T>
void treewalker::_write_call(std::string& dst, const std::string_view function_name, T&&... args) {
   this->_write_indent(dst);
   dst += function_name;
   dst += '(';

   size_t i = 0;
   (this->_write_arg(dst, args, ++i == sizeof...(args)), ...);

   dst += ");\n";
}

template<typename... T>
void treewalker::_write_assign(std::string& dst, const std::string_view assign_to, const std::string_view function_name, T&&... args) {
   this->_write_indent(dst);
   dst += assign_to;
   dst += " = ";
   dst += function_name;
   dst += '(';

   size_t i = 0;
   (this->_write_arg(dst, args, ++i == sizeof...(args)), ...);

   dst += ");\n";
}