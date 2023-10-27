#include "./tests.serialize//serialize_BattleTowerInterview.h"

#include "string_util.h" // gflib; for StringLength

#include "global.h"

// TODO:
// void lu_BitstreamRead_BattleTowerInterview(struct lu_BitstreamState* state, struct BattleTowerInterview* dst);

void lu_BitstreamWrite_BattleTowerInterview(struct lu_BitstreamState* state, const struct BattleTowerInterview* src) {
   lu_BitstreamWrite_u16(state, src.playerSpecies, 11);
   lu_BitstreamWrite_u16(state, src.opponentSpecies, 11);
   lu_BitstreamWrite_string(state, src.opponentName, 7, 3);
   lu_BitstreamWrite_string(state, src.opponentMonNickname, 10, 4);
   lu_BitstreamWrite_u8(state, src.opponentLanguage, 3);
}