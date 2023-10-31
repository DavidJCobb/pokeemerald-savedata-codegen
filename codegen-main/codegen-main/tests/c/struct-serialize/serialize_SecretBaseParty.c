#include "struct-serialize//serialize_SecretBaseParty.h"

#include "global.h" // struct definition

// check constants:
#if PARTY_SIZE != 6
   #error Constant `PARTY_SIZE` changed in C, but XML not updated or codegen not re-run!
#endif

// TODO:
// void lu_BitstreamRead_SecretBaseParty(struct lu_BitstreamState* state, struct SecretBaseParty* dst);

void lu_BitstreamWrite_SecretBaseParty(struct lu_BitstreamState* state, const struct SecretBaseParty* src) {
   {
      u16 i;
      for (i = 0; i < PARTY_SIZE; ++i) { 
            lu_BitstreamWrite_u32(state, src.personality[i], 32);
      }
   }
   {
      u16 i;
      for (i = 0; i < PARTY_SIZE * MAX_MON_MOVES; ++i) { 
            lu_BitstreamWrite_u16(state, src.moves[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < PARTY_SIZE; ++i) { 
            lu_BitstreamWrite_u16(state, src.species[i], 11);
      }
   }
   {
      u16 i;
      for (i = 0; i < PARTY_SIZE; ++i) { 
            lu_BitstreamWrite_u16(state, src.heldItems[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < PARTY_SIZE; ++i) { 
            lu_BitstreamWrite_u8(state, src.levels[i], 7);
      }
   }
   {
      u16 i;
      for (i = 0; i < PARTY_SIZE; ++i) { 
            lu_BitstreamWrite_u8(state, src.EVs[i], 8);
      }
   }
}