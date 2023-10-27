#include "./tests.serialize//serialize_SaveBlock2.h"

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

#include "global.h"

// TODO:
// void lu_BitstreamRead_SaveBlock2(struct lu_BitstreamState* state, struct SaveBlock2* dst);

void lu_BitstreamWrite_SaveBlock2(struct lu_BitstreamState* state, const struct SaveBlock2* src) {
   lu_BitstreamWrite_string(state, src.playerName, 7, 3);
   lu_BitstreamWrite_bool(state, src.playerGender);
   lu_BitstreamWrite_u8(state, src.specialSaveWarpFlags, 8);
   {
      u16 i;
      for (i = 0; i < 4; ++i) { 
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
      for (i = 0; i < 4; ++i) { 
            lu_BitstreamWrite_Apprentice(state, &src.apprentices[i]);
      }
   }
   lu_BitstreamWrite_BerryCrush(state, &src.berryCrush);
   lu_BitstreamWrite_PokemonJumpRecords(state, &src.pokeJump);
   lu_BitstreamWrite_BerryPickingResults(state, &src.berryPick);
   {
      u16 i, j, k;
      for (i = 0; i < 9; ++i) { 
         for (j = 0; j < 2; ++j) { 
            for (k = 0; k < 3; ++k) { 
                  lu_BitstreamWrite_RankingHall1P(state, &src.hallRecords1P[i][j][k]);
            }
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < 3; ++j) { 
               lu_BitstreamWrite_RankingHall2P(state, &src.hallRecords2P[i][j]);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 5; ++i) { 
         for (j = 0; j < 4; ++j) { 
               lu_BitstreamWrite_u16(state, src.contestLinkResults[i][j], 16);
         }
      }
   }
   lu_BitstreamWrite_BattleFrontier(state, &src.frontier);
}