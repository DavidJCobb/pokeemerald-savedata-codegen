#include "struct-serialize//serialize_RecordMixingGift.h"

#include "global.h" // struct definition

// dependencies
#include "./serialize_RecordMixingGiftData.h"

// TODO:
// void lu_BitstreamRead_RecordMixingGift(struct lu_BitstreamState* state, struct RecordMixingGift* dst);

void lu_BitstreamWrite_RecordMixingGift(struct lu_BitstreamState* state, const struct RecordMixingGift* src) {
   lu_BitstreamWrite_u32(state, src.checksum, 32);
   lu_BitstreamWrite_RecordMixingGiftData(state, &src.data);
}