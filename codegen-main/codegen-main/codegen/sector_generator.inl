#pragma once
#include "./sector_generator.h"
#include <algorithm>
#include <bit>
#include "lu/strings/from_integer.h"
#include "./ast/member_types/_all.h"

namespace codegen {
   constexpr std::vector<serialization_item::item_list> sector_generator::perform_expansions_and_splits(serialization_item::item_list src) const {
      std::vector<serialization_item::item_list> out;
      out.push_back({});

      size_t current_bits = 0;
      for (size_t i = 0; i < src.size(); ++i) {
         auto* item = src[i];

         auto bitcount = item->bitcount();
         if (bytespan_of(current_bits + bitcount) < this->sector_max_bytes) {
            current_bits += bitcount;
            out.back().push_back(item);
            continue;
         }

         auto expanded = item->expand();
         if (!expanded.empty()) {
            src.insert(src.erase(src.begin() + i), expanded.begin(), expanded.end());
            delete item;

            // Repeat this iteration.
            --i;
            continue;
         }

         // Expansion failed, i.e. this item cannot be expanded. Move to the next sector and try again.
         current_bits = 0;
         out.push_back({});
         --i;
      }

      return std::move(out);
   }

   constexpr sector_generator::output sector_generator::generate(const std::vector<const ast::structure*>& structures) const {
      std::vector<serialization_item::item_list> items_by_sector;
      {
         serialization_item::item_list base_list;
         for (auto* s : structures) {
            auto item = serialization_item::for_top_level_struct(*s);
            base_list.push_back(new serialization_item(item));
         }
         items_by_sector = std::move(perform_expansions_and_splits(base_list));
      }

      output out;

      auto sector_id_to_function_decl = [this, &structures](size_t i, bool is_read) -> std::string {
         std::string out = "void lu_";
         out += (is_read) ? "Read" : "Write";
         out += "SaveSector_";
         out += this->function_name_fragment;

         std::string id = lu::strings::from_integer(i);
         while (id.size() < 2)
            id.insert(id.begin(), '0');
         out += id;

         out += "(u8* dst, ";
         //
         // TODO: Only take args that are actually used within this sector.
         //
         for (size_t j = 0; j < structures.size(); ++j) {
            out += "const ";
            out += structures[j]->name;
            out += "* p_";
            out += structures[j]->name;
            if (j + 1 < structures.size()) {
               out += ", ";
            }
         }
         out += ")";
         return out;
      };

      {
         out.header += "#include \"lu/bitstreams.h\"\n\n";
         for (size_t i = 0; i < items_by_sector.size(); ++i) {
            out.header += sector_id_to_function_decl(i, true); // read
            out.header += ";\n";
            out.header += sector_id_to_function_decl(i, false); // write
            out.header += ";\n";
         }
      }

      out.implementation += "#include \"";
      out.implementation += this->sector_serialize_header_folder;
      if (!this->sector_serialize_header_folder.empty()) {
         if (this->sector_serialize_header_folder.back() != '/')
            out.implementation += '/';
      }
      out.implementation += this->function_name_fragment;
      out.implementation += ".h\"\n\n";
      {
         std::vector<const ast::structure*> dependencies;
         std::string code_read;
         std::string code_write;

         for (size_t i = 0; i < items_by_sector.size(); ++i) {
            code_read  += sector_id_to_function_decl(i, true);
            code_write += sector_id_to_function_decl(i, false);
            code_read  += " {\n";
            code_write += " {\n";

            bool array_indices_are_numbered = false;
            bool array_indices_are_all_u8   = true;
            auto rank_to_var = [array_indices_are_numbered](size_t i) -> std::string {
               if (array_indices_are_numbered) {
                  return "index_"s + lu::strings::from_integer(i);
               }
               return std::string{} += (char)('i' + i);
            };
            //
            {  // Forward-declare loop index variables
               size_t deepest_rank = 0;
               for (const auto& item_ptr : items_by_sector[i]) {
                  if (item_ptr->member_definition) {
                     auto& extents = item_ptr->member_definition->array_extents;

                     size_t rank = extents.size();
                     if (rank > deepest_rank)
                        deepest_rank = rank;

                     if (array_indices_are_all_u8) {
                        for (const auto& e : extents) {
                           if (e.extent.value > 255) {
                              array_indices_are_all_u8 = false;
                              break;
                           }
                        }
                     }
                  }
               }

               if (deepest_rank > 0) {
                  if (deepest_rank > ('z' - 'i')) {
                     array_indices_are_numbered = true;
                  }

                  std::string common;
                  common += "   u"s + (array_indices_are_all_u8 ? "8" : "16") + " ";
                  for (size_t i = 0; i < deepest_rank; ++i) {
                     common += rank_to_var(i);
                     if (i + 1 < deepest_rank)
                        common += ", ";
                  }
                  common += ";\n";

                  code_read  += common;
                  code_write += common;
               }
            }

            {
               std::string common;
               common += "   struct lu_BitstreamState state;\n";
               common += "   state.target = dst;\n";
               common += "   state.shift  = 0;\n\n";

               code_read += common;
               code_write += common;
            }

            // Function bodies
            for (size_t j = 0; j < items_by_sector[i].size(); ++j) {
               auto& item_list = items_by_sector[i];

               const auto& item_ptr = item_list[j];
               std::string computed_accessor = item_ptr->accessor;

               size_t coalesce_array_siblings_up_through = j;

               std::string indent = "   ";
               if (item_ptr->is_array()) {
                  std::string common;

                  auto& indices = item_ptr->array_indices;
                  auto& extents = item_ptr->member_definition->array_extents;

                  if (indices.size() == extents.size()) {
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
                     size_t i = indices.size() - 1;

                     computed_accessor += '[';
                     if (array_indices_are_numbered) {
                        computed_accessor += "index_";
                        computed_accessor += lu::strings::from_integer(i);
                     } else {
                        computed_accessor += (char)('i' + i);
                     }
                     computed_accessor += ']';

                     common += indent;
                     common += "for (";
                     common += rank_to_var(i);
                     common += " = ";
                     common += lu::strings::from_integer(item_list[j]->array_indices.back());
                     common += "; ";
                     common += rank_to_var(i);
                     common += " < ";
                     common += lu::strings::from_integer(item_list[coalesce_array_siblings_up_through]->array_indices.back() + 1);
                     common += "; ++";
                     common += rank_to_var(i);
                     common += ") {\n";

                     indent += "   ";
                  }
                  for (size_t i = indices.size(); i < extents.size(); ++i) {
                     computed_accessor += '[';
                     if (array_indices_are_numbered) {
                        computed_accessor += "index_";
                        computed_accessor += lu::strings::from_integer(i);
                     } else {
                        computed_accessor += (char)('i' + i);
                     }
                     computed_accessor += ']';

                     common += indent;
                     common += "for (";
                     common += rank_to_var(i);
                     common += " = 0; ";
                     common += rank_to_var(i);
                     common += " < ";
                     common += extents[i].as_c_expression();
                     common += "; ++";
                     common += rank_to_var(i);
                     common += ") {\n";

                     indent += "   ";
                  }

                  code_read  += common;
                  code_write += common;
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

                  code_read  += indent;
                  code_write += indent;
                  code_read  += "lu_BitstreamRead_";
                  code_write += "lu_BitstreamWrite_";
                  {
                     std::string common;
                     common += struct_type->name;
                     common += "(&state, &";
                     common += computed_accessor;
                     common += ");\n";

                     code_read  += common;
                     code_write += common;
                  }

                  dependencies.push_back(struct_type);
               } else if (auto* casted = dynamic_cast<const ast::blind_union_member*>(item_ptr->member_definition)) {
                  code_read  += indent;
                  code_write += indent;
                  code_read  += "lu_BitstreamRead_buffer(&state, &";
                  code_write += "lu_BitstreamWrite_buffer(&state, &";
                  {
                     std::string common;
                     common += computed_accessor;
                     common += ", ";
                     common += lu::strings::from_integer(casted->type_def->size_in_bytes);
                     common += ");\n";

                     code_read  += common;
                     code_write += common;
                  }
               } else if (auto* casted = dynamic_cast<const ast::string_member*>(item_ptr->member_definition)) {
                  if (casted->max_length.value <= 0)
                     throw;
                  
                  code_read  += indent;
                  code_write += indent;
                  code_read  += "lu_BitstreamRead_string(&state, ";
                  code_write += "lu_BitstreamWrite_string(&state, ";
                  {
                     std::string common;
                     common += computed_accessor;
                     common += ", ";
                     common += casted->max_length.as_c_expression();
                     common += ", ";
                     common += lu::strings::from_integer(std::bit_width(casted->max_length.value));
                     common += ");\n";

                     code_read  += common;
                     code_write += common;
                  }
               } else if (auto* casted = dynamic_cast<const ast::integral_member*>(item_ptr->member_definition)) {
                  code_read  += indent;
                  code_write += indent;

                  code_read += computed_accessor;
                  code_read += " = ";

                  code_read  += "lu_BitstreamRead_";
                  code_write += "lu_BitstreamWrite_";

                  size_t bitcount_per = casted->compute_single_element_bitcount();

                  {
                     std::string common;

                     if (bitcount_per == 1) {
                        common += "bool";
                     } else {
                        if (casted->is_signed()) {
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
                     }
                     common += "(&state, ";

                     code_read  += common;
                     code_write += common;
                  }

                  code_write += computed_accessor;
                  if (casted->min.has_value()) {
                     code_write += " - ";
                     code_write += lu::strings::from_integer(casted->min.value());
                  }
                  code_write += ", ";

                  if (bitcount_per != 1) {
                     code_read  += lu::strings::from_integer(bitcount_per);
                     code_write += lu::strings::from_integer(bitcount_per);
                  }
                  code_read += ")";
                  if (casted->min.has_value()) {
                     code_read += " + ";
                     code_read += lu::strings::from_integer(casted->min.value());
                  }
                  code_read += ";\n";

                  code_write += ");\n";
               } else if (auto* casted = dynamic_cast<const ast::pointer_member*>(item_ptr->member_definition)) {
                  code_read  += indent;
                  code_write += indent;

                  code_read += computed_accessor;
                  code_read += " = (";
                  code_read += ast::integral_type_to_string(casted->pointed_to_type.value());
                  code_read += "*) ";

                  code_read  += "lu_BitstreamRead_u32(&state, ";
                  code_write += "lu_BitstreamWrite_u32(&state, ";

                  code_write += computed_accessor;
                  code_write += ", ";

                  code_read  += "32);\n";
                  code_write += "32);\n";
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
                     indent = indent.substr(3);
                     common += indent;
                     common += "}\n";
                  }

                  for (size_t i = indices.size(); i < extents.size(); ++i) {
                     indent = indent.substr(3);
                     common += indent;
                     common += "}\n";
                  }

                  code_read  += common;
                  code_write += common;
               }

               if (coalesce_array_siblings_up_through > j) {
                  j = coalesce_array_siblings_up_through;
               }
            }

            code_read  += "};\n\n";
            code_write += "};\n\n";
         }

         if (!dependencies.empty()) {
            auto it = std::unique(dependencies.begin(), dependencies.end());
            dependencies.erase(it, dependencies.end());

            out.implementation += "// whole-struct serialize funcs:\n";
            for (const auto* structure : dependencies) {
               out.implementation += "#include \"";
               {
                  const auto& folder = this->whole_struct_serialize_header_folder;
                  if (!folder.empty()) {
                     out.implementation += folder;
                     if (folder.back() != '/')
                        out.implementation += '/';
                  }
               }
               out.implementation += "serialize_";
               out.implementation += structure->name;
               out.implementation += ".h\"\n";
            }
            out.implementation += '\n';
         }
         out.implementation += code_read;
         out.implementation += code_write;
      }

      for (auto& list : items_by_sector) {
         for (auto* item : list)
            delete item;
         list.clear();
      }

      return out;
   }
}