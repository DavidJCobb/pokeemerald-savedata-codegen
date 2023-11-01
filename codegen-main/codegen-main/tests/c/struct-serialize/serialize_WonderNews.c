#include "struct-serialize//serialize_WonderNews.h"

#include "global.h" // struct definition

#include "string_util.h" // gflib; for StringLength

// check constants:
#if WONDER_NEWS_TEXT_LENGTH != 40
   #error Constant `WONDER_NEWS_TEXT_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if WONDER_NEWS_BODY_TEXT_LINES != 10
   #error Constant `WONDER_NEWS_BODY_TEXT_LINES` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_WonderNews(struct lu_BitstreamState* state, const struct WonderNews* src) {
   src.id = lu_BitstreamRead_u16(state, 16);
   src.sendType = lu_BitstreamRead_u8(state, 8);
   src.bgType = lu_BitstreamRead_u8(state, 8);
   lu_BitstreamRead_string_optional_terminator(state, src.titleText, WONDER_NEWS_TEXT_LENGTH, 6);
   {
      u16 i;
      for (i = 0; i < WONDER_NEWS_BODY_TEXT_LINES; ++i) { 
            lu_BitstreamRead_string_optional_terminator(state, src.bodyText[i], WONDER_NEWS_TEXT_LENGTH, 6);
      }
   }
}
void lu_BitstreamWrite_WonderNews(struct lu_BitstreamState* state, const struct WonderNews* src) {
   lu_BitstreamWrite_u16(state, src.id, 16);
   lu_BitstreamWrite_u8(state, src.sendType, 8);
   lu_BitstreamWrite_u8(state, src.bgType, 8);
   lu_BitstreamWrite_string_optional_terminator(state, src.titleText, WONDER_NEWS_TEXT_LENGTH, 6);
   {
      u16 i;
      for (i = 0; i < WONDER_NEWS_BODY_TEXT_LINES; ++i) { 
            lu_BitstreamWrite_string_optional_terminator(state, src.bodyText[i], WONDER_NEWS_TEXT_LENGTH, 6);
      }
   }
}