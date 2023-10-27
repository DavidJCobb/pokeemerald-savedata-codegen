#pragma once
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include "RapidXml/rapidxml.hpp"
#include "lu/rapidxml_helpers/parsing_scaffold.h"
#include "lu/singleton.h"

#include "./ast/integral_constant.h"

namespace ast {
   class constant_definition;
   class heritable;
   class structure;
   class member;
}
// HA HA HA, JUST KIDDING, std::unique_ptr CHOKES ON FORWARD DECLARATIONS
#include "./ast/constant_definition.h"
#include "./ast/heritable.h"
#include "./ast/structure.h"

class registry;
class registry : public lu::singleton<registry> {
   protected:
      using parse_wrapper = lu::rapidxml_helpers::parsing_scaffold;

   protected:
      std::unordered_map<std::string, std::unique_ptr<ast::constant_definition>> constants;
      std::unordered_map<std::string, std::unique_ptr<ast::heritable>> heritables;
      std::unordered_map<std::string, std::unique_ptr<ast::structure>> structs;

      std::vector<std::filesystem::path> seen_files;
      
      std::filesystem::path path;

   public:
      std::optional<ast::size_constant> interpret_size_constant(const std::string_view&);

   protected:
      std::filesystem::path _normalize_path(const std::filesystem::path&);
      std::optional<std::intmax_t> _int_or_constant_name_to_int(const std::string_view&);

      void _parse_and_handle_dependencies(parse_wrapper&, rapidxml::xml_node<>&);
      //
      void _parse_constants(parse_wrapper&, rapidxml::xml_node<>&);
      void _parse_heritables(parse_wrapper&, rapidxml::xml_node<>&);
      //
      std::unique_ptr<ast::member> _parse_member(parse_wrapper&, rapidxml::xml_node<>&, bool& out_is_checksum);
      void _parse_types(parse_wrapper&, rapidxml::xml_node<>&);

      void parse_xml(parse_wrapper& scaffold);

   public:
      void set_base_xml_path(std::filesystem::path p) { path = _normalize_path(p); }

      void parse_all_xml_files();

      size_t bitcount_of_struct(std::string) const;

      void generate_all_struct_body_files(std::filesystem::path out_folder);

      void generate_serialization_code(std::filesystem::path out_folder);
};