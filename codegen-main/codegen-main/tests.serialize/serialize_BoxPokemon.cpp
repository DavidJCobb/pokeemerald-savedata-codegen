#include "./tests.serialize//serialize_BoxPokemon.h"

#include "string_util.h" // gflib; for StringLength

#include "pokemon.h"

// TODO:
// void lu_BitstreamRead_BoxPokemon(struct lu_BitstreamState* state, struct BoxPokemon* dst);

void lu_BitstreamWrite_BoxPokemon(struct lu_BitstreamState* state, struct BoxPokemon* src) {
   lu_BitstreamWrite_u32(state, src.personality, 32);
   lu_BitstreamWrite_u32(state, src.otId, 32);
   lu_BitstreamWrite_string(state, src.nickname, 10, 4);
   lu_BitstreamWrite_u8(state, src.language, 3);
   lu_BitstreamWrite_u8(state, src.isBadEgg, 8);
   lu_BitstreamWrite_u8(state, src.hasSpecies, 8);
   lu_BitstreamWrite_u8(state, src.isEgg, 8);
   lu_BitstreamWrite_u8(state, src.unused, 8);
   lu_BitstreamWrite_string(state, src.otName, 7, 3);
   lu_BitstreamWrite_u8(state, src.markings, 8);
   lu_BitstreamWrite_u16(state, src.checksum, 16);
   lu_BitstreamWrite_u16(state, src.unknown, 16);
   lu_BitstreamWrite_u32(state, src.secure, 32);
}