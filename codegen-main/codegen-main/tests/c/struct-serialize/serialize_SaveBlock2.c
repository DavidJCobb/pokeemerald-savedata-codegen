#include "struct-serialize//serialize_SaveBlock2.h"

#include "global.h" // struct definition

// dependencies
#include "./serialize_Pokedex.h"
#include "./serialize_Time.h"
#include "./serialize_Time.h"
#include "./serialize_PlayersApprentice.h"
#include "./serialize_Apprentice.h"
#include "./serialize_BerryCrush.h"
#include "./serialize_PokemonJumpRecords.h"
#include "./serialize_BerryPickingResults.h"
#include "./serialize_RankingHall1P.h"
#include "./serialize_RankingHall2P.h"
#include "./serialize_BattleFrontier.h"

#include "string_util.h" // gflib; for StringLength

// check constants:
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if TRAINER_ID_LENGTH != 4
   #error Constant `TRAINER_ID_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if APPRENTICE_COUNT != 4
   #error Constant `APPRENTICE_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if HALL_FACILITIES_COUNT != 9
   #error Constant `HALL_FACILITIES_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if FRONTIER_LVL_MODE_COUNT != 2
   #error Constant `FRONTIER_LVL_MODE_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if HALL_RECORDS_COUNT != 3
   #error Constant `HALL_RECORDS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if FRONTIER_LVL_MODE_COUNT != 2
   #error Constant `FRONTIER_LVL_MODE_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if HALL_RECORDS_COUNT != 3
   #error Constant `HALL_RECORDS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if CONTEST_CATEGORIES_COUNT != 5
   #error Constant `CONTEST_CATEGORIES_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if CONTESTANT_COUNT != 4
   #error Constant `CONTESTANT_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif

// TODO:
// void lu_BitstreamRead_SaveBlock2(struct lu_BitstreamState* state, struct SaveBlock2* dst);

void lu_BitstreamWrite_SaveBlock2(struct lu_BitstreamState* state, const struct SaveBlock2* src) {
   lu_BitstreamWrite_string(state, src.playerName, PLAYER_NAME_LENGTH, 3);
   lu_BitstreamWrite_bool(state, src.playerGender);
   lu_BitstreamWrite_u8(state, src.specialSaveWarpFlags, 8);
   {
      u16 i;
      for (i = 0; i < TRAINER_ID_LENGTH; ++i) { 
            lu_BitstreamWrite_u8(state, src.playerTrainerId[i], 8);
      }
   }
   lu_BitstreamWrite_u16(state, src.playTimeHours, 16);
   lu_BitstreamWrite_u8(state, src.playTimeMinutes, 6);
   lu_BitstreamWrite_u8(state, src.playTimeSeconds, 6);
   lu_BitstreamWrite_u8(state, src.playTimeVBlanks, 8);
   lu_BitstreamWrite_u8(state, src.optionsButtonMode, 2);
   lu_BitstreamWrite_u8(state, src.optionsTextSpeed, 3);
   lu_BitstreamWrite_u8(state, src.optionsWindowFrameType, 5);
   lu_BitstreamWrite_bool(state, src.optionsSound);
   lu_BitstreamWrite_bool(state, src.optionsBattleStyle);
   lu_BitstreamWrite_bool(state, src.optionsBattleSceneOff);
   lu_BitstreamWrite_bool(state, src.regionMapZoom);
   lu_BitstreamWrite_bool(state, src.optionsRunningToggle);
   lu_BitstreamWrite_Pokedex(state, &src.pokedex);
   lu_BitstreamWrite_Time(state, &src.localTimeOffset);
   lu_BitstreamWrite_Time(state, &src.lastBerryTreeUpdate);
   lu_BitstreamWrite_u32(state, src.gcnLinkFlags, 32);
   lu_BitstreamWrite_u32(state, src.encryptionKey, 32);
   lu_BitstreamWrite_PlayersApprentice(state, &src.playerApprentice);
   {
      u16 i;
      for (i = 0; i < APPRENTICE_COUNT; ++i) { 
            lu_BitstreamWrite_Apprentice(state, &src.apprentices[i]);
      }
   }
   lu_BitstreamWrite_BerryCrush(state, &src.berryCrush);
   lu_BitstreamWrite_PokemonJumpRecords(state, &src.pokeJump);
   lu_BitstreamWrite_BerryPickingResults(state, &src.berryPick);
   {
      u16 i, j, k;
      for (i = 0; i < HALL_FACILITIES_COUNT; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
            for (k = 0; k < HALL_RECORDS_COUNT; ++k) { 
                  lu_BitstreamWrite_RankingHall1P(state, &src.hallRecords1P[i][j][k]);
            }
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < FRONTIER_LVL_MODE_COUNT; ++i) { 
         for (j = 0; j < HALL_RECORDS_COUNT; ++j) { 
               lu_BitstreamWrite_RankingHall2P(state, &src.hallRecords2P[i][j]);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < CONTEST_CATEGORIES_COUNT; ++i) { 
         for (j = 0; j < CONTESTANT_COUNT; ++j) { 
               lu_BitstreamWrite_u16(state, src.contestLinkResults[i][j], 16);
         }
      }
   }
   lu_BitstreamWrite_BattleFrontier(state, &src.frontier);
}