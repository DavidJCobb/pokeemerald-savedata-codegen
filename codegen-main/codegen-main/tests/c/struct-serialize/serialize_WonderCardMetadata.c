#include "struct-serialize//serialize_WonderCardMetadata.h"

#include "global.h" // struct definition

// check constants:
#if MAX_STAMP_CARD_STAMPS != 7
   #error Constant `MAX_STAMP_CARD_STAMPS` changed in C, but XML not updated or codegen not re-run!
#endif

// TODO:
// void lu_BitstreamRead_WonderCardMetadata(struct lu_BitstreamState* state, struct WonderCardMetadata* dst);

void lu_BitstreamWrite_WonderCardMetadata(struct lu_BitstreamState* state, const struct WonderCardMetadata* src) {
   lu_BitstreamWrite_u16(state, src.battlesWon, 16);
   lu_BitstreamWrite_u16(state, src.battlesLost, 16);
   lu_BitstreamWrite_u16(state, src.numTrades, 16);
   lu_BitstreamWrite_u16(state, src.iconSpecies, 11);
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < MAX_STAMP_CARD_STAMPS; ++j) { 
               lu_BitstreamWrite_u16(state, src.stampData[i][j], 16);
         }
      }
   }
}