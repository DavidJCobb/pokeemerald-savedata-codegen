#include "./parsing_scaffold.h"
#include <algorithm>
#include <iostream>
#include "../strings/from_integer.h"

namespace lu::rapidxml_helpers {
   size_t parsing_scaffold::_line_of(std::ptrdiff_t offset) const {
      auto& newlines = this->newline_offsets;
      if (newlines.empty())
         return 0;

      for (size_t i = 1; i < newlines.size(); ++i) {
         if (offset < newlines[i]) {
            //
            // We're on Line N, and we want to figure out what N actually is.
            // 
            // We're between `newlines[i - 1]` and `newlines[i]`. (N - 1) == (i - 2), because 
            // N is one-indexed and i is zero-indexed.
            //
            return i + 1;
         }
      }
      return newlines.size() - 1;
   }

   void parsing_scaffold::_warn_at(const std::string& message, size_t line) const {
      std::cout << "XML Warning: Line " << line << ": " << message << "\n";
      std::cout << "   in file: " << this->path << '\n';
   }
   void parsing_scaffold::_error_at(const std::string& message, size_t line) const {
      std::cout << "XML ERROR: Line " << line << ": " << message << "\n";
      std::cout << "   in file: " << this->path << '\n';
      throw std::runtime_error(std::string("XML parse error at line ") + lu::strings::from_integer(line) + ": " + message);
   }

   void parsing_scaffold::file_error(const std::string& message) {
      std::cout << "FILE ERROR: " << message << "\n";
      std::cout << "   in file: " << this->path << '\n';
      throw std::runtime_error("File error: " + message);
   }

   char* parsing_scaffold::get_data() {
      return const_cast<char*>(this->data.c_str());
   }
   void parsing_scaffold::set_data(const std::string& v) {
      this->data = v;

      this->newline_offsets.clear();
      //
      size_t size = this->data.size();
      for (size_t i = 0; i < size; ++i) {
         if (this->data[i] == '\n') {
            this->newline_offsets.push_back(i);
         }
      }
   }

   size_t parsing_scaffold::line_of(const rapidxml::xml_attribute<>& item) const {
      auto name = item.name();
      auto pos  = (std::ptrdiff_t)(name - this->data.c_str());
      return this->_line_of(pos);
   }
   size_t parsing_scaffold::line_of(const rapidxml::xml_node<>& item) const {
      auto name = item.name();
      auto pos  = (std::ptrdiff_t)(name - this->data.c_str());
      return this->_line_of(pos);
   }
   size_t parsing_scaffold::line_of(const rapidxml::parse_error& e) const {
      auto pos  = (std::ptrdiff_t)(e.where<char>() - this->data.c_str());
      return this->_line_of(pos);
   }
}