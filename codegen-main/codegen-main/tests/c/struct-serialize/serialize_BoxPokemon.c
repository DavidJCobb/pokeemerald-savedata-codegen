#include "struct-serialize//serialize_BoxPokemon.h"

#include "pokemon.h" // struct definition

// dependencies
#include "./serialize_PokemonSubstruct.h"

#include "string_util.h" // gflib; for StringLength

// check constants:
#if POKEMON_NAME_LENGTH != 10
   #error Constant `POKEMON_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_BoxPokemon(struct lu_BitstreamState* state, const struct BoxPokemon* src) {
   src.personality = lu_BitstreamRead_u32(state, 32);
   src.otId = lu_BitstreamRead_u32(state, 32);
   lu_BitstreamRead_string_optional_terminator(state, src.nickname, POKEMON_NAME_LENGTH);
   src.language = lu_BitstreamRead_u8(state, 3);
   src.isBadEgg = lu_BitstreamRead_bool(state, src.isBadEgg);
   src.hasSpecies = lu_BitstreamRead_bool(state, src.hasSpecies);
   src.isEgg = lu_BitstreamRead_bool(state, src.isEgg);
   src.unused = lu_BitstreamRead_u8(state, 5);
   lu_BitstreamRead_string_optional_terminator(state, src.otName, PLAYER_NAME_LENGTH);
   src.markings = lu_BitstreamRead_u8(state, 8);
   src.checksum = lu_BitstreamRead_u16(state, 16);
   src.unknown = lu_BitstreamRead_u16(state, 16);
   {
      u16 i;
      for (i = 0; i < (NUM_SUBSTRUCT_BYTES * 4) / 4; ++i) { 
            src.secure.raw[i] = lu_BitstreamRead_u32(state, 32);
      }
   }
}
void lu_BitstreamWrite_BoxPokemon(struct lu_BitstreamState* state, const struct BoxPokemon* src) {
   lu_BitstreamWrite_u32(state, src.personality, 32);
   lu_BitstreamWrite_u32(state, src.otId, 32);
   lu_BitstreamWrite_string_optional_terminator(state, src.nickname, POKEMON_NAME_LENGTH);
   lu_BitstreamWrite_u8(state, src.language, 3);
   lu_BitstreamWrite_bool(state, src.isBadEgg);
   lu_BitstreamWrite_bool(state, src.hasSpecies);
   lu_BitstreamWrite_bool(state, src.isEgg);
   lu_BitstreamWrite_u8(state, src.unused, 5);
   lu_BitstreamWrite_string_optional_terminator(state, src.otName, PLAYER_NAME_LENGTH);
   lu_BitstreamWrite_u8(state, src.markings, 8);
   lu_BitstreamWrite_u16(state, src.checksum, 16);
   lu_BitstreamWrite_u16(state, src.unknown, 16);
   {
      u16 i;
      for (i = 0; i < (NUM_SUBSTRUCT_BYTES * 4) / 4; ++i) { 
            lu_BitstreamWrite_u32(state, src.secure.raw[i], 32);
      }
   }
}