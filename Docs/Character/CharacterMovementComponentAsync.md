---
layout: page
title: Character Movement Component
parent: Character
nav_order: 1
---

Link: [Character Movement Component Async]([URL](https://github.com/EpicGames/UnrealEngine/blob/072300df18a94f18077ca20a14224b5d99fee872/Engine/Source/Runtime/Engine/Private/CharacterMovementComponentAsync.cpp#L106)https://github.com/EpicGames/UnrealEngine/blob/072300df18a94f18077ca20a14224b5d99fee872/Engine/Source/Runtime/Engine/Private/CharacterMovementComponentAsync.cpp#L106 "You need access to Epic Games github repository to see this link")

# FCharacterMovementComponentAsyncInput Class Documentation

## Overview

`FCharacterMovementComponentAsyncInput` is a class designed for handling character movement in a game environment, particularly focusing on asynchronous processing. This class is part of the Unreal Engine 5 framework and extends basic character movement functionalities with enhanced network and computational performance capabilities.

## Header Inclusions

The `FCharacterMovementComponentAsyncInput` class includes several header files necessary for its operation within the Unreal Engine framework:

1. **CharacterMovementComponentAsync.h**: The primary header for the asynchronous character movement component.
2. **GameFramework/CharacterMovementComponent.h**: Provides the base functionalities of character movement within the game framework.
3. **GameFramework/Pawn.h**: Includes the essential functionalities of a Pawn, which is a type of Actor that can be controlled by players or AI.
4. **PhysicsProxy/SingleParticlePhysicsProxy.h**: Supports physics interactions, particularly relevant for handling physics-based movement and interactions.
5. **Components/PrimitiveComponent.h**: Includes functionalities for primitive components that are used in the construction of Actors, aiding in character representation and interaction with the environment.
6. **PBDRigidsSolver.h**: Pertains to the physics solver for rigid bodies, crucial for realistic physics calculations in character movement.
7. **Engine/World.h**: Provides access to the game world, necessary for character interaction with various elements of the game environment.

Additionally, it utilizes a macro `UE_INLINE_GENERATED_CPP_BY_NAME(CharacterMovementComponentAsync)` for inline generation of specific functionalities.

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
- `Output`: A reference to `FCharacterMovementComponentAsyncOutput`, where the results of the simulation are stored.

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
- `Output`: A reference to `FCharacterMovementComponentAsyncOutput` for storing the results.

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
- `Output`: A reference to `FCharacterMovementComponentAsyncOutput` for outputting the movement results.

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
- `Output`: A reference to `FCharacterMovementComponentAsyncOutput` for outputting physics simulation results.

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
- `Output`: A reference to `FCharacterMovementComponentAsyncOutput` for walking physics results.

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
- `Output`: A reference to `FCharacterMovementComponentAsyncOutput` for outputting falling physics results.

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
- `Output`: A reference to `FCharacterMovementComponentAsyncOutput`, where the results of the rotation adjustment are stored.

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
- `Output`: A reference to `FCharacterMovementComponentAsyncOutput` for movement results.

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
- `Output`: A reference to `FCharacterMovementComponentAsyncOutput` for the results.

### Process
1. **Surface Angle Check**: Determines if the surface angle is within a walkable range.
2. **Parallel Movement Calculation**: If on a slope, calculates movement that is parallel to the surface.
3. **Movement Adaptation**: Depending on the character's settings, adapts the movement to either maintain or adjust its horizontal velocity in response to the surface slope.

## `CanCrouchInCurrentState`

### Description
`CanCrouchInCurrentState` determines whether the character is able to crouch based on the current movement state and game settings. This function is vital for gameplay mechanics that involve crouching or stealth movements.

### Parameters
- `Output`: A reference to `FCharacterMovementComponentAsyncOutput`, which contains the current movement state and other relevant data.

### Process
1. **Crouch Capability Check**: Verifies if the character is generally capable of crouching based on game settings.
2. **Movement State Assessment**: Determines if the current movement state (e.g., walking, falling) permits crouching.
3. **Crouch Feasibility**: Returns a boolean indicating whether crouching is feasible at the current moment.

## `MaintainHorizontalGroundVelocity`

### Description
The `MaintainHorizontalGroundVelocity` function ensures that the character's horizontal velocity is maintained while moving on the ground. This is crucial for creating a natural and consistent movement experience, particularly when navigating uneven terrain.

### Parameters
- `Output`: A reference to `FCharacterMovementComponentAsyncOutput`, containing the character's current velocity and movement state.

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
- `Output`: A reference to `FCharacterMovementComponentAsyncOutput` for the results of the move.
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
- `Output`: A reference to `FCharacterMovementComponentAsyncOutput` for outputting results.
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
- `Output`: A reference to `FCharacterMovementComponentAsyncOutput` where the results of force application are stored.

### Process
1. **Force Application**: Adds any pending impulses and scaled forces to the character's velocity.
2. **Special Handling for Ground Movement**: If the character is on the ground and the applied forces result in an upward movement, the function may transition the character to a falling state.
3. **Force Clearance**: Clears the accumulated forces to prevent them from being applied repeatedly.

## `SetMovementMode`

### Description
`SetMovementMode` sets the movement mode of the character, changing how the character interacts with the game world based on their current state, such as walking, falling, or flying.

### Parameters
- `NewMovementMode`: An `EMovementMode` enum value representing the new movement mode.
- `Output`: A reference to `FCharacterMovementComponentAsyncOutput` for updating the character's movement state.
- `NewCustomMode`: An optional `uint8` for specifying a custom sub-mode within the primary movement mode.

### Process
1. **Mode Validation**: Ensures the new mode is valid and appropriate for the current game context.
2. **Mode Transition**: Updates the character's movement mode to the new mode, handling any necessary adjustments or initializations.
3. **Post-Mode-Change Handling**: Performs additional logic that may be required after changing the movement mode, such as updating physics or animation states.

# Documentation for `FCharacterMovementComponentAsyncInput` Utility Functions and Private Members

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

