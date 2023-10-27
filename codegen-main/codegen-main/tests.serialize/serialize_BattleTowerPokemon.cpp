#include "./tests.serialize//serialize_BattleTowerPokemon.h"

#include "string_util.h" // gflib; for StringLength

#include "global.h"

// TODO:
// void lu_BitstreamRead_BattleTowerPokemon(struct lu_BitstreamState* state, struct BattleTowerPokemon* dst);

void lu_BitstreamWrite_BattleTowerPokemon(struct lu_BitstreamState* state, struct BattleTowerPokemon* src) {
   lu_BitstreamWrite_u16(state, src.species, 11);
   lu_BitstreamWrite_u16(state, src.heldItem, 16);
   {
      u16 i;
      for (i = 0; i < 4; ++i) { 
            lu_BitstreamWrite_u16(state, src.moves[i], 16);
      }
   }
   lu_BitstreamWrite_u8(state, src.level, 7);
   lu_BitstreamWrite_u8(state, src.ppBonuses, 8);
   lu_BitstreamWrite_u8(state, src.hpEV, 8);
   lu_BitstreamWrite_u8(state, src.attackEV, 8);
   lu_BitstreamWrite_u8(state, src.defenseEV, 8);
   lu_BitstreamWrite_u8(state, src.speedEV, 8);
   lu_BitstreamWrite_u8(state, src.spAttackEV, 8);
   lu_BitstreamWrite_u8(state, src.spDefenseEV, 8);
   lu_BitstreamWrite_u32(state, src.otId, 32);
   lu_BitstreamWrite_u8(state, src.hpIV, 8);
   lu_BitstreamWrite_u8(state, src.attackIV, 8);
   lu_BitstreamWrite_u8(state, src.defenseIV, 8);
   lu_BitstreamWrite_u8(state, src.speedIV, 8);
   lu_BitstreamWrite_u8(state, src.spAttackIV, 8);
   lu_BitstreamWrite_u8(state, src.spDefenseIV, 8);
   lu_BitstreamWrite_u8(state, src.gap, 8);
   lu_BitstreamWrite_u32(state, src.abilityNum, 32);
   lu_BitstreamWrite_u32(state, src.personality, 32);
   lu_BitstreamWrite_string(state, src.nickname, 10, 4);
   lu_BitstreamWrite_u8(state, src.friendship, 8);
}