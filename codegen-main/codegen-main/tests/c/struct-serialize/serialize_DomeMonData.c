#include "struct-serialize//serialize_DomeMonData.h"

#include "global.h" // struct definition

// check constants:
#if MAX_MON_MOVES != 4
   #error Constant `MAX_MON_MOVES` changed in C, but XML not updated or codegen not re-run!
#endif
#if NUM_STATS != 6
   #error Constant `NUM_STATS` changed in C, but XML not updated or codegen not re-run!
#endif

// TODO:
// void lu_BitstreamRead_DomeMonData(struct lu_BitstreamState* state, struct DomeMonData* dst);

void lu_BitstreamWrite_DomeMonData(struct lu_BitstreamState* state, const struct DomeMonData* src) {
   {
      u16 i;
      for (i = 0; i < MAX_MON_MOVES; ++i) { 
            lu_BitstreamWrite_u16(state, src.moves[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < NUM_STATS; ++i) { 
            lu_BitstreamWrite_u8(state, src.evs[i], 8);
      }
   }
   lu_BitstreamWrite_u8(state, src.nature, 8);
}