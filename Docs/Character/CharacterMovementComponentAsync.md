---
layout: page
title: Character Movement Component
parent: Character
nav_order: 1
---

Link: [Character Movement Component Async](https://github.com/EpicGames/UnrealEngine/blob/072300df18a94f18077ca20a14224b5d99fee872/Engine/Source/Runtime/Engine/Private/CharacterMovementComponentAsync.cpp#L106 "You need access to Epic Games github repository to see this link")

# Character Movement Component Async Class Documentation

## Overview

`FCharacterMovementComponentAsyncInput` is a class designed for handling character movement in a game environment, particularly focusing on asynchronous processing. This class is part of the Unreal Engine 5 framework and extends basic character movement functionalities with enhanced network and computational performance capabilities.

## Header Inclusions

The `FCharacterMovementComponentAsyncInput` class includes several header files necessary for its operation within the Unreal Engine framework:

1. **CharacterMovementComponentAsyncInput.h**: The primary header for the asynchronous character movement component.
2. **GameFramework/CharacterMovementComponent.h**: Provides the base functionalities of character movement within the game framework.
3. **GameFramework/Pawn.h**: Includes the essential functionalities of a Pawn, which is a type of Actor that can be controlled by players or AI.
4. **PhysicsProxy/SingleParticlePhysicsProxy.h**: Supports physics interactions, particularly relevant for handling physics-based movement and interactions.
5. **Components/PrimitiveComponent.h**: Includes functionalities for primitive components that are used in the construction of Actors, aiding in character representation and interaction with the environment.
6. **PBDRigidsSolver.h**: Pertains to the physics solver for rigid bodies, crucial for realistic physics calculations in character movement.
7. **Engine/World.h**: Provides access to the game world, necessary for character interaction with various elements of the game environment.

Additionally, it utilizes a macro `UE_INLINE_GENERATED_CPP_BY_NAME(CharacterMovementComponentAsyncInput)` for inline generation of specific functionalities.

## Class Declaration

### Synopsis

The `FCharacterMovementComponentAsyncInput` class is designed to process and simulate character movement asynchronously. This approach is particularly beneficial in networked games or scenarios with high computational demands, as it allows for more responsive and accurate character movements.

### Responsibilities

- **Simulating Character Movement**: Handles the simulation of character movements over time, factoring in game physics and player inputs.
- **Movement Mode Management**: Controls different modes of character movement such as walking, falling, and jumping.
- **Interaction with Game World**: Manages the character's interaction with the physical game world, including collisions and physics-based reactions.
- **Asynchronous Processing**: Enhances movement responsiveness and accuracy in networked or computationally complex scenarios by processing movements asynchronously.

# Documentation for `FCharacterMovementComponentAsyncInput` Member Functions

## `Simulate`

### Description
The `Simulate` function is responsible for processing the simulation of character movement within a given time frame. This function plays a pivotal role in updating the movement state of the character based on inputs and time delta.

### Parameters
- `DeltaSeconds`: A `float` representing the time step for the simulation.
- `Output`: A reference to `FCharacterMovementComponentAsyncInputOutput`, where the results of the simulation are stored.

### Process
1. **Time Delta Update**: The function begins by updating the `Output.DeltaTime` with `DeltaSeconds`.
2. **Role Check**: It checks the character's role in the network (e.g., autonomous proxy, simulated proxy) to determine the appropriate movement handling.
3. **Controlled Movement**: If the character is locally controlled, it calls `ControlledCharacterMove` to process the movement specifically for controlled characters.
4. **Simulation Proxy Role**: For characters in the role of `ROLE_SimulatedProxy`, it ensures a certain condition is false, which might be related to error handling or specific game logic.

## `ControlledCharacterMove`

### Description
This function manages the movement of characters that are under local control, factoring in player inputs and game physics.

### Parameters
- `DeltaSeconds`: A `float` representing the time step for the movement calculation.
- `Output`: A reference to `FCharacterMovementComponentAsyncInputOutput` for storing the results.

### Process
1. **Jump State Check**: Initially checks the character's jump state before adjusting input acceleration, ensuring responsiveness and accuracy in jump mechanics.
2. **Acceleration Application**: Applies input to the character's acceleration, scaling and constraining it as necessary.
3. **Analog Input Modifier**: Computes an analog input modifier based on the output acceleration.
4. **Perform Movement**: Calls `PerformMovement` to carry out the actual movement logic.

## `PerformMovement`

### Description
`PerformMovement` is the core function that handles various aspects of character movement, including walking, falling, root motion application, and physics interactions.

### Parameters
- `DeltaSeconds`: A `float` representing the time step for the movement.
- `Output`: A reference to `FCharacterMovementComponentAsyncInputOutput` for outputting the movement results.

### Process
1. **Initial Setup**: Sets up initial movement parameters and checks conditions like movement mode and ground status.
2. **Root Motion Updates**: Updates and applies root motion to the character's velocity.
3. **Jump Input Clearing**: Clears jump input to allow for subsequent movement events.
4. **New Physics Start**: Initiates new physics calculations based on the movement mode.
5. **Character State Update**: Updates the character's state after movement, including rotation if allowed during root motion.
6. **Root Motion Application**: Applies root motion rotation after movement completion.
7. **Path Following**: Consumes path-following requested velocity and updates relevant movement flags.
8. **Final State Update**: Updates the final location, rotation, and velocity of the character.

## `StartNewPhysics`

### Description
The `StartNewPhysics` function initiates a new series of physics calculations based on the current movement mode of the character. It is a central part of the physics simulation process, ensuring that character movements align with the physical rules of the game world.

### Parameters
- `deltaTime`: A `float` representing the time step for the physics calculation.
- `Iterations`: An `int32` indicating the number of iterations for the physics simulation.
- `Output`: A reference to `FCharacterMovementComponentAsyncInputOutput` for outputting physics simulation results.

### Process
1. **Initial Checks**: Ensures that the deltaTime is sufficient and the maximum number of iterations is not exceeded. It also checks if the character has valid data for physics simulation.
2. **Movement Mode Handling**: Depending on the character's movement mode (e.g., walking, falling), it calls the respective physics function like `PhysWalking` or `PhysFalling`.
3. **Movement Mode Change**: If necessary, the movement mode is changed to `MOVE_None`.

## `PhysWalking`

### Description
`PhysWalking` handles the physics of character movement when the character is in the walking state. It manages ground interactions, steps, slopes, and other factors influencing walking movement.

### Parameters
- `deltaTime`: A `float` representing the time step for walking physics.
- `Iterations`: An `int32` for the number of iterations.
- `Output`: A reference to `FCharacterMovementComponentAsyncInputOutput` for walking physics results.

### Process
1. **Initial Setup**: Sets up velocity and acceleration for walking.
2. **Collision Handling**: Manages collision detection and response during walking.
3. **Step Up Handling**: Processes the character's ability to step up onto ledges or obstacles.
4. **Floor Checking**: Continuously checks for the floor to ensure the character remains grounded.
5. **Velocity Maintenance**: Maintains or adjusts the character's velocity based on the walking movement.

## `PhysFalling`

### Description
This function manages the physics when the character is in a falling state, typically after a jump or when losing ground contact. It deals with gravity, air control, and landing logic.

### Parameters
- `deltaTime`: A `float` representing the time step for falling physics.
- `Iterations`: An `int32` for the number of iterations.
- `Output`: A reference to `FCharacterMovementComponentAsyncInputOutput` for outputting falling physics results.

### Process
1. **Gravity Application**: Applies gravity to the character, affecting the fall speed.
2. **Air Control**: Manages the degree of control the character has while in the air.
3. **Collision Detection**: Detects collisions during the fall and determines if the character lands on a walkable surface.
4. **Landing Process**: Handles the transition from falling to landing, adjusting the character's state and velocity accordingly.

## `PhysicsRotation`

### Description
The `PhysicsRotation` function is responsible for adjusting the character's rotation based on their current movement and orientation preferences. This function plays a crucial role in ensuring that the character's rotation aligns with their movement direction and user input.

### Parameters
- `DeltaTime`: A `float` representing the time step for the rotation calculation.
- `Output`: A reference to `FCharacterMovementComponentAsyncInputOutput`, where the results of the rotation adjustment are stored.

### Process
1. **Rotation Mode Check**: Determines if the character should orient its rotation to the movement direction or follow the controller's desired rotation.
2. **Current Rotation Calculation**: Computes the current rotation of the character.
3. **Desired Rotation Determination**: Based on the character's settings, calculates the desired rotation either towards the movement direction or as specified by the controller.
4. **Vertical Alignment**: If required, adjusts the rotation to keep the character vertically aligned (e.g., upright).
5. **Rotation Application**: Applies the calculated rotation to the character, ensuring smooth and responsive changes in direction.

## `MoveAlongFloor`

### Description
`MoveAlongFloor` handles the character's movement along a walkable floor. It ensures that the character moves correctly on slopes and steps, adhering to the game world's physics.

### Parameters
- `InVelocity`: A `FVector` representing the character's intended velocity.
- `DeltaSeconds`: A `float` representing the time step for the movement.
- `OutStepDownResult`: A pointer to `FStepDownResult`, storing results of any step down actions during movement.
- `Output`: A reference to `FCharacterMovementComponentAsyncInputOutput` for movement results.

### Process
1. **Walkable Floor Check**: Verifies if the current surface is walkable.
2. **Movement Delta Calculation**: Computes the movement delta based on the character's velocity and the floor's characteristics.
3. **Collision Handling**: Detects and responds to collisions encountered during movement.
4. **Step Handling**: Processes the ability to step over small obstacles or changes in elevation.
5. **Surface Adaptation**: Adjusts the character's movement to conform to the surface's slope or irregularities.

## `ComputeGroundMovementDelta`

### Description
The `ComputeGroundMovementDelta` function calculates the movement delta for ground-based movement, taking into account the surface's slope and the character's movement direction.

### Parameters
- `Delta`: A `FVector` representing the desired movement delta.
- `RampHit`: A `FHitResult` containing information about any contact with a ramp-like surface.
- `bHitFromLineTrace`: A `bool` indicating whether the contact with the surface was determined through a line trace.
- `Output`: A reference to `FCharacterMovementComponentAsyncInputOutput` for the results.

### Process
1. **Surface Angle Check**: Determines if the surface angle is within a walkable range.
2. **Parallel Movement Calculation**: If on a slope, calculates movement that is parallel to the surface.
3. **Movement Adaptation**: Depending on the character's settings, adapts the movement to either maintain or adjust its horizontal velocity in response to the surface slope.

## `CanCrouchInCurrentState`

### Description
`CanCrouchInCurrentState` determines whether the character is able to crouch based on the current movement state and game settings. This function is vital for gameplay mechanics that involve crouching or stealth movements.

### Parameters
- `Output`: A reference to `FCharacterMovementComponentAsyncInputOutput`, which contains the current movement state and other relevant data.

### Process
1. **Crouch Capability Check**: Verifies if the character is generally capable of crouching based on game settings.
2. **Movement State Assessment**: Determines if the current movement state (e.g., walking, falling) permits crouching.
3. **Crouch Feasibility**: Returns a boolean indicating whether crouching is feasible at the current moment.

## `MaintainHorizontalGroundVelocity`

### Description
The `MaintainHorizontalGroundVelocity` function ensures that the character's horizontal velocity is maintained while moving on the ground. This is crucial for creating a natural and consistent movement experience, particularly when navigating uneven terrain.

### Parameters
- `Output`: A reference to `FCharacterMovementComponentAsyncInputOutput`, containing the character's current velocity and movement state.

### Process
1. **Vertical Velocity Check**: Inspects if there's any vertical component in the character's velocity.
2. **Horizontal Velocity Maintenance**: If required, adjusts the character's velocity to maintain its horizontal component while removing or recalculating the vertical component.

## `MoveUpdatedComponent`

### Description
`MoveUpdatedComponent` moves the character's component based on the provided delta and rotation. It is a fundamental function for translating character input and physics calculations into actual movement within the game world.

### Parameters
- `Delta`: A `FVector` representing the desired movement.
- `NewRotation`: A `FQuat` indicating the new rotation for the character.
- `bSweep`: A `bool` indicating whether to perform a sweep test when moving.
- `Output`: A reference to `FCharacterMovementComponentAsyncInputOutput` for the results of the move.
- `OutHitResult`: An optional pointer to `FHitResult` to store any hit results from the move.
- `TeleportType`: An `ETeleportType` enum value specifying the type of teleportation, if any, to use during the move.

### Process
1. **Directional Constraint**: Applies any necessary constraints to the movement direction.
2. **Component Movement**: Executes the movement of the character's component, taking into account collision detection and response.
3. **Hit Result Processing**: Optionally processes any hit results from the movement, providing information about collisions or interactions with the game world.

## `SafeMoveUpdatedComponent`

### Description
`SafeMoveUpdatedComponent` is responsible for safely moving the character's component, taking into account potential collisions and obstacles. It ensures that the character's movement respects the physical environment and game world boundaries.

### Parameters
- `Delta`: A `FVector` indicating the movement delta.
- `NewRotation`: A `FQuat` specifying the new rotation for the character.
- `bSweep`: A `bool` flag indicating whether to perform collision detection during the move.
- `OutHit`: An `FHitResult` structure to store information about any collisions encountered.
- `Output`: A reference to `FCharacterMovementComponentAsyncInputOutput` for outputting results.
- `Teleport`: An `ETeleportType` enum value specifying the type of teleportation, if any, to use during the move.

### Process
1. **Movement Execution**: Attempts to move the character's component according to the specified delta and rotation.
2. **Collision Handling**: If a collision is detected (when `bSweep` is true), the function processes the collision and adjusts the movement accordingly.
3. **Penetration Resolution**: If the movement results in penetration (overlapping another object), the function attempts to resolve it and retry the move.

## `ApplyAccumulatedForces`

### Description
`ApplyAccumulatedForces` applies forces that have been accumulated over time to the character's movement. This includes impulses and continuous forces, which can affect the character's velocity and trajectory.

### Parameters
- `DeltaSeconds`: A `float` representing the time step over which the forces are applied.
- `Output`: A reference to `FCharacterMovementComponentAsyncInputOutput` where the results of force application are stored.

### Process
1. **Force Application**: Adds any pending impulses and scaled forces to the character's velocity.
2. **Special Handling for Ground Movement**: If the character is on the ground and the applied forces result in an upward movement, the function may transition the character to a falling state.
3. **Force Clearance**: Clears the accumulated forces to prevent them from being applied repeatedly.

## `SetMovementMode`

### Description
`SetMovementMode` sets the movement mode of the character, changing how the character interacts with the game world based on their current state, such as walking, falling, or flying.

### Parameters
- `NewMovementMode`: An `EMovementMode` enum value representing the new movement mode.
- `Output`: A reference to `FCharacterMovementComponentAsyncInputOutput` for updating the character's movement state.
- `NewCustomMode`: An optional `uint8` for specifying a custom sub-mode within the primary movement mode.

### Process
1. **Mode Validation**: Ensures the new mode is valid and appropriate for the current game context.
2. **Mode Transition**: Updates the character's movement mode to the new mode, handling any necessary adjustments or initializations.
3. **Post-Mode-Change Handling**: Performs additional logic that may be required after changing the movement mode, such as updating physics or animation states.

## OnMovementModeChanged Method

### Description
The `OnMovementModeChanged` method is responsible for updating the character's state and movement parameters when there is a change in the movement mode, such as transitioning from walking to falling.

#### Parameters
- `PreviousMovementMode`: The movement mode the character was in before the change.
- `PreviousCustomMode`: A custom mode value associated with the previous movement mode.
- `Output`: Reference to `FCharacterMovementComponentAsyncInputOutput`, where the resulting state and movement parameters are stored.

#### Behavior
- Checks if valid data is present; if not, the function returns early.
- Updates collision settings if transitioning to or from navigation walking (`MOVE_NavWalking`).
- Adjusts velocity, maintains base location, and updates the ground movement mode when changing to walking (`MOVE_Walking`).
- Clears current floor information and resets base-related properties when transitioning to other movement modes like falling (`MOVE_Falling`) or none (`MOVE_None`).

---

## FindFloor Method

### Description
`FindFloor` determines if there is a floor beneath the character, and if so, it characterizes that floor based on its properties.

#### Parameters
- `CapsuleLocation`: The location of the character's capsule component.
- `OutFloorResult`: The result of the floor finding, including information about the floor's properties.
- `bCanUseCachedLocation`: A flag indicating if cached location data can be used.
- `Output`: Reference to `FCharacterMovementComponentAsyncInputOutput`.
- `DownwardSweepResult`: Optional parameter providing results from a previous downward sweep.

#### Behavior
- If collision is disabled or no valid data is present, the method returns early, indicating no floor.
- Adjusts the floor search parameters based on whether the character is moving on the ground.
- Performs a sweep test to find the floor if conditions like teleportation or forced floor check are met.
- Validates the floor result and makes adjustments if necessary, such as checking for perchable surfaces.

---

## ComputeFloorDist Method

### Description
`ComputeFloorDist` calculates the distance from the character to the floor and determines the floor's properties, including whether it's walkable.

#### Parameters
- `CapsuleLocation`: The location of the character's capsule component.
- `LineDistance`: The maximum distance for line trace checks.
- `SweepDistance`: The maximum distance for sweep checks.
- `OutFloorResult`: The result of the floor computation.
- `SweepRadius`: The radius used for sweep checks.
- `Output`: Reference to `FCharacterMovementComponentAsyncInputOutput`.
- `DownwardSweepResult`: Optional results from a previous downward sweep.

#### Behavior
- Clears the previous floor result and calculates the character's size for collision checks.
- Performs a sweep test if conditions are met, such as no valid previous sweep result being provided.
- Conducts a line trace to find the floor if the sweep doesn't provide a blocking hit.
- Determines whether the detected floor is walkable based on the hit result.

---

## FloorSweepTest Method

### Description
`FloorSweepTest` conducts a sweep test to detect the floor beneath the character, considering the character's shape and collision settings.

#### Parameters
- `OutHit`: The hit result of the sweep test.
- `Start`: The starting location of the sweep.
- `End`: The ending location of the sweep.
- `TraceChannel`: The collision channel to use for the sweep.
- `CollisionShape`: The shape used for the sweep.
- `Params`: The collision query parameters.
- `ResponseParam`: The collision response parameters.
- `Output`: Reference to `FCharacterMovementComponentAsyncInputOutput`.

#### Behavior
- Determines whether to perform a standard capsule sweep or a flat base check, based on the `bUseFlatBaseForFloorChecks` flag.
- Executes the sweep test using the world's collision system.
- Optionally performs the test with a box shape if the flat base check is enabled.

---

## IsWithinEdgeTolerance Method

### Description
`IsWithinEdgeTolerance` checks if a point (usually from a hit result) is within a specified tolerance from the edge of the character's capsule.

#### Parameters
- `CapsuleLocation`: The center location of the character's capsule.
- `TestImpactPoint`: The point to test against the capsule edge.
- `CapsuleRadius`: The radius of the character's capsule.

#### Behavior
- Computes the 2D distance squared from the capsule's center to the test impact point.
- Compares this distance against a calculated tolerance radius, determining if the impact point is within edge tolerance.
- Returns true if the impact point is within the tolerance, indicating it's near the edge of the capsule.

## IsWalkable Method

### Description
The `IsWalkable` method determines if a surface, based on the hit result from collision detection, is walkable for the character.

#### Parameters
- `Hit`: The hit result from a collision check which contains information about the surface.

#### Behavior
- Checks if the hit result is a valid blocking hit; if not, the surface is considered non-walkable.
- Ensures that the surface is not excessively steep by comparing the surface normal's Z-component to a predefined walkable floor angle (stored in `WalkableFloorZ`).
- Returns `true` if the surface is not too steep, otherwise `false`.

## UpdateCharacterStateAfterMovement Method

### Description
`UpdateCharacterStateAfterMovement` updates the character's state, like crouching, after movement has occurred.

#### Parameters
- `DeltaSeconds`: The time elapsed since the last movement update.
- `Output`: Reference to `FCharacterMovementComponentAsyncInputOutput`, used to update the character's state.

#### Behavior
- Checks the character's role and updates the crouched state if necessary.
- This method is specifically designed to handle state changes in non-proxy characters.

## GetSimulationTimeStep Method

### Description
`GetSimulationTimeStep` computes the time step to be used for the physics simulation, ensuring that the simulation remains stable under various conditions.

#### Parameters
- `RemainingTime`: The remaining time for which the simulation needs to be run.
- `Iterations`: The number of iterations the simulation has already performed.

#### Behavior
- Adjusts the `RemainingTime` if it exceeds the maximum allowed time step (`MaxSimulationTimeStep`), to avoid large time steps that can lead to instability.
- The method ensures the time step is not smaller than a minimum threshold (`MIN_TICK_TIME`) to avoid potential divide-by-zero errors.
- Returns the computed time step to be used for the simulation.

## CalcVelocity Method

### Description
`CalcVelocity` calculates the character's velocity based on factors like acceleration, friction, and applied braking.

#### Parameters
- `DeltaTime`: The time elapsed since the last update.
- `Friction`: The friction coefficient affecting the character's movement.
- `bFluid`: A flag indicating if the character is moving through a fluid, affecting drag.
- `BrakingDeceleration`: The deceleration applied when the character is braking.
- `Output`: Reference to `FCharacterMovementComponentAsyncInputOutput`, containing the resulting velocity.

#### Behavior
- Handles cases where velocity calculation should not occur, like when root motion is used.
- Applies friction and deceleration to adjust the character's velocity.
- Handles acceleration, including forced acceleration, based on the character's current state and movement requests.
- Caps the velocity based on maximum speed limits, considering various movement modes and conditions.

## ApplyRequestedMove Method

### Description
`ApplyRequestedMove` processes a requested movement, adjusting the character's acceleration and velocity accordingly.

#### Parameters
- `DeltaTime`: The time elapsed since the last movement update.
- `MaxAccel`: The maximum allowed acceleration.
- `MaxSpeed`: The maximum allowed speed.
- `Friction`: The friction coefficient affecting movement.
- `BrakingDeceleration`: The deceleration applied when braking.
- `OutAcceleration`: Output parameter for the resulting acceleration.
- `OutRequestedSpeed`: Output parameter for the requested speed after processing.
- `Output`: Reference to `FCharacterMovementComponentAsyncInputOutput`.

#### Behavior
- Processes the character's requested velocity if available (`Output.bHasRequestedVelocity`).
- Computes the requested movement direction and speed, considering the maximum speed and acceleration limits.
- Determines the appropriate acceleration to reach the requested velocity, considering current speed and direction.
- Applies the computed acceleration and velocity adjustments to the character's movement output.
- Returns `true` if a requested move was applied, otherwise `false`.

# FCharacterMovementComponentAsyncInput Documentation

## ShouldComputeAccelerationToReachRequestedVelocity Method

### Description
Determines whether acceleration should be computed to reach a specified requested velocity during character movement.

#### Parameters
- `RequestedSpeed`: The speed that the character is trying to achieve.
- `Output`: Reference to `FCharacterMovementComponentAsyncInputOutput` containing the character's current movement state.

#### Behavior
- Compares the character's current velocity against the requested speed.
- If the current velocity is less than the requested speed (with a small buffer), returns `true` to indicate that acceleration calculation is necessary.
- Otherwise, returns `false`, suggesting direct velocity adjustment can be used.

## GetMinAnalogSpeed Method

### Description
Retrieves the minimum analog speed for the character based on the current movement mode.

#### Parameters
- `Output`: Reference to `FCharacterMovementComponentAsyncInputOutput` containing the current movement mode.

#### Behavior
- Switches based on the character's current movement mode (`Output.MovementMode`).
- Returns `MinAnalogWalkSpeed` for walking, navigation walking, and falling modes.
- Returns `0.f` for other movement modes, indicating no minimum analog speed is applicable.

## GetMaxBrakingDeceleration Method

### Description
Provides the maximum braking deceleration value for the character based on the current movement mode.

#### Parameters
- `Output`: Reference to `FCharacterMovementComponentAsyncInputOutput` containing the current movement mode.

#### Behavior
- Switches based on the character's current movement mode (`Output.MovementMode`).
- Returns different deceleration values for walking, falling, swimming, and flying modes, each defined by respective member variables like `BrakingDecelerationWalking`.
- Returns `0.f` for custom and none movement modes, indicating no braking deceleration.

## ApplyVelocityBraking Method

### Description
Applies braking to the character's velocity to slow down or stop, based on current friction and braking deceleration.

#### Parameters
- `DeltaTime`: The time elapsed since the last update.
- `Friction`: The friction coefficient affecting movement.
- `BrakingDeceleration`: The deceleration applied when braking.
- `Output`: Reference to `FCharacterMovementComponentAsyncInputOutput` containing the character's velocity.

#### Behavior
- Checks for conditions where braking should not be applied, such as zero velocity or when root motion is active.
- Applies a braking force to the character's velocity, influenced by friction and deceleration parameters.
- Ensures velocity does not reverse direction during braking.
- Clamps velocity to zero if it falls below a small threshold or if the character is effectively stopped.

## GetPenetrationAdjustment Method

### Description
Calculates an adjustment vector to resolve penetration with another object, based on the hit result from a collision.

#### Parameters
- `HitResult`: The hit result from a collision, providing details about the penetration.

#### Behavior
- Determines the penetration depth and calculates a vector to move the character out of penetration.
- Adjusts the vector's magnitude based on character's role (e.g., proxy or not) and the type of object penetrated (geometry or pawn).
- Clamps the adjustment vector to a maximum size defined by `MaxDepenetrationWithGeometry` or related variables.
- Returns the calculated adjustment vector.

# FCharacterMovementComponentAsyncInput Documentation

## ResolvePenetration Method

### Description
Resolves penetration of the character's collision component with world geometry or other objects.

#### Parameters
- `ProposedAdjustment`: Vector proposing how to adjust the character's position to resolve penetration.
- `Hit`: Hit result containing information about the penetration.
- `NewRotation`: Proposed new rotation of the character.
- `Output`: Reference to `FCharacterMovementComponentAsyncInputOutput`.

#### Behavior
- Tries resolving penetration without causing new overlaps.
- Attempts various strategies if initial resolution fails, like adjusting position and rotation.
- Combines multiple Minimum Translation Distance (MTD) results for better resolution.
- Returns `true` if successfully resolved, else `false`.

## MoveComponent_GetPenetrationAdjustment Method

### Description
Calculates a penetration adjustment vector from the collision hit result.

#### Parameters
- `Hit`: Hit result from a collision.

#### Behavior
- Calculates adjustment vector based on penetration depth and a pullback distance.
- Ensures the vector is constrained to the movement plane.
- Returns the computed adjustment vector.

## MoveComponent_SlideAlongSurface Method

### Description
Manages character sliding along a surface after a collision.

#### Parameters
- `Delta`: Attempted movement delta.
- `Time`: Fraction of the movement timestep.
- `Normal`: Normal of the surface for sliding.
- `Hit`: Collision hit result.
- `Output`: Reference to `FCharacterMovementComponentAsyncInputOutput`.
- `bHandleImpact`: Flag to handle impact events.

#### Behavior
- Calculates new movement delta for sliding.
- Performs a safe movement update with the new delta.
- Adjusts velocity after a secondary collision.

## MoveComponent_ComputeSlideVector Method

### Description
Computes the vector for sliding along a surface during collision.

#### Parameters
- `Delta`: Movement delta before the collision.
- `Time`: Time fraction of the movement step.
- `Normal`: Normal of the collision surface.
- `Hit`: Collision hit result.
- `Output`: Reference to `FCharacterMovementComponentAsyncInputOutput`.

#### Behavior
- Projects the Delta onto the plane defined by the Normal.
- Adjusts the projected vector based on movement constraints.
- Returns the computed slide vector.

## SlideAlongSurface Method

### Description
Wrapper for `MoveComponent_SlideAlongSurface`, tailored for character movement.

#### Parameters
- `Delta`: Movement delta.
- `Time`: Time fraction of the movement step.
- `InNormal`: Normal of the collision surface.
- `Hit`: Collision hit result.
- `bHandleImpact`: Flag to handle impact events.
- `Output`: Reference to `FCharacterMovementComponentAsyncInputOutput`.

#### Behavior
- Adapts normal for ground movement, preventing upward pushes on steep surfaces.
- Calls `MoveComponent_SlideAlongSurface` with adjusted parameters.
- Returns the fraction of Delta applied during the slide.

## ComputeSlideVector Method

### Description
Calculates the slide vector for character movement.

#### Parameters
- `Delta`: Original movement delta.
- `Time`: Time fraction of the movement step.
- `Normal`: Normal of the collision surface.
- `Hit`: Collision hit result.
- `Output`: Reference to `FCharacterMovementComponentAsyncInputOutput`.

#### Behavior
- Invokes `MoveComponent_ComputeSlideVector` for initial computation.
- Handles slope boosting adjustments for falling movement.
- Returns the final adjusted slide vector.

#  Private Helper Methods

## FUpdatedComponentAsyncInput::MoveComponent Method

### Description
Manages the movement of the component in the world, handling collisions and overlaps.

#### Parameters
- `Delta`: The vector by which to move the component.
- `NewRotationQuat`: The new rotation for the component after moving.
- `bSweep`: Flag to determine if sweep tests should be used for collision detection.
- `OutHit`: Optional parameter to store the result of a collision hit.
- `MoveFlags`: Flags controlling the movement behavior.
- `Teleport`: Specifies the type of teleport, if any, to be used during movement.
- `Input`: Reference to `FUpdatedComponentAsyncInput` containing the current movement state.
- `Output`: Reference to `FCharacterMovementComponentAsyncInputOutput`.

#### Behavior
- Adjusts the component's position and rotation based on `Delta` and `NewRotationQuat`.
- Performs collision checks if `bSweep` is true and resolves any collisions encountered.
- Manages overlapping components and triggers appropriate events.
- Returns `true` if the component successfully moved, otherwise `false`.

## FUpdatedComponentAsyncInput::AreSymmetricRotations Method

### Description
Checks if two rotations are symmetric, considering the scale of the component.

#### Parameters
- `A`, `B`: The two rotations to compare.
- `Scale3D`: The scale of the component.

#### Behavior
- Compares the Z-axis of both rotations to determine if they are symmetrical.
- Returns `true` if the rotations are symmetrical, otherwise `false`.

## FUpdatedComponentAsyncInput::PullBackHit Method

### Description
Adjusts a hit result to account for the desired distance from the surface hit.

#### Parameters
- `Hit`: The hit result from a collision.
- `Start`, `End`: Start and end points of the trace that resulted in the hit.
- `Dist`: The distance of the trace.

#### Behavior
- Modifies the `Time` property of the hit result to pull back the hit location from the surface.
- Ensures the adjusted hit result remains within valid bounds.

## FUpdatedComponentAsyncInput::ShouldCheckOverlapFlagToQueueOverlaps Method

### Description
Determines whether overlap flags should be checked to queue overlaps during movement.

#### Parameters
- `ThisComponent`: The component being checked for overlaps.

#### Behavior
- Checks if the current movement requires overlap events to be flagged.
- Returns `true` if overlaps need to be queued, otherwise `false`.

## FUpdatedComponentAsyncInput::ShouldIgnoreHitResult Method

### Description
Decides whether a hit result should be ignored based on movement direction and other conditions.

#### Parameters
- `InWorld`: The world in which the movement occurs.
- `TestHit`: The hit result to be evaluated.
- `MovementDirDenormalized`: The direction of movement.
- `MovingActor`: The actor that is moving.
- `MoveFlags`: Flags controlling movement behavior.

#### Behavior
- Evaluates the hit result against various conditions, like "ignore bases" and moving out of penetration.
- Returns `true` if the hit result should be ignored, otherwise `false`.

## FUpdatedComponentAsyncInput::ShouldIgnoreOverlapResult Method

### Description
Determines if an overlap result should be ignored based on the actors and components involved.

#### Parameters
- `World`: The world context.
- `ThisActor`: The actor performing the movement.
- `ThisComponent`: The component involved in the overlap.
- `OtherActor`: The other actor involved in the overlap.
- `OtherComponent`: The other component involved in the overlap.

#### Behavior
- Checks if the overlap result is relevant or should be ignored based on the actors and components involved.
- Returns `true` if the overlap should be ignored, otherwise `false`.


# Utility Functions and Private Members

## Utility Functions

### Description
Utility functions in `FCharacterMovementComponentAsyncInput` provide additional support and calculations necessary for character movement. These functions handle tasks like constraining movement, applying forces, and managing movement-related calculations.

### Key Utility Functions
- **ConstrainInputAcceleration**: Restricts the input acceleration to ensure it aligns with the character's current movement state.
- **ScaleInputAcceleration**: Scales the input acceleration based on maximum acceleration values and character capabilities.
- **ComputeAnalogInputModifier**: Calculates a modifier for analog input, which can affect movement smoothness and responsiveness.
- **ConstrainDirectionToPlane**: Ensures that movement direction adheres to any plane constraints, useful in scenarios like wall running or slope movement.
- **ConstrainNormalToPlane**: Adjusts normals (used in collision responses) to align with plane constraints.
- **ConstrainLocationToPlane**: Modifies character location to respect plane constraints, preventing undesired movement outside the constrained area.

## Private Members

### Description
Private members in `FCharacterMovementComponentAsyncInput` store essential data and state information used in the movement calculations. These members are not directly accessible outside the class but play a crucial role in the internal workings of character movement.

### Key Private Members
- **MaxAcceleration**: Stores the maximum acceleration value for the character.
- **GravityZ**: Holds the gravity value applied along the Z-axis, affecting vertical movement.
- **GroundFriction**: Represents the friction experienced by the character when moving on ground surfaces.
- **bCanEverCrouch**: A boolean flag indicating whether the character is capable of crouching.
- **bOrientRotationToMovement**: Determines if the character's rotation should automatically orient to the direction of movement.
- **bUseControllerDesiredRotation**: A flag to use the rotation desired by the controller, often used in AI-controlled characters.
- **bMaintainHorizontalGroundVelocity**: Controls whether to maintain horizontal velocity when moving on slopes or steps.
- **bConstrainToPlane**: Indicates whether movement is constrained to a specific plane.
- **PlaneConstraintNormal**: The normal vector of the plane to which movement is constrained.
- **PlaneConstraintOrigin**: The origin point of the plane used for movement constraints.
