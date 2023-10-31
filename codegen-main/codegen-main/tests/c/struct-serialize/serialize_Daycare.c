#include "struct-serialize//serialize_Daycare.h"

#include "global.h" // struct definition

// dependencies
#include "./serialize_DaycareMon.h"

// check constants:
#if DAYCARE_MON_COUNT != 2
   #error Constant `DAYCARE_MON_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif

// TODO:
// void lu_BitstreamRead_Daycare(struct lu_BitstreamState* state, struct Daycare* dst);

void lu_BitstreamWrite_Daycare(struct lu_BitstreamState* state, const struct Daycare* src) {
   {
      u16 i;
      for (i = 0; i < DAYCARE_MON_COUNT; ++i) { 
            lu_BitstreamWrite_DaycareMon(state, &src.mons[i]);
      }
   }
   lu_BitstreamWrite_u32(state, src.offspringPersonality, 32);
   lu_BitstreamWrite_u8(state, src.stepCounter, 8);
}