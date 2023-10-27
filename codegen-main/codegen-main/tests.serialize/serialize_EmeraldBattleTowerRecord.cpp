#include "./tests.serialize//serialize_EmeraldBattleTowerRecord.h"

// dependencies
#include "./serialize_BattleTowerPokemon.h"

#include "string_util.h" // gflib; for StringLength

#include "global.h"

// TODO:
// void lu_BitstreamRead_EmeraldBattleTowerRecord(struct lu_BitstreamState* state, struct EmeraldBattleTowerRecord* dst);

void lu_BitstreamWrite_EmeraldBattleTowerRecord(struct lu_BitstreamState* state, const struct EmeraldBattleTowerRecord* src) {
   lu_BitstreamWrite_u8(state, src.lvlMode, 8);
   lu_BitstreamWrite_u8(state, src.facilityClass, 8);
   lu_BitstreamWrite_u16(state, src.winStreak, 16);
   lu_BitstreamWrite_string(state, src.name, 7, 3);
   {
      u16 i;
      for (i = 0; i < 4; ++i) { 
            lu_BitstreamWrite_u8(state, src.trainerId[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 6; ++i) { 
            lu_BitstreamWrite_u16(state, src.greeting[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < 6; ++i) { 
            lu_BitstreamWrite_u16(state, src.speechWon[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < 6; ++i) { 
            lu_BitstreamWrite_u16(state, src.speechLost[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < 3; ++i) { 
            lu_BitstreamWrite_BattleTowerPokemon(state, &src.party[i]);
      }
   }
   lu_BitstreamWrite_u8(state, src.language, 3);
   lu_BitstreamWrite_u32(state, src.checksum, 32);
}