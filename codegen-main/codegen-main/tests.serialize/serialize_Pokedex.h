#ifndef GUARD_LU_SERIALIZE_Pokedex
#define GUARD_LU_SERIALIZE_Pokedex

void lu_BitstreamRead_Pokedex(struct lu_BitstreamState*, struct Pokedex* dst);void lu_BitstreamWrite_Pokedex(struct lu_BitstreamState*, struct Pokedex* src);
#endif