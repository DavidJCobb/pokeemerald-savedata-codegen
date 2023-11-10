# Codegen for bitpacked savedata
This report describes the last successful attempt at code generation.

In vanilla `pokeemerald`, the `SaveBlock1` (world state) and `SaveBlock2` (character state) structs consume 99% of the space allotted to them in flash memory (savedata). This is because they are blindly `memcpy`'d from RAM. A bitpacked format would consume substantially less space; however, maintaining the code to bitpack these structs would be prohibitively difficult by hand because savedata is split into ~4KiB strips ("sectors"). Instead, we use custom code generation to produce the serialization code, minding sector boundaries.

## Overall stats
0 bytes in RAM / 0 bytes available across 0 sectors (-2147483648% space usage)  
0 packed bits = 0 packed bytes (-2147483648% space usage)  


## Struct stats
| Name | Bytes in RAM | Packed bits | Packed bytes | Savings |
| - | -: | -: | -: | -: |
| **Total** | 0 | 0 |0 | 0 (-9223372036854775808%) |




## Every struct
The "Savings" columns are measured in bytes. The percentage in the "Savings per" column indicates by how much the struct itself has been made smaller thanks to bitpacking. The percentage in the "Savings total" column indicates by how much the total size of all serialized data has been made smaller as a result of bitpacking that row's specific struct type.

| Name | Bytes in RAM | Packed bits | Packed bytes | Savings per | Count | Savings total |
| - | -: | -: | -: | -: | -: | -: |



## Every heritable data definition

| Name | Total uses |
| - | -: |
| $player-name | 0 |
| $game-language | 0 |
| $item-quantity | 0 |
| $global-item-id | 0 |
| $pokemon-move-id | 0 |
| $pokemon-nickname | 0 |
| $pokemon-species-id | 0 |
| $easy-chat-word | 0 |

