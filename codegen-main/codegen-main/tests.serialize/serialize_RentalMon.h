#ifndef GUARD_LU_SERIALIZE_RentalMon
#define GUARD_LU_SERIALIZE_RentalMon

void lu_BitstreamRead_RentalMon(struct lu_BitstreamState*, struct RentalMon* dst);void lu_BitstreamWrite_RentalMon(struct lu_BitstreamState*, struct RentalMon* src);
#endif