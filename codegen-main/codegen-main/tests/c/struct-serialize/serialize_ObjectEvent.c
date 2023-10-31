#include "struct-serialize//serialize_ObjectEvent.h"

#include "global.fieldmap.h" // struct definition

// dependencies
#include "./serialize_Coords16.h"
#include "./serialize_Coords16.h"
#include "./serialize_Coords16.h"

// TODO:
// void lu_BitstreamRead_ObjectEvent(struct lu_BitstreamState* state, struct ObjectEvent* dst);

void lu_BitstreamWrite_ObjectEvent(struct lu_BitstreamState* state, const struct ObjectEvent* src) {
   lu_BitstreamWrite_bool(state, src.active);
   lu_BitstreamWrite_bool(state, src.singleMovementActive);
   lu_BitstreamWrite_bool(state, src.triggerGroundEffectsOnMove);
   lu_BitstreamWrite_bool(state, src.triggerGroundEffectsOnStop);
   lu_BitstreamWrite_bool(state, src.disableCoveringGroundEffects);
   lu_BitstreamWrite_bool(state, src.landingJump);
   lu_BitstreamWrite_bool(state, src.heldMovementActive);
   lu_BitstreamWrite_bool(state, src.heldMovementFinished);
   lu_BitstreamWrite_bool(state, src.frozen);
   lu_BitstreamWrite_bool(state, src.facingDirectionLocked);
   lu_BitstreamWrite_bool(state, src.disableAnim);
   lu_BitstreamWrite_bool(state, src.enableAnim);
   lu_BitstreamWrite_bool(state, src.inanimate);
   lu_BitstreamWrite_bool(state, src.invisible);
   lu_BitstreamWrite_bool(state, src.offScreen);
   lu_BitstreamWrite_bool(state, src.trackedByCamera);
   lu_BitstreamWrite_bool(state, src.isPlayer);
   lu_BitstreamWrite_bool(state, src.hasReflection);
   lu_BitstreamWrite_bool(state, src.inShortGrass);
   lu_BitstreamWrite_bool(state, src.inShallowFlowingWater);
   lu_BitstreamWrite_bool(state, src.inSandPile);
   lu_BitstreamWrite_bool(state, src.inHotSprings);
   lu_BitstreamWrite_bool(state, src.hasShadow);
   lu_BitstreamWrite_bool(state, src.spriteAnimPausedBackup);
   lu_BitstreamWrite_bool(state, src.spriteAffineAnimPausedBackup);
   lu_BitstreamWrite_bool(state, src.disableJumpLandingGroundEffect);
   lu_BitstreamWrite_bool(state, src.fixedPriority);
   lu_BitstreamWrite_bool(state, src.hideReflection);
   lu_BitstreamWrite_u8(state, src.spriteId, 8);
   lu_BitstreamWrite_u8(state, src.graphicsId, 8);
   lu_BitstreamWrite_u8(state, src.movementType, 8);
   lu_BitstreamWrite_u8(state, src.trainerType, 8);
   lu_BitstreamWrite_u8(state, src.localId, 8);
   lu_BitstreamWrite_u8(state, src.mapNum, 8);
   lu_BitstreamWrite_u8(state, src.mapGroup, 8);
   lu_BitstreamWrite_u8(state, src.currentElevation, 4);
   lu_BitstreamWrite_u8(state, src.previousElevation, 4);
   lu_BitstreamWrite_Coords16(state, &src.initialCoords);
   lu_BitstreamWrite_Coords16(state, &src.currentCoords);
   lu_BitstreamWrite_Coords16(state, &src.previousCoords);
   lu_BitstreamWrite_u8(state, src.facingDirection, 4);
   lu_BitstreamWrite_u8(state, src.movementDirection, 4);
   lu_BitstreamWrite_u8(state, src.rangeX, 4);
   lu_BitstreamWrite_u8(state, src.rangeY, 4);
   lu_BitstreamWrite_u8(state, src.fieldEffectSpriteId, 8);
   lu_BitstreamWrite_u8(state, src.warpArrowSpriteId, 8);
   lu_BitstreamWrite_u8(state, src.movementActionId, 8);
   lu_BitstreamWrite_u8(state, src.trainerRange_berryTreeId, 8);
   lu_BitstreamWrite_u8(state, src.currentMetatileBehavior, 8);
   lu_BitstreamWrite_u8(state, src.previousMetatileBehavior, 8);
   lu_BitstreamWrite_u8(state, src.previousMovementDirection, 8);
   lu_BitstreamWrite_u8(state, src.directionSequenceIndex, 8);
   lu_BitstreamWrite_u8(state, src.playerCopyableMovement, 8);
}