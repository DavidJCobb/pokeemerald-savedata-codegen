#include "lu/generated/struct-serialize//serialize_RecordMixingGiftData.h"

#include "global.h" // struct definition

void lu_BitstreamRead_RecordMixingGiftData(struct lu_BitstreamState* state, const struct RecordMixingGiftData* src) {
   src.unk0 = lu_BitstreamRead_u8(state, 8);
   src.quantity = lu_BitstreamRead_u8(state, 8);
   src.itemId = lu_BitstreamRead_u16(state, 9);
   {
      u16 i;
      for (i = 0; i < 8; ++i) { 
            src.filler4[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
}
void lu_BitstreamWrite_RecordMixingGiftData(struct lu_BitstreamState* state, const struct RecordMixingGiftData* src) {
   lu_BitstreamWrite_u8(state, src.unk0, 8);
   lu_BitstreamWrite_u8(state, src.quantity, 8);
   lu_BitstreamWrite_u16(state, src.itemId, 9);
   {
      u16 i;
      for (i = 0; i < 8; ++i) { 
            lu_BitstreamWrite_u8(state, src.filler4[i], 8);
      }
   }
}