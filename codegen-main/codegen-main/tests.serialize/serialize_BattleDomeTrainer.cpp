#include "./tests.serialize//serialize_BattleDomeTrainer.h"

#include "global.h"

// TODO:
// void lu_BitstreamRead_BattleDomeTrainer(struct lu_BitstreamState* state, struct BattleDomeTrainer* dst);

void lu_BitstreamWrite_BattleDomeTrainer(struct lu_BitstreamState* state, const struct BattleDomeTrainer* src) {
   lu_BitstreamWrite_u16(state, src.trainerId, 10);
   lu_BitstreamWrite_bool(state, src.isEliminated);
   lu_BitstreamWrite_u8(state, src.eliminatedAt, 2);
   lu_BitstreamWrite_u8(state, src.forfeited, 3);
}