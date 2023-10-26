#pragma once
#include <stdexcept>
#include <string>
#include "RapidXml/rapidxml.hpp"

namespace lu::rapidxml_helpers {
   template<bool AllowNonText, typename Char>
   std::string get_element_text_content(rapidxml::xml_node<Char>& parent) {
      std::string out;
      for (auto* child = parent.first_node(); child; child = child->next_sibling()) {
         switch (child->type()) {
            using enum rapidxml::node_type;
            case node_comment:
            case node_declaration:
            case node_doctype:
            case node_document: // should be impossible
               continue;
            case node_pi: // unsupported
               continue;
            case node_element:
               if constexpr (AllowNonText) {
                  out += get_element_text_content(*child);
               } else {
                  throw std::runtime_error("non-text content inside");
               }
               break;
            case node_data:
            case node_cdata:
               out += std::string_view(child->value(), child->value_size());
         }
      }
      return out;
   }
}