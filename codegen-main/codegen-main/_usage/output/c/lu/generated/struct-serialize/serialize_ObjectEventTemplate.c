#include "lu/generated/struct-serialize//serialize_ObjectEventTemplate.h"

#include "global.fieldmap.h" // struct definition

void lu_BitstreamRead_ObjectEventTemplate(struct lu_BitstreamState* state, const struct ObjectEventTemplate* src) {
   src.localId = lu_BitstreamRead_u8(state, 8);
   src.graphicsId = lu_BitstreamRead_u8(state, 8);
   src.kind = lu_BitstreamRead_u8(state, 8);
   src.x = lu_BitstreamRead_u16(state, 16);
   src.y = lu_BitstreamRead_u16(state, 16);
   src.elevation = lu_BitstreamRead_u8(state, 8);
   src.movementType = lu_BitstreamRead_u8(state, 8);
   src.movementRangeX = lu_BitstreamRead_u8(state, 4);
   src.movementRangeY = lu_BitstreamRead_u8(state, 4);
   src.trainerType = lu_BitstreamRead_u16(state, 16);
   src.trainerRange_berryTreeId = lu_BitstreamRead_u16(state, 16);
   script = (u8*) lu_BitstreamREad_u32(state, 32);
   src.flagId = lu_BitstreamRead_u16(state, 16);
}
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