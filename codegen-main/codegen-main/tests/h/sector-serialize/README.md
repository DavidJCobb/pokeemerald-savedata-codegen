# Last failed codegen
This report describes the last failed attempt at code generation.

## Overall stats
19664 bytes in RAM / 19840 bytes available across 5 sectors (99% space usage)  
136421 packed bits = 17053 packed bytes (85% space usage)  
### Debug output: struct RAM sizes
#### SaveBlock2

| Member | Offset | Size |
| - | - |
| playerName | 0 | 8 |
| playerGender | 8 | 1 |
| specialSaveWarpFlags | 9 | 1 |
| playerTrainerId | 10 | 4 |
| playTimeHours | 14 | 2 |
| playTimeMinutes | 16 | 1 |
| playTimeSeconds | 17 | 1 |
| playTimeVBlanks | 18 | 1 |
| optionsButtonMode | 19 | 1 |
| optionsTextSpeed | 20 + 0 bits | 3 bits |
| optionsWindowFrameType | 20 + 3 bits | 5 bits |
| optionsSound | 20 + 8 bits | 1 bits |
| optionsBattleStyle | 21 + 1 bits | 1 bits |
| optionsBattleSceneOff | 21 + 2 bits | 1 bits |
| regionMapZoom | 21 + 3 bits | 1 bits |
| optionsRunningToggle | 21 + 4 bits | 1 bits |
| pokedex | 24 | 120 |
| filler_90 | 144 | 8 |
| localTimeOffset | 152 | 8 |
| lastBerryTreeUpdate | 160 | 8 |
| gcnLinkFlags | 168 | 4 |
| encryptionKey | 172 | 4 |
| playerApprentice | 176 | 44 |
| apprentices | 220 | 272 |
| berryCrush | 492 | 16 |
| pokeJump | 508 | 16 |
| berryPick | 524 | 16 |
| hallRecords1P | 540 | 864 |
| hallRecords2P | 1404 | 168 |
| contestLinkResults | 1572 | 40 |
| frontier | 1612 | 2300 |
| -end- | 3912 | |

#### SaveBlock1

