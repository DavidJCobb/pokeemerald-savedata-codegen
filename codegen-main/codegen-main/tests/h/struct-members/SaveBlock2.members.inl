   u8 playerName[PLAYER_NAME_LENGTH + 1];
   u8 playerGender;
   u8 specialSaveWarpFlags;
   u8 playerTrainerId[TRAINER_ID_LENGTH];
   u16 playTimeHours;
   u8 playTimeMinutes;
   u8 playTimeSeconds;
   u8 playTimeVBlanks;
   u8 optionsButtonMode;
   u16 optionsTextSpeed : 3;
   u16 optionsWindowFrameType : 5;
   u16 optionsSound : 1;
   u16 optionsBattleStyle : 1;
   u16 optionsBattleSceneOff : 1;
   u16 regionMapZoom : 1;
   u16 optionsRunningToggle : 1;
   struct Pokedex pokedex;
   u8 filler_90[8];
   struct Time localTimeOffset;
   struct Time lastBerryTreeUpdate;
   u32 gcnLinkFlags; // Read by Pokemon Colosseum/XD
   u32 encryptionKey;
   struct PlayersApprentice playerApprentice;
   struct Apprentice apprentices[APPRENTICE_COUNT];
   struct BerryCrush berryCrush;
   struct PokemonJumpRecords pokeJump;
   struct BerryPickingResults berryPick;
   struct RankingHall1P hallRecords1P[HALL_FACILITIES_COUNT][FRONTIER_LVL_MODE_COUNT][HALL_RECORDS_COUNT]; // From record mixing.
   struct RankingHall2P hallRecords2P[FRONTIER_LVL_MODE_COUNT][HALL_RECORDS_COUNT]; // From record mixing.
   u16 contestLinkResults[CONTEST_CATEGORIES_COUNT][CONTESTANT_COUNT]; // From record mixing.
   struct BattleFrontier frontier;
