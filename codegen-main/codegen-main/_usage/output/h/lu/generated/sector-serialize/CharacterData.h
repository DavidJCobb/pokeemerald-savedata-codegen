#ifndef GUARD_LU_SERIALIZE_SECTOR_CharacterData
#define GUARD_LU_SERIALIZE_SECTOR_CharacterData

#include "lu/bitstreams.h"

void lu_ReadSaveSector_CharacterData00(u8* dst, struct SaveBlock2* p_SaveBlock2);
void lu_WriteSaveSector_CharacterData00(u8* dst, const struct SaveBlock2* p_SaveBlock2);

#endif