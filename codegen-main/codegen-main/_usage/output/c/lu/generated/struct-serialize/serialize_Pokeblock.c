#include "lu/generated/struct-serialize//serialize_Pokeblock.h"

#include "global.h" // struct definition

void lu_BitstreamRead_Pokeblock(struct lu_BitstreamState* state, const struct Pokeblock* src) {
   src.color = lu_BitstreamRead_u8(state, 8);
   src.spicy = lu_BitstreamRead_u8(state, 8);
   src.dry = lu_BitstreamRead_u8(state, 8);
   src.sweet = lu_BitstreamRead_u8(state, 8);
   src.bitter = lu_BitstreamRead_u8(state, 8);
   src.sour = lu_BitstreamRead_u8(state, 8);
   src.feel = lu_BitstreamRead_u8(state, 8);
}
void lu_BitstreamWrite_Pokeblock(struct lu_BitstreamState* state, const struct Pokeblock* src) {
   lu_BitstreamWrite_u8(state, src.color, 8);
   lu_BitstreamWrite_u8(state, src.spicy, 8);
   lu_BitstreamWrite_u8(state, src.dry, 8);
   lu_BitstreamWrite_u8(state, src.sweet, 8);
   lu_BitstreamWrite_u8(state, src.bitter, 8);
   lu_BitstreamWrite_u8(state, src.sour, 8);
   lu_BitstreamWrite_u8(state, src.feel, 8);
}