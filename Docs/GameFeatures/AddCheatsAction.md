---
layout: default
title: Action - Add Cheats
parent: GameFeatures
nav_order: 1
---
# Add Cheats Action

## Description

The UGameFeatureAction_AddCheats class in Unreal Engine 5 is a specialized class designed to augment game features by integrating cheat functionalities. This class is part of the game feature plugin system, which allows developers to modularly add or modify features in a game. Specifically, UGameFeatureAction_AddCheats focuses on extending the game's cheat system through the addition of custom cheat manager extensions.

**Key aspects of this class include:**

Cheat Manager Extensions Integration: The primary purpose of this class is to add custom cheat manager extensions to the game. These extensions allow for the implementation of new cheat commands or functionalities that can be used during game development for testing or debugging purposes.

**Game Feature Plugin Enhancement:**
As part of the game feature plugin architecture, this class provides a way to dynamically augment a game's capabilities, particularly in terms of cheat-related features, without altering the core game code.

**Activation and Deactivation Handling:**
The class manages the lifecycle of cheat extensions by properly handling their activation when a game feature is turned on and ensuring clean deactivation and cleanup when the feature is turned off.

**Asynchronous Loading Support:**
It supports the asynchronous loading of cheat managers, enabling efficient management of resources and smoother gameplay experience, particularly when dealing with large or complex game features.

**Editor-Level Data Validation:**
In the Unreal Engine editor environment, the class includes functionality to validate the data related to cheat managers, ensuring that the setup is correct and ready for use within the game.

**Modular and Reusable Design:**
The design of the class aligns with Unreal Engine's emphasis on modular and reusable components, making it easier for developers to implement and manage cheat-related features across different projects or game modules.

---

## Public Interface

### Methods

### `virtual void OnGameFeatureActivating() override`

This method is called when the game feature is activating. It sets the `bIsActive` flag to true and registers a delegate with the cheat manager to handle cheat manager creation events.

**Parameters**: None

**Return**: None

### `virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override`

This method is invoked during the deactivation of the game feature. It unregisters the delegate from the cheat manager and removes all cheat manager extensions that were added.

**Parameters**:
- `FGameFeatureDeactivatingContext& Context`: A reference to the context of the game feature deactivating.

**Return**: None

### `virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override` (Editor Only)

Used in the editor to validate the data of this class. It checks if all entries in `CheatManagers` are valid and reports any null entries.

**Parameters**:
- `FDataValidationContext& Context`: A reference to the data validation context.

**Return**:
- `EDataValidationResult`: The result of the data validation, indicating if the data is valid or invalid.

### Properties

### `TArray<TSoftClassPtr<UCheatManagerExtension>> CheatManagers`

An array of soft class pointers to `UCheatManagerExtension` classes. These classes are set up as cheat managers for the game feature plugin.

**Type**: `TArray<TSoftClassPtr<UCheatManagerExtension>>`

### `bool bLoadCheatManagersAsync`

A boolean flag indicating whether cheat managers should be loaded asynchronously.

**Type**: `bool`

---

## Private Methods and Properties

### Methods

### `void OnCheatManagerCreated(UCheatManager* CheatManager)`

This method is a callback that is triggered when a new `UCheatManager` instance is created. It cleans out stale pointers from `SpawnedCheatManagers` and either spawns new cheat manager extensions immediately, loads them asynchronously, or synchronously based on the `bLoadCheatManagersAsync` flag.

**Parameters**:
- `UCheatManager* CheatManager`: Pointer to the `UCheatManager` instance that was created.

**Return**: None

### `void SpawnCheatManagerExtension(UCheatManager* CheatManager, const TSubclassOf<UCheatManagerExtension>& CheatManagerClass)`

Spawns a new cheat manager extension of the specified class and adds it to the `SpawnedCheatManagers` array and the provided `UCheatManager`.

**Parameters**:
- `UCheatManager* CheatManager`: The cheat manager to which the extension is to be added.
- `const TSubclassOf<UCheatManagerExtension>& CheatManagerClass`: The class of the cheat manager extension to be spawned.

**Return**: None

### Properties

### `FDelegateHandle CheatManagerRegistrationHandle`

