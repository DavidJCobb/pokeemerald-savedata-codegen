#include "lu/generated/struct-serialize//serialize_LinkBattleRecords.h"

#include "global.h" // struct definition

// dependencies
#include "./serialize_LinkBattleRecord.h"

// check constants:
#if LINK_B_RECORDS_COUNT != 5
   #error Constant `LINK_B_RECORDS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_LinkBattleRecords(struct lu_BitstreamState* state, const struct LinkBattleRecords* src) {
   {
      u16 i;
      for (i = 0; i < LINK_B_RECORDS_COUNT; ++i) { 
            lu_BitstreamRead_LinkBattleRecord(state, &src.entries[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < LINK_B_RECORDS_COUNT; ++i) { 
            src.languages[i] = lu_BitstreamRead_u8(state, 3);
      }
   }
}
void lu_BitstreamWrite_LinkBattleRecords(struct lu_BitstreamState* state, const struct LinkBattleRecords* src) {
   {
      u16 i;
      for (i = 0; i < LINK_B_RECORDS_COUNT; ++i) { 
            lu_BitstreamWrite_LinkBattleRecord(state, &src.entries[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < LINK_B_RECORDS_COUNT; ++i) { 
            lu_BitstreamWrite_u8(state, src.languages[i], 3);
      }
   }
}