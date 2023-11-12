#pragma once
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include "RapidXml/rapidxml.hpp"
#include "lu/rapidxml_helpers/parsing_scaffold.h"
#include "lu/singleton.h"

#include "./ast/integral_constant.h"

namespace ast {
   class constant_definition;

   class blind_union;
   class heritable;
   class structure;
   class member;
}
// HA HA HA, JUST KIDDING, std::unique_ptr CHOKES ON FORWARD DECLARATIONS
#include "./ast/constant_definition.h"
#include "./ast/blind_union.h"
#include "./ast/heritable.h"
#include "./ast/structure.h"

class registry;
class registry : public lu::singleton<registry> {
   public:
      using parse_wrapper = lu::rapidxml_helpers::parsing_scaffold;

      struct path_set {
         struct {
            std::filesystem::path xml;
         } input_paths;
         struct {
            std::filesystem::path h;
            std::filesystem::path c;
            //
            std::filesystem::path struct_members;
            std::filesystem::path struct_serialize;
            std::filesystem::path sector_serialize;
            //
            std::filesystem::path save_functors;
            //
            std::filesystem::path codegen_report;
         } output_paths;
      };

      struct sector_group {
         struct structure_to_target {
            const ast::structure* definition = nullptr;
            struct {
               std::string name; // e.g. gSaveBlock1Ptr
               std::string header;
               bool is_pointer = false;
            } c_variable;
         };

         std::string name;
         size_t sector_base_id   = (size_t)-1;
         size_t max_sector_count = 0;
         std::vector<structure_to_target> top_level_structs;
      };
      
   protected:
      struct union_stack_frame {
         const ast::inlined_union_member* target_union = nullptr;
         bool in_member_to_serialize = false;
      };

      path_set paths;

      std::unordered_map<std::string, std::unique_ptr<ast::constant_definition>> constants;
      std::unordered_map<std::string, std::unique_ptr<ast::heritable>> heritables;
      std::unordered_map<std::string, std::unique_ptr<ast::structure>> structs;
      std::unordered_map<std::string, std::unique_ptr<ast::blind_union>> blind_unions;

      std::vector<sector_group> sector_groups;

      std::vector<std::filesystem::path> seen_files;
      
      struct {
         std::vector<union_stack_frame> unions; // i.e. ast::inlined_union_member
      } load_state;

      void _enter_union(const ast::inlined_union_member&);
      void _exit_union(const ast::inlined_union_member&);

      // If we're inside of a union member that isn't serialized, then we don't care whether 
      // structs inside of that member are actually defined/laid out within the XML.
      bool _in_inactive_union_member(const std::string& member_name) const;

   public:
      std::optional<ast::size_constant> interpret_size_constant(const std::string_view&);

   protected:
      std::filesystem::path _normalize_xml_path(const std::filesystem::path&);
      std::optional<std::intmax_t> _int_or_constant_name_to_int(const std::string_view&);

      void _parse_and_handle_dependencies(parse_wrapper&, rapidxml::xml_node<>&);
      //
      void _parse_constants(parse_wrapper&, rapidxml::xml_node<>&);
      void _parse_heritables(parse_wrapper&, rapidxml::xml_node<>&);
      //
      std::unique_ptr<ast::member> _parse_member(parse_wrapper&, rapidxml::xml_node<>&, bool& out_is_checksum);
      void _parse_types(parse_wrapper&, rapidxml::xml_node<>&);
      //
      void _parse_sector_group(parse_wrapper&, rapidxml::xml_node<>&);

      void parse_xml(parse_wrapper& scaffold);

   public:
      void set_paths(const path_set&);

      void parse_all_xml_files();

      const ast::constant_definition* constant_by_name(const std::string_view) const;

      constexpr std::add_lvalue_reference_t<std::add_const_t<decltype(heritables)>> all_heritables() const {
         return this->heritables;
      }
      constexpr std::add_lvalue_reference_t<std::add_const_t<decltype(sector_groups)>> all_sector_groups() const {
         return this->sector_groups;
      }
      constexpr std::add_lvalue_reference_t<std::add_const_t<decltype(structs)>> all_structs() const {
         return this->structs;
      }

      void clear();
};