A handle to the delegate registered with the cheat manager. This handle is used to unregister the delegate when the game feature is deactivating.

**Type**: `FDelegateHandle`

### `TArray<TWeakObjectPtr<UCheatManagerExtension>> SpawnedCheatManagers`

An array of weak object pointers to `UCheatManagerExtension` instances. This array keeps track of all cheat manager extensions spawned by this class.

**Type**: `TArray<TWeakObjectPtr<UCheatManagerExtension>>`

### `bool bIsActive`

A boolean flag indicating whether the game feature is currently active. This flag is set to true when the game feature is activating and set to false when deactivating.

**Type**: `bool`

---

## Class Members' Details

### Methods

### `OnGameFeatureActivating`

- **Description**: Called when the game feature is activated. It sets the internal state to active and registers for cheat manager creation events.
- **Implementation Details**: 
  - Sets `bIsActive` to `true`.
  - Registers a delegate with `UCheatManager::RegisterForOnCheatManagerCreated` using `FOnCheatManagerCreated::FDelegate::CreateUObject`.

### `OnGameFeatureDeactivating`

- **Description**: Invoked during the deactivation of the game feature. Cleans up by unregistering the delegate and removing extensions from cheat managers.
- **Parameters**:
  - `FGameFeatureDeactivatingContext& Context`: Context information for game feature deactivation.
- **Implementation Details**: 
  - Calls `UCheatManager::UnregisterFromOnCheatManagerCreated`.
  - Iterates over `SpawnedCheatManagers` to remove and empty extensions.

### `IsDataValid` (Editor Only)

- **Description**: Validates the data of the class in the editor. Checks for null entries in `CheatManagers`.
- **Parameters**:
  - `FDataValidationContext& Context`: The context for data validation.
- **Implementation Details**: 
  - Iterates over `CheatManagers` and validates each entry.
  - Uses `LOCTEXT` for error formatting.

### `OnCheatManagerCreated`

- **Description**: Callback for when a new `UCheatManager` is created. Manages spawning and loading of cheat manager extensions.
- **Parameters**:
  - `UCheatManager* CheatManager`: The created cheat manager instance.
- **Implementation Details**: 
  - Cleans out stale pointers from `SpawnedCheatManagers`.
  - Handles synchronous or asynchronous loading of cheat manager extensions.

### `SpawnCheatManagerExtension`

- **Description**: Spawns a cheat manager extension and adds it to the cheat manager.
- **Parameters**:
  - `UCheatManager* CheatManager`: The cheat manager to add the extension to.
  - `const TSubclassOf<UCheatManagerExtension>& CheatManagerClass`: The class of the cheat manager extension to spawn.
- **Implementation Details**: 
  - Validates the `CheatManagerClass`.
  - Creates a new instance of `UCheatManagerExtension` and adds it to `SpawnedCheatManagers` and the cheat manager.

### Properties

### `CheatManagers`

- **Description**: Array of cheat manager extension classes to be set up for the game feature plugin.
- **Type**: `TArray<TSoftClassPtr<UCheatManagerExtension>>`

### `bLoadCheatManagersAsync`

- **Description**: Flag indicating whether to load cheat manager extensions asynchronously.
- **Type**: `bool`

### `CheatManagerRegistrationHandle`

- **Description**: Handle for the delegate registered with the cheat manager for creation events.
- **Type**: `FDelegateHandle`

### `SpawnedCheatManagers`

- **Description**: Tracks the cheat manager extensions spawned by this class.
- **Type**: `TArray<TWeakObjectPtr<UCheatManagerExtension>>`

### `bIsActive`

- **Description**: Indicates whether the game feature is currently active.
- **Type**: `bool`

---

## Inheritance and Dependencies

### Inheritance

- **Base Class**: `UGameFeatureAction`
  - `UGameFeatureAction_AddCheats` is derived from `UGameFeatureAction`, an Unreal Engine base class for game feature actions. It overrides specific methods to provide functionality related to adding cheat managers.

### Dependencies

### `UCheatManager`

- **Usage**: This class interacts with instances of `UCheatManager`, primarily in `OnCheatManagerCreated` and `SpawnCheatManagerExtension` methods.
- **Purpose**: `UCheatManager` is utilized to add or remove cheat manager extensions when the game feature is activated or deactivated.

