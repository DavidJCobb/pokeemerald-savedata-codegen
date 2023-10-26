# `pokeemerald` savedata code generator

The goal of this project is to be able to do code generation for Pokemon Emerald savedata.

The vanilla game stores most of its persistent game state in two key data structures: `SaveBlock1`, used for world state; and `SaveBlock2`, used for character state. When it comes time to save that data, these structures are `memcpy`'d from RAM into flash memory in slices no larger than 4096 bytes. In practice, `SaveBlock1` fills most of the two slices allotted for it, while `SaveBlock2` fills most of the single slice allotted for it. This does not leave room for saving additional data, e.g. for adding new features and options to the game.

We can gain significant space reductions from bitpacking all of the saved data, instead of just `memcpy`ing it. However, this is not as simple as it sounds, because again, data is saved in 4KB slices. With `memcpy`, that's an easy limitation to work around. With bitpacking and no code generation or metaprogramming, you'd have to manually write serialization calls *right up* to the edge of the first slice, and then write calls for the second slice to continue from where you left off; this isn't maintainable.

The purpose of this project, then, is to facilitate generating serialization code. The ultimate goal is for the program to be able to count how many bits and bytes it's serializing at a time, and be able to "slice" the serialization code at the appropriate boundary &mdash; possibly in the middle of serializing a struct, even, with some members in the former slice and the rest in the latter slice.

## Why are structs defined the way that they are?

My initial plan was to use the game's original C headers as a single source of truth for what the definition of a struct actually is. However, I quickly realized that this would require having a C parser that could generate a full AST of all involved structs, including comments used to annotate fields with their minimums, maximums, and other information that could be used to automatically generate the minimum necessary bitcount needed for serialization. This seemed untenable.

I eventually had an idea: what if we just generate the struct definitions, too? We can use XML as our single source of truth; plenty of parsers exist for that.