#ifndef GUARD_LU_SERIALIZE_EmeraldBattleTowerRecord
#define GUARD_LU_SERIALIZE_EmeraldBattleTowerRecord

void lu_BitstreamRead_EmeraldBattleTowerRecord(struct lu_BitstreamState*, struct EmeraldBattleTowerRecord* dst);void lu_BitstreamWrite_EmeraldBattleTowerRecord(struct lu_BitstreamState*, struct EmeraldBattleTowerRecord* src);
#endif