#include "struct-serialize//serialize_ExternalEventData.h"

#include "global.h" // struct definition

// TODO:
// void lu_BitstreamRead_ExternalEventData(struct lu_BitstreamState* state, struct ExternalEventData* dst);

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