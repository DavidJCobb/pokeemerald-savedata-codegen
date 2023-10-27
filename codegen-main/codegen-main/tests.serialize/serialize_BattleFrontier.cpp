#include "./tests.serialize//serialize_BattleFrontier.h"

#include "global.h" // struct definition

// dependencies
#include "./serialize_EmeraldBattleTowerRecord.h"
#include "./serialize_EmeraldBattleTowerRecord.h"
#include "./serialize_BattleTowerInterview.h"
#include "./serialize_BattleTowerEReaderTrainer.h"
#include "./serialize_BattleDomeTrainer.h"
#include "./serialize_PyramidBag.h"
#include "./serialize_RentalMon.h"
#include "./serialize_DomeMonData.h"

#include "string_util.h" // gflib; for StringLength

// check constants:
#if BATTLE_TOWER_RECORD_COUNT != 5
   #error Constant `BATTLE_TOWER_RECORD_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if MAX_FRONTIER_PARTY_SIZE != 4
   #error Constant `MAX_FRONTIER_PARTY_SIZE` changed in C, but XML not updated or codegen not re-run!
#endif
#if FRONTIER_LVL_MODE_COUNT != 2
   #error Constant `FRONTIER_LVL_MODE_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if DOME_TOURNAMENT_TRAINERS_COUNT != 16
   #error Constant `DOME_TOURNAMENT_TRAINERS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if FRONTIER_PARTY_SIZE != 3
   #error Constant `FRONTIER_PARTY_SIZE` changed in C, but XML not updated or codegen not re-run!
#endif
#if FRONTIER_LVL_MODE_COUNT != 2
   #error Constant `FRONTIER_LVL_MODE_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if FRONTIER_PARTY_SIZE != 3
   #error Constant `FRONTIER_PARTY_SIZE` changed in C, but XML not updated or codegen not re-run!
#endif
#if FRONTIER_LVL_MODE_COUNT != 2
   #error Constant `FRONTIER_LVL_MODE_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if DOME_TOURNAMENT_TRAINERS_COUNT != 16
   #error Constant `DOME_TOURNAMENT_TRAINERS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if FRONTIER_LVL_MODE_COUNT != 2
   #error Constant `FRONTIER_LVL_MODE_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if TRAINER_ID_LENGTH != 4
   #error Constant `TRAINER_ID_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if FRONTIER_PARTY_SIZE != 3
   #error Constant `FRONTIER_PARTY_SIZE` changed in C, but XML not updated or codegen not re-run!
#endif

// TODO:
// void lu_BitstreamRead_BattleFrontier(struct lu_BitstreamState* state, struct BattleFrontier* dst);

