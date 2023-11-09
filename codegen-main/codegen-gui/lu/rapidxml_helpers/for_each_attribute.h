#pragma once
#include <string_view>
#include "RapidXml/rapidxml.hpp"

namespace lu::rapidxml_helpers {
   template<typename Char, typename Functor>
   void for_each_attribute(rapidxml::xml_node<Char>& node, Functor&& functor) {
      for (rapidxml::xml_attribute<Char>* attr = node.first_attribute(); attr; attr = attr->next_attribute()) {
         auto attr_name  = std::string_view(attr->name(), attr->name_size());
         auto attr_value = std::string_view(attr->value(), attr->value_size());
         functor(attr_name, attr_value, *attr);
      }
   }
}