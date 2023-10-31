#ifndef GUARD_LU_SERIALIZE_Daycare
#define GUARD_LU_SERIALIZE_Daycare

void lu_BitstreamRead_Daycare(struct lu_BitstreamState*, struct Daycare* dst);
void lu_BitstreamWrite_Daycare(struct lu_BitstreamState*, const struct Daycare* src);

#endif