void lu_BitstreamWrite_BattleFrontier(struct lu_BitstreamState* state, const struct BattleFrontier* src) {
   lu_BitstreamWrite_EmeraldBattleTowerRecord(state, &src.towerPlayer);
   {
      u16 i;
      for (i = 0; i < BATTLE_TOWER_RECORD_COUNT; ++i) { 
            lu_BitstreamWrite_EmeraldBattleTowerRecord(state, &src.towerRecords[i]);
      }
   }
   lu_BitstreamWrite_BattleTowerInterview(state, &src.towerInterview);
   lu_BitstreamWrite_BattleTowerEReaderTrainer(state, &src.ereaderTrainer);
   lu_BitstreamWrite_u8(state, src.challengeStatus, 8);
   lu_BitstreamWrite_u8(state, src.lvlMode, 2);
   lu_BitstreamWrite_bool(state, src.challengePaused);
   lu_BitstreamWrite_bool(state, src.disableRecordBattle);
   {
      u16 i;
      for (i = 0; i < MAX_FRONTIER_PARTY_SIZE; ++i) { 
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
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
               lu_BitstreamWrite_u16(state, src.towerWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 4; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
               lu_BitstreamWrite_u16(state, src.towerRecordWinStreaks[i][j], 16);
         }
      }
   }
   lu_BitstreamWrite_u16(state, src.battledBrainFlags, 16);
   lu_BitstreamWrite_u16(state, src.towerSinglesStreak, 16);
   lu_BitstreamWrite_u16(state, src.towerNumWins, 16);
   lu_BitstreamWrite_u8(state, src.towerBattleOutcome, 8);
   lu_BitstreamWrite_u8(state, src.towerLvlMode, 8);
   lu_BitstreamWrite_bool(state, src.domeAttemptedSingles50);
   lu_BitstreamWrite_bool(state, src.domeAttemptedSinglesOpen);
   lu_BitstreamWrite_bool(state, src.domeHasWonSingles50);
   lu_BitstreamWrite_bool(state, src.domeHasWonSinglesOpen);
   lu_BitstreamWrite_bool(state, src.domeAttemptedDoubles50);
   lu_BitstreamWrite_bool(state, src.domeAttemptedDoublesOpen);
   lu_BitstreamWrite_bool(state, src.domeHasWonDoubles50);
   lu_BitstreamWrite_bool(state, src.domeHasWonDoublesOpen);
   lu_BitstreamWrite_u8(state, src.domeUnused, 8);
   lu_BitstreamWrite_u8(state, src.domeLvlMode, 8);
   lu_BitstreamWrite_u8(state, src.domeBattleMode, 8);
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
               lu_BitstreamWrite_u16(state, src.domeWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
               lu_BitstreamWrite_u16(state, src.domeRecordWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
               lu_BitstreamWrite_u16(state, src.domeTotalChampionships[i][j], 16);
         }
      }
   }
   {
      u16 i;
      for (i = 0; i < DOME_TOURNAMENT_TRAINERS_COUNT; ++i) { 
            lu_BitstreamWrite_BattleDomeTrainer(state, &src.domeTrainers[i]);
      }
   }
   {
      u16 i, j;
      for (i = 0; i < DOME_TOURNAMENT_TRAINERS_COUNT; ++i) { 
         for (j = 0; j < FRONTIER_PARTY_SIZE; ++j) { 
               lu_BitstreamWrite_u16(state, src.domeMonIds[i][j], 11);
         }
      }
   }
   lu_BitstreamWrite_u16(state, src.unused_DC4, 16);
   lu_BitstreamWrite_u16(state, src.palacePrice, 16);
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
               lu_BitstreamWrite_u16(state, src.palaceWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
               lu_BitstreamWrite_u16(state, src.palaceRecordWinStreaks[i][j], 16);
         }
      }
   }
   lu_BitstreamWrite_u16(state, src.arenaPrize, 16);
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
               lu_BitstreamWrite_u16(state, src.arenaWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
               lu_BitstreamWrite_u16(state, src.arenaRecordWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
               lu_BitstreamWrite_u16(state, src.factoryWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
               lu_BitstreamWrite_u16(state, src.factoryRecordWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
               lu_BitstreamWrite_u16(state, src.factoryRentsCount[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
               lu_BitstreamWrite_u16(state, src.factoryRecordRentsCount[i][j], 16);
         }
      }
   }
   lu_BitstreamWrite_u16(state, src.pikePrize, 16);
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
               lu_BitstreamWrite_u16(state, src.pikeWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
               lu_BitstreamWrite_u16(state, src.pikeRecordStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
               lu_BitstreamWrite_u16(state, src.pikeTotalStreaks[i][j], 16);
         }
      }
   }
   lu_BitstreamWrite_u8(state, src.pikeHintedRoomIndex, 3);
   lu_BitstreamWrite_u8(state, src.pikeHintedRoomType, 3);
   lu_BitstreamWrite_bool(state, src.pikeHealingRoomsDisabled);
   {
      u16 i;
      for (i = 0; i < FRONTIER_PARTY_SIZE; ++i) { 
            lu_BitstreamWrite_u16(state, src.pikeHeldItemsBackup[i], 16);
      }
   }
   lu_BitstreamWrite_u16(state, src.pyramidPrize, 16);
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
               lu_BitstreamWrite_u16(state, src.pyramidWinStreaks[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < FRONTIER_LVL_MODE_COUNT; ++j) { 
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
   lu_BitstreamWrite_PyramidBag(state, &src.pyramidBag);
   lu_BitstreamWrite_u8(state, src.pyramidLightRadius, 8);
   lu_BitstreamWrite_u16(state, src.verdanturfTentPrize, 16);
   lu_BitstreamWrite_u16(state, src.fallaborTentPrize, 16);
   lu_BitstreamWrite_u16(state, src.slateportTentPrize, 16);
   {
      u16 i;
      for (i = 0; i < FRONTIER_PARTY_SIZE * 2; ++i) { 
            lu_BitstreamWrite_RentalMon(state, &src.rentalMons[i]);
      }
   }
   lu_BitstreamWrite_u16(state, src.battlePoints, 16);
   lu_BitstreamWrite_u16(state, src.cardBattlePoints, 16);
   lu_BitstreamWrite_u32(state, src.battlesCount, 32);
   {
      u16 i;
      for (i = 0; i < DOME_TOURNAMENT_TRAINERS_COUNT; ++i) { 
            lu_BitstreamWrite_u16(state, src.domeWinningMoves[i], 16);
      }
   }
   lu_BitstreamWrite_u8(state, src.trainerFlags, 8);
   {
      u16 i;
      for (i = 0; i < FRONTIER_LVL_MODE_COUNT; ++i) { 
            lu_BitstreamWrite_string(state, src.opponentNames[i], PLAYER_NAME_LENGTH, 3);
      }
   }
   {
      u16 i, j;
      for (i = 0; i < FRONTIER_LVL_MODE_COUNT; ++i) { 
         for (j = 0; j < TRAINER_ID_LENGTH; ++j) { 
               lu_BitstreamWrite_u8(state, src.opponentTrainerIds[i][j], 8);
         }
      }
   }
   lu_BitstreamWrite_u8(state, src.unk_EF9, 7);
   lu_BitstreamWrite_bool(state, src.savedGame);
   lu_BitstreamWrite_u8(state, src.unused_EFA, 8);
   lu_BitstreamWrite_u8(state, src.unused_EFB, 8);
   {
      u16 i;
      for (i = 0; i < FRONTIER_PARTY_SIZE; ++i) { 
            lu_BitstreamWrite_DomeMonData(state, &src.domePlayerPartyData[i]);
      }
   }
}