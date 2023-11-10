# `pokeemerald` savedata code generator

The goal of this project is to be able to do code generation for Pokemon Emerald savedata.

The vanilla game stores most of its persistent game state in two key data structures: `SaveBlock1`, used for world state; and `SaveBlock2`, used for character state. When it comes time to save that data, these structures are `memcpy`'d from RAM into flash memory in slices no larger than 4096 bytes. In practice, `SaveBlock1` fills most of the two slices allotted for it, while `SaveBlock2` fills most of the single slice allotted for it. This does not leave room for saving additional data, e.g. for adding new features and options to the game.

We can gain significant space reductions from bitpacking all of the saved data, instead of just `memcpy`ing it. However, this is not as simple as it sounds, because again, data is saved in 4KB slices. With `memcpy`, that's an easy limitation to work around. With bitpacking and no code generation or metaprogramming, you'd have to manually write serialization calls *right up* to the edge of the first slice, and then write calls for the second slice to continue from where you left off; this isn't maintainable.

The purpose of this project, then, is to facilitate generating serialization code. The ultimate goal is for the program to be able to count how many bits and bytes it's serializing at a time, and be able to "slice" the serialization code at the appropriate boundary &mdash; possibly in the middle of serializing a struct, even, with some members in the former slice and the rest in the latter slice.

## Why are structs defined the way that they are?

My initial plan was to use the game's original C headers as a single source of truth for what the definition of a struct actually is. However, I quickly realized that this would require having a C parser that could generate a full AST of all involved structs, including comments used to annotate fields with their minimums, maximums, and other information that could be used to automatically generate the minimum necessary bitcount needed for serialization. This seemed untenable.

I eventually had an idea: what if we just generate the struct definitions, too? We can use XML as our single source of truth; plenty of parsers exist for that. So you end up with code like

```c
struct Foo {
#include "lu/generated/struct-members/Foo.members.inl"
};
```

## Feature set

* Structs and their members are defined in XML
* Any constants related to these structs must be mirrored in XML
   * We generate preprocessor directives in serialization code to check at compile-time that the constant has consistent values between XMl and C.
* We generate...
   * Struct member lists
   * Code to serialize an entire struct
   * Code to serialize a sector group one sector at a time
   * Function tables and definitions referenced from `save.c`

## Usage

The `pokeemerald` branch I've set up to use this generated code has an altered `save.h` and `save.c`, and as such requires specific folder paths. Additionally, all savedata-related structs have been hollowed out and pointed at generated member lists. You'll therefore need to use the following paths:

| Purpose | Path |
| - | - |
| Sector group serialize | lu/generated/sector-serialize/ |
| Struct members | lu/generated/struct-members/ |
| Struct serialize | lu/generated/struct-serialize/ |
| Save functors | lu/generated/save-functors/ |

Additionally, the bitstream read/write functions are stored in `lu/bitstreams.h` / `lu/bitstreams.c`, and some changes are made to `item.c` to ensure that saving a 10-bit item quantity (for the range [0, 999]) doesn't break the game's inventory encryption when the unused upper bits get truncated.

## The XML format

