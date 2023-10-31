#include "struct-serialize//serialize_TrainerNameRecord.h"

#include "global.h" // struct definition

#include "string_util.h" // gflib; for StringLength

// check constants:
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif

// TODO:
// void lu_BitstreamRead_TrainerNameRecord(struct lu_BitstreamState* state, struct TrainerNameRecord* dst);

void lu_BitstreamWrite_TrainerNameRecord(struct lu_BitstreamState* state, const struct TrainerNameRecord* src) {
   lu_BitstreamWrite_u32(state, src.trainerId, 32);
   lu_BitstreamWrite_string(state, src.trainerName, PLAYER_NAME_LENGTH, 3);
}