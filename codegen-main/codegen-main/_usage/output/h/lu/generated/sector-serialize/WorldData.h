#ifndef GUARD_LU_SERIALIZE_SECTOR_WorldData
#define GUARD_LU_SERIALIZE_SECTOR_WorldData

#include "lu/bitstreams.h"

void lu_ReadSaveSector_WorldData00(u8* dst, struct SaveBlock1* p_SaveBlock1);
void lu_WriteSaveSector_WorldData00(u8* dst, const struct SaveBlock1* p_SaveBlock1);
void lu_ReadSaveSector_WorldData01(u8* dst, struct SaveBlock1* p_SaveBlock1);
void lu_WriteSaveSector_WorldData01(u8* dst, const struct SaveBlock1* p_SaveBlock1);
void lu_ReadSaveSector_WorldData02(u8* dst, struct SaveBlock1* p_SaveBlock1);
void lu_WriteSaveSector_WorldData02(u8* dst, const struct SaveBlock1* p_SaveBlock1);
void lu_ReadSaveSector_WorldData03(u8* dst, struct SaveBlock1* p_SaveBlock1);
void lu_WriteSaveSector_WorldData03(u8* dst, const struct SaveBlock1* p_SaveBlock1);

#endif