#include "./struct_serialization_function_generator.h"
#include "lu/strings/from_integer.h"
#include "../ast/structure.h"
#include "../registry.h"

#include "./serialization_function_body.h"
#include "./serialization_item.h"

namespace codegen {
   std::string struct_serialization_function_generator::generate_h() const {
      std::string out;
      out += "#ifndef GUARD_LU_SERIALIZE_" + this->definition.name + "\n";
      out += "#define GUARD_LU_SERIALIZE_" + this->definition.name + "\n";
      out += '\n';
      out += "struct lu_BitstreamState;\n"; // must declare struct for it to be globally scoped
      if (this->definition.c_type_info.is_defined_via_typedef) {
         //
         // Cannot forward-declare structs defined as `typedef struct { ... } name`.
         //
         if (!this->definition.header.empty()) {
            if (this->definition.header != "global.h") {
               out += "#include \"global.h\"\n"; // lots of pokeemerald stuff breaks if this isn't included first/at all
            }
            out += "#include \"" + this->definition.header + "\"\n";
         }
      } else {
         out += "struct " + this->definition.name + ";\n"; // must declare struct for it to be globally scoped
      }
      out += '\n';

      out += "void lu_BitstreamRead_" + this->definition.name + "(struct lu_BitstreamState*, ";
      if (!this->definition.c_type_info.is_defined_via_typedef) {
         out += "struct ";
      }
      out += this->definition.name + "* dst);\n";

      out += "void lu_BitstreamWrite_" + this->definition.name + "(struct lu_BitstreamState*, const ";
      if (!this->definition.c_type_info.is_defined_via_typedef) {
         out += "struct ";
      }
      out += this->definition.name + "* src);\n";

      out += '\n';
      out += "#endif";
      return out;
   }

   std::string struct_serialization_function_generator::generate_c() const {
      auto& reg = registry::get(); // needed to access integral constant definitions

      std::string out;
      
      out += "#include \"";
      out += this->destination_folder;
      if (!this->destination_folder.empty() && this->destination_folder.back() != '/')
         out += '/';
      out += "serialize_" + this->definition.name + ".h\"\n\n";

      out += "#include \"global.h\"\n"; // lots of pokeemerald stuff breaks if this isn't included first/at all
      if (!this->definition.header.empty() && this->definition.header != "global.h") {
         // this should also take care of including all preprocessor constants referenced by the struct's members
         out += "#include \"" + this->definition.header + "\" // struct definition\n";
      }
      out += '\n';

      out += "#include \"lu/bitstreams.h\"\n\n";

      std::vector<std::string> dependencies = this->definition.get_all_direct_struct_dependencies();
      bool has_any_strings = this->definition.has_any_string_members();

      if (dependencies.size()) {
         out += "// dependencies\n";
         for (const auto& name : dependencies) {
            out += "#include \"";
            out += this->destination_folder;
            if (!this->destination_folder.empty() && this->destination_folder.back() != '/')
               out += '/';
            out += "serialize_" + name + ".h\"\n";
         }
         out += '\n';
      }
      if (has_any_strings) {
         out += "#include \"string_util.h\" // gflib; for StringLength\n\n";
      }

      {
         auto constants = this->definition.get_all_direct_constant_dependencies();
         if (!constants.empty()) {
            out += "// check constants:\n";
            for (const auto& item : constants) {
               const auto* entry = reg.constant_by_name(item);
               assert(entry);

               out += "#if " + item + " != " + lu::strings::from_integer(entry->value) + "\n";
               out += "   #error Constant `" + item + "` has been changed in C, but XML not updated or codegen not re-run!\n";
               out += "#endif\n";
            }
            out += "\n";
         }
      }

      std::string code_read;
      std::string code_write;

      {
         code_read   = "void lu_BitstreamRead_";
         code_write  = "void lu_BitstreamWrite_";
         code_read  += this->definition.name;
         code_write += this->definition.name;
         code_read  += "(struct lu_BitstreamState* state, ";
         code_write += "(struct lu_BitstreamState* state, const ";
         if (!this->definition.c_type_info.is_defined_via_typedef) {
            code_read  += "struct ";
            code_write += "struct ";
         }
         code_read  += this->definition.name;
         code_write += this->definition.name;
         code_read  += "* v) {\n";
         code_write += "* v) {\n";

         {
            auto item = codegen::serialization_item::for_top_level_struct(this->definition);
            item.accessor = "v"; // override the default, which is p_StructName

            codegen::serialization_function_body body;
            body.item_list = item.expand();
            //
            body.state_is_argument      = true;
            body.writes_can_debug_print = false;

            body.generate();

            code_read  += body.out.read;
            code_write += body.out.write;

            for (auto* item : body.item_list)
               delete item;
         }

         code_read  += "}\n";
         code_write += "}\n";
      }

      out += code_read;
      out += '\n';
      out += code_write;

      return out;
   }
}
