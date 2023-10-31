#include "struct-serialize//serialize_GabbyAndTyData.h"

#include "global.tv.h" // struct definition

void lu_BitstreamRead_GabbyAndTyData(struct lu_BitstreamState* state, const struct GabbyAndTyData* src) {
   src.mon1 = lu_BitstreamRead_u16(state, 11);
   src.mon2 = lu_BitstreamRead_u16(state, 11);
   src.lastMove = lu_BitstreamRead_u16(state, 16);
   {
      u16 i;
      for (i = 0; i < 1; ++i) { 
            src.quote[i] = lu_BitstreamRead_u16(state, 16);
      }
   }
   src.mapnum = lu_BitstreamRead_u8(state, 8);
   src.battleNum = lu_BitstreamRead_u8(state, 8);
   src.battleTookMoreThanOneTurn = lu_BitstreamRead_bool(state, src.battleTookMoreThanOneTurn);
   src.playerLostAMon = lu_BitstreamRead_bool(state, src.playerLostAMon);
   src.playerUsedHealingItem = lu_BitstreamRead_bool(state, src.playerUsedHealingItem);
   src.playerThrewABall = lu_BitstreamRead_bool(state, src.playerThrewABall);
   src.onAir = lu_BitstreamRead_bool(state, src.onAir);
   src.valA_5 = lu_BitstreamRead_u8(state, 3);
   src.battleTookMoreThanOneTurn2 = lu_BitstreamRead_bool(state, src.battleTookMoreThanOneTurn2);
   src.playerLostAMon2 = lu_BitstreamRead_bool(state, src.playerLostAMon2);
   src.playerUsedHealingItem2 = lu_BitstreamRead_bool(state, src.playerUsedHealingItem2);
   src.playerThrewABall2 = lu_BitstreamRead_bool(state, src.playerThrewABall2);
   src.valB_5 = lu_BitstreamRead_u8(state, 4);
}
void lu_BitstreamWrite_GabbyAndTyData(struct lu_BitstreamState* state, const struct GabbyAndTyData* src) {
   lu_BitstreamWrite_u16(state, src.mon1, 11);
   lu_BitstreamWrite_u16(state, src.mon2, 11);
   lu_BitstreamWrite_u16(state, src.lastMove, 16);
   {
      u16 i;
      for (i = 0; i < 1; ++i) { 
            lu_BitstreamWrite_u16(state, src.quote[i], 16);
      }
   }
   lu_BitstreamWrite_u8(state, src.mapnum, 8);
   lu_BitstreamWrite_u8(state, src.battleNum, 8);
   lu_BitstreamWrite_bool(state, src.battleTookMoreThanOneTurn);
   lu_BitstreamWrite_bool(state, src.playerLostAMon);
   lu_BitstreamWrite_bool(state, src.playerUsedHealingItem);
   lu_BitstreamWrite_bool(state, src.playerThrewABall);
   lu_BitstreamWrite_bool(state, src.onAir);
   lu_BitstreamWrite_u8(state, src.valA_5, 3);
   lu_BitstreamWrite_bool(state, src.battleTookMoreThanOneTurn2);
   lu_BitstreamWrite_bool(state, src.playerLostAMon2);
   lu_BitstreamWrite_bool(state, src.playerUsedHealingItem2);
   lu_BitstreamWrite_bool(state, src.playerThrewABall2);
   lu_BitstreamWrite_u8(state, src.valB_5, 4);
}