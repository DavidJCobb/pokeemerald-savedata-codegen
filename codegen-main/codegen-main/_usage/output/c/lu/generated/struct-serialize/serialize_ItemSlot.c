#include "lu/generated/struct-serialize//serialize_ItemSlot.h"

#include "global.h" // struct definition

void lu_BitstreamRead_ItemSlot(struct lu_BitstreamState* state, const struct ItemSlot* src) {
   src.itemId = lu_BitstreamRead_u16(state, 9);
   src.quantity = lu_BitstreamRead_u8(state, 7);
}
void lu_BitstreamWrite_ItemSlot(struct lu_BitstreamState* state, const struct ItemSlot* src) {
   lu_BitstreamWrite_u16(state, src.itemId, 9);
   lu_BitstreamWrite_u8(state, src.quantity, 7);
}