#include "struct-serialize//serialize_RamScriptData.h"

#include "global.h" // struct definition

// TODO:
// void lu_BitstreamRead_RamScriptData(struct lu_BitstreamState* state, struct RamScriptData* dst);

void lu_BitstreamWrite_RamScriptData(struct lu_BitstreamState* state, const struct RamScriptData* src) {
   lu_BitstreamWrite_u8(state, src.magic, 8);
   lu_BitstreamWrite_u8(state, src.mapGroup, 8);
   lu_BitstreamWrite_u8(state, src.mapNum, 8);
   lu_BitstreamWrite_u8(state, src.objectId, 8);
   {
      u16 i;
      for (i = 0; i < 995; ++i) { 
            lu_BitstreamWrite_u8(state, src.script[i], 8);
      }
   }
}