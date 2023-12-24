---
layout: page
title: Action - Add Components
parent: Game Features
nav_order: 1
---
# Add Components Action

The `UGameFeatureAction_AddComponents` class in Unreal Engine 5 is designed to dynamically manage the addition and removal of components to actors based on the activation and deactivation of game features. This functionality is integral to supporting modular gameplay elements, allowing for a flexible and extensible approach to game development.

## Key Functionalities

- **Dynamic Component Management**: 
  The class automatically adds specified components to actors when a game feature is activated and removes them when the feature is deactivated. This process is integral to adapting the game's behavior and features in real-time.

- **Game Feature Context Sensitivity**: 
  It operates based on the context of game feature activation and deactivation, ensuring that components are added or removed appropriately in different game instances and world contexts.

- **Editor Support for Asset Bundling and Validation**: 
  In the Unreal Editor, the class includes functions for managing asset bundle data related to the components and for validating the data to ensure that the component and actor classes are correctly specified and no null references exist.

- **Support for Networked Environments**: 
  The class accounts for different network modes, handling component addition for both clients and servers. This is crucial for multiplayer games where different logic might be needed on the server and client sides.

- **Extensibility and Customization**: 
  By allowing developers to specify which components should be added to which actors, the class provides a high degree of customization and extensibility. This makes it easier to develop game features that can be toggled on or off or modified without major changes to the core game code.

- **Error Logging and Data Validation (Editor Only)**: 
  It includes robust error logging and data validation mechanisms, particularly useful during the game development process for ensuring integrity and correctness of game feature implementations.

---

## Public Interface

### Overview
`UGameFeatureAction_AddComponents` is a class derived from `UGameFeatureAction` that facilitates the dynamic addition and removal of components to actors in Unreal Engine 5. This class is primarily used in the modular gameplay framework, particularly in managing game features' runtime behavior.

### Methods

### `virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override`
Called when a game feature is being activated. This method handles the initial setup required for adding components to actors based on the current game feature activation context.

- **Parameters:**
  - `FGameFeatureActivatingContext& Context`: The context of the game feature being activated.
- **Operation:**
  - Registers a delegate with `FWorldDelegates::OnStartGameInstance`.
  - Iterates through all world contexts to add components to applicable worlds.

### `virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override`
Invoked when a game feature is being deactivated. It cleans up and removes the components added to actors during the feature's activation.

- **Parameters:**
  - `FGameFeatureDeactivatingContext& Context`: The context of the game feature being deactivated.
- **Operation:**
  - Removes the delegate from `FWorldDelegates::OnStartGameInstance`.
  - Empties the handles to the component requests, effectively cleaning up the added components.

### `virtual void AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData) override` [WITH_EDITORONLY_DATA]
Only compiled in editor builds, this method adds additional asset bundle data necessary for the game feature.

- **Parameters:**
  - `FAssetBundleData& AssetBundleData`: The asset bundle data to be modified.
- **Operation:**
  - Iterates through the `ComponentList` and adds assets to the bundle data based on client and server component flags.

### `virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override` [WITH_EDITOR]
Also specific to editor builds, this method validates the data of the `UGameFeatureAction_AddComponents` instance.

- **Parameters:**
  - `FDataValidationContext& Context`: Context for data validation.
- **Returns:**
  - `EDataValidationResult`: The result of the data validation process.
- **Operation:**
  - Checks each entry in the `ComponentList` for null actor or component classes and logs errors accordingly.

### Properties

### `TArray<FGameFeatureComponentEntry> ComponentList`
Stores a list of components to add to gameplay actors when the game feature is enabled.

- **Type:** `TArray<FGameFeatureComponentEntry>`
- **Usage:**
  - Each entry specifies an actor class and a corresponding component class that should be added to actors of that type.
  - It includes flags to determine if the component should be added on clients, servers, or both.

### Notes
- This class is a part of Unreal Engine 5's game features system and is used to dynamically modify actors in response to game feature state changes.
- Editor-specific methods (`AddAdditionalAssetBundleData` and `IsDataValid`) are included for asset management and data validation during development.

---

## Private Methods

