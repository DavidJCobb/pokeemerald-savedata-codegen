#include "./treewalker.h"
//
#include "./ast/member.h"
#include "./ast/structure.h"
#include "./registry.h"

using namespace std::literals::string_literals;

namespace {
   constexpr const bool always_descend = false;

   constexpr const char* const bitstream_state_argument_name = "state";

   constexpr const size_t bytespan_of(size_t bitcount) {
      return (bitcount / 8) + (bitcount % 8 ? 1 : 0);
   }
}


void treewalker::_write_indent(std::string& dst) {
   size_t char_count = this->indent * 3;
   dst.reserve(dst.size() + char_count);
   for (size_t i = 0; i < char_count; ++i) {
      dst += ' ';
   }
}

void treewalker::_enter_extent_loops(std::string& dst, const std::vector<ast::member::array_rank>& extents) {
   if (extents.empty())
      return;

   size_t rank = extents.size();

   constexpr const char first_array_index_name = 'i';
   bool array_indices_are_numbered = rank > ('z' - 'i');

   this->_write_indent(dst);
   dst += "{\n";
   ++this->indent;

   this->_write_indent(dst);
   {
      bool type_is_wide = false;
      for (const auto& extent : extents) {
         if (extent.extent.value > 255) {
            type_is_wide = true;
            break;
         }
      }
      if (type_is_wide) {
         dst += "u16";
      } else {
         dst += "u8";
      }
      dst += ' ';
   }

   for (size_t i = 0; i < rank; ++i) {
      if (array_indices_are_numbered) {
         dst += "index_";
         dst += lu::strings::from_integer(i);
      } else {
         dst += (char)(first_array_index_name + (char)i);
      }
      if (i + 1 < rank) {
         dst += ", ";
      }
   }
   dst += ";\n";

   for (size_t i = 0; i < rank; ++i) {
      std::string var;
      if (array_indices_are_numbered) {
         var = "index_"s + lu::strings::from_integer(i);
      } else {
         var = (char)(first_array_index_name + (char)i);
      }

      this->_write_indent(dst);
      dst += "for (";
      dst += var;
      dst += " = 0; ";
      dst += var;
      dst += " < ";
      dst += extents[i].as_c_expression();
      dst += "; ++";
      dst += var;
      dst += ") {\n";

      ++this->indent;
   }
}
void treewalker::_exit_extent_loops(std::string& dst, const std::vector<ast::member::array_rank>& extents) {
   if (extents.empty())
      return;

   size_t rank = extents.size();
   assert(this->indent >= rank);

   for (size_t i = 0; i < rank; ++i) {
      --this->indent;
      this->_write_indent(dst);
      dst += "}\n";
   }
}
std::string treewalker::_indices_as_string(const std::vector<ast::member::array_rank>& extents) {
   if (extents.empty())
      return {};

   size_t rank = extents.size();

   constexpr const char first_array_index_name = 'i';
   bool array_indices_are_numbered = rank > ('z' - 'i');

   std::string out;
   for (size_t i = 0; i < rank; ++i) {
      out += '[';
      if (array_indices_are_numbered) {
         out += "index_"s + lu::strings::from_integer(i);
      } else {
         out += (char)(first_array_index_name + (char)i);
      }
      out += ']';
   }
   return out;
}

void treewalker::_serialize_single_element(std::string accessor, const ast::member& member) {
   auto& current_gen = this->generated_funcs.back();

   struct {
      size_t single = 0;
      size_t total  = 0;
   } bitcounts = {
      .single = member.compute_single_element_bitcount(),
      .total  = member.compute_total_bitcount(),
   };
   
   if (auto* casted = dynamic_cast<const ast::struct_member*>(&member)) {
      this->_write_call(
         current_gen.bodies.read,
         "lu_BitstreamRead_"s + casted->type_name,
         //
         bitstream_state_argument_name,
         "&"s + accessor
      );
                  
      this->_write_call(
         current_gen.bodies.write,
         "lu_BitstreamWrite_"s + casted->type_name,
         //
         bitstream_state_argument_name,
         "&"s + accessor
      );
   } else if (auto* casted = dynamic_cast<const ast::integral_member*>(&member)) {
      if (bitcounts.single == 1) {
         this->_write_assign(
            current_gen.bodies.read,
            accessor,
            "lu_BitstreamRead_bool",
            //
            bitstream_state_argument_name
         );
                  
         this->_write_call(
            current_gen.bodies.write,
            "lu_BitstreamWrite_bool",
            //
            bitstream_state_argument_name,
            accessor
         );
      } else {
         std::string func_discriminator = "32";
         if (bitcounts.single <= 16) {
            func_discriminator = "16";
            if (bitcounts.single <= 8) {
               func_discriminator = "8";
            }
         }
            
         this->_write_assign(
            current_gen.bodies.read,
            accessor,
            "lu_BitstreamRead_"s + func_discriminator,
            //
            bitstream_state_argument_name,
            bitcounts.single
         );
                  
         this->_write_call(
            current_gen.bodies.write,
            "lu_BitstreamWrite_"s + func_discriminator,
            //
            bitstream_state_argument_name,
            accessor,
            bitcounts.single
         );
      }

   } else if (auto* casted = dynamic_cast<const ast::string_member*>(&member)) {
      this->_write_call(
         current_gen.bodies.write,
         "lu_BitstreamRead_string",
         //
         bitstream_state_argument_name,
         accessor,
         casted->max_length,
         std::bit_width(casted->max_length.value)
      );
      this->_write_call(
         current_gen.bodies.write,
         "lu_BitstreamWrite_string",
         //
         bitstream_state_argument_name,
         accessor,
         casted->max_length,
         std::bit_width(casted->max_length.value)
      );
   }
}
void treewalker::_serialize_whole_member(std::string accessor, const ast::member& member) {
   auto& current_gen = this->generated_funcs.back();

   struct {
      size_t single = 0;
      size_t total  = 0;
   } bitcounts = {
      .single = member.compute_single_element_bitcount(),
      .total  = member.compute_total_bitcount(),
   };

   this->_enter_extent_loops(current_gen.bodies.read,  member.array_extents);
   this->_enter_extent_loops(current_gen.bodies.write, member.array_extents);

   std::string indexed_accessor = accessor + _indices_as_string(member.array_extents);
   this->_serialize_single_element(indexed_accessor, member);

   this->_exit_extent_loops(current_gen.bodies.read,  member.array_extents);
   this->_exit_extent_loops(current_gen.bodies.write, member.array_extents);

   current_gen.bits_used += bitcounts.total;
}

