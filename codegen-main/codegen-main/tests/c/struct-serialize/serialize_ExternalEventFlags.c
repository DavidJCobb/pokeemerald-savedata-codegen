#include "struct-serialize//serialize_ExternalEventFlags.h"

#include "global.h" // struct definition

// TODO:
// void lu_BitstreamRead_ExternalEventFlags(struct lu_BitstreamState* state, struct ExternalEventFlags* dst);

void lu_BitstreamWrite_ExternalEventFlags(struct lu_BitstreamState* state, const struct ExternalEventFlags* src) {
   lu_BitstreamWrite_bool(state, src.usedBoxRS);
   lu_BitstreamWrite_u8(state, src.boxRSEggsUnlocked, 2);
   lu_BitstreamWrite_u8(state, src.unknownFlag1, 8);
   lu_BitstreamWrite_u8(state, src.receivedGCNJirachi, 8);
   lu_BitstreamWrite_u8(state, src.unknownFlag3, 8);
   lu_BitstreamWrite_u8(state, src.unknownFlag4, 8);
   lu_BitstreamWrite_u8(state, src.unknownFlag5, 8);
   lu_BitstreamWrite_u8(state, src.unknownFlag6, 8);
   lu_BitstreamWrite_u8(state, src.unknownFlag7, 8);
   lu_BitstreamWrite_u8(state, src.unknownFlag8, 8);
   lu_BitstreamWrite_u8(state, src.unknownFlag9, 8);
   lu_BitstreamWrite_u8(state, src.unknownFlag10, 8);
   lu_BitstreamWrite_u8(state, src.unknownFlag11, 8);
   lu_BitstreamWrite_u8(state, src.unknownFlag12, 8);
   lu_BitstreamWrite_u8(state, src.unknownFlag13, 8);
   lu_BitstreamWrite_u8(state, src.unknownFlag14, 8);
   lu_BitstreamWrite_u8(state, src.unknownFlag15, 8);
   lu_BitstreamWrite_u8(state, src.unknownFlag16, 8);
   lu_BitstreamWrite_u8(state, src.unknownFlag17, 8);
   lu_BitstreamWrite_u8(state, src.unknownFlag18, 8);
   lu_BitstreamWrite_u8(state, src.unknownFlag19, 8);
   lu_BitstreamWrite_u8(state, src.unknownFlag20, 8);
}