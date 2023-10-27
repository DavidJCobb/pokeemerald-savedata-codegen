   struct EmeraldBattleTowerRecord towerPlayer;
   struct EmeraldBattleTowerRecord towerRecords[5]; // From record mixing.
   struct BattleTowerInterview towerInterview;
   struct BattleTowerEReaderTrainer ereaderTrainer;
   u8 challengeStatus;
   u8 lvlMode : 2;
   u8 challengePaused : 1;
   u8 disableRecordBattle : 1;
   u16 selectedPartyMons[4];
   u16 curChallengeBattleNum; // Battle number / room number (Pike) / floor number (Pyramid)
   u16 trainerIds[20];
   u32 winStreakActiveFlags;
   u16 towerWinStreaks[4][2];
   u16 towerRecordWinStreaks[4][2];
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
   u16 domeWinStreaks[2][2];
   u16 domeRecordWinStreaks[2][2];
   u16 domeTotalChampionships[2][2];
   struct BattleDomeTrainer domeTrainers[16];
   u16 domeMonIds[16][3];
   u16 unused_DC4;
   u16 palacePrice;
   u16 palaceWinStreaks[2][2];
   u16 palaceRecordWinStreaks[2][2];
   u16 arenaPrize;
   u16 arenaWinStreaks[2][2];
   u16 arenaRecordWinStreaks[2][2];
   u16 factoryWinStreaks[2][2];
   u16 factoryRecordWinStreaks[2][2];
   u16 factoryRentsCount[2][2];
   u16 factoryRecordRentsCount[2][2];
   u16 pikePrize;
   u16 pikeWinStreaks[2][2];
   u16 pikeRecordStreaks[2][2];
   u16 pikeTotalStreaks[2][2];
   u8 pikeHintedRoomIndex : 3;
   u8 pikeHintedRoomType : 3;
   u8 pikeHealingRoomsDisabled : 1;
   u16 pikeHeldItemsBackup[3];
   u16 pyramidPrize;
   u16 pyramidWinStreaks[2][2];
   u16 pyramidRecordWinStreaks[2][2];
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
   u16 domeWinningMoves[16];
   u8 trainerFlags;
   u8 opponentNames[2][8];
   u8 opponentTrainerIds[2][4];
   u8 unk_EF9 : 7; // Never read
   u8 savedGame : 1;
   u8 unused_EFA;
   u8 unused_EFB;
   struct DomeMonData domePlayerPartyData[3];