void treewalker::_walk_array_rank(std::string top_level_struct_name, std::string accessor, const ast::member& member, size_t rank) {
   assert(rank < member.array_extents.size());

   auto& reg = registry::get();
   auto* current_gen = &this->generated_funcs.back();
   
   struct {
      size_t single = 0;
      size_t total  = 0;
   } bitcounts = {
      .single = member.compute_single_element_bitcount(),
      .total  = member.compute_total_bitcount(),
   };

   size_t bits_remaining      = (this->bytes_per_sector * 8) - current_gen->bits_used;
   size_t room_for_n_elements = bits_remaining / bitcounts.single;

   size_t bitcount_per_whole_iteration = bitcounts.single;
   for (size_t i = rank; i < member.array_extents.size(); ++i)
      bitcount_per_whole_iteration *= member.array_extents[i].extent.value;

   if (bitcount_per_whole_iteration < bits_remaining) {
      std::string var = "index_"s + lu::strings::from_integer(rank);

      std::string intro;
      this->_write_indent(intro);
      intro += "{\n";
      ++this->indent;
      this->_write_indent(intro);
      intro += "u16 ";
      intro += var;
      intro += ";\n";
      this->_write_indent(intro);
      intro += "for (";
      intro += var;
      intro += " = 0; ";
      intro += var;
      intro += " < ";
      intro += member.array_extents[rank].as_c_expression();
      intro += "; ++";
      intro += var;
      intro += ") {\n";
      ++this->indent;
      this->_write_indent(intro);

      current_gen->bodies.read  += intro;
      current_gen->bodies.write += intro;

      if (rank == member.array_extents.size() - 1) {
         this->_serialize_single_element(accessor + "[" + var + "]", member);
      } else {
         _walk_array_rank(top_level_struct_name, accessor + "[" + var + "]", member, rank + 1);
      }

      std::string outro;
      --this->indent;
      this->_write_indent(outro);
      outro += "}\n";
      --this->indent;
      this->_write_indent(outro);
      outro += "}\n";

      current_gen->bodies.read  += outro;
      current_gen->bodies.write += outro;

      if (rank == member.array_extents.size() - 1) {
         current_gen->bits_used += bitcount_per_whole_iteration; // since `_serialize_single_element` doesn't do this
      }
      return;
   }

   static_assert(false, "TODO: serialize as much as is possible up to the sector boundary");
   static_assert(false, "TODO: 'as much as is possible' means the first few whole iterations, and then a partial iteration if possible");

   this->_write_indent(current_gen->bodies.read);
   current_gen->bodies.read += "// data to be continued in next sector...\n";
   this->_write_indent(current_gen->bodies.write);
   current_gen->bodies.write += "// data to be continued in next sector...\n";
   //
   // Advance to next sector:
   //
   current_gen = &(this->generated_funcs.emplace_back());
   current_gen->top_level_types_touched.push_back(top_level_struct_name);
   //
   this->_write_indent(current_gen->bodies.read);
   current_gen->bodies.read += "// data continued from previous sector...\n";
   this->_write_indent(current_gen->bodies.write);
   current_gen->bodies.write += "// data continued from previous sector...\n";

   static_assert(false, "serialize what remains");
}

