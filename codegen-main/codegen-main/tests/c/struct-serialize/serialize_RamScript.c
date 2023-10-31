#include "struct-serialize//serialize_RamScript.h"

#include "global.h" // struct definition

// dependencies
#include "./serialize_RamScriptData.h"

void lu_BitstreamRead_RamScript(struct lu_BitstreamState* state, const struct RamScript* src) {
   src.checksum = lu_BitstreamRead_u32(state, 32);
   lu_BitstreamRead_RamScriptData(state, &src.data);
}
void lu_BitstreamWrite_RamScript(struct lu_BitstreamState* state, const struct RamScript* src) {
   lu_BitstreamWrite_u32(state, src.checksum, 32);
   lu_BitstreamWrite_RamScriptData(state, &src.data);
}