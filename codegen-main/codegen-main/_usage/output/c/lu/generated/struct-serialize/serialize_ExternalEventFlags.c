#include "lu/generated/struct-serialize//serialize_ExternalEventFlags.h"

#include "global.h" // struct definition

void lu_BitstreamRead_ExternalEventFlags(struct lu_BitstreamState* state, const struct ExternalEventFlags* src) {
   src.usedBoxRS = lu_BitstreamRead_bool(state, src.usedBoxRS);
   src.boxRSEggsUnlocked = lu_BitstreamRead_u8(state, 2);
   src.unknownFlag1 = lu_BitstreamRead_u8(state, 8);
   src.receivedGCNJirachi = lu_BitstreamRead_u8(state, 8);
   src.unknownFlag3 = lu_BitstreamRead_u8(state, 8);
   src.unknownFlag4 = lu_BitstreamRead_u8(state, 8);
   src.unknownFlag5 = lu_BitstreamRead_u8(state, 8);
   src.unknownFlag6 = lu_BitstreamRead_u8(state, 8);
   src.unknownFlag7 = lu_BitstreamRead_u8(state, 8);
   src.unknownFlag8 = lu_BitstreamRead_u8(state, 8);
   src.unknownFlag9 = lu_BitstreamRead_u8(state, 8);
   src.unknownFlag10 = lu_BitstreamRead_u8(state, 8);
   src.unknownFlag11 = lu_BitstreamRead_u8(state, 8);
   src.unknownFlag12 = lu_BitstreamRead_u8(state, 8);
   src.unknownFlag13 = lu_BitstreamRead_u8(state, 8);
   src.unknownFlag14 = lu_BitstreamRead_u8(state, 8);
   src.unknownFlag15 = lu_BitstreamRead_u8(state, 8);
   src.unknownFlag16 = lu_BitstreamRead_u8(state, 8);
   src.unknownFlag17 = lu_BitstreamRead_u8(state, 8);
   src.unknownFlag18 = lu_BitstreamRead_u8(state, 8);
   src.unknownFlag19 = lu_BitstreamRead_u8(state, 8);
   src.unknownFlag20 = lu_BitstreamRead_u8(state, 8);
}
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