| Member | Offset | Size |
| - | - |
| pos | 0 | 4 |
| location | 4 | 8 |
| continueGameWarp | 12 | 8 |
| dynamicWarp | 20 | 8 |
| lastHealLocation | 28 | 8 |
| escapeWarp | 36 | 8 |
| savedMusic | 44 | 2 |
| weather | 46 | 1 |
| weatherCycleStage | 47 | 1 |
| flashLevel | 48 | 1 |
| mapLayoutId | 50 | 2 |
| mapView | 52 | 512 |
| playerPartyCount | 564 | 1 |
| playerParty | 568 | 600 |
| money | 1168 | 4 |
| coins | 1172 | 2 |
| registeredItem | 1174 | 2 |
| pcItems | 1176 | 200 |
| bagPocket_Items | 1376 | 120 |
| bagPocket_KeyItems | 1496 | 120 |
| bagPocket_PokeBalls | 1616 | 64 |
| bagPocket_TMHM | 1680 | 256 |
| bagPocket_Berries | 1936 | 184 |
| pokeblocks | 2120 | 320 |
| seen1 | 2440 | 52 |
| berryBlenderRecords | 2492 | 6 |
| unused_9C2 | 2498 | 6 |
| trainerRematchStepCounter | 2504 | 2 |
| trainerRematches | 2506 | 100 |
| objectEvents | 2608 | 576 |
| objectEventTemplates | 3184 | 1536 |
| flags | 4720 | 300 |
| vars | 5020 | 512 |
| gameStats | 5532 | 256 |
| berryTrees | 5788 | 1024 |
| secretBases | 6812 | 3200 |
| playerRoomDecorations | 10012 | 12 |
| playerRoomDecorationPositions | 10024 | 12 |
| decorationDesks | 10036 | 10 |
| decorationChairs | 10046 | 10 |
| decorationPlants | 10056 | 10 |
| decorationOrnaments | 10066 | 30 |
| decorationMats | 10096 | 30 |
| decorationPosters | 10126 | 10 |
| decorationDolls | 10136 | 40 |
| decorationCushions | 10176 | 10 |
| tvShows | 10188 | 900 |
| pokeNews | 11088 | 64 |
| outbreakPokemonSpecies | 11152 | 2 |
| outbreakLocationMapNum | 11154 | 1 |
| outbreakLocationMapGroup | 11155 | 1 |
| outbreakPokemonLevel | 11156 | 1 |
| outbreakUnused1 | 11157 | 1 |
| outbreakUnused2 | 11158 | 1 |
| outbreakPokemonMoves | 11160 | 8 |
| outbreakUnused3 | 11168 | 1 |
| outbreakPokemonProbability | 11169 | 1 |
| outbreakDaysLeft | 11170 | 2 |
| gabbyAndTyData | 11172 | 12 |
| easyChatProfile | 11184 | 12 |
| easyChatBattleStart | 11196 | 12 |
| easyChatBattleWon | 11208 | 12 |
| easyChatBattleLost | 11220 | 12 |
| mail | 11232 | 576 |
| unlockedTrendySayings | 11808 | 7 |
| oldMan | 11816 | 64 |
| dewfordTrends | 11880 | 40 |
| contestWinners | 11920 | 416 |
| daycare | 12336 | 288 |
| linkBattleRecords | 12624 | 88 |
| giftRibbons | 12712 | 11 |
| externalEventData | 12723 | 20 |
| externalEventFlags | 12743 | 21 |
| roamer | 12764 | 28 |
| enigmaBerry | 12792 | 52 |
| mysteryGift | 12844 | 876 |
| unused_3598 | 13720 | 384 |
| trainerHillTimes | 14104 | 16 |
| ramScript | 14120 | 1004 |
| recordMixingGift | 15124 | 16 |
| seen2 | 15140 | 52 |
| lilycoveLady | 15192 | 64 |
| trainerNameRecords | 15256 | 240 |
| registeredTexts | 15496 | 210 |
| unused_3D5A | 15706 | 10 |
| trainerHill | 15716 | 12 |
| waldaPhrase | 15728 | 24 |
| -end- | 15752 | |

#### Roamer

| Member | Offset | Size |
| - | - |
| ivs | 0 | 4 |
| personality | 4 | 4 |
| species | 8 | 2 |
| hp | 10 | 2 |
| level | 12 | 1 |
| status | 13 | 1 |
| cool | 14 | 1 |
| beauty | 15 | 1 |
| cute | 16 | 1 |
| smart | 17 | 1 |
| tough | 18 | 1 |
| active | 19 | 1 |
| filler | 20 | 8 |
| -end- | 28 | |



## Struct stats
| Name | Bytes in RAM (single) | Packed bitcount (single) | Count | Total bytes | Total packed bits |
| - | -: | -: | -: | -: | -: |
| SaveBlock2 | 3912 | 28029 |1 | 3912 | 28029 |
| SaveBlock1 | 15752 | 108392 |1 | 15752 | 108392 |


## Sector group: CharacterData
**Status:** Successfully packed into 1 sectors.


| Sector # | Bits used | Bits available | % |
| - | -: | -: |
| 0 | 28029 | 31744 | 88% |
| Total | 28029 | 31744 | 88% |

### Member info
* Last field serialized to sector 0: `p_SaveBlock2`
## Sector group: WorldData
**Status:** Successfully packed into 4 sectors.


| Sector # | Bits used | Bits available | % |
| - | -: | -: |
| 0 | 31730 | 31744 | 99% |
| 1 | 31726 | 31744 | 99% |
| 2 | 31714 | 31744 | 99% |
| 3 | 13222 | 31744 | 41% |
| Total | 108392 | 126976 | 85% |

### Member info
* Last field serialized to sector 0: `p_SaveBlock1->vars[9]`
* Last field serialized to sector 1: `p_SaveBlock1->secretBases[16].party.moves[3]`
* Last field serialized to sector 2: `p_SaveBlock1->mysteryGift.trainerIds[0][1]`
* Last field serialized to sector 3: `p_SaveBlock1->waldaPhrase`
