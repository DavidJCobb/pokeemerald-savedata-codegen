#include "./tests.serialize//serialize_BattleFrontier.h"

// dependencies
#include "./serialize_EmeraldBattleTowerRecord.h"
#include "./serialize_BattleTowerInterview.h"
#include "./serialize_BattleTowerEReaderTrainer.h"
#include "./serialize_BattleDomeTrainer.h"
#include "./serialize_PyramidBag.h"
#include "./serialize_RentalMon.h"
#include "./serialize_DomeMonData.h"

#include "string_util.h" // gflib; for StringLength

#include "global.h"

// TODO:
// void lu_BitstreamRead_BattleFrontier(struct lu_BitstreamState* state, struct BattleFrontier* dst);

void lu_BitstreamWrite_BattleFrontier(struct lu_BitstreamState* state, struct BattleFrontier* src) {
   lu_BitstreamWrite_EmeraldBattleTowerRecord(&src.towerPlayer);
   {
      u16 i;
      for (i = 0; i < 5; ++i) { 
            lu_BitstreamWrite_EmeraldBattleTowerRecord(&src.towerRecords[i]);
      }
   }
   lu_BitstreamWrite_BattleTowerInterview(&src.towerInterview);
   lu_BitstreamWrite_BattleTowerEReaderTrainer(&src.ereaderTrainer);
   lu_BitstreamWrite_u8(state, src.challengeStatus, 8);
   lu_BitstreamWrite_u8(state, src.lvlMode, 8);
   lu_BitstreamWrite_u8(state, src.challengePaused, 8);
   lu_BitstreamWrite_u8(state, src.disableRecordBattle, 8);
   {
      u16 i;
      for (i = 0; i < 4; ++i) { 
            lu_BitstreamWrite_u16(state, src.selectedPartyMons[i], 11);
      }
   }
   lu_BitstreamWrite_u16(state, src.curChallengeBattleNum, 16);
   {
      u16 i;
      for (i = 0; i < 20; ++i) { 
            lu_BitstreamWrite_u16(state, src.trainerIds[i], 16);
      }
   }
   lu_BitstreamWrite_u32(state, src.winStreakActiveFlags, 32);
   {
      u16 i, j;
      for (i = 0; i < 4; ++i) { 
         for (j = 0; j < 2; ++j) { 
               lu_BitstreamWrite_u16(state, src.towerWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 4; ++i) { 
         for (j = 0; j < 2; ++j) { 
               lu_BitstreamWrite_u16(state, src.towerRecordWinStreaks[i][j], 16);
         }
      }
   }
   lu_BitstreamWrite_u16(state, src.battledBrainFlags, 16);
   lu_BitstreamWrite_u16(state, src.towerSinglesStreak, 16);
   lu_BitstreamWrite_u16(state, src.towerNumWins, 16);
   lu_BitstreamWrite_u8(state, src.towerBattleOutcome, 8);
   lu_BitstreamWrite_u8(state, src.towerLvlMode, 8);
   lu_BitstreamWrite_u8(state, src.domeAttemptedSingles50, 8);
   lu_BitstreamWrite_u8(state, src.domeAttemptedSinglesOpen, 8);
   lu_BitstreamWrite_u8(state, src.domeHasWonSingles50, 8);
   lu_BitstreamWrite_u8(state, src.domeHasWonSinglesOpen, 8);
   lu_BitstreamWrite_u8(state, src.domeAttemptedDoubles50, 8);
   lu_BitstreamWrite_u8(state, src.domeAttemptedDoublesOpen, 8);
   lu_BitstreamWrite_u8(state, src.domeHasWonDoubles50, 8);
   lu_BitstreamWrite_u8(state, src.domeHasWonDoublesOpen, 8);
   lu_BitstreamWrite_u8(state, src.domeUnused, 8);
   lu_BitstreamWrite_u8(state, src.domeLvlMode, 8);
   lu_BitstreamWrite_u8(state, src.domeBattleMode, 8);
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < 2; ++j) { 
               lu_BitstreamWrite_u16(state, src.domeWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < 2; ++j) { 
               lu_BitstreamWrite_u16(state, src.domeRecordWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < 2; ++j) { 
               lu_BitstreamWrite_u16(state, src.domeTotalChampionships[i][j], 16);
         }
      }
   }
   {
      u16 i;
      for (i = 0; i < 16; ++i) { 
            lu_BitstreamWrite_BattleDomeTrainer(&src.domeTrainers[i]);
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 16; ++i) { 
         for (j = 0; j < 3; ++j) { 
               lu_BitstreamWrite_u16(state, src.domeMonIds[i][j], 11);
         }
      }
   }
   lu_BitstreamWrite_u16(state, src.unused_DC4, 16);
   lu_BitstreamWrite_u16(state, src.palacePrice, 16);
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < 2; ++j) { 
               lu_BitstreamWrite_u16(state, src.palaceWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < 2; ++j) { 
               lu_BitstreamWrite_u16(state, src.palaceRecordWinStreaks[i][j], 16);
         }
      }
   }
   lu_BitstreamWrite_u16(state, src.arenaPrize, 16);
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < 2; ++j) { 
               lu_BitstreamWrite_u16(state, src.arenaWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < 2; ++j) { 
               lu_BitstreamWrite_u16(state, src.arenaRecordWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < 2; ++j) { 
               lu_BitstreamWrite_u16(state, src.factoryWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < 2; ++j) { 
               lu_BitstreamWrite_u16(state, src.factoryRecordWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < 2; ++j) { 
               lu_BitstreamWrite_u16(state, src.factoryRentsCount[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < 2; ++j) { 
               lu_BitstreamWrite_u16(state, src.factoryRecordRentsCount[i][j], 16);
         }
      }
   }
   lu_BitstreamWrite_u16(state, src.pikePrize, 16);
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < 2; ++j) { 
               lu_BitstreamWrite_u16(state, src.pikeWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < 2; ++j) { 
               lu_BitstreamWrite_u16(state, src.pikeRecordStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < 2; ++j) { 
               lu_BitstreamWrite_u16(state, src.pikeTotalStreaks[i][j], 16);
         }
      }
   }
   lu_BitstreamWrite_u8(state, src.pikeHintedRoomIndex, 8);
   lu_BitstreamWrite_u8(state, src.pikeHintedRoomType, 8);
   lu_BitstreamWrite_u8(state, src.pikeHealingRoomsDisabled, 8);
   {
      u16 i;
      for (i = 0; i < 3; ++i) { 
            lu_BitstreamWrite_u16(state, src.pikeHeldItemsBackup[i], 16);
      }
   }
   lu_BitstreamWrite_u16(state, src.pyramidPrize, 16);
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < 2; ++j) { 
               lu_BitstreamWrite_u16(state, src.pyramidWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < 2; ++j) { 
               lu_BitstreamWrite_u16(state, src.pyramidRecordWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i;
      for (i = 0; i < 4; ++i) { 
            lu_BitstreamWrite_u16(state, src.pyramidRandoms[i], 16);
      }
   }
   lu_BitstreamWrite_u8(state, src.pyramidTrainerFlags, 8);
   lu_BitstreamWrite_PyramidBag(&src.pyramidBag);
   lu_BitstreamWrite_u8(state, src.pyramidLightRadius, 8);
   lu_BitstreamWrite_u16(state, src.verdanturfTentPrize, 16);
   lu_BitstreamWrite_u16(state, src.fallaborTentPrize, 16);
   lu_BitstreamWrite_u16(state, src.slateportTentPrize, 16);
   {
      u16 i;
      for (i = 0; i < 6; ++i) { 
            lu_BitstreamWrite_RentalMon(&src.rentalMons[i]);
      }
   }
   lu_BitstreamWrite_u16(state, src.battlePoints, 16);
   lu_BitstreamWrite_u16(state, src.cardBattlePoints, 16);
   lu_BitstreamWrite_u32(state, src.battlesCount, 32);
   {
      u16 i;
      for (i = 0; i < 16; ++i) { 
            lu_BitstreamWrite_u16(state, src.domeWinningMoves[i], 16);
      }
   }
   lu_BitstreamWrite_u8(state, src.trainerFlags, 8);
   {
      u16 i;
      for (i = 0; i < 2; ++i) { 
            lu_BitstreamWrite_string(state, src.opponentNames[i], 7, 3);
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < 4; ++j) { 
               lu_BitstreamWrite_u8(state, src.opponentTrainerIds[i][j], 8);
         }
      }
   }
   lu_BitstreamWrite_u8(state, src.unk_EF9, 8);
   lu_BitstreamWrite_u8(state, src.savedGame, 8);
   lu_BitstreamWrite_u8(state, src.unused_EFA, 8);
   lu_BitstreamWrite_u8(state, src.unused_EFB, 8);
   {
      u16 i;
      for (i = 0; i < 3; ++i) { 
            lu_BitstreamWrite_DomeMonData(&src.domePlayerPartyData[i]);
      }
   }
}