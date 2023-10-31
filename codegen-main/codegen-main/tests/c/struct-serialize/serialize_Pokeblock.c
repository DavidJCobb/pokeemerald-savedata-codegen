#include "struct-serialize//serialize_Pokeblock.h"

#include "global.h" // struct definition

// TODO:
// void lu_BitstreamRead_Pokeblock(struct lu_BitstreamState* state, struct Pokeblock* dst);

void lu_BitstreamWrite_Pokeblock(struct lu_BitstreamState* state, const struct Pokeblock* src) {
   lu_BitstreamWrite_u8(state, src.color, 8);
   lu_BitstreamWrite_u8(state, src.spicy, 8);
   lu_BitstreamWrite_u8(state, src.dry, 8);
   lu_BitstreamWrite_u8(state, src.sweet, 8);
   lu_BitstreamWrite_u8(state, src.bitter, 8);
   lu_BitstreamWrite_u8(state, src.sour, 8);
   lu_BitstreamWrite_u8(state, src.feel, 8);
}