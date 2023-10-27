#include "./tests.serialize//serialize_Time.h"

#include "global.h" // struct definition

// TODO:
// void lu_BitstreamRead_Time(struct lu_BitstreamState* state, struct Time* dst);

void lu_BitstreamWrite_Time(struct lu_BitstreamState* state, const struct Time* src) {
   lu_BitstreamWrite_u16(state, src.days, 16);
   lu_BitstreamWrite_u8(state, src.hours, 6);
   lu_BitstreamWrite_u8(state, src.minutes, 7);
   lu_BitstreamWrite_u8(state, src.seconds, 7);
}