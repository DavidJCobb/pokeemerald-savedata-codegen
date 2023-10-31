#include "struct-serialize//serialize_SaveBlock1.h"

#include "global.h" // struct definition

// dependencies
#include "./serialize_Coords16.h"
#include "./serialize_WarpData.h"
#include "./serialize_WarpData.h"
#include "./serialize_WarpData.h"
#include "./serialize_WarpData.h"
#include "./serialize_WarpData.h"
#include "./serialize_Pokemon.h"
#include "./serialize_ItemSlot.h"
#include "./serialize_ItemSlot.h"
#include "./serialize_ItemSlot.h"
#include "./serialize_ItemSlot.h"
#include "./serialize_ItemSlot.h"
#include "./serialize_ItemSlot.h"
#include "./serialize_Pokeblock.h"
#include "./serialize_ObjectEvent.h"
#include "./serialize_ObjectEventTemplate.h"
#include "./serialize_BerryTree.h"
#include "./serialize_SecretBase.h"
#include "./serialize_PokeNews.h"
#include "./serialize_GabbyAndTyData.h"
#include "./serialize_Mail.h"
#include "./serialize_DewfordTrend.h"
#include "./serialize_ContestWinner.h"
#include "./serialize_DayCare.h"
#include "./serialize_LinkBattleRecords.h"
#include "./serialize_ExternalEventData.h"
#include "./serialize_ExternalEventFlags.h"
#include "./serialize_Roamer.h"
#include "./serialize_EnigmaBerry.h"
#include "./serialize_MysteryGiftSave.h"
#include "./serialize_RamScript.h"
#include "./serialize_RecordMixingGift.h"
#include "./serialize_TrainerNameRecord.h"
#include "./serialize_TrainerHillSave.h"
#include "./serialize_WaldaPhrase.h"

// check constants:
#if PARTY_SIZE != 6
   #error Constant `PARTY_SIZE` changed in C, but XML not updated or codegen not re-run!
#endif
#if PC_ITEMS_COUNT != 50
   #error Constant `PC_ITEMS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if BAG_ITEMS_COUNT != 30
   #error Constant `BAG_ITEMS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if BAG_KEYITEMS_COUNT != 30
   #error Constant `BAG_KEYITEMS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if BAG_POKEBALLS_COUNT != 16
   #error Constant `BAG_POKEBALLS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if BAG_TMHM_COUNT != 64
   #error Constant `BAG_TMHM_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if BAG_BERRIES_COUNT != 46
   #error Constant `BAG_BERRIES_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if POKEBLOCKS_COUNT != 40
   #error Constant `POKEBLOCKS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if NUM_DEX_FLAG_BYTES != 52
   #error Constant `NUM_DEX_FLAG_BYTES` changed in C, but XML not updated or codegen not re-run!
#endif
#if MAX_REMATCH_ENTRIES != 100
   #error Constant `MAX_REMATCH_ENTRIES` changed in C, but XML not updated or codegen not re-run!
#endif
#if OBJECT_EVENTS_COUNT != 16
   #error Constant `OBJECT_EVENTS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if OBJECT_EVENT_TEMPLATES_COUNT != 64
   #error Constant `OBJECT_EVENT_TEMPLATES_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if NUM_FLAG_BYTES != 300
   #error Constant `NUM_FLAG_BYTES` changed in C, but XML not updated or codegen not re-run!
#endif
#if VARS_COUNT != 256
   #error Constant `VARS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if NUM_GAME_STATS != 64
   #error Constant `NUM_GAME_STATS` changed in C, but XML not updated or codegen not re-run!
#endif
#if BERRY_TREES_COUNT != 128
   #error Constant `BERRY_TREES_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if SECRET_BASES_COUNT != 20
   #error Constant `SECRET_BASES_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if DECOR_MAX_PLAYERS_HOUSE != 12
   #error Constant `DECOR_MAX_PLAYERS_HOUSE` changed in C, but XML not updated or codegen not re-run!
