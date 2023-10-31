#include "struct-serialize//serialize_WonderNewsMetadata.h"

#include "global.h" // struct definition

// TODO:
// void lu_BitstreamRead_WonderNewsMetadata(struct lu_BitstreamState* state, struct WonderNewsMetadata* dst);

void lu_BitstreamWrite_WonderNewsMetadata(struct lu_BitstreamState* state, const struct WonderNewsMetadata* src) {
   lu_BitstreamWrite_u8(state, src.newsType, 2);
   lu_BitstreamWrite_u8(state, src.sentRewardCounter, 3);
   lu_BitstreamWrite_u8(state, src.rewardCounter, 3);
   lu_BitstreamWrite_u8(state, src.berry, 8);
}