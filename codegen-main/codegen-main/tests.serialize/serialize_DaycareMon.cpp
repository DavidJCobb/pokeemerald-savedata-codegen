#include "./tests.serialize//serialize_DaycareMon.h"

// dependencies
#include "./serialize_BoxPokemon.h"
#include "./serialize_DaycareMail.h"

#include "global.h"

// TODO:
// void lu_BitstreamRead_DaycareMon(struct lu_BitstreamState* state, struct DaycareMon* dst);

void lu_BitstreamWrite_DaycareMon(struct lu_BitstreamState* state, const struct DaycareMon* src) {
   lu_BitstreamWrite_BoxPokemon(state, &src.mon);
   lu_BitstreamWrite_DaycareMail(state, &src.mail);
   lu_BitstreamWrite_u32(state, src.steps, 32);
}