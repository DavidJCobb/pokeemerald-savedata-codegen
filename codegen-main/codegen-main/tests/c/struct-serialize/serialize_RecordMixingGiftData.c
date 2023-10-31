#include "struct-serialize//serialize_RecordMixingGiftData.h"

#include "global.h" // struct definition

// TODO:
// void lu_BitstreamRead_RecordMixingGiftData(struct lu_BitstreamState* state, struct RecordMixingGiftData* dst);

void lu_BitstreamWrite_RecordMixingGiftData(struct lu_BitstreamState* state, const struct RecordMixingGiftData* src) {
   lu_BitstreamWrite_u8(state, src.unk0, 8);
   lu_BitstreamWrite_u8(state, src.quantity, 8);
   lu_BitstreamWrite_u16(state, src.itemId, 16);
   {
      u16 i;
      for (i = 0; i < 8; ++i) { 
            lu_BitstreamWrite_u8(state, src.filler4[i], 8);
      }
   }
}