#include "struct-serialize//serialize_DaycareMail.h"

#include "global.h" // struct definition

// dependencies
#include "./serialize_Mail.h"

#include "string_util.h" // gflib; for StringLength

// check constants:
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if POKEMON_NAME_LENGTH != 10
   #error Constant `POKEMON_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_DaycareMail(struct lu_BitstreamState* state, const struct DaycareMail* src) {
   lu_BitstreamRead_Mail(state, &src.message);
   lu_BitstreamRead_string(state, src.otName, PLAYER_NAME_LENGTH, 3);
   lu_BitstreamRead_string(state, src.monName, POKEMON_NAME_LENGTH, 4);
   src.gameLanguage = lu_BitstreamRead_u8(state, 3);
   src.monLanguage = lu_BitstreamRead_u8(state, 3);
}
void lu_BitstreamWrite_DaycareMail(struct lu_BitstreamState* state, const struct DaycareMail* src) {
   lu_BitstreamWrite_Mail(state, &src.message);
   lu_BitstreamWrite_string(state, src.otName, PLAYER_NAME_LENGTH, 3);
   lu_BitstreamWrite_string(state, src.monName, POKEMON_NAME_LENGTH, 4);
   lu_BitstreamWrite_u8(state, src.gameLanguage, 3);
   lu_BitstreamWrite_u8(state, src.monLanguage, 3);
}