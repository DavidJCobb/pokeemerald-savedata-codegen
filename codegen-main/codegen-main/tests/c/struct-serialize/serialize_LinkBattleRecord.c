#include "struct-serialize//serialize_LinkBattleRecord.h"

#include "global.h" // struct definition

#include "string_util.h" // gflib; for StringLength

// check constants:
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif

// TODO:
// void lu_BitstreamRead_LinkBattleRecord(struct lu_BitstreamState* state, struct LinkBattleRecord* dst);

void lu_BitstreamWrite_LinkBattleRecord(struct lu_BitstreamState* state, const struct LinkBattleRecord* src) {
   lu_BitstreamWrite_string(state, src.name, PLAYER_NAME_LENGTH, 3);
   lu_BitstreamWrite_u16(state, src.trainerId, 16);
   lu_BitstreamWrite_u16(state, src.wins, 16);
   lu_BitstreamWrite_u16(state, src.losses, 16);
   lu_BitstreamWrite_u16(state, src.draws, 16);
}