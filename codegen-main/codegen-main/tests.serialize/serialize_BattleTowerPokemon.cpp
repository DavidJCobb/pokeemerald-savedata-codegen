#include "./tests.serialize//serialize_BattleTowerPokemon.h"

#include "global.h" // struct definition

#include "string_util.h" // gflib; for StringLength

// check constants:
#if MAX_MON_MOVES != 4
   #error Constant `MAX_MON_MOVES` changed in C, but XML not updated or codegen not re-run!
#endif
#if POKEMON_NAME_LENGTH != 10
   #error Constant `POKEMON_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif

// TODO:
// void lu_BitstreamRead_BattleTowerPokemon(struct lu_BitstreamState* state, struct BattleTowerPokemon* dst);

void lu_BitstreamWrite_BattleTowerPokemon(struct lu_BitstreamState* state, const struct BattleTowerPokemon* src) {
   lu_BitstreamWrite_u16(state, src.species, 11);
   lu_BitstreamWrite_u16(state, src.heldItem, 16);
   {
      u16 i;
      for (i = 0; i < MAX_MON_MOVES; ++i) { 
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
   lu_BitstreamWrite_u8(state, src.hpIV, 5);
   lu_BitstreamWrite_u8(state, src.attackIV, 5);
   lu_BitstreamWrite_u8(state, src.defenseIV, 5);
   lu_BitstreamWrite_u8(state, src.speedIV, 5);
   lu_BitstreamWrite_u8(state, src.spAttackIV, 5);
   lu_BitstreamWrite_u8(state, src.spDefenseIV, 5);
   lu_BitstreamWrite_bool(state, src.gap);
   lu_BitstreamWrite_bool(state, src.abilityNum);
   lu_BitstreamWrite_u32(state, src.personality, 32);
   lu_BitstreamWrite_string(state, src.nickname, POKEMON_NAME_LENGTH, 4);
   lu_BitstreamWrite_u8(state, src.friendship, 8);
}