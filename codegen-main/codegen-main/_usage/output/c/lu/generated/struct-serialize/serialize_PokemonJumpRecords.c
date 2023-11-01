#include "lu/generated/struct-serialize//serialize_PokemonJumpRecords.h"

#include "global.h" // struct definition

void lu_BitstreamRead_PokemonJumpRecords(struct lu_BitstreamState* state, const struct PokemonJumpRecords* src) {
   src.jumpsInRow = lu_BitstreamRead_u16(state, 16);
   src.unused1 = lu_BitstreamRead_u16(state, 16);
   src.excellentsInRow = lu_BitstreamRead_u16(state, 16);
   src.gamesWithMaxPlayers = lu_BitstreamRead_u16(state, 16);
   src.unused2 = lu_BitstreamRead_u16(state, 16);
   src.bestJumpScore = lu_BitstreamRead_u32(state, 32);
}
void lu_BitstreamWrite_PokemonJumpRecords(struct lu_BitstreamState* state, const struct PokemonJumpRecords* src) {
   lu_BitstreamWrite_u16(state, src.jumpsInRow, 16);
   lu_BitstreamWrite_u16(state, src.unused1, 16);
   lu_BitstreamWrite_u16(state, src.excellentsInRow, 16);
   lu_BitstreamWrite_u16(state, src.gamesWithMaxPlayers, 16);
   lu_BitstreamWrite_u16(state, src.unused2, 16);
   lu_BitstreamWrite_u32(state, src.bestJumpScore, 32);
}