* `data` element
   * `dependencies` element
      * `xml` element(s)
         * `path` attribute specifies another XML file that the current file depends on
   * `sector-groups` element
      * `group` element(s)
         * attribute: `name`: used to identify the sector group, and substituted into C function names (so, it must be valid within a C identifier)
         * attribute: `sector-count`: the maximum number of sectors that this group can consume.
         * attribute: `base-sector-id`: the first sector within a save slot that this group uses; required for dumping savegame contents.
         * `struct` element(s)
            * attribute: `name`: the typename of the struct
            * attribute: `var`: the name of the global variable (of this type) to serialize (if it's not a pointer)
            * attribute: `ptr`: the name of the global variable (of this type) to serialize (if it's a pointer)
            * attribute: `var-header`: the C include path for the header declaring the variable
   * `constants` element
      * `constant` element(s)
         * attribute: `name`: the C-language name for the constant
         * attribute: `value`: the integer value of this constant; expressions are not supported
         * attribute: `header-path`: the C include path for the header that defines this constant
      * `header` element(s)
         * attribute: `path`: the C include path for the header that defines these constants
         * `constant` element(s)
            * As above, but specifying `header-path` here is an error.
   * `heritables` element
      * Element(s) for each heritable. See below.
   * `types` element
      * `struct` element(s)
         * See below.
      * `union` element(s)
         * See below.

### Heritables

Heritables act as templates, of a sort, for data members in a struct. If a struct member references a heritable via the `inherit` attribute, then the loader acts as though all attributes specified on that heritable were also specified on the struct member (unless the struct member overrides them).

The following tagnames are supported:

| Tagname | Details |
| - | - |
| `u8`  | This heritable is a number-type, and uses `u8` as its C-level type. |
| `u16` | This heritable is a number-type, and uses `u16` as its C-level type. |
| `u32` | This heritable is a number-type, and uses `u32` as its C-level type. |
| `s8`  | This heritable is a number-type, and uses `s8` as its C-level type. |
| `s16` | This heritable is a number-type, and uses `s16` as its C-level type. |
| `s32` | This heritable is a number-type, and uses `s32` as its C-level type. |
| `int` | This heritable is a number-type, and uses `int` as its C-level type. |
| `number` | This heritable is a number-type, but does not specify a C-level type. |
| `string` | This heritable is a string-type. |

The `name` attribute uniquely identifies the heritable. All other attributes are the same as on members (see below).

### Unions

"Blind" unions are unions that we make no attempt to bitpack or introspect; we instead just `memcpy` them. Dealing with these would require generating code for checking the union's type, and this won't work consistently for all unions; it's not something that can easily be done generically.

Blind unions are defined with tagname `union` and the following attributes:

| Attribute | Details |
| - | - |
| `name` | Required: the name of the union as declared in C. |
| `header` | The header the union is declared in. Required if the union is declared via the `typedef` keyword. |
| `defined-via-typedef` | Must be `true` or `false` depending on how the union is declared in C code. |
| `do-not-codegen` | Must be `true`. We currently cannot generate serialization code for the inside of a named union type. |
| `sizeof` | The size in bytes of the union. |

### Structs

Structs always use the `struct` tagname and have the following attributes:

| Attribute | Details |
| - | - |
| `name` | Required: the name of the struct as declared in C. |
| `header` | The header the struct is declared in. Required if the struct is declared via the `typedef` keyword. |
| `defined-via-typedef` | Must be `true` or `false` depending on how the union is declared in C code. |
| `is-packed` | Set this to `true` if the struct uses GCC's `packed` attribute. When the codegen system produces a results report, this attribute affects how we calculate the amount of space saved for the struct. |

Structs can contain a single child element, `fields`. This child element can contain children representing each member.

#### Members

The following tagnames are supported:

| Tagname | Details |
| - | - |
| `u8`  | This member is a number-type, and uses `u8` as its C-level type. |
| `u16` | This member is a number-type, and uses `u16` as its C-level type. |
| `u32` | This member is a number-type, and uses `u32` as its C-level type. |
| `s8`  | This member is a number-type, and uses `s8` as its C-level type. |
| `s16` | This member is a number-type, and uses `s16` as its C-level type. |
| `s32` | This member is a number-type, and uses `s32` as its C-level type. |
| `int` | This member is a number-type, and uses `int` as its C-level type. |
| `pointer` | This member is a pointer. |
| `number` | This member is a number-type, and specifies a C-level type via the `c-type` attribute (directly or through a heritable). |
| `string` | This member is a string-type. |
| `field` | This member is a struct, union, or otherwise specifies a C-level type via the `c-type` attribute (directly or through a heritable). |

##### Member types

###### Common attributes on all members

| Attribute | Details |
| - | - |
| `name` | Required. The name of the member; its C identifier. |
| `inherit` | Optional. The name of a heritable to pull values from. |
| `do-not-serialize` | Optional. If `true`, this member is skipped during serialization &mdash; neither saved nor loaded. |
| `c-alignment` | Optional. Use if a member has the `ALIGNED(...)` specifier; value is the alignment in bytes. |
| `const` | Optional. Set to `true` or `false`; defaults to `false`. For pointers, this affects mutability of the pointed-to value, not the pointer itself (i.e. `const u8*`, not `u8* const`). |

###### Numeric

For numbers, the following attributes are valid:

| Attribute | Details |
| - | - |
| `c-type` | Can be any of the following: `u8`; `u16`; `u32`; `s8`; `s16`; `s32`; `int`. Optional if any of those was used as the tagname of the member, the tagname of its heritable, or the `c-type` of its heritable. |
| `c-bitfield` | Optional. If the struct member is a bitfield in RAM, this is the number of bits, e.g. `5` for `u8 foo : 5`. This can be the name of a `<constant />`. |
| `min` | Optional. The minimum allowed value that this member can have during normal gameplay. This can be the name of a `<constant />`. |
| `max` | Optional. The maximum allowed value that this member can have during normal gameplay. This can be the name of a `<constant />`. |
| `serialization-bitcount` | Optional. Force the value to be serialized with a given bitcount, regardless of all other considerations. |
| `is-checksum` | Optional. Currently read but doesn't affect anything. Set to `true` or `false`; defaults to `false`. |

The bitcount used to serialize a number depends on its underlying C type and its minimum and maximum, when specified. For example, a `u8` with an unspecified minimum and a `max` of `31` implicitly has the range [0, 31] and so is serialized with 5 bits; and a `u16` declared as a 10-bit bitfield in C will be serialized with 10 bits. If both the minimum and maximum are declared, they can be used to constrain the bitcount further; for example, given a range [16, 47], you still only need 5 bits, because one can subtract 16 before writing and add it back after reading.

###### Pointer

For pointers, the following attributes are valid:

| Attribute | Details |
| - | - |
| `c-type` | Required. Can be any of the following: `u8`; `u16`; `u32`; `s8`; `s16`; `s32`; `int`. Defines the pointed-to type. |

###### String

For strings, the following attributes are valid:

| Attribute | Details |
| - | - |
| `char-type` | Required. Can be any of the following: `u8`; `u16`; `u32`; `s8`; `s16`; `s32`; `int`. However, currently only `u8` will serialize properly, and Game Freak exclusively uses `u8` as their character type anyway. |
| `length` | The maximum length of the string, not including a terminator byte if one is mandatory. This can be the name of a `<constant />`. |
| `only-early-terminator` | If `true`, then the string doesn't require a terminator byte, and only includes one if it uses fewer than the allotted number of characters. Defaults to `false`. If you see a string in C source code that's declared as `u8 otName[PLAYER_NAME_LENGTH]` and not `u8 otName[PLAYER_NAME_LENGTH + 1]`, then set this attribute to `true` for it. |

###### Named struct/union

For struct or blind-union members, the following attributes are valid:

| Attribute | Details |
| - | - |
| `c-type` | Required. The typename of the struct or union. |
| `c-type-decl` | The keyword to use when specifying the type; allowed values are `struct`, `union`, or blank. (A future update to the codegen system may remove this attribute, since it can theoretically be deduced from whether the typename is a struct or union, and whether it was declared with `typedef`.) Defaults to `struct` if unspecified. |

###### Inlined union

Inlined union members are those which look like this:

```c
struct ContainingStruct {
   union {
      u8 bar;
      u16 baz;
   } foo; // <---------- this lil' guy
};
```

For inline union members, the following attributes are valid:

| Attribute | Details |
| - | - |
| `member-to-serialize` | Required. Specifies, by name, one of this union's members &mdash; the one that serialization code should access and serialize. It is assumed that this nested member is the largest in size of all nested members, or that all nested members are equal in size. |

Inline union members must contain other members as child elements. They do not need to be wrapped in `<fields />`.

##### Member child elements

The following child elements are valid:

| Tagname | Details |
| - | - |
| `line-comment` | Text-content is a line comment to be written after the member in its member list. Line breaks are not checked for and will likely produce broken code as output. |
| `array-rank` | Used to indicate that this member is an array; the `extent` attribute is the array dimension (as a numeric constant or the name of a `<constant />`; the `extent-expr` attribute allows you to specify that in generated code, an expression should be written. For multi-dimensional arrays, use multiple `array-rank` children in order from left to right. |



## Known issues

* If a sector group ends up using fewer sectors than you've allotted to it (e.g. only using 3 out of 4), then the generated code will likely break. Nothing exists to handle this case. Using our "three out of four" example, we may generate function calls to serialize all four sectors yet only generate actual functions for the first three; and other issues like that.
   * Error reporting does exist for if a sector group consumes more sectors than it is allowed to.
* Line breaks in a `line-comment` element are not stripped or otherwise handled gracefully.
* XML parse warnings (e.g. on unrecognized attributes) are currently not displayed in the UI.
* The `c-type-decl` attribute should be removed everywhere it appears; we should deduce its value based on whether the typename is a `struct` or `union`, and whether the entity was declared via `typedef`. We already require those facts for other reasons.
* We require a `char-type` for strings, but the serialization code only supports `u8` strings. We don't generate different code for other character types, so they would just produce broken code. We should remove the `char-type` attribute and explicitly only support `u8` strings.
* The program contains several `assert`s instead of proper error handling and reporting, e.g. if directory creation fails during codegen.
* Code exists to dump bitpacked or bytepacked savegames. The latter code is unreachable (I used it when testing and debugging my changes to Emerald's save code), which is good because IIRC it doesn't bounds-check.
* The code for dumping bitpacked savegames will display dialog boxes upon hitting a bitstream read exception, but the dialog boxes don't show the type of exception reached or any information specific to each exception type.
* The code for loading data from XML is very messy, even with helper functions to remove boilerplate.
* The `registry` should be split apart: the `registry` should only handle loading and remembering XML definitions, and codegen and savegame dumping should go through separate singletons/structs that communicate with the `registry` to read the loaded struct and sector-group definitions as needed.