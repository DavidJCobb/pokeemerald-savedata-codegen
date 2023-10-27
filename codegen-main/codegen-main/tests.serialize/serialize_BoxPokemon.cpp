#include "./tests.serialize//serialize_BoxPokemon.h"

// dependencies
#include "./serialize_PokemonSubstruct.h"

#include "string_util.h" // gflib; for StringLength

#include "pokemon.h"

// TODO:
// void lu_BitstreamRead_BoxPokemon(struct lu_BitstreamState* state, struct BoxPokemon* dst);

void lu_BitstreamWrite_BoxPokemon(struct lu_BitstreamState* state, const struct BoxPokemon* src) {
   lu_BitstreamWrite_u32(state, src.personality, 32);
   lu_BitstreamWrite_u32(state, src.otId, 32);
   lu_BitstreamWrite_string(state, src.nickname, 10, 4);
   lu_BitstreamWrite_u8(state, src.language, 3);
   lu_BitstreamWrite_bool(state, src.isBadEgg);
   lu_BitstreamWrite_bool(state, src.hasSpecies);
   lu_BitstreamWrite_bool(state, src.isEgg);
   lu_BitstreamWrite_u8(state, src.unused, 5);
   lu_BitstreamWrite_string(state, src.otName, 7, 3);
   lu_BitstreamWrite_u8(state, src.markings, 8);
   lu_BitstreamWrite_u16(state, src.checksum, 16);
   lu_BitstreamWrite_u16(state, src.unknown, 16);
   {
      u16 i;
      for (i = 0; i < 4; ++i) { 
            lu_BitstreamWrite_u32(state, src.secure.raw[i], 32);
      }
   }
}