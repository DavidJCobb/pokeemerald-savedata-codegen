#include "struct-serialize//serialize_ExternalEventData.h"

#include "global.h" // struct definition

void lu_BitstreamRead_ExternalEventData(struct lu_BitstreamState* state, const struct ExternalEventData* src) {
   {
      u16 i;
      for (i = 0; i < 7; ++i) { 
            src.unknownExternalDataFields1[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   src.unknownExternalDataFields2 = lu_BitstreamRead_u8(state, 8);
   src.currentPokeCoupons = lu_BitstreamRead_u32(state, 24);
   src.gotGoldPokeCouponTitleReward = lu_BitstreamRead_bool(state, src.gotGoldPokeCouponTitleReward);
   src.gotSilverPokeCouponTitleReward = lu_BitstreamRead_bool(state, src.gotSilverPokeCouponTitleReward);
   src.gotBronzePokeCouponTitleReward = lu_BitstreamRead_bool(state, src.gotBronzePokeCouponTitleReward);
   src.receivedAgetoCelebi = lu_BitstreamRead_bool(state, src.receivedAgetoCelebi);
   src.unknownExternalDataFields3 = lu_BitstreamRead_u8(state, 4);
   src.totalEarnedPokeCoupons = lu_BitstreamRead_u32(state, 24);
   {
      u16 i;
      for (i = 0; i < 5; ++i) { 
            src.unknownExternalDataFields4[i] = lu_BitstreamRead_u32(state, 32);
      }
   }
}
void lu_BitstreamWrite_ExternalEventData(struct lu_BitstreamState* state, const struct ExternalEventData* src) {
   {
      u16 i;
      for (i = 0; i < 7; ++i) { 
            lu_BitstreamWrite_u8(state, src.unknownExternalDataFields1[i], 8);
      }
   }
   lu_BitstreamWrite_u8(state, src.unknownExternalDataFields2, 8);
   lu_BitstreamWrite_u32(state, src.currentPokeCoupons, 24);
   lu_BitstreamWrite_bool(state, src.gotGoldPokeCouponTitleReward);
   lu_BitstreamWrite_bool(state, src.gotSilverPokeCouponTitleReward);
   lu_BitstreamWrite_bool(state, src.gotBronzePokeCouponTitleReward);
   lu_BitstreamWrite_bool(state, src.receivedAgetoCelebi);
   lu_BitstreamWrite_u8(state, src.unknownExternalDataFields3, 4);
   lu_BitstreamWrite_u32(state, src.totalEarnedPokeCoupons, 24);
   {
      u16 i;
      for (i = 0; i < 5; ++i) { 
            lu_BitstreamWrite_u32(state, src.unknownExternalDataFields4[i], 32);
      }
   }
}