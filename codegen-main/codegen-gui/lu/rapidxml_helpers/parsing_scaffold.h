#pragma once
#include <filesystem>
#include <string>
#include <type_traits>
#include <vector>
#include "RapidXml/rapidxml.hpp"

namespace lu::rapidxml_helpers {
   class parsing_scaffold {

      /*

      RapidXML operates on the XML string in situ: it stores pointers to that string's content
      for element names, attribute names and values, text content, and similar. If you allow it
      to modify the string, then it writes null terminators directly into it; otherwise, it has
      to retain slice sizes on its own (and it retains those sizes either way). In other words,
      RapidXML data operates as a bunch of std::string_view objects with some metadata.

      One thing it notably does not retain is line numbers, which... complicates... reporting
      errors to the user. Fortunately, because it stores strings in situ, we can engineer our
      own workaround to that.

      If, prior to parsing the XML, we store a list of the offsets of every line break, then
      it becomes possible to know what line an element is on. All we have to do to know what
      line a given rapidxml::xml_node<> is on is grab its name() pointer and subtract from
      that the pointer to the entire XML string: this tells us the exact offset into the file
      for the node name. Then, we can just count how many line breaks came before that offset.
      (Binary-searching the list of line break offsets would likely be faster, but conceptually
      we want the count of line breaks preceding the given XML node name offset.)

      Accordingly, this class should:

       - Store a modifiable std::string containing the XML data.

       - Upon storing the string, scan it for line breaks and store a list of their offsets.

       - Have a member function that can be invoked to log a warning or throw an exception.
         This function should be able to take an xml_node, xml_attribute, or similar, and
         should derive from that the line number for the error location. Bonus points if we
         can get column numbers but that's optional for now.

      */

      protected:
         std::filesystem::path path;
         std::string data;
         std::vector<size_t> newline_offsets;

         size_t _line_of(std::ptrdiff_t offset) const;

         void _warn_at(const std::string& message, size_t line) const;
         [[noreturn]] void _error_at(const std::string& message, size_t line) const;

      public:
         char* get_data();
         void set_data(const std::string&);

         void set_path(const std::filesystem::path& p) { path = p; }

         size_t line_of(const rapidxml::xml_attribute<>&) const;
         size_t line_of(const rapidxml::xml_node<>&) const;
         size_t line_of(const rapidxml::parse_error&) const;

         void file_error(const std::string& message);

         template<typename T> requires (!std::is_pointer_v<T>)
         void warn(const std::string& message, T& xml_item) const {
            this->_warn_at(message, this->line_of(xml_item));
         }

         template<typename T> requires (!std::is_pointer_v<T>)
         [[noreturn]] void error(const std::string& message, T& xml_item) const {
            this->_error_at(message, this->line_of(xml_item));
         }

         [[noreturn]] void error(const rapidxml::parse_error& e) const {
            this->_error_at(e.what(), this->line_of(e));
         }
   };
}
