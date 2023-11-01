# Last failed codegen
This report describes the last failed attempt at code generation.

## Overall stats
19664 bytes in RAM / 19840 bytes available across 5 sectors (99% space usage)  
136179 packed bits = 17023 packed bytes (85% space usage)  


## Struct stats
| Name | Bytes in RAM | Packed bits | Packed bytes | Savings |
| - | -: | -: | -: | -: |
| SaveBlock2 | 3912 | 28017 |3503 | 409 (10 %) |
| SaveBlock1 | 15752 | 108162 |13521 | 2231 (14 %) |
| **Total** | 19664 | 136179 |17023 | 2641 (13 %) |


## Sector group: CharacterData
**Status:** Successfully packed into 1 sectors.


| Sector # | Bits used | Bits available | % |
| - | -: | -: |
| 0 | 28017 | 31744 | 88% |
| Total | 28017 | 31744 | 88% |

### Member info
* Last field serialized to sector 0: `p_SaveBlock2`
## Sector group: WorldData
**Status:** Successfully packed into 4 sectors.


| Sector # | Bits used | Bits available | % |
| - | -: | -: |
| 0 | 31736 | 31744 | 99% |
| 1 | 31723 | 31744 | 99% |
| 2 | 31705 | 31744 | 99% |
| 3 | 12998 | 31744 | 40% |
| Total | 108162 | 126976 | 85% |

### Member info
* Last field serialized to sector 0: `p_SaveBlock1->vars[12]`
* Last field serialized to sector 1: `p_SaveBlock1->secretBases[16].party.moves[9]`
* Last field serialized to sector 2: `p_SaveBlock1->mysteryGift.trainerIds[1][3]`
* Last field serialized to sector 3: `p_SaveBlock1->waldaPhrase`