### Overview
The private methods in `UGameFeatureAction_AddComponents` are crucial for the internal workings of the class, handling the specifics of adding components to actors within a game world context and managing the game instance start process.

### Methods

### `void AddToWorld(const FWorldContext& WorldContext, FContextHandles& Handles)`
Adds component requests to the `UGameFrameworkComponentManager` for actors in a given world context.

- **Parameters:**
  - `const FWorldContext& WorldContext`: The context of the world where components need to be added.
  - `FContextHandles& Handles`: Handles associated with the current game feature activation context.
- **Operation:**
  - Checks if the world is a game world and retrieves the `UGameFrameworkComponentManager`.
  - Determines the network mode (client/server) and iterates through the `ComponentList`.
  - Adds component requests for applicable actor-component pairs based on network mode.

### `void HandleGameInstanceStart(UGameInstance* GameInstance, FGameFeatureStateChangeContext ChangeContext)`
Handles the start of a game instance, adding components to actors as necessary.

- **Parameters:**
  - `UGameInstance* GameInstance`: The game instance that has started.
  - `FGameFeatureStateChangeContext ChangeContext`: Context for the state change triggering this method.
- **Operation:**
  - Retrieves the world context from the game instance.
  - If the change context is applicable to the world context, it triggers `AddToWorld` using the retrieved world context and associated handles.

### Notes
- These methods are integral to the dynamic nature of game feature activations and deactivations in Unreal Engine 5, particularly in the modular gameplay framework.
- They ensure that the addition and management of components on actors are handled efficiently and contextually, adhering to the state changes of game features.

---

## Structs and Enums

### `FGameFeatureComponentEntry`
Defines a structure for mapping an actor class to a component class, along with flags indicating client and server applicability.

- **Members:**
  - `TSoftClassPtr<AActor> ActorClass`: Specifies the base actor class to add a component to.
  - `TSoftClassPtr<UActorComponent> ComponentClass`: Indicates the component class to add to the specified type of actor.
  - `uint8 bClientComponent`: Boolean flag indicating if the component should be added for clients.
  - `uint8 bServerComponent`: Boolean flag indicating if the component should be added on servers.
- **Usage:**
  - Utilized within `UGameFeatureAction_AddComponents` to define the list of actor-component pairings that are to be added when a game feature is activated.

### `FContextHandles`
A structure to store handles related to a specific game feature activation context, including handles for game instance start and component requests.

- **Members:**
  - `FDelegateHandle GameInstanceStartHandle`: Handle for the game instance start delegate.
  - `TArray<TSharedPtr<FComponentRequestHandle>> ComponentRequestHandles`: List of handles for the component requests made.

## Member Variables

### `TArray<FGameFeatureComponentEntry> ComponentList`
An array of `FGameFeatureComponentEntry` structs, defining the components to be added to actors when a game feature is activated.

### `TMap<FGameFeatureStateChangeContext, FContextHandles> ContextHandles`
A mapping from a game feature state change context to its associated `FContextHandles`. This map is used to manage and access context-specific handles and component requests.

## Preprocessor Directives

### Conditional Compilation
- `#if WITH_EDITOR`
  - Includes editor-specific functionality for data validation.
- `#if WITH_EDITORONLY_DATA`
  - Encloses methods used only in editor builds, such as `AddAdditionalAssetBundleData`.

### Include Guards
- `#pragma once`
  - Ensures the header file is included only once in a single compilation.

### Namespace Definition
- `#define LOCTEXT_NAMESPACE "GameFeatures"`
  - Defines a namespace for localization texts, particularly for use in logging and error messages within this class.

### Unreal Engine Generated Macros
- `GENERATED_BODY()`
  - Used in structs and classes for necessary Unreal Engine boilerplate code.

### Deprecated Include Order Check
- `#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_2`
  - Checks for the deprecated include order in Unreal Engine 5.2, indicating a shift in how includes are handled in the engine.

## Notes
- The use of structs like `FGameFeatureComponentEntry` and `FContextHandles` demonstrates the encapsulation of related data for ease of management and clarity in the game feature system.
- Preprocessor directives, especially those for conditional compilation, highlight the flexibility of the class in different build environments (editor vs runtime).