void treewalker::_walk_struct(std::string top_level_struct_name, std::string accessor, const ast::structure& dfn) {
   auto& reg = registry::get();
   auto* current_gen = &this->generated_funcs.back();

   for (const auto& member_ptr : dfn.members) { // ref, because these are unique_ptrs
      std::string current_accessor = accessor;

      // Resolve the accessor fully, if the member is an inlined union member.
      const auto* member = member_ptr.get();
      assert(member != nullptr);
      while (auto* casted = dynamic_cast<const ast::inlined_union_member*>(member)) {
         current_accessor += member->name;
         current_accessor += '.';
         member = &(casted->get_member_to_serialize());
      }
      current_accessor += member->name;

      struct {
         size_t single = 0;
         size_t total  = 0;
      } bitcounts = {
         .single = member->compute_single_element_bitcount(),
         .total  = member->compute_total_bitcount(),
      };
      bool would_overflow_sector = bytespan_of(current_gen->bits_used + bitcounts.total) <= this->bytes_per_sector;

      if (!would_overflow_sector) {
         //
         // If this member (the whole thing, i.e. if it's an array then all elements) would fit 
         // in the current sector, then just serialize it as a single unit.
         //
         bool do_whole = true;
         if constexpr (always_descend) {
            if (dynamic_cast<const ast::struct_member*>(member))
               do_whole = false;
         }
         if (do_whole) {
            this->_serialize_whole_member(current_accessor, *member);
            continue;
         }
      }

      //
      // If we make it here, then the whole member would not fit.
      //

      if (member->array_extents.size()) {
         //
         // Handle an array.
         //
         this->_walk_array_rank(top_level_struct_name, current_accessor, *member, 0);
         continue;
      }

      //
      // Handle all other members.
      //
      size_t struct_paused_at = 0;
      if (auto* casted = dynamic_cast<const ast::struct_member*>(member)) {
         const auto* casted_dfn = reg.lookup_struct_definition(casted->type_name);
         assert(casted_dfn);

         size_t i;
         for (i = 0; i < casted_dfn->members.size(); ++i) {
            std::string accessor = current_accessor + '.';
            const auto* nested   = casted_dfn->members[i].get();
            while (auto* ium = dynamic_cast<const ast::inlined_union_member*>(nested)) {
               accessor += nested->name;
               accessor += '.';
               nested = &(ium->get_member_to_serialize());
            }
            accessor += nested->name;

            const auto size = nested->compute_total_bitcount();
            if (bytespan_of(current_gen->bits_used + size) <= this->bytes_per_sector) {
               this->_serialize_whole_member(accessor, *nested);
            } else {
               break;
            }
         }
         struct_paused_at = i;
         assert(struct_paused_at < casted_dfn->members.size()); // if the whole struct fit, then why are we here?
      }

      this->_write_indent(current_gen->bodies.read);
      current_gen->bodies.read += "// data to be continued in next sector...\n";
      this->_write_indent(current_gen->bodies.write);
      current_gen->bodies.write += "// data to be continued in next sector...\n";
      //
      // Advance to next sector:
      //
      current_gen = &(this->generated_funcs.emplace_back());
      current_gen->top_level_types_touched.push_back(top_level_struct_name);
      //
      this->_write_indent(current_gen->bodies.read);
      current_gen->bodies.read += "// data continued from previous sector...\n";
      this->_write_indent(current_gen->bodies.write);
      current_gen->bodies.write += "// data continued from previous sector...\n";

      if (auto* casted = dynamic_cast<const ast::struct_member*>(member)) {
         if (struct_paused_at > 0) {
            const auto* casted_dfn = reg.lookup_struct_definition(casted->type_name);
            assert(casted_dfn);

            for (size_t i = struct_paused_at; i < casted_dfn->members.size(); ++i) {
               std::string accessor = current_accessor + '.';
               const auto* nested   = casted_dfn->members[i].get();
               while (auto* ium = dynamic_cast<const ast::inlined_union_member*>(nested)) {
                  accessor += nested->name;
                  accessor += '.';
                  nested = &(ium->get_member_to_serialize());
               }
               accessor += nested->name;

               const auto size = nested->compute_total_bitcount();
               if (bytespan_of(current_gen->bits_used + size) <= this->bytes_per_sector) {
                  this->_serialize_whole_member(accessor, *nested);
               } else {
                  break;
               }
            }
            continue;
         }
      }

      this->_serialize_whole_member(current_accessor, *member);
   }
}

void treewalker::generate(const serialization_params& params) {
   this->bytes_per_sector = params.bytes_per_sector;
   this->max_sectors      = params.sector_count;

   this->current_sector = 0;

   auto& reg = registry::get_or_create();
   this->generated_funcs.emplace_back();
   for (const auto& struct_name : params.top_level_structs) {
      const auto* dfn = reg.lookup_struct_definition(struct_name);
      assert(dfn);

      std::string accessor_base = "p_"s + struct_name + ".";

      auto* current_gen = &this->generated_funcs.back();
      current_gen->top_level_types_touched.push_back(struct_name);

      this->_walk_struct(struct_name, accessor_base, *dfn);
   }

   // Done scanning all top-level structs
   static_assert(false, "TODO: anything else?");
}