#endif
#if POKE_NEWS_COUNT != 16
   #error Constant `POKE_NEWS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if MAX_MON_MOVES != 4
   #error Constant `MAX_MON_MOVES` changed in C, but XML not updated or codegen not re-run!
#endif
#if EASY_CHAT_BATTLE_WORDS_COUNT != 6
   #error Constant `EASY_CHAT_BATTLE_WORDS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if MAIL_COUNT != 16
   #error Constant `MAIL_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if NUM_TRENDY_SAYING_BYTES != 7
   #error Constant `NUM_TRENDY_SAYING_BYTES` changed in C, but XML not updated or codegen not re-run!
#endif
#if SAVED_TRENDS_COUNT != 5
   #error Constant `SAVED_TRENDS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if NUM_CONTEST_WINNERS != 13
   #error Constant `NUM_CONTEST_WINNERS` changed in C, but XML not updated or codegen not re-run!
#endif
#if GIFT_RIBBONS_COUNT != 11
   #error Constant `GIFT_RIBBONS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if NUM_TRAINER_HILL_MODES != 4
   #error Constant `NUM_TRAINER_HILL_MODES` changed in C, but XML not updated or codegen not re-run!
#endif
#if NUM_DEX_FLAG_BYTES != 52
   #error Constant `NUM_DEX_FLAG_BYTES` changed in C, but XML not updated or codegen not re-run!
#endif
#if UNION_ROOM_KB_ROW_COUNT != 10
   #error Constant `UNION_ROOM_KB_ROW_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif

// TODO:
// void lu_BitstreamRead_SaveBlock1(struct lu_BitstreamState* state, struct SaveBlock1* dst);