### `UCheatManagerExtension`

- **Usage**: Used in `SpawnCheatManagerExtension` method and referenced in `CheatManagers` property.
- **Purpose**: Represents the cheat manager extensions that are spawned and managed by this class.

### `TSubclassOf`

- **Usage**: Employed in `SpawnCheatManagerExtension` as a parameter type and in `CheatManagers` property.
- **Purpose**: A template class used to represent subclasses of `UCheatManagerExtension`. It ensures type safety when dealing with class references.

### `TSoftClassPtr`

- **Usage**: Used in `CheatManagers` property.
- **Purpose**: Represents a soft reference to a `UCheatManagerExtension` class, allowing for more flexible and efficient class loading, especially relevant for the `bLoadCheatManagersAsync` property.

### `TWeakObjectPtr`

- **Usage**: Utilized in `SpawnedCheatManagers` property.
- **Purpose**: Maintains weak references to `UCheatManagerExtension` instances. This is important for managing the lifecycle of these extensions without directly owning them, preventing potential memory issues.

### `FDelegateHandle`

- **Usage**: Used in `CheatManagerRegistrationHandle` property.
- **Purpose**: Stores a handle to the delegate registered with the `UCheatManager`, allowing for proper management and unregistration of the delegate.

### Macros and Constants

- `LOCTEXT_NAMESPACE`: Defined as `"GameFeatures"`, it's used in the `IsDataValid` method for localization purposes.

### Preprocessor Directives

- `#if WITH_EDITOR`: Wraps around the `IsDataValid` method, indicating that this method is only compiled and relevant in the editor environment, not in the final game build.

### Additional Class Metadata

- `UCLASS(MinimalAPI, meta=(DisplayName="Add Cheats"))`: 
  - `MinimalAPI`: Indicates minimal exposure of the class to the engine's API.
  - `meta=(DisplayName="Add Cheats")`: Sets a more user-friendly display name for the class in the Unreal Editor.

---

## Macros and Constants

### `LOCTEXT_NAMESPACE`

- **Definition**: `#define LOCTEXT_NAMESPACE "GameFeatures"`
- **Purpose**: 
  - Used to define a namespace for localization purposes. 
  - In `IsDataValid`, it helps in formatting localized error messages.

## Preprocessor Directives

### `#if WITH_EDITOR`

- **Usage**: This directive wraps around the `IsDataValid` method.
- **Purpose**: 
  - Ensures that the `IsDataValid` method is only included in the build when compiling for the Unreal Editor. 
  - This method is not relevant in non-editor (runtime) builds of the game.

### `#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_AddCheats)`

- **Usage**: Inclusion of inline-generated C++ code specific to the `GameFeatureAction_AddCheats` class.
- **Purpose**: 
  - Automatically includes necessary C++ code generated by Unreal's build system, tailored for this specific class.

### `#pragma once`

- **Usage**: At the beginning of the header file.
- **Purpose**: 
  - Ensures the file is only included once in a single compilation, preventing duplicate definitions.

### `#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_2`

- **Usage**: Wrapping include directives for core and framework headers.
- **Purpose**: 
  - Provides backward compatibility with Unreal Engine versions where include order was crucial. 
  - Ensures newer versions of Unreal that deprecate specific include order dependencies don't process this code.

## Additional Class Metadata

### `UCLASS` Declaration

- **Syntax**: `UCLASS(MinimalAPI, meta=(DisplayName="Add Cheats"))`
- **Components**:
  - `MinimalAPI`: Indicates that the class has minimal exposure to the engine's API. This often means that only a limited set of functionalities are exposed for use in other modules or plugins.
  - `meta=(DisplayName="Add Cheats")`: A metadata specifier that sets a user-friendly name for the class in the Unreal Editor. This name is more readable and can be different from the actual class name, which is beneficial for editor users.

### `GENERATED_BODY()`

- **Usage**: Inside the class definition.
- **Purpose**: 
  - Marks the point in the class definition where generated code by Unreal Engine's UHT (Unreal Header Tool) should be inserted.
  - Essential for Unreal's reflection system, allowing the class to integrate with the engine's serialization, networking, and editor functionalities.
