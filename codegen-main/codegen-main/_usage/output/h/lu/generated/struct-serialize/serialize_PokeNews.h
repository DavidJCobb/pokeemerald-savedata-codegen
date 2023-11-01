#ifndef GUARD_LU_SERIALIZE_PokeNews
#define GUARD_LU_SERIALIZE_PokeNews

void lu_BitstreamRead_PokeNews(struct lu_BitstreamState*, struct PokeNews* dst);
void lu_BitstreamWrite_PokeNews(struct lu_BitstreamState*, const struct PokeNews* src);

#endif