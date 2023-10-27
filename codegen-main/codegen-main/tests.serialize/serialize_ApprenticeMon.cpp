#include "./tests.serialize//serialize_ApprenticeMon.h"

#include "global.h" // struct definition

// check constants:
#if MAX_MON_MOVES != 4
   #error Constant `MAX_MON_MOVES` changed in C, but XML not updated or codegen not re-run!
#endif

// TODO:
// void lu_BitstreamRead_ApprenticeMon(struct lu_BitstreamState* state, struct ApprenticeMon* dst);

void lu_BitstreamWrite_ApprenticeMon(struct lu_BitstreamState* state, const struct ApprenticeMon* src) {
   lu_BitstreamWrite_u16(state, src.species, 11);
   {
      u16 i;
      for (i = 0; i < MAX_MON_MOVES; ++i) { 
            lu_BitstreamWrite_u16(state, src.moves[i], 16);
      }
   }
   lu_BitstreamWrite_u16(state, src.item, 16);
}