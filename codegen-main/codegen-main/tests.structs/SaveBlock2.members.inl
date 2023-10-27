   u8 playerName[8];
   u8 playerGender;
   u8 specialSaveWarpFlags;
   u8 playerTrainerId[4];
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
   struct Time localTimeOffset;
   struct Time lastBerryTreeUpdate;
   u32 gcnLinkFlags; // Read by Pokemon Colosseum/XD
   u32 encryptionKey;
   struct PlayersApprentice playerApprentice;
   struct Apprentice apprentices[4];
   struct BerryCrush berryCrush;
   struct PokemonJumpRecords pokeJump;
   struct BerryPickingResults berryPick;
   struct RankingHall1P hallRecords1P[9][2][3]; // From record mixing.
   struct RankingHall2P hallRecords2P[2][3]; // From record mixing.
   u16 contestLinkResults[5][4]; // From record mixing.
   struct BattleFrontier frontier;
