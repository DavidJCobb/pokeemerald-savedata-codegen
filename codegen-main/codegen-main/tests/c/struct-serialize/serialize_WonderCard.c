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

// TODO:
// void lu_BitstreamRead_WonderCard(struct lu_BitstreamState* state, struct WonderCard* dst);

void lu_BitstreamWrite_WonderCard(struct lu_BitstreamState* state, const struct WonderCard* src) {
   lu_BitstreamWrite_u16(state, src.flagId, 16);
   lu_BitstreamWrite_u16(state, src.iconSpecies, 11);
   lu_BitstreamWrite_u32(state, src.idNumber, 32);
   lu_BitstreamWrite_u8(state, src.type, 2);
   lu_BitstreamWrite_u8(state, src.bgType, 4);
   lu_BitstreamWrite_u8(state, src.sendType, 2);
   lu_BitstreamWrite_u8(state, src.maxStamps, 8);
   lu_BitstreamWrite_string(state, src.titleText, WONDER_CARD_TEXT_LENGTH, 6);
   lu_BitstreamWrite_string(state, src.subtitleText, WONDER_CARD_TEXT_LENGTH, 6);
   {
      u16 i;
      for (i = 0; i < WONDER_CARD_BODY_TEXT_LINES; ++i) { 
            lu_BitstreamWrite_string(state, src.bodyText[i], WONDER_CARD_TEXT_LENGTH, 6);
      }
   }
   lu_BitstreamWrite_string(state, src.footerLine1Text, WONDER_CARD_TEXT_LENGTH, 6);
   lu_BitstreamWrite_string(state, src.footerLine2Text, WONDER_CARD_TEXT_LENGTH, 6);
}