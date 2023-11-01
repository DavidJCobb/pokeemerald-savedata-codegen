#include "lu/generated/struct-serialize//serialize_MysteryGiftSave.h"

#include "global.h" // struct definition

// dependencies
#include "./serialize_WonderNews.h"
#include "./serialize_WonderCard.h"
#include "./serialize_WonderCardMetadata.h"
#include "./serialize_WonderNewsMetadata.h"

// check constants:
#if NUM_QUESTIONNAIRE_WORDS != 4
   #error Constant `NUM_QUESTIONNAIRE_WORDS` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_MysteryGiftSave(struct lu_BitstreamState* state, const struct MysteryGiftSave* src) {
   src.newsCrc = lu_BitstreamRead_u32(state, 32);
   lu_BitstreamRead_WonderNews(state, &src.news);
   src.cardCrc = lu_BitstreamRead_u32(state, 32);
   lu_BitstreamRead_WonderCard(state, &src.card);
   src.cardMetadataCrc = lu_BitstreamRead_u32(state, 32);
   lu_BitstreamRead_WonderCardMetadata(state, &src.cardMetadata);
   {
      u16 i;
      for (i = 0; i < NUM_QUESTIONNAIRE_WORDS; ++i) { 
            src.questionnaireWords[i] = lu_BitstreamRead_u16(state, 16);
      }
   }
   lu_BitstreamRead_WonderNewsMetadata(state, &src.newsMetadata);
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < 5; ++j) { 
               src.trainerIds[i][j] = lu_BitstreamRead_u32(state, 32);
         }
      }
   }
}
void lu_BitstreamWrite_MysteryGiftSave(struct lu_BitstreamState* state, const struct MysteryGiftSave* src) {
   lu_BitstreamWrite_u32(state, src.newsCrc, 32);
   lu_BitstreamWrite_WonderNews(state, &src.news);
   lu_BitstreamWrite_u32(state, src.cardCrc, 32);
   lu_BitstreamWrite_WonderCard(state, &src.card);
   lu_BitstreamWrite_u32(state, src.cardMetadataCrc, 32);
   lu_BitstreamWrite_WonderCardMetadata(state, &src.cardMetadata);
   {
      u16 i;
      for (i = 0; i < NUM_QUESTIONNAIRE_WORDS; ++i) { 
            lu_BitstreamWrite_u16(state, src.questionnaireWords[i], 16);
      }
   }
   lu_BitstreamWrite_WonderNewsMetadata(state, &src.newsMetadata);
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < 5; ++j) { 
               lu_BitstreamWrite_u32(state, src.trainerIds[i][j], 32);
         }
      }
   }
}