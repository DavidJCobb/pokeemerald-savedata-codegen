#include "struct-serialize//serialize_ApprenticeQuestion.h"

#include "global.h" // struct definition

// TODO:
// void lu_BitstreamRead_ApprenticeQuestion(struct lu_BitstreamState* state, struct ApprenticeQuestion* dst);

void lu_BitstreamWrite_ApprenticeQuestion(struct lu_BitstreamState* state, const struct ApprenticeQuestion* src) {
   lu_BitstreamWrite_u8(state, src.questionId, 2);
   lu_BitstreamWrite_u8(state, src.monId, 2);
   lu_BitstreamWrite_u8(state, src.moveSlot, 2);
   lu_BitstreamWrite_u8(state, src.suggestedChange, 2);
   lu_BitstreamWrite_u16(state, src.data, 16);
}