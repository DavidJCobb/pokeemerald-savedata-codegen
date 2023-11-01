#include "lu/generated/struct-serialize//serialize_Time.h"

#include "global.h" // struct definition

void lu_BitstreamRead_Time(struct lu_BitstreamState* state, const struct Time* src) {
   src.days = lu_BitstreamRead_u16(state, 16);
   src.hours = lu_BitstreamRead_u8(state, 6);
   src.minutes = lu_BitstreamRead_u8(state, 7);
   src.seconds = lu_BitstreamRead_u8(state, 7);
}
void lu_BitstreamWrite_Time(struct lu_BitstreamState* state, const struct Time* src) {
   lu_BitstreamWrite_u16(state, src.days, 16);
   lu_BitstreamWrite_u8(state, src.hours, 6);
   lu_BitstreamWrite_u8(state, src.minutes, 7);
   lu_BitstreamWrite_u8(state, src.seconds, 7);
}