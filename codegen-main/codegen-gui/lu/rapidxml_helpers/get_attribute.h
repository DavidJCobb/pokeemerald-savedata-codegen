#pragma once
#include <string_view>
#include "RapidXml/rapidxml.hpp"

namespace lu::rapidxml_helpers {
   template<typename Char>
   rapidxml::xml_attribute<Char>* get_attribute(rapidxml::xml_node<Char>& node, const char* desired) {
      for (rapidxml::xml_attribute<Char>* attr = node.first_attribute(); attr; attr = attr->next_attribute()) {
         auto name = std::string_view(attr->name(), attr->name_size());
         if (name == desired)
            return attr;
      }
      return nullptr;
   }
}