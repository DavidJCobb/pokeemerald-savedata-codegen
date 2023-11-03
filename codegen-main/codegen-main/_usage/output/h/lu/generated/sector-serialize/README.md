# Codegen for bitpacked savedata
This report describes the last successful attempt at code generation.

In vanilla `pokeemerald`, the `SaveBlock1` (world state) and `SaveBlock2` (character state) structs consume 99% of the space allotted to them in flash memory (savedata). This is because they are blindly `memcpy`'d from RAM. A bitpacked format would consume substantially less space; however, maintaining the code to bitpack these structs would be prohibitively difficult by hand because savedata is split into ~4KiB strips ("sectors"). Instead, we use custom code generation to produce the serialization code, minding sector boundaries.

## Overall stats
19664 bytes in RAM / 19840 bytes available across 5 sectors (99% space usage)  
138980 packed bits = 17373 packed bytes (87% space usage)  


## Struct stats
| Name | Bytes in RAM | Packed bits | Packed bytes | Savings |
| - | -: | -: | -: | -: |
| SaveBlock2 | 3912 | 28018 |3503 | 409 (10%) |
| SaveBlock1 | 15752 | 110962 |13871 | 1881 (11%) |
| **Total** | 19664 | 138980 |17373 | 2291 (11%) |


## Sector group: CharacterData
**Status:** Successfully packed into 1 sectors.


| Sector # | Bits used | Bits available | % |
| - | -: | -: |
| 0 | 28018 | 31744 | 88% |
| Total | 28018 | 31744 | 88% |

### Member info
* Last field serialized to sector 0: `p_SaveBlock2`
## Sector group: WorldData
**Status:** Successfully packed into 4 sectors.


| Sector # | Bits used | Bits available | % |
| - | -: | -: |
| 0 | 31736 | 31744 | 99% |
| 1 | 31730 | 31744 | 99% |
| 2 | 31575 | 31744 | 99% |
| 3 | 15921 | 31744 | 50% |
| Total | 110962 | 126976 | 87% |

### Member info
* Last field serialized to sector 0: `p_SaveBlock1->flags[61]`
* Last field serialized to sector 1: `p_SaveBlock1->secretBases[14].party.species[2]`
* Last field serialized to sector 2: `p_SaveBlock1->mysteryGift.card.titleText`
* Last field serialized to sector 3: `p_SaveBlock1->waldaPhrase`


## Every struct
The "Savings" columns are measured in bytes. The percentage in the "Savings per" column indicates by how much the struct itself has been made smaller thanks to bitpacking. The percentage in the "Savings total" column indicates by how much the total size of all serialized data has been made smaller as a result of bitpacking that row's specific struct type.

