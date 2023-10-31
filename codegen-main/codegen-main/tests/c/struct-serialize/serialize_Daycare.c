#include "struct-serialize//serialize_DayCare.h"

#include "global.h" // struct definition

// dependencies
#include "./serialize_DaycareMon.h"

// check constants:
#if DAYCARE_MON_COUNT != 2
   #error Constant `DAYCARE_MON_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_DayCare(struct lu_BitstreamState* state, const struct DayCare* src) {
   {
      u16 i;
      for (i = 0; i < DAYCARE_MON_COUNT; ++i) { 
            lu_BitstreamRead_DaycareMon(state, &src.mons[i]);
      }
   }
   src.offspringPersonality = lu_BitstreamRead_u32(state, 32);
   src.stepCounter = lu_BitstreamRead_u8(state, 8);
}
void lu_BitstreamWrite_DayCare(struct lu_BitstreamState* state, const struct DayCare* src) {
   {
      u16 i;
      for (i = 0; i < DAYCARE_MON_COUNT; ++i) { 
            lu_BitstreamWrite_DaycareMon(state, &src.mons[i]);
      }
   }
   lu_BitstreamWrite_u32(state, src.offspringPersonality, 32);
   lu_BitstreamWrite_u8(state, src.stepCounter, 8);
}