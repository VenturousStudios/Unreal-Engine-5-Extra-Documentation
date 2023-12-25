---
layout: page
title: Action - Add Data Registry
parent: Game Features
nav_order: 3
---
`UGameFeatureAction_DataRegistry` is a subclass of `UGameFeatureAction`, designed for handling data registries within the Unreal Engine 5 framework. This class focuses on the lifecycle management of data registries associated with game features, including their registration, activation, deactivation, and unregistration.

## Functionality

### Core Functions

- `OnGameFeatureRegistering()`: Called when a game feature is being registered. It preloads data registries if required.
- `OnGameFeatureUnregistering()`: Invoked during the unregistering phase of a game feature, handling the temporary disabling of preloaded data registries.
- `OnGameFeatureActivating()`: Executes when a game feature is activated, ensuring data registries are loaded and initialized if not preloaded during registration.
- `OnGameFeatureDeactivating()`: Called when deactivating a game feature, handling the removal of data registries from consideration.

### Utility Functions

- `ShouldPreloadAtRegistration()`: Determines whether data registries should be preloaded during the registration phase, primarily based on the editor context and specific flags.

### Editor-Only Functions

- `AddAdditionalAssetBundleData()`: (Editor-only) Enhances asset bundle data with additional data registry assets.
- `IsDataValid()`: (Editor-only) Validates the data registry setup, ensuring all registries are correctly specified and valid.

## Properties

- `RegistriesToAdd`: A list of data registry assets (`UDataRegistry`) to load and initialize.
- `bPreloadInEditor`: A boolean flag indicating whether data registries should be preloaded in the Unreal Editor for supporting editor pickers.

## Implementation Details

- Utilizes `LOCTEXT_NAMESPACE` for localization support.
- Employs conditional compilation with `WITH_EDITOR` and `WITH_EDITORONLY_DATA` for editor-specific functionalities.
- Implements robust logging and debugging, leveraging `UE_LOG` for logging key actions and conditions.
- Includes comprehensive error handling and validation mechanisms, especially within the editor context.
- Compatible with editor mode and handles deprecated include orders (`UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_2`).

## Remarks

`UGameFeatureAction_DataRegistry` is a specialized class for managing the lifecycle and integration of data registries in game features, particularly focusing on the preloading and initialization of these registries. It plays a crucial role in ensuring data registries are appropriately handled during different phases of a game feature's lifecycle, especially in an editor environment.
