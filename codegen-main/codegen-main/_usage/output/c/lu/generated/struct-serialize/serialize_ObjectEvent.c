#include "lu/generated/struct-serialize//serialize_ObjectEvent.h"

#include "global.fieldmap.h" // struct definition

// dependencies
#include "./serialize_Coords16.h"
#include "./serialize_Coords16.h"
#include "./serialize_Coords16.h"

void lu_BitstreamRead_ObjectEvent(struct lu_BitstreamState* state, const struct ObjectEvent* src) {
   src.active = lu_BitstreamRead_bool(state, src.active);
   src.singleMovementActive = lu_BitstreamRead_bool(state, src.singleMovementActive);
   src.triggerGroundEffectsOnMove = lu_BitstreamRead_bool(state, src.triggerGroundEffectsOnMove);
   src.triggerGroundEffectsOnStop = lu_BitstreamRead_bool(state, src.triggerGroundEffectsOnStop);
   src.disableCoveringGroundEffects = lu_BitstreamRead_bool(state, src.disableCoveringGroundEffects);
   src.landingJump = lu_BitstreamRead_bool(state, src.landingJump);
   src.heldMovementActive = lu_BitstreamRead_bool(state, src.heldMovementActive);
   src.heldMovementFinished = lu_BitstreamRead_bool(state, src.heldMovementFinished);
   src.frozen = lu_BitstreamRead_bool(state, src.frozen);
   src.facingDirectionLocked = lu_BitstreamRead_bool(state, src.facingDirectionLocked);
   src.disableAnim = lu_BitstreamRead_bool(state, src.disableAnim);
   src.enableAnim = lu_BitstreamRead_bool(state, src.enableAnim);
   src.inanimate = lu_BitstreamRead_bool(state, src.inanimate);
   src.invisible = lu_BitstreamRead_bool(state, src.invisible);
   src.offScreen = lu_BitstreamRead_bool(state, src.offScreen);
   src.trackedByCamera = lu_BitstreamRead_bool(state, src.trackedByCamera);
   src.isPlayer = lu_BitstreamRead_bool(state, src.isPlayer);
   src.hasReflection = lu_BitstreamRead_bool(state, src.hasReflection);
   src.inShortGrass = lu_BitstreamRead_bool(state, src.inShortGrass);
   src.inShallowFlowingWater = lu_BitstreamRead_bool(state, src.inShallowFlowingWater);
   src.inSandPile = lu_BitstreamRead_bool(state, src.inSandPile);
   src.inHotSprings = lu_BitstreamRead_bool(state, src.inHotSprings);
   src.hasShadow = lu_BitstreamRead_bool(state, src.hasShadow);
   src.spriteAnimPausedBackup = lu_BitstreamRead_bool(state, src.spriteAnimPausedBackup);
   src.spriteAffineAnimPausedBackup = lu_BitstreamRead_bool(state, src.spriteAffineAnimPausedBackup);
   src.disableJumpLandingGroundEffect = lu_BitstreamRead_bool(state, src.disableJumpLandingGroundEffect);
   src.fixedPriority = lu_BitstreamRead_bool(state, src.fixedPriority);
   src.hideReflection = lu_BitstreamRead_bool(state, src.hideReflection);
   src.spriteId = lu_BitstreamRead_u8(state, 8);
   src.graphicsId = lu_BitstreamRead_u8(state, 8);
   src.movementType = lu_BitstreamRead_u8(state, 8);
   src.trainerType = lu_BitstreamRead_u8(state, 8);
   src.localId = lu_BitstreamRead_u8(state, 8);
   src.mapNum = lu_BitstreamRead_u8(state, 8);
   src.mapGroup = lu_BitstreamRead_u8(state, 8);
   src.currentElevation = lu_BitstreamRead_u8(state, 4);
   src.previousElevation = lu_BitstreamRead_u8(state, 4);
   lu_BitstreamRead_Coords16(state, &src.initialCoords);
   lu_BitstreamRead_Coords16(state, &src.currentCoords);
   lu_BitstreamRead_Coords16(state, &src.previousCoords);
   src.facingDirection = lu_BitstreamRead_u8(state, 4);
   src.movementDirection = lu_BitstreamRead_u8(state, 4);
   src.rangeX = lu_BitstreamRead_u8(state, 4);
   src.rangeY = lu_BitstreamRead_u8(state, 4);
   src.fieldEffectSpriteId = lu_BitstreamRead_u8(state, 8);
   src.warpArrowSpriteId = lu_BitstreamRead_u8(state, 8);
   src.movementActionId = lu_BitstreamRead_u8(state, 8);
   src.trainerRange_berryTreeId = lu_BitstreamRead_u8(state, 8);
   src.currentMetatileBehavior = lu_BitstreamRead_u8(state, 8);
   src.previousMetatileBehavior = lu_BitstreamRead_u8(state, 8);
   src.previousMovementDirection = lu_BitstreamRead_u8(state, 8);
   src.directionSequenceIndex = lu_BitstreamRead_u8(state, 8);
   src.playerCopyableMovement = lu_BitstreamRead_u8(state, 8);
}
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