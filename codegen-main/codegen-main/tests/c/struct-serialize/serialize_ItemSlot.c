#include "struct-serialize//serialize_ItemSlot.h"

#include "global.h" // struct definition

// TODO:
// void lu_BitstreamRead_ItemSlot(struct lu_BitstreamState* state, struct ItemSlot* dst);

void lu_BitstreamWrite_ItemSlot(struct lu_BitstreamState* state, const struct ItemSlot* src) {
   lu_BitstreamWrite_u16(state, src.itemId, 16);
   lu_BitstreamWrite_u8(state, src.quantity, 7);
}