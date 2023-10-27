   struct EmeraldBattleTowerRecord towerPlayer;
   struct EmeraldBattleTowerRecord towerRecords[BATTLE_TOWER_RECORD_COUNT]; // From record mixing.
   struct BattleTowerInterview towerInterview;
   struct BattleTowerEReaderTrainer ereaderTrainer;
   u8 challengeStatus;
   u8 lvlMode : 2;
   u8 challengePaused : 1;
   u8 disableRecordBattle : 1;
   u16 selectedPartyMons[MAX_FRONTIER_PARTY_SIZE];
   u16 curChallengeBattleNum; // Battle number / room number (Pike) / floor number (Pyramid)
   u16 trainerIds[20];
   u32 winStreakActiveFlags;
   u16 towerWinStreaks[4][FRONTIER_LVL_MODE_COUNT];
   u16 towerRecordWinStreaks[4][FRONTIER_LVL_MODE_COUNT];
   u16 battledBrainFlags;
   u16 towerSinglesStreak; // Never read
   u16 towerNumWins; // Increments to MAX_STREAK but never read otherwise
   u8 towerBattleOutcome;
   u8 towerLvlMode;
   u8 domeAttemptedSingles50 : 1;
   u8 domeAttemptedSinglesOpen : 1;
   u8 domeHasWonSingles50 : 1;
   u8 domeHasWonSinglesOpen : 1;
   u8 domeAttemptedDoubles50 : 1;
   u8 domeAttemptedDoublesOpen : 1;
   u8 domeHasWonDoubles50 : 1;
   u8 domeHasWonDoublesOpen : 1;
   u8 domeUnused;
   u8 domeLvlMode;
   u8 domeBattleMode;
   u16 domeWinStreaks[2][FRONTIER_LVL_MODE_COUNT];
   u16 domeRecordWinStreaks[2][FRONTIER_LVL_MODE_COUNT];
   u16 domeTotalChampionships[2][FRONTIER_LVL_MODE_COUNT];
   struct BattleDomeTrainer domeTrainers[DOME_TOURNAMENT_TRAINERS_COUNT];
   u16 domeMonIds[DOME_TOURNAMENT_TRAINERS_COUNT][FRONTIER_PARTY_SIZE];
   u16 unused_DC4;
   u16 palacePrice;
   u16 palaceWinStreaks[2][FRONTIER_LVL_MODE_COUNT];
   u16 palaceRecordWinStreaks[2][FRONTIER_LVL_MODE_COUNT];
   u16 arenaPrize;
   u16 arenaWinStreaks[2][FRONTIER_LVL_MODE_COUNT];
   u16 arenaRecordWinStreaks[2][FRONTIER_LVL_MODE_COUNT];
   u16 factoryWinStreaks[2][FRONTIER_LVL_MODE_COUNT];
   u16 factoryRecordWinStreaks[2][FRONTIER_LVL_MODE_COUNT];
   u16 factoryRentsCount[2][FRONTIER_LVL_MODE_COUNT];
   u16 factoryRecordRentsCount[2][FRONTIER_LVL_MODE_COUNT];
   u16 pikePrize;
   u16 pikeWinStreaks[2][FRONTIER_LVL_MODE_COUNT];
   u16 pikeRecordStreaks[2][FRONTIER_LVL_MODE_COUNT];
   u16 pikeTotalStreaks[2][FRONTIER_LVL_MODE_COUNT];
   u8 pikeHintedRoomIndex : 3;
   u8 pikeHintedRoomType : 3;
   u8 pikeHealingRoomsDisabled : 1;
   u16 pikeHeldItemsBackup[FRONTIER_PARTY_SIZE];
   u16 pyramidPrize;
   u16 pyramidWinStreaks[2][FRONTIER_LVL_MODE_COUNT];
   u16 pyramidRecordWinStreaks[2][FRONTIER_LVL_MODE_COUNT];
   u16 pyramidRandoms[4];
   u8 pyramidTrainerFlags; // 1 bit for each trainer (MAX_PYRAMID_TRAINERS)
   struct PyramidBag pyramidBag;
   u8 pyramidLightRadius;
   u16 verdanturfTentPrize;
   u16 fallaborTentPrize;
   u16 slateportTentPrize;
   struct RentalMon rentalMons[FRONTIER_PARTY_SIZE * 2];
   u16 battlePoints;
   u16 cardBattlePoints;
   u32 battlesCount;
   u16 domeWinningMoves[DOME_TOURNAMENT_TRAINERS_COUNT];
   u8 trainerFlags;
   u8 opponentNames[FRONTIER_LVL_MODE_COUNT][PLAYER_NAME_LENGTH + 1];
   u8 opponentTrainerIds[FRONTIER_LVL_MODE_COUNT][TRAINER_ID_LENGTH];
   u8 unk_EF9 : 7; // Never read
   u8 savedGame : 1;
   u8 unused_EFA;
   u8 unused_EFB;
   struct DomeMonData domePlayerPartyData[FRONTIER_PARTY_SIZE];
