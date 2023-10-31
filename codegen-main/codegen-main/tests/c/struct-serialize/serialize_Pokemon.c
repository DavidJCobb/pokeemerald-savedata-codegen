#include "struct-serialize//serialize_Pokemon.h"

#include "pokemon.h" // struct definition

// dependencies
#include "./serialize_BoxPokemon.h"

void lu_BitstreamRead_Pokemon(struct lu_BitstreamState* state, const struct Pokemon* src) {
   lu_BitstreamRead_BoxPokemon(state, &src.box);
   src.status = lu_BitstreamRead_u32(state, 32);
   src.level = lu_BitstreamRead_u8(state, 7);
   src.mail = lu_BitstreamRead_u8(state, 8);
   src.hp = lu_BitstreamRead_u16(state, 16);
   src.maxHP = lu_BitstreamRead_u16(state, 16);
   src.attack = lu_BitstreamRead_u16(state, 16);
   src.defense = lu_BitstreamRead_u16(state, 16);
   src.speed = lu_BitstreamRead_u16(state, 16);
   src.spAttack = lu_BitstreamRead_u16(state, 16);
   src.spDefense = lu_BitstreamRead_u16(state, 16);
}
void lu_BitstreamWrite_Pokemon(struct lu_BitstreamState* state, const struct Pokemon* src) {
   lu_BitstreamWrite_BoxPokemon(state, &src.box);
   lu_BitstreamWrite_u32(state, src.status, 32);
   lu_BitstreamWrite_u8(state, src.level, 7);
   lu_BitstreamWrite_u8(state, src.mail, 8);
   lu_BitstreamWrite_u16(state, src.hp, 16);
   lu_BitstreamWrite_u16(state, src.maxHP, 16);
   lu_BitstreamWrite_u16(state, src.attack, 16);
   lu_BitstreamWrite_u16(state, src.defense, 16);
   lu_BitstreamWrite_u16(state, src.speed, 16);
   lu_BitstreamWrite_u16(state, src.spAttack, 16);
   lu_BitstreamWrite_u16(state, src.spDefense, 16);
}