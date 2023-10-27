#include "./tests.serialize//serialize_PokemonJumpRecords.h"

#include "global.h"

// TODO:
// void lu_BitstreamRead_PokemonJumpRecords(struct lu_BitstreamState* state, struct PokemonJumpRecords* dst);

void lu_BitstreamWrite_PokemonJumpRecords(struct lu_BitstreamState* state, struct PokemonJumpRecords* src) {
   lu_BitstreamWrite_u16(state, src.jumpsInRow, 16);
   lu_BitstreamWrite_u16(state, src.unused1, 16);
   lu_BitstreamWrite_u16(state, src.excellentsInRow, 16);
   lu_BitstreamWrite_u16(state, src.gamesWithMaxPlayers, 16);
   lu_BitstreamWrite_u16(state, src.unused2, 16);
   lu_BitstreamWrite_u32(state, src.bestJumpScore, 32);
}