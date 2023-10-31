#include "struct-serialize//serialize_SecretBase.h"

#include "global.h" // struct definition

// dependencies
#include "./serialize_SecretBaseParty.h"

#include "string_util.h" // gflib; for StringLength

// check constants:
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if TRAINER_ID_LENGTH != 4
   #error Constant `TRAINER_ID_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if DECOR_MAX_SECRET_BASE != 16
   #error Constant `DECOR_MAX_SECRET_BASE` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_SecretBase(struct lu_BitstreamState* state, const struct SecretBase* src) {
   src.secretBaseId = lu_BitstreamRead_u8(state, 8);
   src.toRegister = lu_BitstreamRead_u8(state, 4);
   src.gender = lu_BitstreamRead_bool(state, src.gender);
   src.battledOwnerToday = lu_BitstreamRead_bool(state, src.battledOwnerToday);
   src.registryStatus = lu_BitstreamRead_u8(state, 2);
   lu_BitstreamRead_string(state, src.trainerName, PLAYER_NAME_LENGTH, 3);
   {
      u16 i;
      for (i = 0; i < TRAINER_ID_LENGTH; ++i) { 
            src.trainerId[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   src.language = lu_BitstreamRead_u8(state, 3);
   src.numSecretBasesReceived = lu_BitstreamRead_u16(state, 16);
   src.numTimesEntered = lu_BitstreamRead_u8(state, 8);
   src.unused = lu_BitstreamRead_u8(state, 8);
   {
      u16 i;
      for (i = 0; i < DECOR_MAX_SECRET_BASE; ++i) { 
            src.decorations[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < DECOR_MAX_SECRET_BASE; ++i) { 
            src.decorationPositions[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   lu_BitstreamRead_SecretBaseParty(state, &src.party);
}
void lu_BitstreamWrite_SecretBase(struct lu_BitstreamState* state, const struct SecretBase* src) {
   lu_BitstreamWrite_u8(state, src.secretBaseId, 8);
   lu_BitstreamWrite_u8(state, src.toRegister, 4);
   lu_BitstreamWrite_bool(state, src.gender);
   lu_BitstreamWrite_bool(state, src.battledOwnerToday);
   lu_BitstreamWrite_u8(state, src.registryStatus, 2);
   lu_BitstreamWrite_string(state, src.trainerName, PLAYER_NAME_LENGTH, 3);
   {
      u16 i;
      for (i = 0; i < TRAINER_ID_LENGTH; ++i) { 
            lu_BitstreamWrite_u8(state, src.trainerId[i], 8);
      }
   }
   lu_BitstreamWrite_u8(state, src.language, 3);
   lu_BitstreamWrite_u16(state, src.numSecretBasesReceived, 16);
   lu_BitstreamWrite_u8(state, src.numTimesEntered, 8);
   lu_BitstreamWrite_u8(state, src.unused, 8);
   {
      u16 i;
      for (i = 0; i < DECOR_MAX_SECRET_BASE; ++i) { 
            lu_BitstreamWrite_u8(state, src.decorations[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < DECOR_MAX_SECRET_BASE; ++i) { 
            lu_BitstreamWrite_u8(state, src.decorationPositions[i], 8);
      }
   }
   lu_BitstreamWrite_SecretBaseParty(state, &src.party);
}