#pragma once
#include <memory>
#include <string>
#include <vector>
#include "./member.h"

namespace ast {
   class structure {
      public:
         std::string name;
         std::string header;
         std::vector<std::unique_ptr<member>> members;
         std::optional<size_t> checksum_member_index;
         struct {
            bool is_packed = false;
         } c_type_info;
   
         constexpr const std::string to_string() const;

         constexpr const std::string as_c_member_declaration_list() const;

         constexpr std::vector<std::string> get_all_direct_constant_dependencies() const;
         constexpr std::vector<std::string> get_all_direct_struct_dependencies() const;
         constexpr bool has_any_string_members() const;

         constexpr size_t compute_total_bitcount() const;

         constexpr size_t compute_unpacked_bytecount() const;

         // void functor(const ast::member*, size_t offset_bytes, size_t offset_bits, size_t bytecount, size_t bitfield_size);
         template<typename Functor>
         constexpr size_t count_unpacked_bytecounts(Functor&&) const;
   };
}

#include "./structure.inl"