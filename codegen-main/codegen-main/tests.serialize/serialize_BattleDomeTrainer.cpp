#include "./tests.serialize//serialize_BattleDomeTrainer.h"

#include "global.h"

// TODO:
// void lu_BitstreamRead_BattleDomeTrainer(struct lu_BitstreamState* state, struct BattleDomeTrainer* dst);

void lu_BitstreamWrite_BattleDomeTrainer(struct lu_BitstreamState* state, struct BattleDomeTrainer* src) {
   lu_BitstreamWrite_u16(state, src.trainerId, 16);
   lu_BitstreamWrite_u16(state, src.isEliminated, 16);
   lu_BitstreamWrite_u16(state, src.eliminatedAt, 16);
   lu_BitstreamWrite_u16(state, src.forfeited, 16);
}