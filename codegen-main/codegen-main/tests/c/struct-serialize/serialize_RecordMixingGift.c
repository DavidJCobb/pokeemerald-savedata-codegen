#include "struct-serialize//serialize_RecordMixingGift.h"

#include "global.h" // struct definition

// dependencies
#include "./serialize_RecordMixingGiftData.h"

void lu_BitstreamRead_RecordMixingGift(struct lu_BitstreamState* state, const struct RecordMixingGift* src) {
   src.checksum = lu_BitstreamRead_u32(state, 32);
   lu_BitstreamRead_RecordMixingGiftData(state, &src.data);
}
void lu_BitstreamWrite_RecordMixingGift(struct lu_BitstreamState* state, const struct RecordMixingGift* src) {
   lu_BitstreamWrite_u32(state, src.checksum, 32);
   lu_BitstreamWrite_RecordMixingGiftData(state, &src.data);
}