#pragma once
#include <string_view>
#include "RapidXml/rapidxml.hpp"

namespace lu::rapidxml_helpers {
   template<typename Char>
   rapidxml::xml_node<Char>* get_sole_child_element_of_name(rapidxml::xml_node<Char>& parent, const char* desired_tagname, size_t* count_seen) {
      rapidxml::xml_node<Char>* found = nullptr;
      size_t count = 0;

      for (auto* child = parent.first_node(); child; child = child->next_sibling()) {
         if (child->type() != rapidxml::node_type::node_element)
            continue;

         auto tagname = std::string_view(child->name(), child->name_size());
         if (tagname == desired_tagname) {
            ++count;
            found = child;
         }
      }
      if (count_seen)
         *count_seen = count;
      if (count == 1)
         return found;
      return nullptr;
   }
}