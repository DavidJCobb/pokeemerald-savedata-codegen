#pragma once
#include "./struct_body_serializer.inl"

namespace codegen {
   constexpr std::string struct_body_serializer::generate() {
      std::string out;

      out = "// generated\n\n"; // TODO: list the path to the source XML file for this struct
      out += this->definition.as_c_member_declaration_list();

      return out;
   }
}