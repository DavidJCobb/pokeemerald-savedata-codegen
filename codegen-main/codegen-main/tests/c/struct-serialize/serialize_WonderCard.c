#include "struct-serialize//serialize_WonderCard.h"

#include "global.h" // struct definition

#include "string_util.h" // gflib; for StringLength

// check constants:
#if WONDER_CARD_TEXT_LENGTH != 40
   #error Constant `WONDER_CARD_TEXT_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if WONDER_CARD_BODY_TEXT_LINES != 4
   #error Constant `WONDER_CARD_BODY_TEXT_LINES` changed in C, but XML not updated or codegen not re-run!
#endif
#if WONDER_CARD_TEXT_LENGTH != 40
   #error Constant `WONDER_CARD_TEXT_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_WonderCard(struct lu_BitstreamState* state, const struct WonderCard* src) {
   src.flagId = lu_BitstreamRead_u16(state, 16);
   src.iconSpecies = lu_BitstreamRead_u16(state, 11);
   src.idNumber = lu_BitstreamRead_u32(state, 32);
   src.type = lu_BitstreamRead_u8(state, 2);
   src.bgType = lu_BitstreamRead_u8(state, 4);
   src.sendType = lu_BitstreamRead_u8(state, 2);
   src.maxStamps = lu_BitstreamRead_u8(state, 8);
   lu_BitstreamRead_string_optional_terminator(state, src.titleText, WONDER_CARD_TEXT_LENGTH);
   lu_BitstreamRead_string_optional_terminator(state, src.subtitleText, WONDER_CARD_TEXT_LENGTH);
   {
      u16 i;
      for (i = 0; i < WONDER_CARD_BODY_TEXT_LINES; ++i) { 
            lu_BitstreamRead_string_optional_terminator(state, src.bodyText[i], WONDER_CARD_TEXT_LENGTH);
      }
   }
   lu_BitstreamRead_string_optional_terminator(state, src.footerLine1Text, WONDER_CARD_TEXT_LENGTH);
   lu_BitstreamRead_string_optional_terminator(state, src.footerLine2Text, WONDER_CARD_TEXT_LENGTH);
}
void lu_BitstreamWrite_WonderCard(struct lu_BitstreamState* state, const struct WonderCard* src) {
   lu_BitstreamWrite_u16(state, src.flagId, 16);
   lu_BitstreamWrite_u16(state, src.iconSpecies, 11);
   lu_BitstreamWrite_u32(state, src.idNumber, 32);
   lu_BitstreamWrite_u8(state, src.type, 2);
   lu_BitstreamWrite_u8(state, src.bgType, 4);
   lu_BitstreamWrite_u8(state, src.sendType, 2);
   lu_BitstreamWrite_u8(state, src.maxStamps, 8);
   lu_BitstreamWrite_string_optional_terminator(state, src.titleText, WONDER_CARD_TEXT_LENGTH);
   lu_BitstreamWrite_string_optional_terminator(state, src.subtitleText, WONDER_CARD_TEXT_LENGTH);
   {
      u16 i;
      for (i = 0; i < WONDER_CARD_BODY_TEXT_LINES; ++i) { 
            lu_BitstreamWrite_string_optional_terminator(state, src.bodyText[i], WONDER_CARD_TEXT_LENGTH);
      }
   }
   lu_BitstreamWrite_string_optional_terminator(state, src.footerLine1Text, WONDER_CARD_TEXT_LENGTH);
   lu_BitstreamWrite_string_optional_terminator(state, src.footerLine2Text, WONDER_CARD_TEXT_LENGTH);
}