#include "CharacterMovementComponentAsync.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"
#include "Components/PrimitiveComponent.h"
#include "PBDRigidsSolver.h"
#include "Engine/World.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(CharacterMovementComponentAsync)
void FCharacterMovementComponentAsyncInput::Simulate(const float DeltaSeconds, FCharacterMovementComponentAsyncOutput& Output) const
{
Output.DeltaTime = DeltaSeconds;
if (CharacterInput->LocalRole > ROLE_SimulatedProxy)
{
const bool bIsClient = (CharacterInput->LocalRole == ROLE_AutonomousProxy && bIsNetModeClient);
if (CharacterInput->bIsLocallyControlled)
{
ControlledCharacterMove(DeltaSeconds, Output);
}
}
else if (CharacterInput->LocalRole == ROLE_SimulatedProxy)
{
ensure(false);
}
}
void FCharacterMovementComponentAsyncInput::ControlledCharacterMove(const float DeltaSeconds, FCharacterMovementComponentAsyncOutput& Output) const
{
{
// We need to check the jump state before adjusting input acceleration, to minimize latency
// and to make sure acceleration respects our potentially new falling state.
CharacterInput->CheckJumpInput(DeltaSeconds, *this, Output);
// apply input to acceleration
Output.Acceleration = ScaleInputAcceleration(ConstrainInputAcceleration(InputVector, Output), Output);
Output.AnalogInputModifier = ComputeAnalogInputModifier(Output.Acceleration);
}
{
PerformMovement(DeltaSeconds, Output);
}
}
void FCharacterMovementComponentAsyncInput::PerformMovement(float DeltaSeconds, FCharacterMovementComponentAsyncOutput& Output) const
{
EMovementMode& MovementMode = Output.MovementMode;
FVector& LastUpdateLocation = Output.LastUpdateLocation;
const FVector UpdatedComponentLocation = UpdatedComponentInput->GetPosition();
bool& bForceNextFloorCheck = Output.bForceNextFloorCheck;
const FVector& Velocity = Output.Velocity;
const FVector& LastUpdateVelocity = Output.LastUpdateVelocity;
// Force floor update if we've moved outside of CharacterMovement since last update.
bForceNextFloorCheck |= (IsMovingOnGround(Output) && UpdatedComponentLocation != LastUpdateLocation);
// Update saved LastPreAdditiveVelocity with any external changes to character Velocity that happened since last update.
if (RootMotion.bHasAdditiveRootMotion)
{
FVector Adjustment = (Velocity - LastUpdateVelocity);
Output.LastPreAdditiveVelocity += Adjustment;
}
{
MaybeUpdateBasedMovement(DeltaSeconds, Output);
const bool bHasRootMotionSources = Output.bWasSimulatingRootMotion;
Output.OldVelocity = Velocity;
Output.OldLocation = UpdatedComponentLocation;
ApplyAccumulatedForces(DeltaSeconds, Output);
ClearAccumulatedForces(Output);
// Update saved LastPreAdditiveVelocity with any external changes to character Velocity that happened due to ApplyAccumulatedForces/HandlePendingLaunch
if (RootMotion.bHasAdditiveRootMotion)
{
const FVector Adjustment = (Velocity - Output.OldVelocity);
Output.LastPreAdditiveVelocity += Adjustment;
}
// Apply Root Motion to Velocity
if (RootMotion.bHasOverrideRootMotion || RootMotion.bHasAnimRootMotion)
{
// Animation root motion overrides Velocity and currently doesn't allow any other root motion sources
if (RootMotion.bHasAnimRootMotion)
{
// Turn root motion to velocity to be used by various physics modes.
if (RootMotion.TimeAccumulated > 0.f)
{
Output.AnimRootMotionVelocity = CalcAnimRootMotionVelocity(RootMotion.AnimTransform.GetTranslation(), RootMotion.TimeAccumulated, Velocity);
Output.Velocity = ConstrainAnimRootMotionVelocity(Output.AnimRootMotionVelocity, Output.Velocity, Output);
}
}
else
{
// We don't have animation root motion so we apply other sources
if (DeltaSeconds > 0.f)
{
Output.Velocity = RootMotion.OverrideVelocity;
}
}
}
// Clear jump input now, to allow movement events to trigger it for next update.
CharacterInput->ClearJumpInput(DeltaSeconds, *this, Output);
Output.NumJumpApexAttempts = 0;
StartNewPhysics(DeltaSeconds, 0, Output);
if (!bHasValidData)
{
return;
}
UpdateCharacterStateAfterMovement(DeltaSeconds, Output);
if ((bAllowPhysicsRotationDuringAnimRootMotion || !RootMotion.bHasAnimRootMotion))
{
PhysicsRotation(DeltaSeconds, Output);
}
// Apply Root Motion rotation after movement is complete.
if (RootMotion.bHasAnimRootMotion)
{
const FQuat OldActorRotationQuat = UpdatedComponentInput->GetRotation();
const FQuat RootMotionRotationQuat = RootMotion.AnimTransform.GetRotation();
if (!RootMotionRotationQuat.IsIdentity())
{
const FQuat NewActorRotationQuat = RootMotionRotationQuat * OldActorRotationQuat;
MoveUpdatedComponent(FVector::ZeroVector, NewActorRotationQuat, true, Output);
}
}
else if (RootMotion.bHasOverrideRootMotion)
{
if (UpdatedComponentInput && !RootMotion.OverrideRotation.IsIdentity())
{
const FQuat OldActorRotationQuat = UpdatedComponentInput->GetRotation();
const FQuat NewActorRotationQuat = RootMotion.OverrideRotation * OldActorRotationQuat;
MoveUpdatedComponent(FVector::ZeroVector, NewActorRotationQuat, true, Output);
}
}
// consume path following requested velocity
Output.LastUpdateRequestedVelocity = Output.bHasRequestedVelocity ? Output.RequestedVelocity : FVector::ZeroVector;
Output.bHasRequestedVelocity = false;
} // End scoped movement update
const FVector NewLocation = UpdatedComponentInput->GetPosition();
const FQuat NewRotation = UpdatedComponentInput->GetRotation();
Output.LastUpdateLocation = NewLocation;
Output.LastUpdateRotation = NewRotation;
Output.LastUpdateVelocity = Velocity;
}
void FCharacterMovementComponentAsyncInput::MaybeUpdateBasedMovement(float DeltaSeconds, FCharacterMovementComponentAsyncOutput& Output) const
{
bool& bDeferUpdateBasedMovement = Output.bDeferUpdateBasedMovement;
MovementBaseAsyncData.Validate(Output);
const bool& bMovementBaseUsesRelativeLocation = MovementBaseAsyncData.bMovementBaseUsesRelativeLocationCached;
const bool& bMovementBaseIsSimulated = MovementBaseAsyncData.bMovementBaseIsSimulatedCached;
bDeferUpdateBasedMovement = false;
if (bMovementBaseUsesRelativeLocation) 
{
// Need to see if anything we're on is simulating physics or has a parent that is.
if (bMovementBaseIsSimulated == false)
{
bDeferUpdateBasedMovement = false;
UpdateBasedMovement(DeltaSeconds, Output);
Output.bShouldDisablePostPhysicsTick = true;
Output.bShouldAddMovementBaseTickDependency = true;
}
else
{
// defer movement base update until after physics
bDeferUpdateBasedMovement = true;
Output.bShouldEnablePostPhysicsTick = true;
Output.bShouldRemoveMovementBaseTickDependency = true;
}
}
else
{
Output.bShouldEnablePostPhysicsTick = true;
}
}
void FCharacterMovementComponentAsyncInput::UpdateBasedMovement(float DeltaSeconds, FCharacterMovementComponentAsyncOutput& Output) const
{
MovementBaseAsyncData.Validate(Output);
const bool& bMovementBaseUsesRelativeLocation = MovementBaseAsyncData.bMovementBaseUsesRelativeLocationCached;
const bool& bIsMovementBaseValid = MovementBaseAsyncData.bMovementBaseIsValidCached;
const bool& bIsMovementBaseOwnerValid = MovementBaseAsyncData.bMovementBaseOwnerIsValidCached;
EMoveComponentFlags& MoveComponentFlags = Output.MoveComponentFlags;
const FQuat& OldBaseQuat = MovementBaseAsyncData.OldBaseQuat;
const FVector& OldBaseLocation = MovementBaseAsyncData.OldBaseLocation;
if (bMovementBaseUsesRelativeLocation == false)
{
return;
}
if (!bIsMovementBaseValid || !bIsMovementBaseOwnerValid)
{
Output.NewMovementBase = nullptr; 
Output.NewMovementBaseOwner = nullptr;
return;
}
// Ignore collision with bases during these movements.
TGuardValue<EMoveComponentFlags> ScopedFlagRestore(MoveComponentFlags, MoveComponentFlags | MOVECOMP_IgnoreBases);
Output.DeltaQuat = FQuat::Identity;
Output.DeltaPosition = FVector::ZeroVector;
if (!MovementBaseAsyncData.bIsBaseTransformValid)
{
return;
}
FQuat NewBaseQuat = MovementBaseAsyncData.BaseQuat;
FVector NewBaseLocation = MovementBaseAsyncData.BaseLocation;
// Find change in rotation
const bool bRotationChanged = !OldBaseQuat.Equals(NewBaseQuat, 1e-8f);
if (bRotationChanged)
{
Output.DeltaQuat = NewBaseQuat * OldBaseQuat.Inverse();
}
// only if base moved
if (bRotationChanged || (OldBaseLocation != NewBaseLocation))
{
// Calculate new transform matrix of base actor (ignoring scale).
const FQuatRotationTranslationMatrix OldLocalToWorld(OldBaseQuat, OldBaseLocation);
const FQuatRotationTranslationMatrix NewLocalToWorld(NewBaseQuat, NewBaseLocation);
FQuat FinalQuat = UpdatedComponentInput->GetRotation();
if (bRotationChanged && !bIgnoreBaseRotation)
{
// Apply change in rotation and pipe through FaceRotation to maintain axis restrictions
const FQuat PawnOldQuat = UpdatedComponentInput->GetRotation();
const FQuat TargetQuat = Output.DeltaQuat * FinalQuat;
FRotator TargetRotator(TargetQuat);
CharacterInput->FaceRotation(TargetRotator, 0.0f, *this, Output);
FinalQuat =  Output.CharacterOutput->Rotation.Quaternion();
if (PawnOldQuat.Equals(FinalQuat, 1e-6f))
{
// Nothing changed. This means we probably are using another rotation mechanism (bOrientToMovement etc). We should still follow the base object.
// @todo: This assumes only Yaw is used, currently a valid assumption. This is the only reason FaceRotation() is used above really, aside from being a virtual hook.
if (bOrientRotationToMovement || (bUseControllerDesiredRotation /*&& CharacterOwner->Controller*/))
{
TargetRotator.Pitch = 0.f;
TargetRotator.Roll = 0.f;
MoveUpdatedComponent(FVector::ZeroVector, FQuat(TargetRotator), false, Output);
FinalQuat = UpdatedComponentInput->GetRotation();
}
}
}
// We need to offset the base of the character here, not its origin, so offset by half height
float HalfHeight = Output.ScaledCapsuleHalfHeight;
float Radius = Output.ScaledCapsuleRadius;
FVector const BaseOffset(0.0f, 0.0f, HalfHeight);
FVector const LocalBasePos = OldLocalToWorld.InverseTransformPosition(UpdatedComponentInput->GetPosition() - BaseOffset);
FVector const NewWorldPos = ConstrainLocationToPlane(NewLocalToWorld.TransformPosition(LocalBasePos) + BaseOffset);
Output.DeltaPosition = ConstrainDirectionToPlane(NewWorldPos - UpdatedComponentInput->GetPosition());
// move attached actor
if (false)
{
UpdatedComponentInput->SetPosition(NewWorldPos);
UpdatedComponentInput->SetRotation(FinalQuat);
}
else
{
// hack - transforms between local and world space introducing slight error FIXMESTEVE - discuss with engine team: just skip the transforms if no rotation?
FVector BaseMoveDelta = NewBaseLocation - OldBaseLocation;
if (!bRotationChanged && (BaseMoveDelta.X == 0.f) && (BaseMoveDelta.Y == 0.f))
{
Output.DeltaPosition.X = 0.f;
Output.DeltaPosition.Y = 0.f;
}
FHitResult MoveOnBaseHit(1.f);
const FVector OldLocation = UpdatedComponentInput->GetPosition();
MoveUpdatedComponent(Output.DeltaPosition, FinalQuat, true, Output, &MoveOnBaseHit);
if ((UpdatedComponentInput->GetPosition() - (OldLocation + Output.DeltaPosition)).IsNearlyZero() == false)
{
}
}
MovementBaseAsyncData.Validate(Output); // ensure we haven't changed movement base
if (MovementBaseAsyncData.bMovementBaseIsSimulatedCached )
{
// If we hit this multiple times, our DeltaPostion/DeltaQuat is being stomped. Do we need to call for each, or just latest?
ensure(Output.bShouldApplyDeltaToMeshPhysicsTransforms == false);
Output.bShouldApplyDeltaToMeshPhysicsTransforms = true;
}
}
}
void FCharacterMovementComponentAsyncInput::StartNewPhysics(float deltaTime, int32 Iterations, FCharacterMovementComponentAsyncOutput& Output) const
{
if ((deltaTime < UCharacterMovementComponent::MIN_TICK_TIME) || (Iterations >= MaxSimulationIterations) || !bHasValidData)
{
return;
}
if (UpdatedComponentInput->bIsSimulatingPhysics)
{
return;
}
const bool bSavedMovementInProgress = Output.bMovementInProgress;
Output.bMovementInProgress = true;
switch (Output.MovementMode)
{
case MOVE_None:
break;
case MOVE_Walking:
PhysWalking(deltaTime, Iterations, Output);
break;
case MOVE_Falling:
PhysFalling(deltaTime, Iterations, Output);
break;
default:
SetMovementMode(MOVE_None, Output);
break;
}
Output.bMovementInProgress = bSavedMovementInProgress;
if (bDeferUpdateMoveComponent)
{
ensure(false);
}
}
void FCharacterMovementComponentAsyncInput::PhysWalking(float deltaTime, int32 Iterations, FCharacterMovementComponentAsyncOutput& Output) const
{
const FCharacterMovementComponentAsyncInput& Input = *this;
if (deltaTime < UCharacterMovementComponent::MIN_TICK_TIME)
{
return;
}
FVector& Velocity = Output.Velocity;
FVector& Acceleration = Output.Acceleration;
if (false)
{
Acceleration = FVector::ZeroVector;
Velocity = FVector::ZeroVector;
return;
}
if (!UpdatedComponentInput->bIsQueryCollisionEnabled)
{
SetMovementMode(MOVE_Walking, Output);
return;
}
Output.bJustTeleported = false;
bool bCheckedFall = false;
bool bTriedLedgeMove = false;
float remainingTime = deltaTime;
// Perform the move
while ((remainingTime >= UCharacterMovementComponent::MIN_TICK_TIME) && (Iterations < MaxSimulationIterations)  && ( bRunPhysicsWithNoController
|| RootMotion.bHasAnimRootMotion || RootMotion.bHasOverrideRootMotion || (true)))
{
Iterations++;
Output.bJustTeleported = false;
const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
remainingTime -= timeTick;
// Save current values
MovementBaseAsyncData.Validate(Output);
UPrimitiveComponent* const OldBase = MovementBaseAsyncData.CachedMovementBase;
const FVector PreviousBaseLocation = MovementBaseAsyncData.BaseLocation;
const FVector OldLocation = UpdatedComponentInput->GetPosition();
const FFindFloorResult OldFloor = Output.CurrentFloor;
RestorePreAdditiveRootMotionVelocity(Output);
// Ensure velocity is horizontal.
MaintainHorizontalGroundVelocity(Output);
const FVector OldVelocity = Velocity;
Acceleration.Z = 0.f;
// Apply acceleration
if (!RootMotion.bHasAnimRootMotion && !RootMotion.bHasOverrideRootMotion)
{
CalcVelocity(timeTick, GroundFriction, false, GetMaxBrakingDeceleration(Output), Output);
}
ApplyRootMotionToVelocity(timeTick, Output);
if (IsFalling(Output))
{
StartNewPhysics(remainingTime + timeTick, Iterations - 1, Output);
return;
}
// Compute move parameters
const FVector MoveVelocity = Velocity;
const FVector Delta = timeTick * MoveVelocity;
const bool bZeroDelta = Delta.IsNearlyZero();
FStepDownResult StepDownResult;
if (bZeroDelta)
{
remainingTime = 0.f;
}
else
{
// try to move forward
MoveAlongFloor(MoveVelocity, timeTick, &StepDownResult, Output);
if (IsFalling(Output))
{
// pawn decided to jump up
const float DesiredDist = Delta.Size();
if (DesiredDist > UE_KINDA_SMALL_NUMBER)
{
const float ActualDist = (UpdatedComponentInput->GetPosition() - OldLocation).Size2D();
remainingTime += timeTick * (1.f - FMath::Min(1.f, ActualDist / DesiredDist));
}
StartNewPhysics(remainingTime, Iterations, Output);
return;
}
}
// Update floor.
// StepUp might have already done it for us.
if (StepDownResult.bComputedFloor)
{
Output.CurrentFloor = StepDownResult.FloorResult;
}
else
{
FindFloor(UpdatedComponentInput->GetPosition(), Output.CurrentFloor, bZeroDelta, Output);
}
// check for ledges here
const bool bCheckLedges = !CanWalkOffLedges(Output);
if (bCheckLedges && !Output.CurrentFloor.IsWalkableFloor())
{
// calculate possible alternate movement
const FVector GravDir = FVector(0.f, 0.f, -1.f);
const FVector NewDelta = bTriedLedgeMove ? FVector::ZeroVector : GetLedgeMove(OldLocation, Delta, GravDir, Output);
if (!NewDelta.IsZero())
{
// first revert this move
RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, false, Output);
// avoid repeated ledge moves if the first one fails
bTriedLedgeMove = true;
// Try new movement direction
Velocity = NewDelta / timeTick;
remainingTime += timeTick;
continue;
}
else
{
ensure(false); 
break;
}
}
else
{
// Validate the floor check
if (Output.CurrentFloor.IsWalkableFloor())
{
if (ShouldCatchAir(OldFloor, Output.CurrentFloor))
{
HandleWalkingOffLedge(OldFloor.HitResult.ImpactNormal, OldFloor.HitResult.Normal, OldLocation, timeTick);
if (IsMovingOnGround(Output))
{
// If still walking, then fall. If not, assume the user set a different mode they want to keep.
StartFalling(Iterations, remainingTime, timeTick, Delta, OldLocation, Output);
}
return;
}
AdjustFloorHeight(Output);
Output.NewMovementBase = Output.CurrentFloor.HitResult.Component.Get();
Output.NewMovementBaseOwner = Output.CurrentFloor.HitResult.GetActor();
}
else if (Output.CurrentFloor.HitResult.bStartPenetrating && remainingTime <= 0.f)
{
// The floor check failed because it started in penetration
// We do not want to try to move downward because the downward sweep failed, rather we'd like to try to pop out of the floor.
FHitResult Hit(Output.CurrentFloor.HitResult);
Hit.TraceEnd = Hit.TraceStart + FVector(0.f, 0.f, UCharacterMovementComponent::MAX_FLOOR_DIST);
const FVector RequestedAdjustment = GetPenetrationAdjustment(Hit);
ResolvePenetration(RequestedAdjustment, Hit, UpdatedComponentInput->GetRotation(), Output);
Output.bForceNextFloorCheck = true;
}
// See if we need to start falling.
if (!Output.CurrentFloor.IsWalkableFloor() && !Output.CurrentFloor.HitResult.bStartPenetrating)
{
const bool bMustJump = Output.bJustTeleported || bZeroDelta;// || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, Output.CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump, Output))
{
return;
}
bCheckedFall = true;
}
}
// Allow overlap events and such to change physics state and velocity
if (IsMovingOnGround(Output))
{
// Make velocity reflect actual move
if (!Output.bJustTeleported && !RootMotion.bHasAnimRootMotion && !RootMotion.bHasOverrideRootMotion && timeTick >= UCharacterMovementComponent::MIN_TICK_TIME)
{
Velocity = (UpdatedComponentInput->GetPosition() - OldLocation) / timeTick;
MaintainHorizontalGroundVelocity(Output);
}
}
// If we didn't move at all this iteration then abort (since future iterations will also be stuck).
if (UpdatedComponentInput->GetPosition() == OldLocation)
{
remainingTime = 0.f;
break;
}
}
if (IsMovingOnGround(Output))
{
MaintainHorizontalGroundVelocity(Output);
}
}
void FCharacterMovementComponentAsyncInput::PhysFalling(float deltaTime, int32 Iterations, FCharacterMovementComponentAsyncOutput& Output) const
{
const float MIN_TICK_TIME = UCharacterMovementComponent::MIN_TICK_TIME;
if (deltaTime < MIN_TICK_TIME)
{
return;
}
FVector& Velocity = Output.Velocity;
FVector FallAcceleration = GetFallingLateralAcceleration(deltaTime, Output);
FallAcceleration.Z = 0.f;
const bool bHasLimitedAirControl = ShouldLimitAirControl(deltaTime, FallAcceleration, Output);
float remainingTime = deltaTime;
while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations))
{
Iterations++;
float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
remainingTime -= timeTick;
const FVector OldLocation = UpdatedComponentInput->GetPosition();
const FQuat PawnRotation = UpdatedComponentInput->GetRotation();
Output.bJustTeleported = false;
RestorePreAdditiveRootMotionVelocity(Output);
const FVector OldVelocity = Velocity;
const float MaxDecel = GetMaxBrakingDeceleration(Output);
if (!RootMotion.bHasAnimRootMotion && !RootMotion.bHasOverrideRootMotion)
{
{
TGuardValue<FVector> RestoreAcceleration(Output.Acceleration, FallAcceleration);
Velocity.Z = 0.f;
CalcVelocity(timeTick, FallingLateralFriction, false, MaxDecel, Output);
Velocity.Z = OldVelocity.Z;
}
}
const FVector Gravity(0.f, 0.f, GravityZ);
float GravityTime = timeTick;
// If jump is providing force, gravity may be affected.
bool bEndingJumpForce = false;
if (Output.CharacterOutput->JumpForceTimeRemaining > 0.0f)
{
// Consume some of the force time. Only the remaining time (if any) is affected by gravity when bApplyGravityWhileJumping=false.
const float JumpForceTime = FMath::Min(Output.CharacterOutput->JumpForceTimeRemaining, timeTick);
GravityTime = bApplyGravityWhileJumping ? timeTick : FMath::Max(0.0f, timeTick - JumpForceTime);
Output.CharacterOutput->JumpForceTimeRemaining -= JumpForceTime;
if (Output.CharacterOutput->JumpForceTimeRemaining <= 0.0f)
{
CharacterInput->ResetJumpState(*this, Output);
bEndingJumpForce = true;
}
}
Velocity = NewFallVelocity(Velocity, Gravity, GravityTime, Output);
// See if we need to sub-step to exactly reach the apex. This is important for avoiding "cutting off the top" of the trajectory as framerate varies.
if (CharacterMovementCVars::ForceJumpPeakSubstep && OldVelocity.Z > 0.f && Velocity.Z <= 0.f && Output.NumJumpApexAttempts < MaxJumpApexAttemptsPerSimulation)
{
const FVector DerivedAccel = (Velocity - OldVelocity) / timeTick;
if (!FMath::IsNearlyZero(DerivedAccel.Z))
{
const float TimeToApex = -OldVelocity.Z / DerivedAccel.Z;
// The time-to-apex calculation should be precise, and we want to avoid adding a substep when we are basically already at the apex from the previous iteration's work.
const float ApexTimeMinimum = 0.0001f;
if (TimeToApex >= ApexTimeMinimum && TimeToApex < timeTick)
{
const FVector ApexVelocity = OldVelocity + DerivedAccel * TimeToApex;
Velocity = ApexVelocity;
Velocity.Z = 0.f; // Should be nearly zero anyway, but this makes apex notifications consistent.
// We only want to move the amount of time it takes to reach the apex, and refund the unused time for next iteration.
remainingTime += (timeTick - TimeToApex);
timeTick = TimeToApex;
Iterations--;
Output.NumJumpApexAttempts++;
}
}
}
// Compute change in position (using midpoint integration method).
FVector Adjusted = 0.5f * (OldVelocity + Velocity) * timeTick;
// Special handling if ending the jump force where we didn't apply gravity during the jump.
if (bEndingJumpForce && !bApplyGravityWhileJumping)
{
// We had a portion of the time at constant speed then a portion with acceleration due to gravity.
// Account for that here with a more correct change in position.
const float NonGravityTime = FMath::Max(0.f, timeTick - GravityTime);
Adjusted = (OldVelocity * NonGravityTime) + (0.5f * (OldVelocity + Velocity) * GravityTime);
}
FHitResult Hit(1.f);
SafeMoveUpdatedComponent(Adjusted, PawnRotation, true, Hit, Output);
if (!bHasValidData)
{
return;
}
float LastMoveTimeSlice = timeTick;
float subTimeTickRemaining = timeTick * (1.f - Hit.Time);
if (false)//IsSwimming()) //just entered water
{
}
else if (Hit.bBlockingHit)
{
if (IsValidLandingSpot(UpdatedComponentInput->GetPosition(), Hit, Output))
{
remainingTime += subTimeTickRemaining;
ProcessLanded(Hit, remainingTime, Iterations, Output);
return;
}
else
{
// Compute impact deflection based on final velocity, not integration step.
// This allows us to compute a new velocity from the deflected vector, and ensures the full gravity effect is included in the slide result.
Adjusted = Velocity * timeTick;
// See if we can convert a normally invalid landing spot (based on the hit result) to a usable one.
if (!Hit.bStartPenetrating && ShouldCheckForValidLandingSpot(timeTick, Adjusted, Hit, Output))
{
const FVector PawnLocation = UpdatedComponentInput->GetPosition();
FFindFloorResult FloorResult;
FindFloor(PawnLocation, FloorResult, false, Output);
if (FloorResult.IsWalkableFloor() && IsValidLandingSpot(PawnLocation, FloorResult.HitResult, Output))
{
remainingTime += subTimeTickRemaining;
ProcessLanded(FloorResult.HitResult, remainingTime, Iterations, Output);
return;
}
}
HandleImpact(Hit, Output, LastMoveTimeSlice, Adjusted);
if (!bHasValidData || !IsFalling(Output))
{
return;
}
// Limit air control based on what we hit.
// We moved to the impact point using air control, but may want to deflect from there based on a limited air control acceleration.
FVector VelocityNoAirControl = OldVelocity;
FVector AirControlAccel = Output.Acceleration;
if (bHasLimitedAirControl)
{
{
// Find velocity *without* acceleration.
TGuardValue<FVector> RestoreAcceleration(Output.Acceleration, FVector::ZeroVector);
TGuardValue<FVector> RestoreVelocity(Velocity, OldVelocity);
Velocity.Z = 0.f;
CalcVelocity(timeTick, FallingLateralFriction, false, MaxDecel, Output);
VelocityNoAirControl = FVector(Velocity.X, Velocity.Y, OldVelocity.Z);
VelocityNoAirControl = NewFallVelocity(VelocityNoAirControl, Gravity, GravityTime, Output);
}
const bool bCheckLandingSpot = false; // we already checked above.
AirControlAccel = (Velocity - VelocityNoAirControl) / timeTick;
const FVector AirControlDeltaV = LimitAirControl(LastMoveTimeSlice, AirControlAccel, Hit, bCheckLandingSpot, Output) * LastMoveTimeSlice;
Adjusted = (VelocityNoAirControl + AirControlDeltaV) * LastMoveTimeSlice;
}
const FVector OldHitNormal = Hit.Normal;
const FVector OldHitImpactNormal = Hit.ImpactNormal;
FVector Delta = ComputeSlideVector(Adjusted, 1.f - Hit.Time, OldHitNormal, Hit, Output);
// Compute velocity after deflection (only gravity component for RootMotion)
if (subTimeTickRemaining > UE_KINDA_SMALL_NUMBER && !Output.bJustTeleported)
{
const FVector NewVelocity = (Delta / subTimeTickRemaining);
Velocity = RootMotion.bHasAnimRootMotion || RootMotion.bHasOverrideWithIgnoreZAccumulate ? FVector(Velocity.X, Velocity.Y, NewVelocity.Z) : NewVelocity;
}
if (subTimeTickRemaining > UE_KINDA_SMALL_NUMBER && (Delta | Adjusted) > 0.f)
{
// Move in deflected direction.
SafeMoveUpdatedComponent(Delta, PawnRotation, true, Hit, Output);
if (Hit.bBlockingHit)
{
// hit second wall
LastMoveTimeSlice = subTimeTickRemaining;
subTimeTickRemaining = subTimeTickRemaining * (1.f - Hit.Time);
if (IsValidLandingSpot(UpdatedComponentInput->GetPosition(), Hit, Output))
{
remainingTime += subTimeTickRemaining;
ProcessLanded(Hit, remainingTime, Iterations, Output);
return;
}
HandleImpact(Hit, Output, LastMoveTimeSlice, Delta);
if (!bHasValidData || !IsFalling(Output))
{
return;
}
// Act as if there was no air control on the last move when computing new deflection.
if (bHasLimitedAirControl && Hit.Normal.Z > CharacterMovementConstants::VERTICAL_SLOPE_NORMAL_Z)
{
const FVector LastMoveNoAirControl = VelocityNoAirControl * LastMoveTimeSlice;
Delta = ComputeSlideVector(LastMoveNoAirControl, 1.f, OldHitNormal, Hit, Output);
}
FVector PreTwoWallDelta = Delta;
TwoWallAdjust(Delta, Hit, OldHitNormal, Output);
if (bHasLimitedAirControl)
{
const bool bCheckLandingSpot = false; // we already checked above.
const FVector AirControlDeltaV = LimitAirControl(subTimeTickRemaining, AirControlAccel, Hit, bCheckLandingSpot, Output) * subTimeTickRemaining;
// Only allow if not back in to first wall
if (FVector::DotProduct(AirControlDeltaV, OldHitNormal) > 0.f)
{
Delta += (AirControlDeltaV * subTimeTickRemaining);
}
}
// Compute velocity after deflection (only gravity component for RootMotion)
if (subTimeTickRemaining > UE_KINDA_SMALL_NUMBER && !Output.bJustTeleported)
{
const FVector NewVelocity = (Delta / subTimeTickRemaining);
Velocity = RootMotion.bHasAnimRootMotion || RootMotion.bHasOverrideWithIgnoreZAccumulate ? FVector(Velocity.X, Velocity.Y, NewVelocity.Z) : NewVelocity;
}
// bDitch=true means that pawn is straddling two slopes, neither of which it can stand on
bool bDitch = ((OldHitImpactNormal.Z > 0.f) && (Hit.ImpactNormal.Z > 0.f) && (FMath::Abs(Delta.Z) <= UE_KINDA_SMALL_NUMBER) && ((Hit.ImpactNormal | OldHitImpactNormal) < 0.f));
SafeMoveUpdatedComponent(Delta, PawnRotation, true, Hit, Output);
if (Hit.Time == 0.f)
{
// if we are stuck then try to side step
FVector SideDelta = (OldHitNormal + Hit.ImpactNormal).GetSafeNormal2D();
if (SideDelta.IsNearlyZero())
{
SideDelta = FVector(OldHitNormal.Y, -OldHitNormal.X, 0).GetSafeNormal();
}
SafeMoveUpdatedComponent(SideDelta, PawnRotation, true, Hit, Output);
}
if (bDitch || IsValidLandingSpot(UpdatedComponentInput->GetPosition(), Hit, Output) || Hit.Time == 0.f)
{
remainingTime = 0.f;
ProcessLanded(Hit, remainingTime, Iterations, Output);
return;
}
else if (GetPerchRadiusThreshold() > 0.f && Hit.Time == 1.f && OldHitImpactNormal.Z >= WalkableFloorZ)
{
// We might be in a virtual 'ditch' within our perch radius. This is rare.
const FVector PawnLocation = UpdatedComponentInput->GetPosition();
const float ZMovedDist = FMath::Abs(PawnLocation.Z - OldLocation.Z);
const float MovedDist2DSq = (PawnLocation - OldLocation).SizeSquared2D();
if (ZMovedDist <= 0.2f * timeTick && MovedDist2DSq <= 4.f * timeTick)
{
Velocity.X += 0.25f * GetMaxSpeed(Output) * (RandomStream.FRand() - 0.5f);
Velocity.Y += 0.25f * GetMaxSpeed(Output) * (RandomStream.FRand() - 0.5f);
Velocity.Z = FMath::Max<float>(JumpZVelocity * 0.25f, 1.f);
Delta = Velocity * timeTick;
SafeMoveUpdatedComponent(Delta, PawnRotation, true, Hit,  Output);
}
}
}
}
}
}
if (Velocity.SizeSquared2D() <= UE_KINDA_SMALL_NUMBER * 10.f)
{
Velocity.X = 0.f;
Velocity.Y = 0.f;
}
}
}
void FCharacterMovementComponentAsyncInput::PhysicsRotation(float DeltaTime, FCharacterMovementComponentAsyncOutput& Output) const
{
if (!(bOrientRotationToMovement || bUseControllerDesiredRotation))
{
return;
}
if (!bHasValidData)
{
return;
}
FRotator CurrentRotation = FRotator(UpdatedComponentInput->GetRotation());
CurrentRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): CurrentRotation"));
FRotator DeltaRot = Output.GetDeltaRotation(GetRotationRate(Output), DeltaTime);
DeltaRot.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): GetDeltaRotation"));
FRotator DesiredRotation = CurrentRotation;
if (bOrientRotationToMovement)
{
DesiredRotation = ComputeOrientToMovementRotation(CurrentRotation, DeltaTime, DeltaRot, Output);
}
else if ( bUseControllerDesiredRotation)
{
DesiredRotation = CharacterInput->ControllerDesiredRotation;
}
else
{
return;
}
if (ShouldRemainVertical(Output))
{
DesiredRotation.Pitch = 0.f;
DesiredRotation.Yaw = FRotator::NormalizeAxis(DesiredRotation.Yaw);
DesiredRotation.Roll = 0.f;
}
else
{
DesiredRotation.Normalize();
}
// Accumulate a desired new rotation.
const float AngleTolerance = 1e-3f;
if (!CurrentRotation.Equals(DesiredRotation, AngleTolerance))
{
// PITCH
if (!FMath::IsNearlyEqual(CurrentRotation.Pitch, DesiredRotation.Pitch, AngleTolerance))
{
DesiredRotation.Pitch = FMath::FixedTurn(CurrentRotation.Pitch, DesiredRotation.Pitch, DeltaRot.Pitch);
}
// YAW
if (!FMath::IsNearlyEqual(CurrentRotation.Yaw, DesiredRotation.Yaw, AngleTolerance))
{
DesiredRotation.Yaw = FMath::FixedTurn(CurrentRotation.Yaw, DesiredRotation.Yaw, DeltaRot.Yaw);
}
// ROLL
if (!FMath::IsNearlyEqual(CurrentRotation.Roll, DesiredRotation.Roll, AngleTolerance))
{
DesiredRotation.Roll = FMath::FixedTurn(CurrentRotation.Roll, DesiredRotation.Roll, DeltaRot.Roll);
}
// Set the new rotation.
DesiredRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): DesiredRotation"));
MoveUpdatedComponent(FVector::ZeroVector, DesiredRotation.Quaternion(), /*bSweep*/ false, Output);
}
}
void FCharacterMovementComponentAsyncInput::MoveAlongFloor(const FVector& InVelocity, float DeltaSeconds, FStepDownResult* OutStepDownResult, FCharacterMovementComponentAsyncOutput& Output) const
{
if (!Output.CurrentFloor.IsWalkableFloor())
{
return;
}
// Move along the current floor
const FVector Delta = FVector(InVelocity.X, InVelocity.Y, 0.f) * DeltaSeconds;
FHitResult Hit(1.f);
FVector RampVector = ComputeGroundMovementDelta(Delta, Output.CurrentFloor.HitResult, Output.CurrentFloor.bLineTrace, Output);
SafeMoveUpdatedComponent(RampVector, UpdatedComponentInput->GetRotation(), true, Hit, Output);
float LastMoveTimeSlice = DeltaSeconds;
if (Hit.bStartPenetrating)
{
// Allow this hit to be used as an impact we can deflect off, otherwise we do nothing the rest of the update and appear to hitch.
HandleImpact(Hit, Output);
SlideAlongSurface(Delta, 1.f, Hit.Normal, Hit, true, Output);
if (Hit.bStartPenetrating)
{
OnCharacterStuckInGeometry(&Hit, Output);
}
}
else if (Hit.IsValidBlockingHit())
{
// We impacted something (most likely another ramp, but possibly a barrier).
float PercentTimeApplied = Hit.Time;
if ((Hit.Time > 0.f) && (Hit.Normal.Z > UE_KINDA_SMALL_NUMBER) && IsWalkable(Hit))
{
const float InitialPercentRemaining = 1.f - PercentTimeApplied;
RampVector = ComputeGroundMovementDelta(Delta * InitialPercentRemaining, Hit, false, Output);
LastMoveTimeSlice = InitialPercentRemaining * LastMoveTimeSlice;
SafeMoveUpdatedComponent(RampVector, UpdatedComponentInput->GetRotation(), true, Hit, Output);
const float SecondHitPercent = Hit.Time * InitialPercentRemaining;
PercentTimeApplied = FMath::Clamp(PercentTimeApplied + SecondHitPercent, 0.f, 1.f);
}
if (Hit.IsValidBlockingHit())
{
if (CanStepUp(Hit, Output) || (Output.NewMovementBase != NULL && Output.NewMovementBaseOwner == Hit.GetActor()))
{
// hit a barrier, try to step up
const FVector PreStepUpLocation = UpdatedComponentInput->GetPosition();
const FVector GravDir(0.f, 0.f, -1.f);
if (!StepUp(GravDir, Delta * (1.f - PercentTimeApplied), Hit, Output, OutStepDownResult))
{
HandleImpact(Hit, Output, LastMoveTimeSlice, RampVector);
SlideAlongSurface(Delta, 1.f - PercentTimeApplied, Hit.Normal, Hit, true, Output);
}
else
{
if (!bMaintainHorizontalGroundVelocity)
{
// Don't recalculate velocity based on this height adjustment, if considering vertical adjustments. Only consider horizontal movement.
Output.bJustTeleported = true;
const float StepUpTimeSlice = (1.f - PercentTimeApplied) * DeltaSeconds;
if (!RootMotion.bHasAnimRootMotion && !RootMotion.bHasOverrideRootMotion && StepUpTimeSlice >= UE_KINDA_SMALL_NUMBER)
{
Output.Velocity = (UpdatedComponentInput->GetPosition() - PreStepUpLocation) / StepUpTimeSlice;
Output.Velocity.Z = 0;
}
}
}
}
}
}
}
FVector FCharacterMovementComponentAsyncInput::ComputeGroundMovementDelta(const FVector& Delta, const FHitResult& RampHit, const bool bHitFromLineTrace, FCharacterMovementComponentAsyncOutput& Output) const
{
const FVector FloorNormal = RampHit.ImpactNormal;
const FVector ContactNormal = RampHit.Normal;
if (FloorNormal.Z < (1.f - UE_KINDA_SMALL_NUMBER) && FloorNormal.Z > UE_KINDA_SMALL_NUMBER && ContactNormal.Z > UE_KINDA_SMALL_NUMBER && !bHitFromLineTrace && IsWalkable(RampHit))
{
// Compute a vector that moves parallel to the surface, by projecting the horizontal movement direction onto the ramp.
const float FloorDotDelta = (FloorNormal | Delta);
FVector RampMovement(Delta.X, Delta.Y, -FloorDotDelta / FloorNormal.Z);
if (bMaintainHorizontalGroundVelocity)
{
return RampMovement;
}
else
{
return RampMovement.GetSafeNormal() * Delta.Size();
}
}
return Delta;
}
bool FCharacterMovementComponentAsyncInput::CanCrouchInCurrentState(FCharacterMovementComponentAsyncOutput& Output) const
{
if (!bCanEverCrouch)
{
return false;
}
return (IsFalling(Output) || IsMovingOnGround(Output)) && UpdatedComponentInput->bIsSimulatingPhysics;
}
FVector FCharacterMovementComponentAsyncInput::ConstrainInputAcceleration(FVector InputAcceleration, const FCharacterMovementComponentAsyncOutput& Output) const
{
// walking or falling pawns ignore up/down sliding
if (InputAcceleration.Z != 0.f && (IsMovingOnGround(Output) || IsFalling(Output)))
{
return FVector(InputAcceleration.X, InputAcceleration.Y, 0.f);
}
return InputAcceleration;
}
FVector FCharacterMovementComponentAsyncInput::ScaleInputAcceleration(FVector InputAcceleration, FCharacterMovementComponentAsyncOutput& Output) const
{
return MaxAcceleration * InputAcceleration.GetClampedToMaxSize(1.0f);
}
float FCharacterMovementComponentAsyncInput::ComputeAnalogInputModifier(FVector Acceleration) const
{
const float MaxAccel = MaxAcceleration;
if (Acceleration.SizeSquared() > 0.f && MaxAccel > UE_SMALL_NUMBER)
{
return FMath::Clamp(Acceleration.Size() / MaxAccel, 0.f, 1.f);
}
return 0.f;
}
FVector FCharacterMovementComponentAsyncInput::ConstrainDirectionToPlane(FVector Direction) const
{
if (bConstrainToPlane)
{
Direction = FVector::VectorPlaneProject(Direction, PlaneConstraintNormal);
}
return Direction;
}
FVector FCharacterMovementComponentAsyncInput::ConstrainNormalToPlane(FVector Normal) const
{
if (bConstrainToPlane)
{
Normal = FVector::VectorPlaneProject(Normal, PlaneConstraintNormal).GetSafeNormal();
}
return Normal;
}
FVector FCharacterMovementComponentAsyncInput::ConstrainLocationToPlane(FVector Location) const
{
if (bConstrainToPlane)
{
Location = FVector::PointPlaneProject(Location, PlaneConstraintOrigin, PlaneConstraintNormal);
}
return Location;
}
void FCharacterMovementComponentAsyncInput::MaintainHorizontalGroundVelocity(FCharacterMovementComponentAsyncOutput& Output) const
{
if (Output.Velocity.Z != 0.f)
{
if (bMaintainHorizontalGroundVelocity)
{
// Ramp movement already maintained the velocity, so we just want to remove the vertical component.
Output.Velocity.Z = 0.f;
}
else
{
// Rescale velocity to be horizontal but maintain magnitude of last update.
Output.Velocity = Output.Velocity.GetSafeNormal2D() * Output.Velocity.Size();
}
}
}
bool FCharacterMovementComponentAsyncInput::MoveUpdatedComponent(const FVector& Delta, const FQuat& NewRotation, bool bSweep, FCharacterMovementComponentAsyncOutput& Output, FHitResult* OutHitResult, ETeleportType TeleportType) const
{
const FVector NewDelta = ConstrainDirectionToPlane(Delta);
return UpdatedComponentInput->MoveComponent(Delta, NewRotation, bSweep, OutHitResult, Output.MoveComponentFlags, TeleportType, *this, Output);
}
bool FCharacterMovementComponentAsyncInput::SafeMoveUpdatedComponent(const FVector& Delta, const FQuat& NewRotation, bool bSweep, FHitResult& OutHit, FCharacterMovementComponentAsyncOutput& Output, ETeleportType Teleport) const
{
bool bMoveResult = false;
// Scope for move flags
{
// Conditionally ignore blocking overlaps (based on CVar)
const EMoveComponentFlags IncludeBlockingOverlapsWithoutEvents = (MOVECOMP_NeverIgnoreBlockingOverlaps | MOVECOMP_DisableBlockingOverlapDispatch);
EMoveComponentFlags& MoveComponentFlags = Output.MoveComponentFlags;
TGuardValue<EMoveComponentFlags> ScopedFlagRestore(MoveComponentFlags, MovementComponentCVars::MoveIgnoreFirstBlockingOverlap ? MoveComponentFlags : (MoveComponentFlags | IncludeBlockingOverlapsWithoutEvents));
bMoveResult = MoveUpdatedComponent(Delta, NewRotation, bSweep, Output, &OutHit, Teleport);
}
// Handle initial penetrations
if (OutHit.bStartPenetrating/* && UpdatedComponent*/)
{
const FVector RequestedAdjustment = GetPenetrationAdjustment(OutHit);
if (ResolvePenetration(RequestedAdjustment, OutHit, NewRotation, Output))
{
// Retry original move
bMoveResult = MoveUpdatedComponent(Delta, NewRotation, bSweep, Output, &OutHit, Teleport);
}
}
return bMoveResult;
}
void FCharacterMovementComponentAsyncInput::ApplyAccumulatedForces(float DeltaSeconds, FCharacterMovementComponentAsyncOutput& Output) const
{
if (Output.PendingImpulseToApply.Z != 0.f || Output.PendingForceToApply.Z != 0.f)
{
// check to see if applied momentum is enough to overcome gravity
if (IsMovingOnGround(Output) && (Output.PendingImpulseToApply.Z + (Output.PendingForceToApply.Z * DeltaSeconds) + (GravityZ * DeltaSeconds) > UE_SMALL_NUMBER))
{
SetMovementMode(MOVE_Falling, Output);
}
}
Output.Velocity += Output.PendingImpulseToApply + (Output.PendingForceToApply * DeltaSeconds);
// Don't call ClearAccumulatedForces() because it could affect launch velocity
Output.PendingImpulseToApply = FVector::ZeroVector;
Output.PendingForceToApply = FVector::ZeroVector;
}
void FCharacterMovementComponentAsyncInput::ClearAccumulatedForces(FCharacterMovementComponentAsyncOutput& Output) const
{
Output.PendingImpulseToApply = FVector::ZeroVector;
Output.PendingForceToApply = FVector::ZeroVector;
Output.PendingLaunchVelocity = FVector::ZeroVector;
}
void FCharacterMovementComponentAsyncInput::SetMovementMode(EMovementMode NewMovementMode, FCharacterMovementComponentAsyncOutput& Output, uint8 NewCustomMode) const
{
if (NewMovementMode != MOVE_Custom)
{
NewCustomMode = 0;
}
// If trying to use NavWalking but there is no navmesh, use walking instead.
if (NewMovementMode == MOVE_NavWalking)
{
ensure(false);
}
// Do nothing if nothing is changing.
if (Output.MovementMode == NewMovementMode)
{
// Allow changes in custom sub-mode.
if ((NewMovementMode != MOVE_Custom) || (NewCustomMode == Output.CustomMovementMode))
{
return;
}
}
const EMovementMode PrevMovementMode = Output.MovementMode;
const uint8 PrevCustomMode = Output.CustomMovementMode;
Output.MovementMode = NewMovementMode;
Output.CustomMovementMode = NewCustomMode;
// We allow setting movement mode before we have a component to update, in case this happens at startup.
if (!bHasValidData)
{
return;
}
// Handle change in movement mode
OnMovementModeChanged(PrevMovementMode, PrevCustomMode, Output);
}
void FCharacterMovementComponentAsyncInput::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode, FCharacterMovementComponentAsyncOutput& Output) const
{
if (!bHasValidData)
{
return;
}
// Update collision settings if needed
if (Output.MovementMode == MOVE_NavWalking)
{
ensure(false);
}
else if (PreviousMovementMode == MOVE_NavWalking)
{
ensure(false);
}
// React to changes in the movement mode.
if (Output.MovementMode == MOVE_Walking)
{
// Walking uses only XY velocity, and must be on a walkable floor, with a Base.
Output.Velocity.Z = 0.f;
Output.bCrouchMaintainsBaseLocation = true;
Output.GroundMovementMode = Output.MovementMode;
// make sure we update our new floor/base on initial entry of the walking physics
FindFloor(UpdatedComponentInput->GetPosition(), Output.CurrentFloor, false, Output);
AdjustFloorHeight(Output);
SetBaseFromFloor(Output.CurrentFloor, Output);
}
else
{
Output.CurrentFloor.Clear();
Output.bCrouchMaintainsBaseLocation = false;
if (Output.MovementMode == MOVE_Falling)
{
}
//SetBase(NULL);
Output.NewMovementBase = nullptr;
Output.NewMovementBaseOwner = nullptr;
if (Output.MovementMode == MOVE_None)
{
ensure(false);
}
}
if (Output.MovementMode == MOVE_Falling && PreviousMovementMode != MOVE_Falling)
{
}
CharacterInput->OnMovementModeChanged(PreviousMovementMode, *this, Output, PreviousCustomMode);
}
void FCharacterMovementComponentAsyncInput::FindFloor(const FVector& CapsuleLocation, FFindFloorResult& OutFloorResult, bool bCanUseCachedLocation, FCharacterMovementComponentAsyncOutput& Output, const FHitResult* DownwardSweepResult) const
{
// No collision, no floor...
if (!bHasValidData || !UpdatedComponentInput->bIsQueryCollisionEnabled)
{
OutFloorResult.Clear();
return;
}
// Increase height check slightly if walking, to prevent floor height adjustment from later invalidating the floor result.
const float MaxFloorDist = UCharacterMovementComponent::MAX_FLOOR_DIST;
const float MinFloorDist = UCharacterMovementComponent::MIN_FLOOR_DIST;
const float HeightCheckAdjust = (IsMovingOnGround(Output) ? MaxFloorDist + UE_KINDA_SMALL_NUMBER : -MaxFloorDist);
float FloorSweepTraceDist = FMath::Max(MaxFloorDist, MaxStepHeight + HeightCheckAdjust);
float FloorLineTraceDist = FloorSweepTraceDist;
bool bNeedToValidateFloor = true;
// Sweep floor
if (FloorLineTraceDist > 0.f || FloorSweepTraceDist > 0.f)
{
if (bAlwaysCheckFloor || !bCanUseCachedLocation || Output.bForceNextFloorCheck || Output.bJustTeleported)
{
Output.bForceNextFloorCheck = false;
ComputeFloorDist(CapsuleLocation, FloorLineTraceDist, FloorSweepTraceDist, OutFloorResult, Output.ScaledCapsuleRadius, Output, DownwardSweepResult);
}
else
{
// Force floor check if base has collision disabled or if it does not block us.
UPrimitiveComponent* MovementBase = Output.NewMovementBase;
//const AActor* BaseActor = MovementBase ? MovementBase->GetOwner() : NULL;
if (MovementBase != NULL)
{
// For now used cached values from original movement base, it could have changed, so this is wrong if so.
MovementBaseAsyncData.Validate(Output);
ensure(false);
}
if (false)
{
OutFloorResult = Output.CurrentFloor;
bNeedToValidateFloor = false;
}
else
{
Output.bForceNextFloorCheck = false;
ComputeFloorDist(CapsuleLocation, FloorLineTraceDist, FloorSweepTraceDist, OutFloorResult, Output.ScaledCapsuleRadius, Output, DownwardSweepResult);
}
}
}
if (bNeedToValidateFloor && OutFloorResult.bBlockingHit && !OutFloorResult.bLineTrace)
{
const bool bCheckRadius = true;
if (ShouldComputePerchResult(OutFloorResult.HitResult, Output, bCheckRadius))
{
float MaxPerchFloorDist = FMath::Max(MaxFloorDist, MaxStepHeight + HeightCheckAdjust);
if (IsMovingOnGround(Output))
{
MaxPerchFloorDist += FMath::Max(0.f, PerchAdditionalHeight);
}
FFindFloorResult PerchFloorResult;
if (ComputePerchResult(GetValidPerchRadius(Output), OutFloorResult.HitResult, MaxPerchFloorDist, PerchFloorResult, Output))
{
// Don't allow the floor distance adjustment to push us up too high, or we will move beyond the perch distance and fall next time.
const float AvgFloorDist = (MinFloorDist + MaxFloorDist) * 0.5f;
const float MoveUpDist = (AvgFloorDist - OutFloorResult.FloorDist);
if (MoveUpDist + PerchFloorResult.FloorDist >= MaxPerchFloorDist)
{
OutFloorResult.FloorDist = AvgFloorDist;
}
// If the regular capsule is on an unwalkable surface but the perched one would allow us to stand, override the normal to be one that is walkable.
if (!OutFloorResult.bWalkableFloor)
{
// Floor distances are used as the distance of the regular capsule to the point of collision, to make sure AdjustFloorHeight() behaves correctly.
OutFloorResult.SetFromLineTrace(PerchFloorResult.HitResult, OutFloorResult.FloorDist, FMath::Max(OutFloorResult.FloorDist, MinFloorDist), true);
}
}
else
{
// We had no floor (or an invalid one because it was unwalkable), and couldn't perch here, so invalidate floor (which will cause us to start falling).
OutFloorResult.bWalkableFloor = false;
}
}
}
}
void FCharacterMovementComponentAsyncInput::ComputeFloorDist(const FVector& CapsuleLocation, float LineDistance, float SweepDistance, FFindFloorResult& OutFloorResult, float SweepRadius, FCharacterMovementComponentAsyncOutput& Output, const FHitResult* DownwardSweepResult) const
{
OutFloorResult.Clear();
float PawnRadius = Output.ScaledCapsuleRadius;
float PawnHalfHeight = Output.ScaledCapsuleHalfHeight;
bool bSkipSweep = false;
if (DownwardSweepResult != NULL && DownwardSweepResult->IsValidBlockingHit())
{
// Only if the supplied sweep was vertical and downward.
if ((DownwardSweepResult->TraceStart.Z > DownwardSweepResult->TraceEnd.Z) &&
(DownwardSweepResult->TraceStart - DownwardSweepResult->TraceEnd).SizeSquared2D() <= UE_KINDA_SMALL_NUMBER)
{
// Reject hits that are barely on the cusp of the radius of the capsule
if (IsWithinEdgeTolerance(DownwardSweepResult->Location, DownwardSweepResult->ImpactPoint, PawnRadius))
{
// Don't try a redundant sweep, regardless of whether this sweep is usable.
bSkipSweep = true;
const bool bIsWalkable = IsWalkable(*DownwardSweepResult);
const float FloorDist = (CapsuleLocation.Z - DownwardSweepResult->Location.Z);
OutFloorResult.SetFromSweep(*DownwardSweepResult, FloorDist, bIsWalkable);
if (bIsWalkable)
{
// Use the supplied downward sweep as the floor hit result.
return;
}
}
}
}
// We require the sweep distance to be >= the line distance, otherwise the HitResult can't be interpreted as the sweep result.
if (SweepDistance < LineDistance)
{
ensure(SweepDistance >= LineDistance);
return;
}
bool bBlockingHit = false;
// Sweep test
if (!bSkipSweep && SweepDistance > 0.f && SweepRadius > 0.f)
{
const float ShrinkScale = 0.9f;
const float ShrinkScaleOverlap = 0.1f;
float ShrinkHeight = (PawnHalfHeight - PawnRadius) * (1.f - ShrinkScale);
float TraceDist = SweepDistance + ShrinkHeight;
FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(SweepRadius, PawnHalfHeight - ShrinkHeight);
FHitResult Hit(1.f);
bBlockingHit = FloorSweepTest(Hit, CapsuleLocation, CapsuleLocation + FVector(0.f, 0.f, -TraceDist), CollisionChannel, CapsuleShape, QueryParams, CollisionResponseParams, Output);
if (bBlockingHit)
{
// Reject hits adjacent to us, we only care about hits on the bottom portion of our capsule.
// Check 2D distance to impact point, reject if within a tolerance from radius.
if (Hit.bStartPenetrating || !IsWithinEdgeTolerance(CapsuleLocation, Hit.ImpactPoint, CapsuleShape.Capsule.Radius))
{
// Use a capsule with a slightly smaller radius and shorter height to avoid the adjacent object.
// Capsule must not be nearly zero or the trace will fall back to a line trace from the start point and have the wrong length.
CapsuleShape.Capsule.Radius = FMath::Max(0.f, CapsuleShape.Capsule.Radius - UCharacterMovementComponent::SWEEP_EDGE_REJECT_DISTANCE - UE_KINDA_SMALL_NUMBER);
if (!CapsuleShape.IsNearlyZero())
{
ShrinkHeight = (PawnHalfHeight - PawnRadius) * (1.f - ShrinkScaleOverlap);
TraceDist = SweepDistance + ShrinkHeight;
CapsuleShape.Capsule.HalfHeight = FMath::Max(PawnHalfHeight - ShrinkHeight, CapsuleShape.Capsule.Radius);
Hit.Reset(1.f, false);
bBlockingHit = FloorSweepTest(Hit, CapsuleLocation, CapsuleLocation + FVector(0.f, 0.f, -TraceDist), CollisionChannel, CapsuleShape, QueryParams, CollisionResponseParams, Output);
}
}
// Reduce hit distance by ShrinkHeight because we shrank the capsule for the trace.
// We allow negative distances here, because this allows us to pull out of penetrations.
const float MaxPenetrationAdjust = FMath::Max(UCharacterMovementComponent::MAX_FLOOR_DIST, PawnRadius);
const float SweepResult = FMath::Max(-MaxPenetrationAdjust, Hit.Time * TraceDist - ShrinkHeight);
OutFloorResult.SetFromSweep(Hit, SweepResult, false);
if (Hit.IsValidBlockingHit() && IsWalkable(Hit))
{
if (SweepResult <= SweepDistance)
{
// Hit within test distance.
OutFloorResult.bWalkableFloor = true;
return;
}
}
}
}
// Since we require a longer sweep than line trace, we don't want to run the line trace if the sweep missed everything.
// We do however want to try a line trace if the sweep was stuck in penetration.
if (!OutFloorResult.bBlockingHit && !OutFloorResult.HitResult.bStartPenetrating)
{
OutFloorResult.FloorDist = SweepDistance;
return;
}
// Line trace
if (LineDistance > 0.f)
{
const float ShrinkHeight = PawnHalfHeight;
const FVector LineTraceStart = CapsuleLocation;
const float TraceDist = LineDistance + ShrinkHeight;
const FVector Down = FVector(0.f, 0.f, -TraceDist);
FHitResult Hit(1.f);
bBlockingHit = World->LineTraceSingleByChannel(Hit, LineTraceStart, LineTraceStart + Down, CollisionChannel, QueryParams, CollisionResponseParams);
if (bBlockingHit)
{
if (Hit.Time > 0.f)
{
// Reduce hit distance by ShrinkHeight because we started the trace higher than the base.
// We allow negative distances here, because this allows us to pull out of penetrations.
const float MaxPenetrationAdjust = FMath::Max(UCharacterMovementComponent::MAX_FLOOR_DIST, PawnRadius);
const float LineResult = FMath::Max(-MaxPenetrationAdjust, Hit.Time * TraceDist - ShrinkHeight);
OutFloorResult.bBlockingHit = true;
if (LineResult <= LineDistance && IsWalkable(Hit))
{
OutFloorResult.SetFromLineTrace(Hit, OutFloorResult.FloorDist, LineResult, true);
return;
}
}
}
}
OutFloorResult.bWalkableFloor = false;
}
bool FCharacterMovementComponentAsyncInput::FloorSweepTest(FHitResult& OutHit, const FVector& Start, const FVector& End, ECollisionChannel TraceChannel, const FCollisionShape& CollisionShape, const FCollisionQueryParams& Params, const FCollisionResponseParams& ResponseParam, FCharacterMovementComponentAsyncOutput& Output) const
{
bool bBlockingHit = false;
if (!bUseFlatBaseForFloorChecks)
{
bBlockingHit = World->SweepSingleByChannel(OutHit, Start, End, FQuat::Identity, TraceChannel, CollisionShape, Params, ResponseParam);
}
else
{
// Test with a box that is enclosed by the capsule.
const float CapsuleRadius = CollisionShape.GetCapsuleRadius();
const float CapsuleHeight = CollisionShape.GetCapsuleHalfHeight();
const FCollisionShape BoxShape = FCollisionShape::MakeBox(FVector(CapsuleRadius * 0.707f, CapsuleRadius * 0.707f, CapsuleHeight));
// First test with the box rotated so the corners are along the major axes (ie rotated 45 degrees).
bBlockingHit = World->SweepSingleByChannel(OutHit, Start, End, FQuat(FVector(0.f, 0.f, -1.f), UE_PI * 0.25f), TraceChannel, BoxShape, Params, ResponseParam);
if (!bBlockingHit)
{
// Test again with the same box, not rotated.
OutHit.Reset(1.f, false);
bBlockingHit = World->SweepSingleByChannel(OutHit, Start, End, FQuat::Identity, TraceChannel, BoxShape, Params, ResponseParam);
}
}
return bBlockingHit;
}
bool FCharacterMovementComponentAsyncInput::IsWithinEdgeTolerance(const FVector& CapsuleLocation, const FVector& TestImpactPoint, const float CapsuleRadius) const
{
const float DistFromCenterSq = (TestImpactPoint - CapsuleLocation).SizeSquared2D();
const float ReducedRadiusSq = FMath::Square(FMath::Max(UCharacterMovementComponent::SWEEP_EDGE_REJECT_DISTANCE + UE_KINDA_SMALL_NUMBER, CapsuleRadius - UCharacterMovementComponent::SWEEP_EDGE_REJECT_DISTANCE));
return DistFromCenterSq < ReducedRadiusSq;
}
bool FCharacterMovementComponentAsyncInput::IsWalkable(const FHitResult& Hit) const
{
if (!Hit.IsValidBlockingHit())
{
// No hit, or starting in penetration
return false;
}
// Never walk up vertical surfaces.
if (Hit.ImpactNormal.Z < UE_KINDA_SMALL_NUMBER)
{
return false;
}
float TestWalkableZ = WalkableFloorZ;
// Can't walk on this surface if it is too steep.
if (Hit.ImpactNormal.Z < TestWalkableZ)
{
return false;
}
return true;
}
void FCharacterMovementComponentAsyncInput::UpdateCharacterStateAfterMovement(float DeltaSeconds, FCharacterMovementComponentAsyncOutput& Output) const
{
// Proxies get replicated crouch state.
if (CharacterInput->LocalRole != ROLE_SimulatedProxy)
{
// Uncrouch if no longer allowed to be crouched
if (Output.bIsCrouched && !CanCrouchInCurrentState(Output))
{
}
}
}
float FCharacterMovementComponentAsyncInput::GetSimulationTimeStep(float RemainingTime, int32 Iterations) const
{
static uint32 s_WarningCount = 0;
if (RemainingTime > MaxSimulationTimeStep)
{
if (Iterations < MaxSimulationIterations)
{
// Subdivide moves to be no longer than MaxSimulationTimeStep seconds
RemainingTime = FMath::Min(MaxSimulationTimeStep, RemainingTime * 0.5f);
}
else
{
}
}
// no less than MIN_TICK_TIME (to avoid potential divide-by-zero during simulation).
return FMath::Max(UCharacterMovementComponent::MIN_TICK_TIME, RemainingTime);
}
void FCharacterMovementComponentAsyncInput::CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration, FCharacterMovementComponentAsyncOutput& Output) const
{
// Do not update velocity when using root motion or when SimulatedProxy and not simulating root motion - SimulatedProxy are repped their Velocity
if (!bHasValidData || RootMotion.bHasAnimRootMotion || DeltaTime < UCharacterMovementComponent::MIN_TICK_TIME
|| (CharacterInput->LocalRole == ROLE_SimulatedProxy && !bWasSimulatingRootMotion))
{
return;
}
Friction = FMath::Max(0.f, Friction);
const float MaxAccel = MaxAcceleration;
float MaxSpeed = GetMaxSpeed(Output);
// Check if path following requested movement
bool bZeroRequestedAcceleration = true;
FVector RequestedAcceleration = FVector::ZeroVector;
float RequestedSpeed = 0.0f;
if (ApplyRequestedMove(DeltaTime, MaxAccel, MaxSpeed, Friction, BrakingDeceleration, RequestedAcceleration, RequestedSpeed, Output))
{
bZeroRequestedAcceleration = false;
}
FVector& Acceleration = Output.Acceleration;
FVector& Velocity = Output.Velocity;
if (bForceMaxAccel)
{
// Force acceleration at full speed.
// In consideration order for direction: Acceleration, then Velocity, then Pawn's rotation.
if (Acceleration.SizeSquared() > UE_SMALL_NUMBER)
{
Acceleration = Acceleration.GetSafeNormal() * MaxAccel;
}
else
{
Acceleration = MaxAccel * (Velocity.SizeSquared() < UE_SMALL_NUMBER ? UpdatedComponentInput->GetForwardVector() : Velocity.GetSafeNormal());
}
Output.AnalogInputModifier = 1.f;
}
// Path following above didn't care about the analog modifier, but we do for everything else below, so get the fully modified value.
// Use max of requested speed and max speed if we modified the speed in ApplyRequestedMove above.
const float MaxInputSpeed = FMath::Max(MaxSpeed * Output.AnalogInputModifier, GetMinAnalogSpeed(Output));
MaxSpeed = FMath::Max(RequestedSpeed, MaxInputSpeed);
// Apply braking or deceleration
const bool bZeroAcceleration = Acceleration.IsZero();
const bool bVelocityOverMax = IsExceedingMaxSpeed(MaxSpeed, Output);
// Only apply braking if there is no acceleration, or we are over our max speed and need to slow down to it.
if ((bZeroAcceleration && bZeroRequestedAcceleration) || bVelocityOverMax)
{
const FVector OldVelocity = Velocity;
const float ActualBrakingFriction = (bUseSeparateBrakingFriction ? BrakingFriction : Friction);
ApplyVelocityBraking(DeltaTime, ActualBrakingFriction, BrakingDeceleration, Output);
// Don't allow braking to lower us below max speed if we started above it.
if (bVelocityOverMax && Velocity.SizeSquared() < FMath::Square(MaxSpeed) && FVector::DotProduct(Acceleration, OldVelocity) > 0.0f)
{
Velocity = OldVelocity.GetSafeNormal() * MaxSpeed;
}
}
else if (!bZeroAcceleration)
{
// Friction affects our ability to change direction. This is only done for input acceleration, not path following.
const FVector AccelDir = Acceleration.GetSafeNormal();
const float VelSize = Velocity.Size();
Velocity = Velocity - (Velocity - AccelDir * VelSize) * FMath::Min(DeltaTime * Friction, 1.f);
}
if (bFluid)
{
Velocity = Velocity * (1.f - FMath::Min(Friction * DeltaTime, 1.f));
}
if (!bZeroAcceleration)
{
const float NewMaxInputSpeed = IsExceedingMaxSpeed(MaxInputSpeed, Output) ? Velocity.Size() : MaxInputSpeed;
Velocity += Acceleration * DeltaTime;
Velocity = Velocity.GetClampedToMaxSize(NewMaxInputSpeed);
}
// Apply additional requested acceleration
if (!bZeroRequestedAcceleration)
{
const float NewMaxRequestedSpeed = IsExceedingMaxSpeed(RequestedSpeed, Output) ? Velocity.Size() : RequestedSpeed;
Velocity += RequestedAcceleration * DeltaTime;
Velocity = Velocity.GetClampedToMaxSize(NewMaxRequestedSpeed);
}
}
bool FCharacterMovementComponentAsyncInput::ApplyRequestedMove(float DeltaTime, float MaxAccel, float MaxSpeed, float Friction, float BrakingDeceleration, FVector& OutAcceleration, float& OutRequestedSpeed, FCharacterMovementComponentAsyncOutput& Output) const
{
if (Output.bHasRequestedVelocity)
{
const float RequestedSpeedSquared = Output.RequestedVelocity.SizeSquared();
if (RequestedSpeedSquared < UE_KINDA_SMALL_NUMBER)
{
return false;
}
// Compute requested speed from path following
float RequestedSpeed = FMath::Sqrt(RequestedSpeedSquared);
const FVector RequestedMoveDir = Output.RequestedVelocity / RequestedSpeed;
RequestedSpeed = (Output.bRequestedMoveWithMaxSpeed ? MaxSpeed : FMath::Min(MaxSpeed, RequestedSpeed));
// Compute actual requested velocity
const FVector MoveVelocity = RequestedMoveDir * RequestedSpeed;
// Compute acceleration. Use MaxAccel to limit speed increase, 1% buffer.
FVector NewAcceleration = FVector::ZeroVector;
const float CurrentSpeedSq = Output.Velocity.SizeSquared();
if (ShouldComputeAccelerationToReachRequestedVelocity(RequestedSpeed, Output))
{
// Turn in the same manner as with input acceleration.
const float VelSize = FMath::Sqrt(CurrentSpeedSq);
Output.Velocity = Output.Velocity - (Output.Velocity - RequestedMoveDir * VelSize) * FMath::Min(DeltaTime * Friction, 1.f);
// How much do we need to accelerate to get to the new velocity?
NewAcceleration = ((MoveVelocity - Output.Velocity) / DeltaTime);
NewAcceleration = NewAcceleration.GetClampedToMaxSize(MaxAccel);
}
else
{
// Just set velocity directly.
// If decelerating we do so instantly, so we don't slide through the destination if we can't brake fast enough.
Output.Velocity = MoveVelocity;
}
OutRequestedSpeed = RequestedSpeed;
OutAcceleration = NewAcceleration;
return true;
}
return false;
}
bool FCharacterMovementComponentAsyncInput::ShouldComputeAccelerationToReachRequestedVelocity(const float RequestedSpeed, FCharacterMovementComponentAsyncOutput& Output) const
{
return bRequestedMoveUseAcceleration && Output.Velocity.SizeSquared() < FMath::Square(RequestedSpeed * 1.01f);
}
float FCharacterMovementComponentAsyncInput::GetMinAnalogSpeed(FCharacterMovementComponentAsyncOutput& Output) const
{
switch (Output.MovementMode)
{
case MOVE_Walking:
case MOVE_NavWalking:
case MOVE_Falling:
return MinAnalogWalkSpeed;
default:
return 0.f;
}
}
float FCharacterMovementComponentAsyncInput::GetMaxBrakingDeceleration(FCharacterMovementComponentAsyncOutput& Output) const
{
switch (Output.MovementMode)
{
case MOVE_Walking:
case MOVE_NavWalking:
return BrakingDecelerationWalking;
case MOVE_Falling:
return BrakingDecelerationFalling;
case MOVE_Swimming:
return BrakingDecelerationSwimming;
case MOVE_Flying:
return BrakingDecelerationFlying;
case MOVE_Custom:
return 0.f;
case MOVE_None:
default:
return 0.f;
}
}
void FCharacterMovementComponentAsyncInput::ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration, FCharacterMovementComponentAsyncOutput& Output) const
{
FVector& Velocity = Output.Velocity;
if (Velocity.IsZero() || !bHasValidData || RootMotion.bHasAnimRootMotion || DeltaTime < UCharacterMovementComponent::MIN_TICK_TIME)
{
return;
}
const float FrictionFactor = FMath::Max(0.f, BrakingFrictionFactor);
Friction = FMath::Max(0.f, Friction * FrictionFactor);
BrakingDeceleration = FMath::Max(0.f, BrakingDeceleration);
const bool bZeroFriction = (Friction == 0.f);
const bool bZeroBraking = (BrakingDeceleration == 0.f);
if (bZeroFriction && bZeroBraking)
{
return;
}
const FVector OldVel = Velocity;
// subdivide braking to get reasonably consistent results at lower frame rates
float RemainingTime = DeltaTime;
const float MaxTimeStep = FMath::Clamp(BrakingSubStepTime, 1.0f / 75.0f, 1.0f / 20.0f);
// Decelerate to brake to a stop
const FVector RevAccel = (bZeroBraking ? FVector::ZeroVector : (-BrakingDeceleration * Velocity.GetSafeNormal()));
while (RemainingTime >= UCharacterMovementComponent::MIN_TICK_TIME)
{
// Zero friction uses constant deceleration, so no need for iteration.
const float dt = ((RemainingTime > MaxTimeStep && !bZeroFriction) ? FMath::Min(MaxTimeStep, RemainingTime * 0.5f) : RemainingTime);
RemainingTime -= dt;
// apply friction and braking
Velocity = Velocity + ((-Friction) * Velocity + RevAccel) * dt;
// Don't reverse direction
if ((Velocity | OldVel) <= 0.f)
{
Velocity = FVector::ZeroVector;
return;
}
}
// Clamp to zero if nearly zero, or if below min threshold and braking.
const float VSizeSq = Velocity.SizeSquared();
if (VSizeSq <= UE_KINDA_SMALL_NUMBER || (!bZeroBraking && VSizeSq <= FMath::Square(UCharacterMovementComponent::BRAKE_TO_STOP_VELOCITY)))
{
Velocity = FVector::ZeroVector;
}
}
FVector FCharacterMovementComponentAsyncInput::GetPenetrationAdjustment(FHitResult& HitResult) const
{
FVector Result = MoveComponent_GetPenetrationAdjustment(HitResult);
{
const bool bIsProxy = (CharacterInput->LocalRole == ROLE_SimulatedProxy);
float MaxDistance = bIsProxy ? MaxDepenetrationWithGeometryAsProxy : MaxDepenetrationWithGeometry;
const AActor* HitActor = HitResult.GetActor();
if (Cast<APawn>(HitActor))
{
MaxDistance = bIsProxy ? MaxDepenetrationWithPawnAsProxy : MaxDepenetrationWithPawn;
}
Result = Result.GetClampedToMaxSize(MaxDistance);
}
return Result;
}
bool FCharacterMovementComponentAsyncInput::ResolvePenetration(const FVector& ProposedAdjustment, const FHitResult& Hit, const FQuat& NewRotation, FCharacterMovementComponentAsyncOutput& Output) const
{
// SceneComponent can't be in penetration, so this function really only applies to PrimitiveComponent.
const FVector Adjustment = ConstrainDirectionToPlane(ProposedAdjustment);
if (!Adjustment.IsZero() && UpdatedComponentInput->UpdatedComponent)
{
bool bEncroached = World->OverlapBlockingTestByChannel(Hit.TraceStart + Adjustment, NewRotation, CollisionChannel, UpdatedComponentInput->CollisionShape, UpdatedComponentInput->MoveComponentQueryParams, UpdatedComponentInput->MoveComponentCollisionResponseParams);
if (!bEncroached)
{
MoveUpdatedComponent(Adjustment, NewRotation, false, Output, nullptr, ETeleportType::TeleportPhysics);
Output.bJustTeleported = true;
return true;
}
else
{
TGuardValue<EMoveComponentFlags> ScopedFlagRestore(Output.MoveComponentFlags, EMoveComponentFlags(Output.MoveComponentFlags & (~MOVECOMP_NeverIgnoreBlockingOverlaps)));
FHitResult SweepOutHit(1.f);
bool bMoved = MoveUpdatedComponent(Adjustment, NewRotation, true, Output, &SweepOutHit, ETeleportType::TeleportPhysics);
if (!bMoved && SweepOutHit.bStartPenetrating)
{
// Combine two MTD results to get a new direction that gets out of multiple surfaces.
const FVector SecondMTD = GetPenetrationAdjustment(SweepOutHit);
const FVector CombinedMTD = Adjustment + SecondMTD;
if (SecondMTD != Adjustment && !CombinedMTD.IsZero())
{
bMoved = MoveUpdatedComponent(CombinedMTD, NewRotation, true, Output, nullptr, ETeleportType::TeleportPhysics);
}
}
if (!bMoved)
{
const FVector MoveDelta = ConstrainDirectionToPlane(Hit.TraceEnd - Hit.TraceStart);
if (!MoveDelta.IsZero())
{
bMoved = MoveUpdatedComponent(Adjustment + MoveDelta, NewRotation, true, Output, nullptr, ETeleportType::TeleportPhysics);
if (!bMoved && FVector::DotProduct(MoveDelta, Adjustment) > 0.f)
{
bMoved = MoveUpdatedComponent(MoveDelta, NewRotation, true, Output, nullptr, ETeleportType::TeleportPhysics);
}
}
}
Output.bJustTeleported |= bMoved;
return bMoved;
}
}
return false;
}
FVector FCharacterMovementComponentAsyncInput::MoveComponent_GetPenetrationAdjustment(FHitResult& Hit) const
{
if (!Hit.bStartPenetrating)
{
return FVector::ZeroVector;
}
FVector Result;
const float PullBackDistance = FMath::Abs(MovementComponentCVars::PenetrationPullbackDistance);
const float PenetrationDepth = (Hit.PenetrationDepth > 0.f ? Hit.PenetrationDepth : 0.125f);
Result = Hit.Normal * (PenetrationDepth + PullBackDistance);
return ConstrainDirectionToPlane(Result);
}
float FCharacterMovementComponentAsyncInput::MoveComponent_SlideAlongSurface(const FVector& Delta, float Time, const FVector& Normal, FHitResult& Hit, FCharacterMovementComponentAsyncOutput& Output, bool bHandleImpact) const
{
if (!Hit.bBlockingHit)
{
return 0.f;
}
float PercentTimeApplied = 0.f;
const FVector OldHitNormal = Normal;
FVector SlideDelta = ComputeSlideVector(Delta, Time, Normal, Hit, Output);
if ((SlideDelta | Delta) > 0.f)
{
const FQuat Rotation = UpdatedComponentInput->GetRotation();
SafeMoveUpdatedComponent(SlideDelta, Rotation, true, Hit, Output);
const float FirstHitPercent = Hit.Time;
PercentTimeApplied = FirstHitPercent;
if (Hit.IsValidBlockingHit())
{
// Notify first impact
if (bHandleImpact)
{
HandleImpact(Hit, Output, FirstHitPercent * Time, SlideDelta);
}
// Compute new slide normal when hitting multiple surfaces.
TwoWallAdjust(SlideDelta, Hit, OldHitNormal, Output);
// Only proceed if the new direction is of significant length and not in reverse of original attempted move.
if (!SlideDelta.IsNearlyZero(1e-3f) && (SlideDelta | Delta) > 0.f)
{
// Perform second move
SafeMoveUpdatedComponent(SlideDelta, Rotation, true, Hit, Output);
const float SecondHitPercent = Hit.Time * (1.f - FirstHitPercent);
PercentTimeApplied += SecondHitPercent;
// Notify second impact
if (bHandleImpact && Hit.bBlockingHit)
{
HandleImpact(Hit, Output, SecondHitPercent * Time, SlideDelta);
}
}
}
return FMath::Clamp(PercentTimeApplied, 0.f, 1.f);
}
return 0.f;
}
FVector FCharacterMovementComponentAsyncInput::MoveComponent_ComputeSlideVector(const FVector& Delta, const float Time, const FVector& Normal, const FHitResult& Hit, FCharacterMovementComponentAsyncOutput& Output) const
{
if (!bConstrainToPlane)
{
return FVector::VectorPlaneProject(Delta, Normal) * Time;
}
else
{
const FVector ProjectedNormal = ConstrainNormalToPlane(Normal);
return FVector::VectorPlaneProject(Delta, ProjectedNormal) * Time;
}
}
bool FUpdatedComponentAsyncInput::MoveComponent(const FVector& Delta, const FQuat& NewRotationQuat, bool bSweep, FHitResult* OutHit,  EMoveComponentFlags MoveFlags, ETeleportType Teleport,  const FCharacterMovementComponentAsyncInput& Input, FCharacterMovementComponentAsyncOutput& Output) const 
{
const FVector TraceStart = GetPosition();
const FVector TraceEnd = TraceStart + Delta;
float DeltaSizeSq = (TraceEnd - TraceStart).SizeSquared();
const FQuat InitialRotationQuat = GetRotation();
const float MinMovementDistSq = (bSweep ? FMath::Square(4.f * UE_KINDA_SMALL_NUMBER) : 0.f);
if (DeltaSizeSq <= MinMovementDistSq)
{
if (NewRotationQuat.Equals(InitialRotationQuat, SCENECOMPONENT_QUAT_TOLERANCE))
{
// copy to optional output param
if (OutHit)
{
OutHit->Init(TraceStart, TraceEnd);
}
return true;
}
DeltaSizeSq = 0.f;
}
const bool bSkipPhysicsMove = ((MoveFlags & MOVECOMP_SkipPhysicsMove) != MOVECOMP_NoFlags);
FHitResult BlockingHit(NoInit);
BlockingHit.bBlockingHit = false;
BlockingHit.Time = 1.f;
bool bFilledHitResult = false;
bool bMoved = false;
bool bIncludesOverlapsAtEnd = false;
bool bRotationOnly = false;
TInlineOverlapInfoArray PendingOverlaps;
if (!bSweep)
{
SetPosition(TraceEnd);
SetRotation(NewRotationQuat);
bRotationOnly = (DeltaSizeSq == 0);
bIncludesOverlapsAtEnd = bRotationOnly && (AreSymmetricRotations(InitialRotationQuat, NewRotationQuat, Input.UpdatedComponentInput->Scale)) && bIsQueryCollisionEnabled;
}
else
{
TArray<FHitResult> Hits;
FVector NewLocation = TraceStart;
// Perform movement collision checking if needed for this actor.
if (bIsQueryCollisionEnabled && (DeltaSizeSq > 0.f))
{
// now capturing params when building inputs.
bool const bHadBlockingHit = Input.World->ComponentSweepMulti(Hits, UpdatedComponent, TraceStart, TraceEnd, InitialRotationQuat, MoveComponentQueryParams);
if (Hits.Num() > 0)
{
const float DeltaSize = FMath::Sqrt(DeltaSizeSq);
for (int32 HitIdx = 0; HitIdx < Hits.Num(); HitIdx++)
{
FUpdatedComponentAsyncInput::PullBackHit(Hits[HitIdx], TraceStart, TraceEnd, DeltaSize);
}
}
// If we are looking for overlaps, store those as well.
int32 FirstNonInitialOverlapIdx = INDEX_NONE;
if (bHadBlockingHit || (bGatherOverlaps))
{
int32 BlockingHitIndex = INDEX_NONE;
float BlockingHitNormalDotDelta = UE_BIG_NUMBER;
for (int32 HitIdx = 0; HitIdx < Hits.Num(); HitIdx++)
{
const FHitResult& TestHit = Hits[HitIdx];
if (TestHit.bBlockingHit)
{
// This ignores if query/hit actor are based on each other. Can we determine this on PT?
if (!FUpdatedComponentAsyncInput::ShouldIgnoreHitResult(Input.World, TestHit, Delta, nullptr/*Actor*/, MoveFlags))
{
if (TestHit.bStartPenetrating)
{
// We may have multiple initial hits, and want to choose the one with the normal most opposed to our movement.
const float NormalDotDelta = (TestHit.ImpactNormal | Delta);
if (NormalDotDelta < BlockingHitNormalDotDelta)
{
BlockingHitNormalDotDelta = NormalDotDelta;
BlockingHitIndex = HitIdx;
}
}
else if (BlockingHitIndex == INDEX_NONE)
{
// First non-overlapping blocking hit should be used, if an overlapping hit was not.
// This should be the only non-overlapping blocking hit, and last in the results.
BlockingHitIndex = HitIdx;
break;
}
}
}
else if (bGatherOverlaps)
{
UPrimitiveComponent* OverlapComponent = TestHit.Component.Get();
// Overlaps are speculative, this flag will be chcked when applying outputs.
if (OverlapComponent && (true || bForceGatherOverlaps))
{
if (!FUpdatedComponentAsyncInput::ShouldIgnoreOverlapResult(Input.World, nullptr, *UpdatedComponent, TestHit.GetActor(), *OverlapComponent ))
{
// don't process touch events after initial blocking hits
if (BlockingHitIndex >= 0 && TestHit.Time > Hits[BlockingHitIndex].Time)
{
break;
}
if (FirstNonInitialOverlapIdx == INDEX_NONE && TestHit.Time > 0.f)
{
// We are about to add the first non-initial overlap.
FirstNonInitialOverlapIdx = PendingOverlaps.Num();
}
Output.UpdatedComponentOutput.AddUniqueSpeculativeOverlap(FOverlapInfo(TestHit));
}
}
}
}
// Update blocking hit, if there was a valid one.
if (BlockingHitIndex >= 0)
{
BlockingHit = Hits[BlockingHitIndex];
bFilledHitResult = true;
}
}
// Update NewLocation based on the hit result
if (!BlockingHit.bBlockingHit)
{
NewLocation = TraceEnd;
}
else
{
check(bFilledHitResult);
NewLocation = TraceStart + (BlockingHit.Time * (TraceEnd - TraceStart));
// Sanity check
const FVector ToNewLocation = (NewLocation - TraceStart);
if (ToNewLocation.SizeSquared() <= MinMovementDistSq)
{
// We don't want really small movements to put us on or inside a surface.
NewLocation = TraceStart;
BlockingHit.Time = 0.f;
// Remove any pending overlaps after this point, we are not going as far as we swept.
if (FirstNonInitialOverlapIdx != INDEX_NONE)
{
const bool bAllowShrinking = false;
PendingOverlaps.SetNum(FirstNonInitialOverlapIdx, bAllowShrinking);
}
}
}
bIncludesOverlapsAtEnd = AreSymmetricRotations(InitialRotationQuat, NewRotationQuat, Input.UpdatedComponentInput->Scale);
}
else if (DeltaSizeSq > 0.f)
{
// apply move delta even if components has collisions disabled
NewLocation += Delta;
bIncludesOverlapsAtEnd = false;
}
else if (DeltaSizeSq == 0.f && bIsQueryCollisionEnabled)
{
bIncludesOverlapsAtEnd = AreSymmetricRotations(InitialRotationQuat, NewRotationQuat, Input.UpdatedComponentInput->Scale);
bRotationOnly = true;
}
SetPosition(NewLocation);
SetRotation(NewRotationQuat);
bMoved = true;
}
// Handle blocking hit notifications. Avoid if pending kill (which could happen after overlaps).
const bool bAllowHitDispatch = !BlockingHit.bStartPenetrating || !(MoveFlags & MOVECOMP_DisableBlockingOverlapDispatch);
if (BlockingHit.bBlockingHit && bAllowHitDispatch/* && IsValid(this)*/)
{
check(bFilledHitResult);
}
// copy to optional output param
if (OutHit)
{
if (bFilledHitResult)
{
*OutHit = BlockingHit;
}
else
{
OutHit->Init(TraceStart, TraceEnd);
}
}
return bMoved;
}
bool FUpdatedComponentAsyncInput::AreSymmetricRotations(const FQuat& A, const FQuat& B, const FVector& Scale3D) const
{
if (Scale3D.X != Scale3D.Y)
{
return false;
}
const FVector AUp = A.GetAxisZ();
const FVector BUp = B.GetAxisZ();
return AUp.Equals(BUp);
}
void FUpdatedComponentAsyncInput::PullBackHit(FHitResult& Hit, const FVector& Start, const FVector& End, const float Dist)
{
const float DesiredTimeBack = FMath::Clamp(0.1f, 0.1f / Dist, 1.f / Dist) + 0.001f;
Hit.Time = FMath::Clamp(Hit.Time - DesiredTimeBack, 0.f, 1.f);
}
bool FUpdatedComponentAsyncInput::ShouldCheckOverlapFlagToQueueOverlaps(const UPrimitiveComponent& ThisComponent)
{
const FScopedMovementUpdate* CurrentUpdate = ThisComponent.GetCurrentScopedMovement();
if (CurrentUpdate)
{
return CurrentUpdate->RequiresOverlapsEventFlag();
}
// By default we require the GetGenerateOverlapEvents() to queue up overlaps, since we require it to trigger events.
return true;
}
bool FUpdatedComponentAsyncInput::ShouldIgnoreHitResult(const UWorld* InWorld, FHitResult const& TestHit, FVector const& MovementDirDenormalized, const AActor* MovingActor, EMoveComponentFlags MoveFlags)
{
if (TestHit.bBlockingHit)
{
// check "ignore bases" functionality
if ((MoveFlags & MOVECOMP_IgnoreBases) && false/*MovingActor*/)//we let overlap components go through because their overlap is still needed and will cause beginOverlap/endOverlap events
{
// ignore if there's a base relationship between moving actor and hit actor
AActor const* const HitActor = TestHit.GetActor();
if (HitActor)
{
if (MovingActor->IsBasedOnActor(HitActor) || HitActor->IsBasedOnActor(MovingActor))
{
return true;
}
}
}
// If we started penetrating, we may want to ignore it if we are moving out of penetration.
// This helps prevent getting stuck in walls.
if ((TestHit.Distance < PrimitiveComponentCVars::HitDistanceToleranceCVar || TestHit.bStartPenetrating) && !(MoveFlags & MOVECOMP_NeverIgnoreBlockingOverlaps))
{
const float DotTolerance = PrimitiveComponentCVars::InitialOverlapToleranceCVar;
// Dot product of movement direction against 'exit' direction
const FVector MovementDir = MovementDirDenormalized.GetSafeNormal();
const float MoveDot = (TestHit.ImpactNormal | MovementDir);
const bool bMovingOut = MoveDot > DotTolerance;
// If we are moving out, ignore this result!
if (bMovingOut)
{
return true;
}
}
}
return false;
}
bool FUpdatedComponentAsyncInput::ShouldIgnoreOverlapResult(const UWorld* World, const AActor* ThisActor, const UPrimitiveComponent& ThisComponent, const AActor* OtherActor, const UPrimitiveComponent& OtherComponent)
{
if (&ThisComponent == &OtherComponent)
{
return true;
}
// We're passing in null for ThisActor as we don't have that data, ensuring owner exists when filling inputs anyway.
if (/*!ThisActor || */!OtherActor)
{
return true;
}
// Cannot read this on PT, check when applying speculative overlaps.
if (!World)
{
return true;
}
return false;
}
void FUpdatedComponentAsyncInput::SetPosition(const FVector& InPosition) const
{
if (PhysicsHandle->GetPhysicsThreadAPI() == nullptr)
{
return;
}
if (auto Rigid = PhysicsHandle->GetHandle_LowLevel()->CastToRigidParticle())
{
PhysicsHandle->GetPhysicsThreadAPI()->SetX(InPosition);
Rigid->SetP(InPosition); 
// Kinematics do not normal marshall changes back to game thread, mark dirty to ensure game thread
// gets change in position.
if (Rigid->ObjectState() == Chaos::EObjectStateType::Kinematic)
{
PhysicsHandle->GetSolver<Chaos::FPBDRigidsSolver>()->GetParticles().MarkTransientDirtyParticle(Rigid);
}
}
else
{
ensure(false);
}
}
FVector FUpdatedComponentAsyncInput::GetPosition() const
{
if (PhysicsHandle && PhysicsHandle->GetPhysicsThreadAPI())
{
return PhysicsHandle->GetPhysicsThreadAPI()->X();
}
return FVector::ZeroVector;
}
void FUpdatedComponentAsyncInput::SetRotation(const FQuat& InRotation) const
{
if (PhysicsHandle->GetPhysicsThreadAPI() == nullptr)
{
return;
}
if (auto Rigid = PhysicsHandle->GetHandle_LowLevel()->CastToRigidParticle())
{
PhysicsHandle->GetPhysicsThreadAPI()->SetR(InRotation);
Rigid->SetQ(InRotation); 
if (Rigid->ObjectState() == Chaos::EObjectStateType::Kinematic)
{
PhysicsHandle->GetSolver<Chaos::FPBDRigidsSolver>()->GetParticles().MarkTransientDirtyParticle(Rigid);
}
}
else
{
ensure(false);
}
}
FQuat FUpdatedComponentAsyncInput::GetRotation() const
{
if (PhysicsHandle && PhysicsHandle->GetPhysicsThreadAPI())
{
return PhysicsHandle->GetPhysicsThreadAPI()->R();
}
return FQuat::Identity;
}
float FCharacterMovementComponentAsyncInput::SlideAlongSurface(const FVector& Delta, float Time, const FVector& InNormal, FHitResult& Hit, bool bHandleImpact, FCharacterMovementComponentAsyncOutput& Output) const 
{
if (!Hit.bBlockingHit)
{
return 0.f;
}
FVector Normal(InNormal);
if (IsMovingOnGround(Output))
{
// We don't want to be pushed up an unwalkable surface.
if (Normal.Z > 0.f)
{
if (!IsWalkable(Hit))
{
Normal = Normal.GetSafeNormal2D();
}
}
else if (Normal.Z < -UE_KINDA_SMALL_NUMBER)
{
// Don't push down into the floor when the impact is on the upper portion of the capsule.
if (Output.CurrentFloor.FloorDist < UCharacterMovementComponent::MIN_FLOOR_DIST && Output.CurrentFloor.bBlockingHit)
{
const FVector FloorNormal = Output.CurrentFloor.HitResult.Normal;
const bool bFloorOpposedToMovement = (Delta | FloorNormal) < 0.f && (FloorNormal.Z < 1.f - UE_DELTA);
if (bFloorOpposedToMovement)
{
Normal = FloorNormal;
}
Normal = Normal.GetSafeNormal2D();
}
}
}
return MoveComponent_SlideAlongSurface(Delta, Time, Normal, Hit, Output, bHandleImpact);
}
FVector FCharacterMovementComponentAsyncInput::ComputeSlideVector(const FVector& Delta, const float Time, const FVector& Normal, const FHitResult& Hit, FCharacterMovementComponentAsyncOutput& Output) const
{
FVector Result = MoveComponent_ComputeSlideVector(Delta, Time, Normal, Hit, Output);
// prevent boosting up slopes
if (IsFalling(Output))
{
Result = HandleSlopeBoosting(Result, Delta, Time, Normal, Hit, Output);
}
return Result;
}
FVector FCharacterMovementComponentAsyncInput::HandleSlopeBoosting(const FVector& SlideResult, const FVector& Delta, const float Time, const FVector& Normal, const FHitResult& Hit, FCharacterMovementComponentAsyncOutput& Output) const
{
FVector Result = SlideResult;
if (Result.Z > 0.f)
{
// Don't move any higher than we originally intended.
const float ZLimit = Delta.Z * Time;
if (Result.Z - ZLimit > UE_KINDA_SMALL_NUMBER)
{
if (ZLimit > 0.f)
{
// Rescale the entire vector (not just the Z component) otherwise we change the direction and likely head right back into the impact.
const float UpPercent = ZLimit / Result.Z;
Result *= UpPercent;
}
else
{
// We were heading down but were going to deflect upwards. Just make the deflection horizontal.
Result = FVector::ZeroVector;
}
// Make remaining portion of original result horizontal and parallel to impact normal.
const FVector RemainderXY = (SlideResult - Result) * FVector(1.f, 1.f, 0.f);
const FVector NormalXY = Normal.GetSafeNormal2D();
const FVector Adjust = MoveComponent_ComputeSlideVector(RemainderXY, 1.f, NormalXY, Hit, Output); //Super::ComputeSlideVector(RemainderXY, 1.f, NormalXY, Hit);
Result += Adjust;
}
}
return Result;
}
void FCharacterMovementComponentAsyncInput::OnCharacterStuckInGeometry(const FHitResult* Hit, FCharacterMovementComponentAsyncOutput& Output) const 
{
// Don't update velocity based on our (failed) change in position this update since we're stuck.
Output.bJustTeleported = true;
}
bool FCharacterMovementComponentAsyncInput::CanStepUp(const FHitResult& Hit, FCharacterMovementComponentAsyncOutput& Output) const
{
if (!Hit.IsValidBlockingHit() || !bHasValidData || Output.MovementMode == MOVE_Falling)
{
return false;
}
// No component for "fake" hits when we are on a known good base.
const UPrimitiveComponent* HitComponent = Hit.Component.Get();
if (!HitComponent)
{
return true;
}
// No actor for "fake" hits when we are on a known good base.
const AActor* HitActor = Hit.GetActor();
if (!HitActor)
{
return true;
}
return true;
}
bool FCharacterMovementComponentAsyncInput::StepUp(const FVector& GravDir, const FVector& Delta, const FHitResult& InHit, FCharacterMovementComponentAsyncOutput& Output, FStepDownResult* OutStepDownResult) const
{
if (!CanStepUp(InHit, Output) || MaxStepHeight <= 0.f)
{
return false;
}
const FVector OldLocation = UpdatedComponentInput->GetPosition();
float PawnRadius = Output.ScaledCapsuleRadius;
float PawnHalfHeight = Output.ScaledCapsuleHalfHeight;
// Don't bother stepping up if top of capsule is hitting something.
const float InitialImpactZ = InHit.ImpactPoint.Z;
if (InitialImpactZ > OldLocation.Z + (PawnHalfHeight - PawnRadius))
{
return false;
}
if (GravDir.IsZero())
{
return false;
}
FFindFloorResult& CurrentFloor = Output.CurrentFloor;
//float MaxStepHeight = MaxStepHeight;
// Gravity should be a normalized direction
ensure(GravDir.IsNormalized());
float StepTravelUpHeight = MaxStepHeight;
float StepTravelDownHeight = StepTravelUpHeight;
const float StepSideZ = -1.f * FVector::DotProduct(InHit.ImpactNormal, GravDir);
float PawnInitialFloorBaseZ = OldLocation.Z - PawnHalfHeight;
float PawnFloorPointZ = PawnInitialFloorBaseZ;
if (IsMovingOnGround(Output) && CurrentFloor.IsWalkableFloor())
{
// Since we float a variable amount off the floor, we need to enforce max step height off the actual point of impact with the floor.
const float FloorDist = FMath::Max(0.f, CurrentFloor.GetDistanceToFloor());
PawnInitialFloorBaseZ -= FloorDist;
StepTravelUpHeight = FMath::Max(StepTravelUpHeight - FloorDist, 0.f);
StepTravelDownHeight = (MaxStepHeight + UCharacterMovementComponent::MAX_FLOOR_DIST * 2.f);
const bool bHitVerticalFace = !IsWithinEdgeTolerance(InHit.Location, InHit.ImpactPoint, PawnRadius);
if (!CurrentFloor.bLineTrace && !bHitVerticalFace)
{
PawnFloorPointZ = CurrentFloor.HitResult.ImpactPoint.Z;
}
else
{
// Base floor point is the base of the capsule moved down by how far we are hovering over the surface we are hitting.
PawnFloorPointZ -= CurrentFloor.FloorDist;
}
}
// Don't step up if the impact is below us, accounting for distance from floor.
if (InitialImpactZ <= PawnInitialFloorBaseZ)
{
return false;
}
// step up - treat as vertical wall
FHitResult SweepUpHit(1.f);
const FQuat PawnRotation = UpdatedComponentInput->GetRotation();
MoveUpdatedComponent(-GravDir * StepTravelUpHeight, PawnRotation, true, Output, &SweepUpHit);
if (SweepUpHit.bStartPenetrating)
{
ensure(false);
return false;
}
FHitResult Hit(1.f);
MoveUpdatedComponent(Delta, PawnRotation, true, Output, &Hit);
// Check result of forward movement
if (Hit.bBlockingHit)
{
if (Hit.bStartPenetrating)
{
ensure(false);
return false;
}
// If we hit something above us and also something ahead of us, we should notify about the upward hit as well.
// The forward hit will be handled later (in the bSteppedOver case below).
// In the case of hitting something above but not forward, we are not blocked from moving so we don't need the notification.
if (SweepUpHit.bBlockingHit && Hit.bBlockingHit)
{
HandleImpact(SweepUpHit, Output);
}
// pawn ran into a wall
HandleImpact(Hit, Output);
if (IsFalling(Output))
{
return true;
}
// adjust and try again
const float ForwardHitTime = Hit.Time;
const float ForwardSlideAmount = SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, true, Output);
if (IsFalling(Output))
{
ensure(false);
//ScopedStepUpMovement.RevertMove();
return false;
}
// If both the forward hit and the deflection got us nowhere, there is no point in this step up.
if (ForwardHitTime == 0.f && ForwardSlideAmount == 0.f)
{
ensure(false);
return false;
}
}
// Step down
MoveUpdatedComponent(GravDir * StepTravelDownHeight, UpdatedComponentInput->GetRotation(), true, Output, &Hit);
if (Hit.bStartPenetrating)
{
ensure(false);
//ScopedStepUpMovement.RevertMove();
return false;
}
FStepDownResult StepDownResult;
if (Hit.IsValidBlockingHit())
{
// See if this step sequence would have allowed us to travel higher than our max step height allows.
const float DeltaZ = Hit.ImpactPoint.Z - PawnFloorPointZ;
if (DeltaZ > MaxStepHeight)
{
ensure(false);
return false;
}
// Reject unwalkable surface normals here.
if (!IsWalkable(Hit))
{
// Reject if normal opposes movement direction
const bool bNormalTowardsMe = (Delta | Hit.ImpactNormal) < 0.f;
if (bNormalTowardsMe)
{
ensure(false);
return false;
}
// Also reject if we would end up being higher than our starting location by stepping down.
// It's fine to step down onto an unwalkable normal below us, we will just slide off. Rejecting those moves would prevent us from being able to walk off the edge.
if (Hit.Location.Z > OldLocation.Z)
{
ensure(false);
return false;
}
}
// Reject moves where the downward sweep hit something very close to the edge of the capsule. This maintains consistency with FindFloor as well.
if (!IsWithinEdgeTolerance(Hit.Location, Hit.ImpactPoint, PawnRadius))
{
ensure(false);
return false;
}
// Don't step up onto invalid surfaces if traveling higher.
if (DeltaZ > 0.f && !CanStepUp(Hit, Output))
{
ensure(false);
return false;
}
// See if we can validate the floor as a result of this step down. In almost all cases this should succeed, and we can avoid computing the floor outside this method.
if (OutStepDownResult != NULL)
{
FindFloor(UpdatedComponentInput->GetPosition(), StepDownResult.FloorResult, false, Output, &Hit);
// Reject unwalkable normals if we end up higher than our initial height.
// It's fine to walk down onto an unwalkable surface, don't reject those moves.
if (Hit.Location.Z > OldLocation.Z)
{
// We should reject the floor result if we are trying to step up an actual step where we are not able to perch (this is rare).
// In those cases we should instead abort the step up and try to slide along the stair.
if (!StepDownResult.FloorResult.bBlockingHit && StepSideZ < CharacterMovementConstants::MAX_STEP_SIDE_Z)
{
//ScopedStepUpMovement.RevertMove();
ensure(false);
return false;
}
}
StepDownResult.bComputedFloor = true;
}
}
// Copy step down result.
if (OutStepDownResult != NULL)
{
*OutStepDownResult = StepDownResult;
}
// Don't recalculate velocity based on this height adjustment, if considering vertical adjustments.
Output.bJustTeleported |= !bMaintainHorizontalGroundVelocity;
return true;
}
bool FCharacterMovementComponentAsyncInput::CanWalkOffLedges(FCharacterMovementComponentAsyncOutput& Output) const
{
if (!bCanWalkOffLedgesWhenCrouching && Output.bIsCrouched)
{
return false;
}
return bCanWalkOffLedges;
}
FVector FCharacterMovementComponentAsyncInput::GetLedgeMove(const FVector& OldLocation, const FVector& Delta, const FVector& GravDir, FCharacterMovementComponentAsyncOutput& Output) const
{
if (!bHasValidData || Delta.IsZero())
{
return FVector::ZeroVector;
}
FVector SideDir(Delta.Y, -1.f * Delta.X, 0.f);
if (CheckLedgeDirection(OldLocation, SideDir, GravDir, Output))
{
return SideDir;
}
SideDir *= -1.f;
if (CheckLedgeDirection(OldLocation, SideDir, GravDir, Output))
{
return SideDir;
}
return FVector::ZeroVector;
}
bool FCharacterMovementComponentAsyncInput::CheckLedgeDirection(const FVector& OldLocation, const FVector& SideStep, const FVector& GravDir, FCharacterMovementComponentAsyncOutput& Output) const
{
const FVector SideDest = OldLocation + SideStep;
const FCollisionShape CapsuleShape = GetPawnCapsuleCollisionShape(EShrinkCapsuleExtent::SHRINK_None, Output);
FHitResult Result(1.f);
World->SweepSingleByChannel(Result, OldLocation, SideDest, FQuat::Identity, CollisionChannel, CapsuleShape, QueryParams, CollisionResponseParams);
if (!Result.bBlockingHit || IsWalkable(Result))
{
if (!Result.bBlockingHit)
{
World->SweepSingleByChannel(Result, SideDest, SideDest + GravDir * (MaxStepHeight + LedgeCheckThreshold), FQuat::Identity, CollisionChannel, CapsuleShape, QueryParams, CollisionResponseParams);
}
if ((Result.Time < 1.f) && IsWalkable(Result))
{
return true;
}
}
return false;
}
FVector FCharacterMovementComponentAsyncInput::GetPawnCapsuleExtent(const EShrinkCapsuleExtent ShrinkMode, const float CustomShrinkAmount, FCharacterMovementComponentAsyncOutput& Output) const
{
float Radius = Output.ScaledCapsuleRadius;
float HalfHeight = Output.ScaledCapsuleHalfHeight;
FVector CapsuleExtent(Radius, Radius, HalfHeight);
float RadiusEpsilon = 0.f;
float HeightEpsilon = 0.f;
switch (ShrinkMode)
{
case EShrinkCapsuleExtent::SHRINK_None:
return CapsuleExtent;
case EShrinkCapsuleExtent::SHRINK_RadiusCustom:
RadiusEpsilon = CustomShrinkAmount;
break;
case EShrinkCapsuleExtent::SHRINK_HeightCustom:
HeightEpsilon = CustomShrinkAmount;
break;
case EShrinkCapsuleExtent::SHRINK_AllCustom:
RadiusEpsilon = CustomShrinkAmount;
HeightEpsilon = CustomShrinkAmount;
break;
default:
ensure(false);
break;
}
// Don't shrink to zero extent.
const float MinExtent = UE_KINDA_SMALL_NUMBER * 10.f;
CapsuleExtent.X = FMath::Max(CapsuleExtent.X - RadiusEpsilon, MinExtent);
CapsuleExtent.Y = CapsuleExtent.X;
CapsuleExtent.Z = FMath::Max(CapsuleExtent.Z - HeightEpsilon, MinExtent);
return CapsuleExtent;
}
FCollisionShape FCharacterMovementComponentAsyncInput::GetPawnCapsuleCollisionShape(const EShrinkCapsuleExtent ShrinkMode, FCharacterMovementComponentAsyncOutput& Output, const float CustomShrinkAmount) const
{
FVector Extent = GetPawnCapsuleExtent(ShrinkMode, CustomShrinkAmount, Output);
return FCollisionShape::MakeCapsule(Extent);
}
void FCharacterMovementComponentAsyncInput::TwoWallAdjust(FVector& OutDelta, const FHitResult& Hit, const FVector& OldHitNormal, FCharacterMovementComponentAsyncOutput& Output) const
{
FVector Delta = OutDelta;
const FVector HitNormal = Hit.Normal;
if ((OldHitNormal | HitNormal) <= 0.f) //90 or less corner, so use cross product for direction
{
const FVector DesiredDir = Delta;
FVector NewDir = (HitNormal ^ OldHitNormal);
NewDir = NewDir.GetSafeNormal();
Delta = (Delta | NewDir) * (1.f - Hit.Time) * NewDir;
if ((DesiredDir | Delta) < 0.f)
{
Delta = -1.f * Delta;
}
}
else //adjust to new wall
{
const FVector DesiredDir = Delta;
Delta = ComputeSlideVector(Delta, 1.f - Hit.Time, HitNormal, Hit, Output);
if ((Delta | DesiredDir) <= 0.f)
{
Delta = FVector::ZeroVector;
}
else if (FMath::Abs((HitNormal | OldHitNormal) - 1.f) < UE_KINDA_SMALL_NUMBER)
{
// we hit the same wall again even after adjusting to move along it the first time
// nudge away from it (this can happen due to precision issues)
Delta += HitNormal * 0.01f;
}
}
OutDelta = Delta;
}
void FCharacterMovementComponentAsyncInput::RevertMove(const FVector& OldLocation, UPrimitiveComponent* OldBase, const FVector& PreviousBaseLocation, const FFindFloorResult& OldFloor, bool bFailMove, FCharacterMovementComponentAsyncOutput& Output) const
{
ETeleportType TeleportType = GetTeleportType(Output);
ensure(TeleportType == ETeleportType::TeleportPhysics);
UpdatedComponentInput->SetPosition(OldLocation);
Output.bJustTeleported = false;
// We can't read off movement base, ensure that our base hasn't changed, and use cached data.
ensure(MovementBaseAsyncData.CachedMovementBase == OldBase);
MovementBaseAsyncData.Validate(Output);
ensure(false);
}
ETeleportType FCharacterMovementComponentAsyncInput::GetTeleportType(FCharacterMovementComponentAsyncOutput& Output) const
{
// ensuring in inputs on networek large correction
return Output.bJustTeleported  ? ETeleportType::TeleportPhysics : ETeleportType::None;
}
void FCharacterMovementComponentAsyncInput::HandleWalkingOffLedge(const FVector& PreviousFloorImpactNormal, const FVector& PreviousFloorContactNormal, const FVector& PreviousLocation, float TimeDelta) const
{
ensure(false);
}
bool FCharacterMovementComponentAsyncInput::ShouldCatchAir(const FFindFloorResult& OldFloor, const FFindFloorResult& NewFloor) const
{
return false;
}
void FCharacterMovementComponentAsyncInput::StartFalling(int32 Iterations, float remainingTime, float timeTick, const FVector& Delta, const FVector& subLoc, FCharacterMovementComponentAsyncOutput& Output) const
{
// start falling 
const float DesiredDist = Delta.Size();
const float ActualDist = (UpdatedComponentInput->GetPosition() - subLoc).Size2D();
remainingTime = (DesiredDist < UE_KINDA_SMALL_NUMBER)
? 0.f
: remainingTime + timeTick * (1.f - FMath::Min(1.f, ActualDist / DesiredDist));
if (IsMovingOnGround(Output))
{
// Probably don't need this hack with async physics.
// This is to catch cases where the first frame of PIE is executed, and the
// level is not yet visible. In those cases, the player will fall out of the
// world... So, don't set MOVE_Falling straight away.
if (!GIsEditor || (World->HasBegunPlay() && (World->GetTimeSeconds() >= 1.f)))
{
SetMovementMode(MOVE_Falling, Output); //default behavior if script didn't change physics
}
else
{
// Make sure that the floor check code continues processing during this delay.
Output.bForceNextFloorCheck = true;
}
}
StartNewPhysics(remainingTime, Iterations, Output);
}
void FCharacterMovementComponentAsyncInput::SetBaseFromFloor(const FFindFloorResult& FloorResult, FCharacterMovementComponentAsyncOutput& Output) const
{
if (FloorResult.IsWalkableFloor())
{
//SetBase(FloorResult.HitResult.GetComponent(), FloorResult.HitResult.BoneName);
Output.NewMovementBase = FloorResult.HitResult.GetComponent();
Output.NewMovementBaseOwner = FloorResult.HitResult.GetActor();
}
else
{
//SetBase(nullptr);
Output.NewMovementBase = nullptr;
Output.NewMovementBaseOwner = nullptr;
}
}
void FCharacterMovementComponentAsyncInput::AdjustFloorHeight(FCharacterMovementComponentAsyncOutput& Output) const
{
FFindFloorResult& CurrentFloor = Output.CurrentFloor;
// If we have a floor check that hasn't hit anything, don't adjust height.
if (!CurrentFloor.IsWalkableFloor())
{
return;
}
float OldFloorDist = CurrentFloor.FloorDist;
if (CurrentFloor.bLineTrace)
{
if (OldFloorDist < UCharacterMovementComponent::MIN_FLOOR_DIST && CurrentFloor.LineDist >= UCharacterMovementComponent::MIN_FLOOR_DIST)
{
return;
}
else
{
// Falling back to a line trace means the sweep was unwalkable (or in penetration). Use the line distance for the vertical adjustment.
OldFloorDist = CurrentFloor.LineDist;
}
}
// Move up or down to maintain floor height.
if (OldFloorDist < UCharacterMovementComponent::MIN_FLOOR_DIST || OldFloorDist > UCharacterMovementComponent::MAX_FLOOR_DIST)
{
FHitResult AdjustHit(1.f);
const float InitialZ = UpdatedComponentInput->GetPosition().Z;
const float AvgFloorDist = (UCharacterMovementComponent::MIN_FLOOR_DIST + UCharacterMovementComponent::MAX_FLOOR_DIST) * 0.5f;
const float MoveDist = AvgFloorDist - OldFloorDist;
SafeMoveUpdatedComponent(FVector(0.f, 0.f, MoveDist), UpdatedComponentInput->GetRotation(), true, AdjustHit, Output);
if (!AdjustHit.IsValidBlockingHit())
{
CurrentFloor.FloorDist += MoveDist;
}
else if (MoveDist > 0.f)
{
const float CurrentZ = UpdatedComponentInput->GetPosition().Z;
CurrentFloor.FloorDist += CurrentZ - InitialZ;
}
else
{
checkSlow(MoveDist < 0.f);
const float CurrentZ = UpdatedComponentInput->GetPosition().Z;
CurrentFloor.FloorDist = CurrentZ - AdjustHit.Location.Z;
if (IsWalkable(AdjustHit))
{
CurrentFloor.SetFromSweep(AdjustHit, CurrentFloor.FloorDist, true);
}
}
// Don't recalculate velocity based on this height adjustment, if considering vertical adjustments.
// Also avoid it if we moved out of penetration
Output.bJustTeleported |= !bMaintainHorizontalGroundVelocity || (OldFloorDist < 0.f);
// If something caused us to adjust our height (especially a depentration) we should ensure another check next frame or we will keep a stale result.
if (CharacterInput->LocalRole != ROLE_SimulatedProxy)
{
Output.bForceNextFloorCheck = true;
}
}
}
bool FCharacterMovementComponentAsyncInput::ShouldComputePerchResult(const FHitResult& InHit, FCharacterMovementComponentAsyncOutput& Output, bool bCheckRadius) const
{
if (!InHit.IsValidBlockingHit())
{
return false;
}
// Don't try to perch if the edge radius is very small.
if (GetPerchRadiusThreshold() <= UCharacterMovementComponent::SWEEP_EDGE_REJECT_DISTANCE)
{
return false;
}
if (bCheckRadius)
{
const float DistFromCenterSq = (InHit.ImpactPoint - InHit.Location).SizeSquared2D();
const float StandOnEdgeRadius = GetValidPerchRadius(Output);
if (DistFromCenterSq <= FMath::Square(StandOnEdgeRadius))
{
// Already within perch radius.
return false;
}
}
return true;
}
bool FCharacterMovementComponentAsyncInput::ComputePerchResult(const float TestRadius, const FHitResult& InHit, const float InMaxFloorDist, FFindFloorResult& OutPerchFloorResult, FCharacterMovementComponentAsyncOutput& Output) const
{
if (InMaxFloorDist <= 0.f)
{
return false;
}
// Sweep further than actual requested distance, because a reduced capsule radius means we could miss some hits that the normal radius would contact.
const float PawnRadius = Output.ScaledCapsuleRadius;
const float PawnHalfHeight = Output.ScaledCapsuleHalfHeight;
//CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);
const float InHitAboveBase = FMath::Max(0.f, InHit.ImpactPoint.Z - (InHit.Location.Z - PawnHalfHeight));
const float PerchLineDist = FMath::Max(0.f, InMaxFloorDist - InHitAboveBase);
const float PerchSweepDist = FMath::Max(0.f, InMaxFloorDist);
const float ActualSweepDist = PerchSweepDist + PawnRadius;
ComputeFloorDist(InHit.Location, PerchLineDist, ActualSweepDist, OutPerchFloorResult, TestRadius, Output);
if (!OutPerchFloorResult.IsWalkableFloor())
{
return false;
}
else if (InHitAboveBase + OutPerchFloorResult.FloorDist > InMaxFloorDist)
{
// Hit something past max distance
OutPerchFloorResult.bWalkableFloor = false;
return false;
}
return true;
}
float FCharacterMovementComponentAsyncInput::GetPerchRadiusThreshold() const
{
// Don't allow negative values.
return FMath::Max(0.f, PerchRadiusThreshold);
}
float FCharacterMovementComponentAsyncInput::GetValidPerchRadius(const FCharacterMovementComponentAsyncOutput& Output) const
{
const float PawnRadius = Output.ScaledCapsuleRadius;
return FMath::Clamp(PawnRadius - GetPerchRadiusThreshold(), 0.11f, PawnRadius);
}
bool FCharacterMovementComponentAsyncInput::CheckFall(const FFindFloorResult& OldFloor, const FHitResult& Hit, const FVector& Delta, const FVector& OldLocation, float remainingTime, float timeTick, int32 Iterations, bool bMustJump, FCharacterMovementComponentAsyncOutput& Output) const
{
if (!bHasValidData)
{
return false;
}
if (bMustJump || CanWalkOffLedges(Output))
{
HandleWalkingOffLedge(OldFloor.HitResult.ImpactNormal, OldFloor.HitResult.Normal, OldLocation, timeTick);
if (IsMovingOnGround(Output))
{
// If still walking, then fall. If not, assume the user set a different mode they want to keep.
StartFalling(Iterations, remainingTime, timeTick, Delta, OldLocation, Output);
}
return true;
}
return false;
}
FVector FCharacterMovementComponentAsyncInput::GetFallingLateralAcceleration(float DeltaTime, FCharacterMovementComponentAsyncOutput& Output) const
{
// No acceleration in Z
FVector FallAcceleration = FVector(Output.Acceleration.X, Output.Acceleration.Y, 0.f);
// bound acceleration, falling object has minimal ability to impact acceleration
if (!RootMotion.bHasAnimRootMotion && FallAcceleration.SizeSquared2D() > 0.f)
{
FallAcceleration = GetAirControl(DeltaTime, AirControl, FallAcceleration, Output);
FallAcceleration = FallAcceleration.GetClampedToMaxSize(MaxAcceleration);
}
return FallAcceleration;
}
float FCharacterMovementComponentAsyncInput::BoostAirControl(float DeltaTime, float TickAirControl, const FVector& FallAcceleration, FCharacterMovementComponentAsyncOutput& Output) const
{
// Allow a burst of initial acceleration
if (AirControlBoostMultiplier > 0.f && Output.Velocity.SizeSquared2D() < FMath::Square(AirControlBoostVelocityThreshold))
{
TickAirControl = FMath::Min(1.f, AirControlBoostMultiplier * TickAirControl);
}
return TickAirControl;
}
bool FCharacterMovementComponentAsyncInput::ShouldLimitAirControl(float DeltaTime, const FVector& FallAcceleration, FCharacterMovementComponentAsyncOutput& Output) const
{
return (FallAcceleration.SizeSquared2D() > 0.f);
}
FVector FCharacterMovementComponentAsyncInput::LimitAirControl(float DeltaTime, const FVector& FallAcceleration, const FHitResult& HitResult, bool bCheckForValidLandingSpot, FCharacterMovementComponentAsyncOutput& Output) const
{
FVector Result(FallAcceleration);
if (HitResult.IsValidBlockingHit() && HitResult.Normal.Z > CharacterMovementConstants::VERTICAL_SLOPE_NORMAL_Z)
{
if (!bCheckForValidLandingSpot || !IsValidLandingSpot(HitResult.Location, HitResult, Output))
{
// If acceleration is into the wall, limit contribution.
if (FVector::DotProduct(FallAcceleration, HitResult.Normal) < 0.f)
{
// Allow movement parallel to the wall, but not into it because that may push us up.
const FVector Normal2D = HitResult.Normal.GetSafeNormal2D();
Result = FVector::VectorPlaneProject(FallAcceleration, Normal2D);
}
}
}
else if (HitResult.bStartPenetrating)
{
// Allow movement out of penetration.
return (FVector::DotProduct(Result, HitResult.Normal) > 0.f ? Result : FVector::ZeroVector);
}
return Result;
}
FVector FCharacterMovementComponentAsyncInput::NewFallVelocity(const FVector& InitialVelocity, const FVector& Gravity, float DeltaTime, FCharacterMovementComponentAsyncOutput& Output) const
{
FVector Result = InitialVelocity;
if (DeltaTime > 0.f)
{
Result += Gravity * DeltaTime;
// Don't exceed terminal velocity.
const float TerminalLimit = FMath::Abs(PhysicsVolumeTerminalVelocity);
if (Result.SizeSquared() > FMath::Square(TerminalLimit))
{
const FVector GravityDir = Gravity.GetSafeNormal();
if ((Result | GravityDir) > TerminalLimit)
{
Result = FVector::PointPlaneProject(Result, FVector::ZeroVector, GravityDir) + GravityDir * TerminalLimit;
}
}
}
return Result;
}
bool FCharacterMovementComponentAsyncInput::IsValidLandingSpot(const FVector& CapsuleLocation, const FHitResult& Hit, FCharacterMovementComponentAsyncOutput& Output) const
{
if (!Hit.bBlockingHit)
{
return false;
}
// Skip some checks if penetrating. Penetration will be handled by the FindFloor call (using a smaller capsule)
if (!Hit.bStartPenetrating)
{
// Reject unwalkable floor normals.
if (!IsWalkable(Hit))
{
return false;
}
float PawnRadius = Output.ScaledCapsuleRadius;
float PawnHalfHeight = Output.ScaledCapsuleHalfHeight;
// Reject hits that are above our lower hemisphere (can happen when sliding down a vertical surface).
const float LowerHemisphereZ = Hit.Location.Z - PawnHalfHeight + PawnRadius;
if (Hit.ImpactPoint.Z >= LowerHemisphereZ)
{
return false;
}
// Reject hits that are barely on the cusp of the radius of the capsule
if (!IsWithinEdgeTolerance(Hit.Location, Hit.ImpactPoint, PawnRadius))
{
return false;
}
}
else
{
// Penetrating
if (Hit.Normal.Z < UE_KINDA_SMALL_NUMBER)
{
// Normal is nearly horizontal or downward, that's a penetration adjustment next to a vertical or overhanging wall. Don't pop to the floor.
return false;
}
}
FFindFloorResult FloorResult;
FindFloor(CapsuleLocation, FloorResult, false, Output, &Hit);
if (!FloorResult.IsWalkableFloor())
{
return false;
}
return true;
}
void FCharacterMovementComponentAsyncInput::ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations, FCharacterMovementComponentAsyncOutput& Output) const
{
if (IsFalling(Output))
{
SetPostLandedPhysics(Hit, Output);
}
StartNewPhysics(remainingTime, Iterations, Output);
}
void FCharacterMovementComponentAsyncInput::SetPostLandedPhysics(const FHitResult& Hit, FCharacterMovementComponentAsyncOutput& Output) const
{
{
const FVector PreImpactAccel = Output.Acceleration + (IsFalling(Output) ? FVector(0.f, 0.f, GravityZ) : FVector::ZeroVector);
const FVector PreImpactVelocity = Output.Velocity;
if (DefaultLandMovementMode == MOVE_Walking ||
DefaultLandMovementMode == MOVE_NavWalking ||
DefaultLandMovementMode == MOVE_Falling)
{
SetMovementMode(Output.GroundMovementMode, Output);
}
else
{
SetDefaultMovementMode(Output);
}
}
}
void FCharacterMovementComponentAsyncInput::SetDefaultMovementMode(FCharacterMovementComponentAsyncOutput& Output) const
{
if (Output.MovementMode != DefaultLandMovementMode)
{
const float SavedVelocityZ = Output.Velocity.Z;
SetMovementMode(DefaultLandMovementMode, Output);
// Avoid 1-frame delay if trying to walk but walking fails at this location.
if (Output.MovementMode == MOVE_Walking && Output.NewMovementBase == NULL)
{
Output.Velocity.Z = SavedVelocityZ; // Prevent temporary walking state from zeroing Z velocity.
SetMovementMode(MOVE_Falling, Output);
}
}
}
bool FCharacterMovementComponentAsyncInput::ShouldCheckForValidLandingSpot(float DeltaTime, const FVector& Delta, const FHitResult& Hit, FCharacterMovementComponentAsyncOutput& Output) const
{
// See if we hit an edge of a surface on the lower portion of the capsule.
// In this case the normal will not equal the impact normal, and a downward sweep may find a walkable surface on top of the edge.
if (Hit.Normal.Z > UE_KINDA_SMALL_NUMBER && !Hit.Normal.Equals(Hit.ImpactNormal))
{
const FVector PawnLocation = UpdatedComponentInput->GetPosition();
if (IsWithinEdgeTolerance(PawnLocation, Hit.ImpactPoint, Output.ScaledCapsuleHalfHeight))
{
return true;
}
}
return false;
}
bool FCharacterMovementComponentAsyncInput::ShouldRemainVertical(FCharacterMovementComponentAsyncOutput& Output) const
{
// Always remain vertical when walking or falling.
return IsMovingOnGround(Output) || IsFalling(Output);
}
bool FCharacterMovementComponentAsyncInput::CanAttemptJump(FCharacterMovementComponentAsyncOutput& Output) const
{
return IsJumpAllowed() &&
!Output.bWantsToCrouch &&
(IsMovingOnGround(Output) || IsFalling(Output));  // Falling included for double-jump and non-zero jump hold time, but validated by character.
}
bool FCharacterMovementComponentAsyncInput::DoJump(bool bReplayingMoves, FCharacterMovementComponentAsyncOutput& Output) const
{
if ( CharacterInput->CanJump(*this, Output))
{
// Don't jump if we can't move up/down.
if (!bConstrainToPlane || FMath::Abs(PlaneConstraintNormal.Z) != 1.f)
{
Output.Velocity.Z = FMath::Max(Output.Velocity.Z, JumpZVelocity);
SetMovementMode(MOVE_Falling, Output);
return true;
}
}
return false;
}
bool FCharacterMovementComponentAsyncInput::IsJumpAllowed() const
{
return bNavAgentPropsCanJump && bMovementStateCanJump;
}
float FCharacterMovementComponentAsyncInput::GetMaxSpeed(FCharacterMovementComponentAsyncOutput& Output) const
{
switch (Output.MovementMode)
{
case MOVE_Walking:
case MOVE_NavWalking:
return IsCrouching(Output) ? MaxWalkSpeedCrouched : MaxWalkSpeed;
case MOVE_Falling:
return MaxWalkSpeed;
case MOVE_Swimming:
return MaxSwimSpeed;
case MOVE_Flying:
return MaxFlySpeed;
case MOVE_Custom:
return MaxCustomMovementSpeed;
case MOVE_None:
default:
return 0.f;
}
}
bool FCharacterMovementComponentAsyncInput::IsCrouching(const FCharacterMovementComponentAsyncOutput& Output) const
{
return Output.bIsCrouched;
}
bool FCharacterMovementComponentAsyncInput::IsFalling(const FCharacterMovementComponentAsyncOutput& Output) const
{
return (Output.MovementMode == MOVE_Falling);
}
bool FCharacterMovementComponentAsyncInput::IsFlying(const FCharacterMovementComponentAsyncOutput& Output) const
{
return (Output.MovementMode == MOVE_Flying);
}
bool FCharacterMovementComponentAsyncInput::IsMovingOnGround(const FCharacterMovementComponentAsyncOutput& Output) const
{
return ((Output.MovementMode == MOVE_Walking) || (Output.MovementMode == MOVE_NavWalking));
}
bool FCharacterMovementComponentAsyncInput::IsExceedingMaxSpeed(float MaxSpeed, const FCharacterMovementComponentAsyncOutput& Output) const
{
MaxSpeed = FMath::Max(0.f, MaxSpeed);
const float MaxSpeedSquared = FMath::Square(MaxSpeed);
// Allow 1% error tolerance, to account for numeric imprecision.
const float OverVelocityPercent = 1.01f;
return (Output.Velocity.SizeSquared() > MaxSpeedSquared * OverVelocityPercent);
}
FRotator FCharacterMovementComponentAsyncInput::ComputeOrientToMovementRotation(const FRotator& CurrentRotation, float DeltaTime, FRotator& DeltaRotation, FCharacterMovementComponentAsyncOutput& Output) const
{
if (Output.Acceleration.SizeSquared() < UE_KINDA_SMALL_NUMBER)
{
// AI path following request can orient us in that direction (it's effectively an acceleration)
if (Output.bHasRequestedVelocity && Output.RequestedVelocity.SizeSquared() > UE_KINDA_SMALL_NUMBER)
{
return Output.RequestedVelocity.GetSafeNormal().Rotation();
}
// Don't change rotation if there is no acceleration.
return CurrentRotation;
}
// Rotate toward direction of acceleration.
return Output.Acceleration.GetSafeNormal().Rotation();
}
void FCharacterMovementComponentAsyncInput::RestorePreAdditiveRootMotionVelocity(FCharacterMovementComponentAsyncOutput& Output) const
{
// Restore last frame's pre-additive Velocity if we had additive applied 
// so that we're not adding more additive velocity than intended
if (Output.bIsAdditiveVelocityApplied)
{
Output.Velocity = Output.LastPreAdditiveVelocity;
Output.bIsAdditiveVelocityApplied = false;
}
}
void FCharacterMovementComponentAsyncInput::ApplyRootMotionToVelocity(float deltaTime, FCharacterMovementComponentAsyncOutput& Output) const
{
// Animation root motion is distinct from root motion sources right now and takes precedence
if (RootMotion.bHasAnimRootMotion && deltaTime > 0.f)
{
Output.Velocity = ConstrainAnimRootMotionVelocity(Output.AnimRootMotionVelocity, Output.Velocity, Output);
return;
}
const FVector OldVelocity = Output.Velocity;
bool bAppliedRootMotion = false;
// Apply override velocity
if (RootMotion.bHasOverrideRootMotion)
{
Output.Velocity = RootMotion.OverrideVelocity;
bAppliedRootMotion = true;
#if ROOT_MOTION_DEBUG
if (RootMotionSourceDebug::CVarDebugRootMotionSources.GetValueOnGameThread() == 1)
{
FString AdjustedDebugString = FString::Printf(TEXT("ApplyRootMotionToVelocity HasOverrideVelocity Velocity(%s)"),
*Output.Velocity.ToCompactString());
}
#endif
}
// Next apply additive root motion
if (RootMotion.bHasAdditiveRootMotion)
{
Output.LastPreAdditiveVelocity = Output.Velocity; // Save off pre-additive Velocity for restoration next tick
Output.Velocity += RootMotion.AdditiveVelocity;
Output.bIsAdditiveVelocityApplied = true; // Remember that we have it applied
bAppliedRootMotion = true;
}
// Switch to Falling if we have vertical velocity from root motion so we can lift off the ground
const FVector AppliedVelocityDelta = Output.Velocity - OldVelocity;
if (bAppliedRootMotion && AppliedVelocityDelta.Z != 0.f && IsMovingOnGround(Output))
{
float LiftoffBound;
if (RootMotion.bUseSensitiveLiftoff)
{
// Sensitive bounds - "any positive force"a
LiftoffBound = UE_SMALL_NUMBER;
}
else
{
// Default bounds - the amount of force gravity is applying this tick
LiftoffBound = FMath::Max(GravityZ * deltaTime, UE_SMALL_NUMBER);
}
if (AppliedVelocityDelta.Z > LiftoffBound)
{
SetMovementMode(MOVE_Falling, Output);
}
}
}
FVector FCharacterMovementComponentAsyncInput::ConstrainAnimRootMotionVelocity(const FVector& RootMotionVelocity, const FVector& CurrentVelocity, FCharacterMovementComponentAsyncOutput& Output) const
{
FVector Result = RootMotionVelocity;
// Do not override Velocity.Z if in falling physics, we want to keep the effect of gravity.
if (IsFalling(Output))
{
Result.Z = CurrentVelocity.Z;
}
return Result;
}
FVector FCharacterMovementComponentAsyncInput::CalcAnimRootMotionVelocity(const FVector& RootMotionDeltaMove, float DeltaSeconds, const FVector& CurrentVelocity) const
{
if (ensure(DeltaSeconds > 0.f))
{
FVector RootMotionVelocity = RootMotionDeltaMove / DeltaSeconds;
return RootMotionVelocity;
}
else
{
return CurrentVelocity;
}
}
FVector FCharacterMovementComponentAsyncInput::GetAirControl(float DeltaTime, float TickAirControl, const FVector& FallAcceleration, FCharacterMovementComponentAsyncOutput& Output) const
{
// Boost
if (TickAirControl != 0.f)
{
TickAirControl = BoostAirControl(DeltaTime, TickAirControl, FallAcceleration, Output);
}
return TickAirControl * FallAcceleration;
}
void FCharacterAsyncInput::FaceRotation(FRotator NewControlRotation, float DeltaTime, const FCharacterMovementComponentAsyncInput& Input, FCharacterMovementComponentAsyncOutput& Output) const
{
// Only if we actually are going to use any component of rotation.
if (bUseControllerRotationPitch || bUseControllerRotationYaw || bUseControllerRotationRoll)
{
FRotator& CurrentRotation = Output.CharacterOutput->Rotation;
if (!bUseControllerRotationPitch)
{
NewControlRotation.Pitch = CurrentRotation.Pitch;
}
if (!bUseControllerRotationYaw)
{
NewControlRotation.Yaw = CurrentRotation.Yaw;
}
if (!bUseControllerRotationRoll)
{
NewControlRotation.Roll = CurrentRotation.Roll;
}
CurrentRotation = NewControlRotation;
}
}
void FCharacterAsyncInput::CheckJumpInput(float DeltaSeconds, const FCharacterMovementComponentAsyncInput& Input, FCharacterMovementComponentAsyncOutput& Output) const
{
Output.CharacterOutput->JumpCurrentCountPreJump = Output.CharacterOutput->JumpCurrentCount;
if (Output.CharacterOutput->bPressedJump)
{
// If this is the first jump and we're already falling,
// then increment the JumpCount to compensate.
const bool bFirstJump = Output.CharacterOutput->JumpCurrentCount == 0;
if (bFirstJump && Input.IsFalling(Output))
{
Output.CharacterOutput->JumpCurrentCount++;
}
// TODO bClientUpdating
const bool bDidJump = CanJump(Input, Output) && Input.DoJump(/*bClientUpdating*/false, Output);
if (bDidJump)
{
if (!Output.CharacterOutput->bWasJumping)
{
Output.CharacterOutput->JumpCurrentCount++;
Output.CharacterOutput->JumpForceTimeRemaining = JumpMaxHoldTime;
}
}
Output.CharacterOutput->bWasJumping = bDidJump;
}
}
void FCharacterAsyncInput::ClearJumpInput(float DeltaSeconds, const FCharacterMovementComponentAsyncInput& Input, FCharacterMovementComponentAsyncOutput& Output) const
{
if (Output.CharacterOutput->bPressedJump)
{
Output.CharacterOutput->JumpKeyHoldTime += DeltaSeconds;
// Don't disable bPressedJump right away if it's still held.
// Don't modify JumpForceTimeRemaining because a frame of update may be remaining.
if (Output.CharacterOutput->JumpKeyHoldTime >= JumpMaxHoldTime)
{
Output.CharacterOutput->bClearJumpInput = true;
Output.CharacterOutput->bPressedJump = false;
}
}
else
{
Output.CharacterOutput->JumpForceTimeRemaining = 0.0f;
Output.CharacterOutput->bWasJumping = false;
}
}
bool FCharacterAsyncInput::CanJump(const FCharacterMovementComponentAsyncInput& Input, FCharacterMovementComponentAsyncOutput& Output) const
{
// Ensure the character isn't currently crouched.
bool bCanJump = !Output.bIsCrouched;
// Ensure that the CharacterMovement state is valid
bCanJump &= Input.CanAttemptJump(Output);
if (bCanJump)
{
// Ensure JumpHoldTime and JumpCount are valid.
if (!Output.CharacterOutput->bWasJumping || Input.CharacterInput->JumpMaxHoldTime <= 0.0f)
{
if (Output.CharacterOutput->JumpCurrentCount == 0 && Input.IsFalling(Output))
{
bCanJump = Output.CharacterOutput->JumpCurrentCount + 1 < Input.CharacterInput->JumpMaxCount;
}
else
{
bCanJump = Output.CharacterOutput->JumpCurrentCount < Input.CharacterInput->JumpMaxCount;
}
}
else
{
const bool bJumpKeyHeld = (Output.CharacterOutput->bPressedJump && Output.CharacterOutput->JumpKeyHoldTime < Input.CharacterInput->JumpMaxHoldTime);
bCanJump = bJumpKeyHeld &&
((Output.CharacterOutput->JumpCurrentCount < Input.CharacterInput->JumpMaxCount) || (Output.CharacterOutput->bWasJumping && Output.CharacterOutput->JumpCurrentCount == Input.CharacterInput->JumpMaxCount));
}
}
return bCanJump;
}
void FCharacterAsyncInput::ResetJumpState(const FCharacterMovementComponentAsyncInput& Input, FCharacterMovementComponentAsyncOutput& Output) const
{
if (Output.CharacterOutput->bPressedJump == true)
{
Output.CharacterOutput->bClearJumpInput = true;
}
Output.CharacterOutput->bPressedJump = false;
Output.CharacterOutput->bWasJumping = false;
Output.CharacterOutput->JumpKeyHoldTime = 0.0f;
Output.CharacterOutput->JumpForceTimeRemaining = 0.0f;
if (!Input.IsFalling(Output))
{
Output.CharacterOutput->JumpCurrentCount = 0;
Output.CharacterOutput->JumpCurrentCountPreJump = 0;
}
}
void FCharacterAsyncInput::OnMovementModeChanged(EMovementMode PrevMovementMode, const FCharacterMovementComponentAsyncInput& Input, FCharacterMovementComponentAsyncOutput& Output, uint8 PreviousCustomMode)
{
if (!Output.CharacterOutput->bPressedJump || !Input.IsFalling(Output))
{
ResetJumpState(Input, Output);
}
}
FName FCharacterMovementComponentAsyncCallback::GetFNameForStatId() const
{
const static FLazyName StaticName("FCharacterMovementComponentAsyncCallback");
return StaticName;
}
void FCharacterMovementComponentAsyncCallback::OnPreSimulate_Internal()
{
PreSimulateImpl<FCharacterMovementComponentAsyncInput, FCharacterMovementComponentAsyncOutput>(*this);
}
void FCharacterMovementComponentAsyncOutput::Copy(const FCharacterMovementComponentAsyncOutput& Value)
{
bIsValid = Value.bIsValid;
bWasSimulatingRootMotion = Value.bWasSimulatingRootMotion;
MovementMode = Value.MovementMode;
GroundMovementMode = Value.GroundMovementMode;
CustomMovementMode = Value.CustomMovementMode;
Acceleration = Value.Acceleration;
AnalogInputModifier = Value.AnalogInputModifier;
LastUpdateLocation = Value.LastUpdateLocation;
LastUpdateRotation = Value.LastUpdateRotation;
LastUpdateVelocity = Value.LastUpdateVelocity;
bForceNextFloorCheck = Value.bForceNextFloorCheck;
Velocity = Value.Velocity;
LastPreAdditiveVelocity = Value.LastPreAdditiveVelocity;
bIsAdditiveVelocityApplied = Value.bIsAdditiveVelocityApplied;
bDeferUpdateBasedMovement = Value.bDeferUpdateBasedMovement;
MoveComponentFlags = Value.MoveComponentFlags;
PendingForceToApply = Value.PendingForceToApply;
PendingImpulseToApply = Value.PendingImpulseToApply;
PendingLaunchVelocity = Value.PendingLaunchVelocity;
bCrouchMaintainsBaseLocation = Value.bCrouchMaintainsBaseLocation;
bJustTeleported = Value.bJustTeleported;
ScaledCapsuleRadius = Value.ScaledCapsuleRadius;
ScaledCapsuleHalfHeight = Value.ScaledCapsuleHalfHeight;
bIsCrouched = Value.bIsCrouched;
bWantsToCrouch = Value.bWantsToCrouch;
bMovementInProgress = Value.bMovementInProgress;
CurrentFloor = Value.CurrentFloor;
bHasRequestedVelocity = Value.bHasRequestedVelocity;
bRequestedMoveWithMaxSpeed = Value.bRequestedMoveWithMaxSpeed;
RequestedVelocity = Value.RequestedVelocity;
LastUpdateRequestedVelocity = Value.LastUpdateRequestedVelocity;
NumJumpApexAttempts = Value.NumJumpApexAttempts;
AnimRootMotionVelocity = Value.AnimRootMotionVelocity;
bShouldApplyDeltaToMeshPhysicsTransforms = Value.bShouldApplyDeltaToMeshPhysicsTransforms;
DeltaPosition = Value.DeltaPosition;
DeltaQuat = Value.DeltaQuat;
DeltaTime = Value.DeltaTime;
OldVelocity = Value.OldVelocity;
OldLocation = Value.OldLocation;
ModifiedRotationRate = Value.ModifiedRotationRate;
bUsingModifiedRotationRate = Value.bUsingModifiedRotationRate;
bShouldDisablePostPhysicsTick = Value.bShouldDisablePostPhysicsTick;
bShouldEnablePostPhysicsTick = Value.bShouldEnablePostPhysicsTick;
bShouldAddMovementBaseTickDependency = Value.bShouldAddMovementBaseTickDependency;
bShouldRemoveMovementBaseTickDependency = Value.bShouldRemoveMovementBaseTickDependency;
NewMovementBase = Value.NewMovementBase;
NewMovementBaseOwner = Value.NewMovementBaseOwner;
UpdatedComponentOutput = Value.UpdatedComponentOutput;
*CharacterOutput = *Value.CharacterOutput;
}
FRotator FCharacterMovementComponentAsyncOutput::GetDeltaRotation(const FRotator& InRotationRate, float InDeltaTime)
{
return FRotator(GetAxisDeltaRotation(InRotationRate.Pitch, InDeltaTime), GetAxisDeltaRotation(InRotationRate.Yaw, InDeltaTime), GetAxisDeltaRotation(InRotationRate.Roll, InDeltaTime));
}
float FCharacterMovementComponentAsyncOutput::GetAxisDeltaRotation(float InAxisRotationRate, float InDeltaTime)
{
// Values over 360 don't do anything, see FMath::FixedTurn. However we are trying to avoid giant floats from overflowing other calculations.
return (InAxisRotationRate >= 0.f) ? FMath::Min(InAxisRotationRate * InDeltaTime, 360.f) : 360.f;
}
