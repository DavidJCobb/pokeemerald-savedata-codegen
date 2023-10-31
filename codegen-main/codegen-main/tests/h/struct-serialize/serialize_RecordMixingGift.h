#ifndef GUARD_LU_SERIALIZE_RecordMixingGift
#define GUARD_LU_SERIALIZE_RecordMixingGift

void lu_BitstreamRead_RecordMixingGift(struct lu_BitstreamState*, struct RecordMixingGift* dst);
void lu_BitstreamWrite_RecordMixingGift(struct lu_BitstreamState*, const struct RecordMixingGift* src);

#endif