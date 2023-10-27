#include "./tests.serialize//serialize_DaycareMail.h"

// dependencies
#include "./serialize_Mail.h"

#include "string_util.h" // gflib; for StringLength

#include "global.h"

// TODO:
// void lu_BitstreamRead_DaycareMail(struct lu_BitstreamState* state, struct DaycareMail* dst);

void lu_BitstreamWrite_DaycareMail(struct lu_BitstreamState* state, const struct DaycareMail* src) {
   lu_BitstreamWrite_Mail(state, &src.message);
   lu_BitstreamWrite_string(state, src.otName, 7, 3);
   lu_BitstreamWrite_string(state, src.monName, 10, 4);
   lu_BitstreamWrite_u8(state, src.gameLanguage, 3);
   lu_BitstreamWrite_u8(state, src.monLanguage, 3);
}