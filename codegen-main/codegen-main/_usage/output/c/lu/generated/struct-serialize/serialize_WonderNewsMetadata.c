#include "lu/generated/struct-serialize//serialize_WonderNewsMetadata.h"

#include "global.h" // struct definition

void lu_BitstreamRead_WonderNewsMetadata(struct lu_BitstreamState* state, const struct WonderNewsMetadata* src) {
   src.newsType = lu_BitstreamRead_u8(state, 2);
   src.sentRewardCounter = lu_BitstreamRead_u8(state, 3);
   src.rewardCounter = lu_BitstreamRead_u8(state, 3);
   src.berry = lu_BitstreamRead_u8(state, 8);
}
void lu_BitstreamWrite_WonderNewsMetadata(struct lu_BitstreamState* state, const struct WonderNewsMetadata* src) {
   lu_BitstreamWrite_u8(state, src.newsType, 2);
   lu_BitstreamWrite_u8(state, src.sentRewardCounter, 3);
   lu_BitstreamWrite_u8(state, src.rewardCounter, 3);
   lu_BitstreamWrite_u8(state, src.berry, 8);
}