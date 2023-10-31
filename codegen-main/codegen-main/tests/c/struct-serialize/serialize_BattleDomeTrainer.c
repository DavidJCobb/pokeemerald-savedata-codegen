#include "struct-serialize//serialize_BattleDomeTrainer.h"

#include "global.h" // struct definition

void lu_BitstreamRead_BattleDomeTrainer(struct lu_BitstreamState* state, const struct BattleDomeTrainer* src) {
   src.trainerId = lu_BitstreamRead_u16(state, 10);
   src.isEliminated = lu_BitstreamRead_bool(state, src.isEliminated);
   src.eliminatedAt = lu_BitstreamRead_u8(state, 2);
   src.forfeited = lu_BitstreamRead_u8(state, 3);
}
void lu_BitstreamWrite_BattleDomeTrainer(struct lu_BitstreamState* state, const struct BattleDomeTrainer* src) {
   lu_BitstreamWrite_u16(state, src.trainerId, 10);
   lu_BitstreamWrite_bool(state, src.isEliminated);
   lu_BitstreamWrite_u8(state, src.eliminatedAt, 2);
   lu_BitstreamWrite_u8(state, src.forfeited, 3);
}