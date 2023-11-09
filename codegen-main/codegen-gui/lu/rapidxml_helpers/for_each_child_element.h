#pragma once
#include <string_view>
#include <type_traits>
#include "RapidXml/rapidxml.hpp"

namespace lu::rapidxml_helpers {
   template<typename Char, typename Functor>
   void for_each_child_element(rapidxml::xml_node<Char>& parent, Functor&& functor) {
      for (auto* child = parent.first_node(); child; child = child->next_sibling()) {
         if (child->type() != rapidxml::node_type::node_element)
            continue;

         auto tagname = std::string_view(child->name(), child->name_size());
         functor(tagname, *child);
      }
   }

   template<typename Char, typename MatchFunctor, typename NoMatchFunctor>
   void for_each_child_element(rapidxml::xml_node<Char>& parent, const char* desired_tagname, MatchFunctor&& on_match, NoMatchFunctor&& on_unwanted) {
      for (auto* child = parent.first_node(); child; child = child->next_sibling()) {
         if (child->type() != rapidxml::node_type::node_element)
            continue;

         auto tagname = std::string_view(child->name(), child->name_size());
         if (tagname == desired_tagname) {
            on_match(*child);
         } else {
            on_unwanted(tagname, *child);
         }
      }
   }
}