#include "struct-serialize//serialize_WaldaPhrase.h"

#include "global.h" // struct definition

// TODO:
// void lu_BitstreamRead_WaldaPhrase(struct lu_BitstreamState* state, struct WaldaPhrase* dst);

void lu_BitstreamWrite_WaldaPhrase(struct lu_BitstreamState* state, const struct WaldaPhrase* src) {
   {
      u16 i;
      for (i = 0; i < 2; ++i) { 
            lu_BitstreamWrite_u16(state, src.colors[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < 16; ++i) { 
            lu_BitstreamWrite_u8(state, src.text[i], 8);
      }
   }
   lu_BitstreamWrite_u8(state, src.iconId, 8);
   lu_BitstreamWrite_u8(state, src.patternId, 8);
   lu_BitstreamWrite_bool(state, src.patternUnlocked);
}