#include "lu/generated/struct-serialize//serialize_ApprenticeQuestion.h"

#include "global.h" // struct definition

void lu_BitstreamRead_ApprenticeQuestion(struct lu_BitstreamState* state, const struct ApprenticeQuestion* src) {
   src.questionId = lu_BitstreamRead_u8(state, 2);
   src.monId = lu_BitstreamRead_u8(state, 2);
   src.moveSlot = lu_BitstreamRead_u8(state, 2);
   src.suggestedChange = lu_BitstreamRead_u8(state, 2);
   src.data = lu_BitstreamRead_u16(state, 16);
}
void lu_BitstreamWrite_ApprenticeQuestion(struct lu_BitstreamState* state, const struct ApprenticeQuestion* src) {
   lu_BitstreamWrite_u8(state, src.questionId, 2);
   lu_BitstreamWrite_u8(state, src.monId, 2);
   lu_BitstreamWrite_u8(state, src.moveSlot, 2);
   lu_BitstreamWrite_u8(state, src.suggestedChange, 2);
   lu_BitstreamWrite_u16(state, src.data, 16);
}