| Name | Bytes in RAM | Packed bits | Packed bytes | Savings per | Count | Savings total |
| - | -: | -: | -: | -: | -: | -: |
| Apprentice | 68 | 494 | 62 | 6 (8%) | 4 | 24 (1%) |
| ApprenticeMon | 12 | 84 | 11 | 1 (8%) | 12 | 12 (0%) |
| ApprenticeQuestion | 4 | 24 | 3 | 1 (25%) | 9 | 9 (0%) |
| BattleDomeTrainer | 4 | 16 | 2 | 2 (50%) | 16 | 32 (0%) |
| BattleFrontier | 2300 | 16700 | 2088 | 212 (9%) | 1 | 212 (10%) |
| BattleTowerEReaderTrainer | 188 | 1445 | 181 | 7 (3%) | 1 | 7 (0%) |
| BattleTowerInterview | 24 | 168 | 21 | 3 (12%) | 1 | 3 (0%) |
| BattleTowerPokemon | 44 | 334 | 42 | 2 (4%) | 27 | 54 (5%) |
| Berry2 | 28 | 211 | 27 | 1 (3%) | 1 | 1 (0%) |
| BerryCrush | 16 | 128 | 16 | 0 (0%) | 1 | 0 (0%) |
| BerryPickingResults | 16 | 128 | 16 | 0 (0%) | 1 | 0 (0%) |
| BerryTree | 8 | 48 | 6 | 2 (25%) | 128 | 256 (3%) |
| BoxPokemon | 80 | 635 | 80 | 0 (0%) | 8 | 0 (3%) |
| ContestWinner | 32 | 234 | 30 | 2 (6%) | 13 | 26 (1%) |
| Coords16 | 4 | 32 | 4 | 0 (0%) | 49 | 0 (0%) |
| DayCare | 288 | 2182 | 273 | 15 (5%) | 1 | 15 (1%) |
| DaycareMail | 56 | 404 | 51 | 5 (8%) | 2 | 10 (0%) |
| DaycareMon | 140 | 1071 | 134 | 6 (4%) | 2 | 12 (1%) |
| DewfordTrend | 8 | 63 | 8 | 0 (0%) | 5 | 0 (0%) |
| DomeMonData | 16 | 120 | 15 | 1 (6%) | 3 | 3 (0%) |
| EmeraldBattleTowerRecord | 236 | 1782 | 223 | 13 (5%) | 6 | 78 (6%) |
| EnigmaBerry | 52 | 403 | 51 | 1 (1%) | 1 | 1 (0%) |
| ExternalEventData | 20 | 160 | 20 | 0 (0%) | 1 | 0 (0%) |
| ExternalEventFlags | 21 | 163 | 21 | 0 (0%) | 1 | 0 (0%) |
| GabbyAndTyData | 12 | 86 | 11 | 1 (8%) | 1 | 1 (0%) |
| ItemSlot | 4 | 16 | 2 | 2 (50%) | 236 | 472 (2%) |
| LinkBattleRecord | 16 | 123 | 16 | 0 (0%) | 5 | 0 (0%) |
| LinkBattleRecords | 88 | 630 | 79 | 9 (10%) | 1 | 9 (0%) |
| Mail | 36 | 255 | 32 | 4 (11%) | 18 | 72 (2%) |
| MysteryGiftSave | 876 | 6966 | 871 | 5 (0%) | 1 | 5 (4%) |
| ObjectEvent | 36 | 276 | 35 | 1 (2%) | 16 | 16 (2%) |
| ObjectEventTemplate | 24 | 160 | 20 | 4 (16%) | 64 | 256 (6%) |
| PlayersApprentice | 44 | 261 | 33 | 11 (25%) | 1 | 11 (0%) |
| PokeNews | 4 | 32 | 4 | 0 (0%) | 16 | 0 (0%) |
| Pokeblock | 8 | 56 | 7 | 1 (12%) | 40 | 40 (1%) |
| Pokedex | 120 | 960 | 120 | 0 (0%) | 1 | 0 (0%) |
| Pokemon | 100 | 794 | 100 | 0 (0%) | 6 | 0 (3%) |
| PokemonJumpRecords | 16 | 112 | 14 | 2 (12%) | 1 | 2 (0%) |
| PyramidBag | 60 | 320 | 40 | 20 (33%) | 1 | 20 (0%) |
| RamScript | 1004 | 8024 | 1003 | 1 (0%) | 1 | 1 (5%) |
| RamScriptData | 1000 | 7992 | 999 | 1 (0%) | 1 | 1 (5%) |
| RankingHall1P | 16 | 110 | 14 | 2 (12%) | 54 | 108 (3%) |
| RankingHall2P | 28 | 201 | 26 | 2 (7%) | 6 | 12 (0%) |
| RecordMixingGift | 16 | 121 | 16 | 0 (0%) | 1 | 0 (0%) |
| RecordMixingGiftData | 12 | 89 | 12 | 0 (0%) | 1 | 0 (0%) |
| RentalMon | 12 | 59 | 8 | 4 (33%) | 6 | 24 (0%) |
| Roamer | 28 | 147 | 19 | 9 (32%) | 1 | 9 (0%) |
| SaveBlock1 | 15752 | 110962 | 13871 | 1881 (11%) | 1 | 1881 (70%) |
| SaveBlock2 | 3912 | 28018 | 3503 | 409 (10%) | 1 | 409 (17%) |
| SecretBase | 160 | 1181 | 148 | 12 (7%) | 20 | 240 (15%) |
| SecretBaseParty | 108 | 786 | 99 | 9 (8%) | 20 | 180 (10%) |
| Time | 8 | 36 | 5 | 3 (37%) | 2 | 6 (0%) |
| TrainerHillSave | 12 | 88 | 11 | 1 (8%) | 1 | 1 (0%) |
| TrainerNameRecord | 12 | 91 | 12 | 0 (0%) | 20 | 0 (1%) |
| WaldaPhrase | 24 | 177 | 23 | 1 (4%) | 1 | 1 (0%) |
| WarpData | 8 | 56 | 7 | 1 (12%) | 5 | 5 (0%) |
| WonderCard | 332 | 2635 | 330 | 2 (0%) | 1 | 2 (1%) |
| WonderCardMetadata | 36 | 283 | 36 | 0 (0%) | 1 | 0 (0%) |
| WonderNews | 444 | 3552 | 444 | 0 (0%) | 1 | 0 (2%) |
| WonderNewsMetadata | 4 | 16 | 2 | 2 (50%) | 1 | 2 (0%) |

