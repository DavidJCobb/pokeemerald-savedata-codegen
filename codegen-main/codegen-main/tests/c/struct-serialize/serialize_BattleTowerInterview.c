#include "struct-serialize//serialize_BattleTowerInterview.h"

#include "global.h" // struct definition

#include "string_util.h" // gflib; for StringLength

// check constants:
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if POKEMON_NAME_LENGTH != 10
   #error Constant `POKEMON_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_BattleTowerInterview(struct lu_BitstreamState* state, const struct BattleTowerInterview* src) {
   src.playerSpecies = lu_BitstreamRead_u16(state, 11);
   src.opponentSpecies = lu_BitstreamRead_u16(state, 11);
   lu_BitstreamRead_string(state, src.opponentName, PLAYER_NAME_LENGTH, 3);
   lu_BitstreamRead_string(state, src.opponentMonNickname, POKEMON_NAME_LENGTH, 4);
   src.opponentLanguage = lu_BitstreamRead_u8(state, 3);
}
void lu_BitstreamWrite_BattleTowerInterview(struct lu_BitstreamState* state, const struct BattleTowerInterview* src) {
   lu_BitstreamWrite_u16(state, src.playerSpecies, 11);
   lu_BitstreamWrite_u16(state, src.opponentSpecies, 11);
   lu_BitstreamWrite_string(state, src.opponentName, PLAYER_NAME_LENGTH, 3);
   lu_BitstreamWrite_string(state, src.opponentMonNickname, POKEMON_NAME_LENGTH, 4);
   lu_BitstreamWrite_u8(state, src.opponentLanguage, 3);
}