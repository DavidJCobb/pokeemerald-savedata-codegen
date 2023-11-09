
This bitstream functionality was copied out of another project of mine, DovahKit, and has been cleaned up slightly for more general use. As such, it has more features than are needed for this project.

# Features

* `reader` and `writer` classes for streaming bitpacked data in a buffer.
* These classes can operate on any struct that defines member functions of the form `void stream(reader&)` and `void stream(writer&)`.
* Multi-stream calls: `my_reader.stream(a, b, c, d)` will stream all of `a`, `b`, `c`, and `d`. This makes it more ergonomic to write code that serializes multiple values in a row.
* Ability to explicitly override the bitcount for a value, by asking to read/write `override_bitcount<5>::of(my_variable)`.
* When streaming a container, you must specify the length bits, e.g. `my_reader.stream<5>(my_string)`.
* "Parse steps" are automatically tracked: as you serialize nested structs and values, the stream classes track the hierarchy by using metaprogramming and compiler-specific macros to grab the names of the types being serialized. Bitstream-specific exceptions contain this information.

## Enum helpers

You can define metadata about any `enum` type, and the bitstream classes will lean on that metadata to control how values are serialized. An example:

```c++
#pragma once

namespace foo {
   enum class comparison_operator {
      equal,
      less,
      greater,
      not_equal,
      less_or_equal,
      greater_or_equal,
   };
}

#include "lu/bitstreams/enum_serialization_options.h"
template<> struct lu::bitstreams::enum_serialization_options<foo::comparison_operator> {
   using value_type = foo::comparison_operator;

   static constexpr const size_t bitcount = 3;
   static constexpr const auto   valid_values = []() {
      using enum value_type;
      return std::array{ equal, less, greater, not_equal, less_or_equal, greater_or_equal };
   }();
};
```

This allows the bitstream classes to know how many bits to use for the enum without you having to specify that information with every serialization call. Additionally, if an invalid enum value (one not in the list above) is ever read from a stream, a read exception will be thrown.

As a bonus, if you *do* specify an explicit bitcount for an individual read, this will be checked via `static_assert` to ensure that all enum values can fit.

Note that as of this writing, you must specialize `lu::bitstreams::enum_serialization_options` from outside of any namespace, due to MSVC's incomplete support for P1787.