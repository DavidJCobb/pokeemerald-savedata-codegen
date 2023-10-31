#include "struct-serialize//serialize_BattleTowerPokemon.h"

#include "global.h" // struct definition

#include "string_util.h" // gflib; for StringLength

// check constants:
#if MAX_MON_MOVES != 4
   #error Constant `MAX_MON_MOVES` changed in C, but XML not updated or codegen not re-run!
#endif
#if POKEMON_NAME_LENGTH != 10
   #error Constant `POKEMON_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_BattleTowerPokemon(struct lu_BitstreamState* state, const struct BattleTowerPokemon* src) {
   src.species = lu_BitstreamRead_u16(state, 11);
   src.heldItem = lu_BitstreamRead_u16(state, 9);
   {
      u16 i;
      for (i = 0; i < MAX_MON_MOVES; ++i) { 
            src.moves[i] = lu_BitstreamRead_u16(state, 16);
      }
   }
   src.level = lu_BitstreamRead_u8(state, 7);
   src.ppBonuses = lu_BitstreamRead_u8(state, 8);
   src.hpEV = lu_BitstreamRead_u8(state, 8);
   src.attackEV = lu_BitstreamRead_u8(state, 8);
   src.defenseEV = lu_BitstreamRead_u8(state, 8);
   src.speedEV = lu_BitstreamRead_u8(state, 8);
   src.spAttackEV = lu_BitstreamRead_u8(state, 8);
   src.spDefenseEV = lu_BitstreamRead_u8(state, 8);
   src.otId = lu_BitstreamRead_u32(state, 32);
   src.hpIV = lu_BitstreamRead_u8(state, 5);
   src.attackIV = lu_BitstreamRead_u8(state, 5);
   src.defenseIV = lu_BitstreamRead_u8(state, 5);
   src.speedIV = lu_BitstreamRead_u8(state, 5);
   src.spAttackIV = lu_BitstreamRead_u8(state, 5);
   src.spDefenseIV = lu_BitstreamRead_u8(state, 5);
   src.gap = lu_BitstreamRead_bool(state, src.gap);
   src.abilityNum = lu_BitstreamRead_bool(state, src.abilityNum);
   src.personality = lu_BitstreamRead_u32(state, 32);
   lu_BitstreamRead_string(state, src.nickname, POKEMON_NAME_LENGTH, 4);
   src.friendship = lu_BitstreamRead_u8(state, 8);
}
void lu_BitstreamWrite_BattleTowerPokemon(struct lu_BitstreamState* state, const struct BattleTowerPokemon* src) {
   lu_BitstreamWrite_u16(state, src.species, 11);
   lu_BitstreamWrite_u16(state, src.heldItem, 9);
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