void lu_BitstreamWrite_SaveBlock1(struct lu_BitstreamState* state, const struct SaveBlock1* src) {
   lu_BitstreamWrite_Coords16(state, &src.pos);
   lu_BitstreamWrite_WarpData(state, &src.location);
   lu_BitstreamWrite_WarpData(state, &src.continueGameWarp);
   lu_BitstreamWrite_WarpData(state, &src.dynamicWarp);
   lu_BitstreamWrite_WarpData(state, &src.lastHealLocation);
   lu_BitstreamWrite_WarpData(state, &src.escapeWarp);
   lu_BitstreamWrite_u16(state, src.savedMusic, 16);
   lu_BitstreamWrite_u8(state, src.weather, 8);
   lu_BitstreamWrite_u8(state, src.weatherCycleStage, 8);
   lu_BitstreamWrite_u8(state, src.flashLevel, 8);
   lu_BitstreamWrite_u16(state, src.mapLayoutId, 16);
   {
      u16 i;
      for (i = 0; i < 0x100; ++i) { 
            lu_BitstreamWrite_u16(state, src.mapView[i], 16);
      }
   }
   lu_BitstreamWrite_u8(state, src.playerPartyCount, 3);
   {
      u16 i;
      for (i = 0; i < PARTY_SIZE; ++i) { 
            lu_BitstreamWrite_Pokemon(state, &src.playerParty[i]);
      }
   }
   lu_BitstreamWrite_u32(state, src.money, 32);
   lu_BitstreamWrite_u16(state, src.coins, 16);
   lu_BitstreamWrite_u16(state, src.registeredItem, 16);
   {
      u16 i;
      for (i = 0; i < PC_ITEMS_COUNT; ++i) { 
            lu_BitstreamWrite_ItemSlot(state, &src.pcItems[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < BAG_ITEMS_COUNT; ++i) { 
            lu_BitstreamWrite_ItemSlot(state, &src.bagPocket_Items[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < BAG_KEYITEMS_COUNT; ++i) { 
            lu_BitstreamWrite_ItemSlot(state, &src.bagPocket_KeyItems[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < BAG_POKEBALLS_COUNT; ++i) { 
            lu_BitstreamWrite_ItemSlot(state, &src.bagPocket_PokeBalls[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < BAG_TMHM_COUNT; ++i) { 
            lu_BitstreamWrite_ItemSlot(state, &src.bagPocket_TMHM[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < BAG_BERRIES_COUNT; ++i) { 
            lu_BitstreamWrite_ItemSlot(state, &src.bagPocket_Berries[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < POKEBLOCKS_COUNT; ++i) { 
            lu_BitstreamWrite_Pokeblock(state, &src.pokeblocks[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < NUM_DEX_FLAG_BYTES; ++i) { 
            lu_BitstreamWrite_u8(state, src.seen1[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 3; ++i) { 
            lu_BitstreamWrite_u16(state, src.berryBlenderRecords[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < 6; ++i) { 
            lu_BitstreamWrite_u8(state, src.unused_9C2[i], 8);
      }
   }
   lu_BitstreamWrite_u16(state, src.trainerRematchStepCounter, 16);
   {
      u16 i;
      for (i = 0; i < MAX_REMATCH_ENTRIES; ++i) { 
            lu_BitstreamWrite_u8(state, src.trainerRematches[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < OBJECT_EVENTS_COUNT; ++i) { 
            lu_BitstreamWrite_ObjectEvent(state, &src.objectEvents[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < OBJECT_EVENT_TEMPLATES_COUNT; ++i) { 
            lu_BitstreamWrite_ObjectEventTemplate(state, &src.objectEventTemplates[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < NUM_FLAG_BYTES; ++i) { 
            lu_BitstreamWrite_u8(state, src.flags[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < VARS_COUNT; ++i) { 
            lu_BitstreamWrite_u16(state, src.vars[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < NUM_GAME_STATS; ++i) { 
            lu_BitstreamWrite_u32(state, src.gameStats[i], 32);
      }
   }
   {
      u16 i;
      for (i = 0; i < BERRY_TREES_COUNT; ++i) { 
            lu_BitstreamWrite_BerryTree(state, &src.berryTrees[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < SECRET_BASES_COUNT; ++i) { 
            lu_BitstreamWrite_SecretBase(state, &src.secretBases[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < DECOR_MAX_PLAYERS_HOUSE; ++i) { 
            lu_BitstreamWrite_u8(state, src.playerRoomDecorations[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < DECOR_MAX_PLAYERS_HOUSE; ++i) { 
            lu_BitstreamWrite_u8(state, src.playerRoomDecorationPositions[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 10; ++i) { 
            lu_BitstreamWrite_u8(state, src.decorationDesks[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 10; ++i) { 
            lu_BitstreamWrite_u8(state, src.decorationChairs[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 10; ++i) { 
            lu_BitstreamWrite_u8(state, src.decorationPlants[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 30; ++i) { 
            lu_BitstreamWrite_u8(state, src.decorationOrnaments[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 30; ++i) { 
            lu_BitstreamWrite_u8(state, src.decorationMats[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 10; ++i) { 
            lu_BitstreamWrite_u8(state, src.decorationPosters[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 40; ++i) { 
            lu_BitstreamWrite_u8(state, src.decorationDolls[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 10; ++i) { 
            lu_BitstreamWrite_u8(state, src.decorationCushions[i], 8);
      }
   }
   lu_BitstreamWrite_buffer(state, &src.tvShows, 36);
   {
      u16 i;
      for (i = 0; i < POKE_NEWS_COUNT; ++i) { 
            lu_BitstreamWrite_PokeNews(state, &src.pokeNews[i]);
      }
   }
   lu_BitstreamWrite_u16(state, src.outbreakPokemonSpecies, 11);
   lu_BitstreamWrite_u8(state, src.outbreakLocationMapNum, 8);
   lu_BitstreamWrite_u8(state, src.outbreakLocationMapGroup, 8);
   lu_BitstreamWrite_u8(state, src.outbreakPokemonLevel, 7);
   lu_BitstreamWrite_u8(state, src.outbreakUnused1, 8);
   lu_BitstreamWrite_u8(state, src.outbreakUnused2, 8);
   {
      u16 i;
      for (i = 0; i < MAX_MON_MOVES; ++i) { 
            lu_BitstreamWrite_u16(state, src.outbreakPokemonMoves[i], 16);
      }
   }
   lu_BitstreamWrite_u8(state, src.outbreakUnused3, 8);
   lu_BitstreamWrite_u8(state, src.outbreakPokemonProbability, 8);
   lu_BitstreamWrite_u16(state, src.outbreakDaysLeft, 16);
   lu_BitstreamWrite_GabbyAndTyData(state, &src.gabbyAndTyData);
   {
      u16 i;
      for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) { 
            lu_BitstreamWrite_u16(state, src.easyChatProfile[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) { 
            lu_BitstreamWrite_u16(state, src.easyChatBattleStart[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) { 
            lu_BitstreamWrite_u16(state, src.easyChatBattleWon[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) { 
            lu_BitstreamWrite_u16(state, src.easyChatBattleLost[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < MAIL_COUNT; ++i) { 
            lu_BitstreamWrite_Mail(state, &src.mail[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < NUM_TRENDY_SAYING_BYTES; ++i) { 
            lu_BitstreamWrite_u8(state, src.unlockedTrendySayings[i], 8);
      }
   }
   lu_BitstreamWrite_buffer(state, &src.oldMan, 64);
   {
      u16 i;
      for (i = 0; i < SAVED_TRENDS_COUNT; ++i) { 
            lu_BitstreamWrite_DewfordTrend(state, &src.dewfordTrends[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < NUM_CONTEST_WINNERS; ++i) { 
            lu_BitstreamWrite_ContestWinner(state, &src.contestWinners[i]);
      }
   }
   lu_BitstreamWrite_DayCare(state, &src.daycare);
   lu_BitstreamWrite_LinkBattleRecords(state, &src.linkBattleRecords);
   {
      u16 i;
      for (i = 0; i < GIFT_RIBBONS_COUNT; ++i) { 
            lu_BitstreamWrite_u8(state, src.giftRibbons[i], 8);
      }
   }
   lu_BitstreamWrite_ExternalEventData(state, &src.externalEventData);
   lu_BitstreamWrite_ExternalEventFlags(state, &src.externalEventFlags);
   lu_BitstreamWrite_Roamer(state, &src.roamer);
   lu_BitstreamWrite_EnigmaBerry(state, &src.enigmaBerry);
   lu_BitstreamWrite_MysteryGiftSave(state, &src.mysteryGift);
   {
      u16 i;
      for (i = 0; i < 0x180; ++i) { 
            lu_BitstreamWrite_u8(state, src.unused_3598[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < NUM_TRAINER_HILL_MODES; ++i) { 
            lu_BitstreamWrite_u32(state, src.trainerHillTimes[i], 32);
      }
   }
   lu_BitstreamWrite_RamScript(state, &src.ramScript);
   lu_BitstreamWrite_RecordMixingGift(state, &src.recordMixingGift);
   {
      u16 i;
      for (i = 0; i < NUM_DEX_FLAG_BYTES; ++i) { 
            lu_BitstreamWrite_u8(state, src.seen2[i], 8);
      }
   }
   lu_BitstreamWrite_buffer(state, &src.lilycoveLady, 64);
   {
      u16 i;
      for (i = 0; i < 20; ++i) { 
            lu_BitstreamWrite_TrainerNameRecord(state, &src.trainerNameRecords[i]);
      }
   }
   {
      u16 i, j;
      for (i = 0; i < UNION_ROOM_KB_ROW_COUNT; ++i) { 
         for (j = 0; j < 21; ++j) { 
               lu_BitstreamWrite_u8(state, src.registeredTexts[i][j], 8);
         }
      }
   }
   {
      u16 i;
      for (i = 0; i < 10; ++i) { 
            lu_BitstreamWrite_u8(state, src.unused_3D5A[i], 8);
      }
   }
   lu_BitstreamWrite_TrainerHillSave(state, &src.trainerHill);
   lu_BitstreamWrite_WaldaPhrase(state, &src.waldaPhrase);
}