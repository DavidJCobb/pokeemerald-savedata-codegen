#include "lu/generated/struct-serialize//serialize_DaycareMon.h"

#include "global.h" // struct definition

// dependencies
#include "./serialize_BoxPokemon.h"
#include "./serialize_DaycareMail.h"

void lu_BitstreamRead_DaycareMon(struct lu_BitstreamState* state, const struct DaycareMon* src) {
   lu_BitstreamRead_BoxPokemon(state, &src.mon);
   lu_BitstreamRead_DaycareMail(state, &src.mail);
   src.steps = lu_BitstreamRead_u32(state, 32);
}
void lu_BitstreamWrite_DaycareMon(struct lu_BitstreamState* state, const struct DaycareMon* src) {
   lu_BitstreamWrite_BoxPokemon(state, &src.mon);
   lu_BitstreamWrite_DaycareMail(state, &src.mail);
   lu_BitstreamWrite_u32(state, src.steps, 32);
}