#pragma once
#include "./serialization_function_body.h"

namespace codegen {
   #pragma region array_rank_info
   constexpr serialization_function_body::array_rank_info::array_rank_info(const serialization_item::item_list& item_list) {
      this->deepest_rank             = 0;
      this->all_indices_are_u8       = true;
      this->all_indices_are_numbered = false;
      for (const auto& item_ptr : item_list) {
         if (item_ptr->member_definition) {
            auto& extents = item_ptr->member_definition->array_extents;

            size_t rank = extents.size();
            if (rank > deepest_rank)
               this->deepest_rank = rank;

            if (this->all_indices_are_u8) {
               for (const auto& e : extents) {
                  if (e.extent.value > 255) {
                     this->all_indices_are_u8 = false;
                     break;
                  }
               }
            }
         }
      }

      this->all_indices_are_numbered = deepest_rank > ('z' - 'i');
   }

   constexpr std::string serialization_function_body::array_rank_info::generate_forward_declarations() const {
      if (this->deepest_rank == 0)
         return {};

      std::string gen;
      gen += "u"s + (all_indices_are_u8 ? "8" : "16") + " ";
      for (size_t i = 0; i < this->deepest_rank; ++i) {
         gen += rank_to_index_variable(i);
         if (i + 1 < this->deepest_rank)
            gen += ", ";
      }
      gen += ";\n";
      return gen;
   }

   constexpr std::string serialization_function_body::array_rank_info::rank_to_index_variable(size_t i) const {
      if (this->all_indices_are_numbered) {
         return "index_"s + lu::strings::from_integer(i);
      }
      return std::string{} += (char)('i' + i);
   };
   #pragma endregion

   constexpr std::string serialization_function_body::_generate_loop_start(const std::string& var, size_t start, const std::string& stop) {
      std::string gen;
      gen += "for (";
      gen += var;
      gen += " = ";
      gen += lu::strings::from_integer(start);
      gen += "; ";
      gen += var;
      gen += " < ";
      gen += stop;
      gen += "; ++";
      gen += var;
      gen += ") {\n";
      return gen;
   }
   constexpr std::string serialization_function_body::_generate_loop_start(const std::string& var, size_t start, size_t stop) {
      return this->_generate_loop_start(var, start, lu::strings::from_integer(stop));
   }
   constexpr std::string serialization_function_body::_generate_loop_start(const std::string& var, size_t start, const ast::member::array_rank& stop) {
      if (this->enable_array_extent_expressions)
         return this->_generate_loop_start(var, start, stop.as_c_expression());
      return this->_generate_loop_start(var, start, stop.extent.value);
   }

