#ifndef GUARD_LU_SERIALIZE_RecordMixingGiftData
#define GUARD_LU_SERIALIZE_RecordMixingGiftData

void lu_BitstreamRead_RecordMixingGiftData(struct lu_BitstreamState*, struct RecordMixingGiftData* dst);
void lu_BitstreamWrite_RecordMixingGiftData(struct lu_BitstreamState*, const struct RecordMixingGiftData* src);

#endif