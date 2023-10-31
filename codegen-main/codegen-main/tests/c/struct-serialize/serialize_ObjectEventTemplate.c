#include "struct-serialize//serialize_ObjectEventTemplate.h"

#include "global.fieldmap.h" // struct definition

// TODO:
// void lu_BitstreamRead_ObjectEventTemplate(struct lu_BitstreamState* state, struct ObjectEventTemplate* dst);

void lu_BitstreamWrite_ObjectEventTemplate(struct lu_BitstreamState* state, const struct ObjectEventTemplate* src) {
   lu_BitstreamWrite_u8(state, src.localId, 8);
   lu_BitstreamWrite_u8(state, src.graphicsId, 8);
   lu_BitstreamWrite_u8(state, src.kind, 8);
   lu_BitstreamWrite_u16(state, src.x, 16);
   lu_BitstreamWrite_u16(state, src.y, 16);
   lu_BitstreamWrite_u8(state, src.elevation, 8);
   lu_BitstreamWrite_u8(state, src.movementType, 8);
   lu_BitstreamWrite_u8(state, src.movementRangeX, 4);
   lu_BitstreamWrite_u8(state, src.movementRangeY, 4);
   lu_BitstreamWrite_u16(state, src.trainerType, 16);
   lu_BitstreamWrite_u16(state, src.trainerRange_berryTreeId, 16);
   lu_BitstreamWrite_u32(state, src.script, 32);
   lu_BitstreamWrite_u16(state, src.flagId, 16);
}