   constexpr void serialization_function_body::generate() {
      auto array_info = array_rank_info(this->item_list);

      {
         auto decl = array_info.generate_forward_declarations();
         if (!decl.empty())
            out += "   "s + decl;
      }

      if (!this->state_is_argument) {
         out       += "   struct lu_BitstreamState state;\n";
         out.read  += "   lu_BitstreamInitialize(&state, (u8*)src); // need to cast away constness to store it here\n";
         out.write += "   lu_BitstreamInitialize(&state, dst);\n";
         out       += '\n';
      }

      std::string state_argument = this->state_is_argument ? "state" : "&state";
      
      for (size_t j = 0; j < item_list.size(); ++j) {
         const auto& item_ptr = item_list[j];
         std::string computed_accessor = item_ptr->accessor;
         if (item_ptr->member_definition) {
            if (item_ptr->member_definition->skip_when_serializing) // `serialization_item`s should never be generated for these members
               throw;
         }

         size_t coalesce_array_siblings_up_through = j;

         std::string indent = "   ";
               
         if (this->writes_can_debug_print) {
            out.write += indent;
            out.write += "#ifdef LOG_FIELD_NAMES_FOR_SAVEGAME_SERIALIZE\n";
            out.write += indent;
            out.write += "   DebugPrintf(\"Writing field: ";
            out.write += computed_accessor;
            out.write += "\", 0);\n";
            out.write += indent;
            out.write += "#endif\n";
         }

         if (item_ptr->is_array()) {
            std::string common;

            auto& indices = item_ptr->array_indices;
            auto& extents = item_ptr->member_definition->array_extents;

            if (indices.size() == extents.size()) {
               //
               // We previously expanded serialization items if they would've overflowed a sector 
               // boundary, so that we could serialize as much of a single item as possible on one 
               // side of the boundary, and then serialize the rest of it on the other side of the 
               // boundary. However, for array items, that means we can end up in situations like 
               // this:
               // 
               //       // ...
               //       lu_BitstreamWrite_u8(&state, array[0], 5);
               //       lu_BitstreamWrite_u8(&state, array[1], 5);
               //       lu_BitstreamWrite_u8(&state, array[2], 5);
               //    }
               // 
               //    void NextSerializationFunction(...) {
               //       // ...
               //       lu_BitstreamWrite_u8(&state, array[3], 5);
               //       lu_BitstreamWrite_u8(&state, array[4], 5);
               //       lu_BitstreamWrite_u8(&state, array[5], 5);
               //       // ...
               // 
               // Upon encountering our hypothetical `array[0]` and `array[3]`, it would be preferable 
               // to "swallow" the "next-array-sibling" items after it -- to emit partial loops; in 
               // the former case, a loop with range [0, 3); and in the latter case, a loop with range 
               // [3, 6). Here, then, we'll see how far we get to swallow.
               //
               for (size_t k = j + 1; k < item_list.size(); ++k) {
                  if (item_list[k - 1]->arg_is_next_array_sibling(*item_list[k])) {
                     coalesce_array_siblings_up_through = k;
                  } else {
                     break;
                  }
               }
            }
            bool coalesce_last_defined_index = coalesce_array_siblings_up_through > j;

            for (size_t i = 0; i < indices.size() - (coalesce_last_defined_index ? 1 : 0); ++i) {
               computed_accessor += '[';
               computed_accessor += lu::strings::from_integer(indices[i]);
               computed_accessor += ']';
            }
            if (coalesce_last_defined_index) {
               size_t i   = indices.size() - 1;
               auto   var = array_info.rank_to_index_variable(i);

               computed_accessor += '[';
               computed_accessor += var;
               computed_accessor += ']';

               common += indent;
               common += _generate_loop_start(var, item_list[j]->array_indices.back(), item_list[coalesce_array_siblings_up_through]->array_indices.back() + 1);

               indent += "   ";
            }
            for (size_t i = indices.size(); i < extents.size(); ++i) {
               auto var = array_info.rank_to_index_variable(i);

               computed_accessor += '[';
               computed_accessor += var;
               computed_accessor += ']';

               common += indent;
               common += _generate_loop_start(var, 0, extents[i]);

               indent += "   ";
            }

            out += common;
         }

         #pragma region Serialize item
         //
         // Generate the function call to serialize the item.
         //
         bool is_struct = item_ptr->struct_definition && !item_ptr->member_definition;
         if (!is_struct) {
            is_struct = dynamic_cast<const ast::struct_member*>(item_ptr->member_definition) != nullptr;
         }

         if (is_struct) {
            const ast::structure* struct_type = nullptr;
            if (item_ptr->member_definition) {
               auto* casted = dynamic_cast<const ast::struct_member*>(item_ptr->member_definition);
               struct_type = casted->type_def;
            } else {
               struct_type = item_ptr->struct_definition;
            }

            out       += indent;
            out.read  += "lu_BitstreamRead_";
            out.write += "lu_BitstreamWrite_";
            {
               std::string common;
               common += struct_type->name;
               common += '(';
               common += state_argument;
               common += ", ";
               if (item_ptr->member_definition)
                  common += '&';
               common += computed_accessor;
               common += ");\n";

               out += common;
            }

            this->results.direct_dependencies.push_back(struct_type);
         } else if (auto* casted = dynamic_cast<const ast::blind_union_member*>(item_ptr->member_definition)) {
            out       += indent;
            out.read  += "lu_BitstreamRead_buffer(";
            out.write += "lu_BitstreamWrite_buffer(";
            {
               std::string common;
               common += state_argument;
               common += ", ";
               if (item_ptr->member_definition)
                  common += '&';
               common += computed_accessor;
               common += ", ";
               common += lu::strings::from_integer(casted->type_def->size_in_bytes);
               common += ");\n";

               out += common;
            }
         } else if (auto* casted = dynamic_cast<const ast::string_member*>(item_ptr->member_definition)) {
            if (casted->max_length.value <= 0)
               throw;
                  
            out       += indent;
            out.read  += "lu_BitstreamRead_string";
            out.write += "lu_BitstreamWrite_string";
            if (casted->only_early_terminator) {
               out += "_optional_terminator";
            }
            out += "(";
            {
               std::string common;
               common += state_argument;
               common += ", ";
               common += computed_accessor;
               common += ", ";
               common += casted->max_length.as_c_expression();
               common += ");\n";

               out += common;
            }
         } else if (auto* casted = dynamic_cast<const ast::integral_member*>(item_ptr->member_definition)) {
            out += indent;

            out.read += computed_accessor;
            out.read += " = ";

            out.read  += "lu_BitstreamRead_";
            out.write += "lu_BitstreamWrite_";

            size_t bitcount_per = casted->compute_single_element_bitcount();
            if (bitcount_per == 1) {
               out += "bool(";
               out += state_argument;

               out.read  += ");\n";

               out.write += ", ";
               out.write += computed_accessor;
               out.write += ");\n";

               continue;
            }

            {
               std::string common;

               if (casted->is_signed() && !casted->min.has_value()) {
                  // If it has a minimum value, then we subtract that minimum value 
                  // when serializing. If that value is negative, then that means we 
                  // *increase* the serialized value to 0 or above.
                  common += "s";
               } else {
                  common += "u";
               }
               if (bitcount_per <= 8) {
                  common += "8";
               } else if (bitcount_per <= 16) {
                  common += "16";
               } else {
                  common += "32";
               }
               common += "(";
               common += state_argument;
               common += ", ";

               out += common;
            }

            out.write += computed_accessor;
            if (casted->min.has_value() && casted->min.value() != 0) {
               out.write += " - ";
               out.write += lu::strings::from_integer(casted->min.value());
            }
            out.write += ", ";

            if (bitcount_per != 1) {
               out.read  += lu::strings::from_integer(bitcount_per);
               out.write += lu::strings::from_integer(bitcount_per);
            }
            out.read += ")";
            if (casted->min.has_value() && casted->min.value() != 0) {
               out.read += " + ";
               out.read += lu::strings::from_integer(casted->min.value());
            }
            out.read += ";\n";

            out.write += ");\n";
         } else if (auto* casted = dynamic_cast<const ast::pointer_member*>(item_ptr->member_definition)) {
            out += indent;

            out.read += computed_accessor;
            out.read += " = (";
            if (casted->is_const)
               out.read += "const ";
            out.read += casted->pointed_to_type_to_string();
            out.read += "*) ";

            out.read  += "lu_BitstreamRead_u32(";
            out.write += "lu_BitstreamWrite_u32(";

            out += state_argument;
            out += ", ";

            out.write += "(u32)";
            out.write += computed_accessor;
            out.write += ", ";

            out += "32);\n";
         } else {
            throw;
         }
         #pragma endregion

         //
         // Close any array loops we're in.
         //

         if (item_ptr->is_array()) {
            std::string common;

            auto& indices = item_ptr->array_indices;
            auto& extents = item_ptr->member_definition->array_extents;

            if (coalesce_array_siblings_up_through > j) {
               //
               // Close the extra loop generated by swallowing "next-array-sibling" serialization items, 
               // above.
               //
               indent = indent.substr(3);
               common += indent;
               common += "}\n";
            }

            for (size_t i = indices.size(); i < extents.size(); ++i) {
               indent = indent.substr(3);
               common += indent;
               common += "}\n";
            }

            out += common;
         }

         if (coalesce_array_siblings_up_through > j) {
            //
            // Skip any swallowed "next-array-sibling" items. Note the use of the wording "up through" 
            // as opposed to "up to;" this variable indicates the last swallowed item. We set `j` to 
            // it, and then the loop we're inside of will increment `j` further and continue past it.
            //
            j = coalesce_array_siblings_up_through;
         }
      }
      //
      // Done.
      //
      {
         auto& list = this->results.direct_dependencies;
         if (!list.empty())
            list.erase(std::unique(list.begin(), list.end()), list.end());
      }
   }
}