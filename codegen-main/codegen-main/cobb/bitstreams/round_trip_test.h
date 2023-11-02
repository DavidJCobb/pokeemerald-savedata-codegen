#pragma once
#include <type_traits>
#include "./reader.h"
#include "./writer.h"
#include "./round_trip_test_scramble.h"

namespace cobb::bitstreams {
   //
   // This template can be used to check whether a struct passes a round-trip 
   // serialization test. That is: if we create an instance, write it, and then 
   // read what we've just written, do we get consistent results?
   //
   template<typename T>
   constexpr const bool round_trip_test = []() {
      if constexpr (
         sizeof(T) == 0
         || std::is_empty_v<T> // guard against a bug in MSVC's bit-cast intrinsic affecting empty structs, which are technically sizeof 1
      ) {
         return true;
      } else {
         T src = {};

         writer w;
         w.stream(w.header());
         w.stream(src);

         T dst = {};
         if constexpr (std::is_trivially_copyable_v<T>) {
            //
            // Scramble `dst` before the read, so that a totally failed or no-op read 
            // doesn't automatically compare as equal.
            //
            round_trip_test_scramble(dst);
         }

         reader r;
         r.set_buffer(w.data(), w.get_bytespan());
         r.stream(dst);

         return src == dst;
      }
   }();
}
