#include "./tests.serialize//serialize_ApprenticeQuestion.h"

#include "global.h"

// TODO:
// void lu_BitstreamRead_ApprenticeQuestion(struct lu_BitstreamState* state, struct ApprenticeQuestion* dst);

void lu_BitstreamWrite_ApprenticeQuestion(struct lu_BitstreamState* state, struct ApprenticeQuestion* src) {
   lu_BitstreamWrite_u8(state, src.questionId, 8);
   lu_BitstreamWrite_u8(state, src.monId, 8);
   lu_BitstreamWrite_u8(state, src.moveSlot, 8);
   lu_BitstreamWrite_u8(state, src.suggestedChange, 8);
   lu_BitstreamWrite_u16(state, src.data, 16);
}