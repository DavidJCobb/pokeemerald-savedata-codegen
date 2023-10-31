#include "struct-serialize//serialize_RamScript.h"

#include "global.h" // struct definition

// dependencies
#include "./serialize_RamScriptData.h"

// TODO:
// void lu_BitstreamRead_RamScript(struct lu_BitstreamState* state, struct RamScript* dst);

void lu_BitstreamWrite_RamScript(struct lu_BitstreamState* state, const struct RamScript* src) {
   lu_BitstreamWrite_u32(state, src.checksum, 32);
   lu_BitstreamWrite_RamScriptData(state, &src.data);
}