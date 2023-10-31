#include "struct-serialize//serialize_ContestWinner.h"

#include "global.h" // struct definition

#include "string_util.h" // gflib; for StringLength

// check constants:
#if POKEMON_NAME_LENGTH != 10
   #error Constant `POKEMON_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif

// TODO:
// void lu_BitstreamRead_ContestWinner(struct lu_BitstreamState* state, struct ContestWinner* dst);

void lu_BitstreamWrite_ContestWinner(struct lu_BitstreamState* state, const struct ContestWinner* src) {
   lu_BitstreamWrite_u32(state, src.personality, 32);
   lu_BitstreamWrite_u32(state, src.trainerId, 32);
   lu_BitstreamWrite_u16(state, src.species, 11);
   lu_BitstreamWrite_u8(state, src.contestCategory, 8);
   lu_BitstreamWrite_string(state, src.monName, POKEMON_NAME_LENGTH, 4);
   lu_BitstreamWrite_string(state, src.trainerName, PLAYER_NAME_LENGTH, 3);
   lu_BitstreamWrite_u8(state, src.contestRank, 8);
}