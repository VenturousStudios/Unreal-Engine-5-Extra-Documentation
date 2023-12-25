---
layout: page
title: Action - Add Data Registry Source
parent: Game Features
nav_order: 3
---
# Data Registry Source Class Reference

## Overview
`UGameFeatureAction_DataRegistrySource` is a class derived from `UGameFeatureAction` in Unreal Engine 5, primarily used for managing data sources in the context of game features. It facilitates the dynamic addition and removal of data tables and curve tables to the data registry when a game feature is activated or deactivated.

## Class Members
- **Public Methods:**
  - `OnGameFeatureRegistering()`: Invoked when a game feature is being registered. It preloads data sources if required.
  - `OnGameFeatureActivating()`: Called during the activation phase of a game feature, responsible for adding specified data sources to the data registry.
  - `OnGameFeatureUnregistering()`: Handles tasks when a game feature is being unregistered, particularly removing preloaded data sources.
  - `OnGameFeatureDeactivating()`: Engaged during the deactivation of a game feature, ensuring the removal of data sources from the registry.
  - `ShouldPreloadAtRegistration()`: Determines whether data sources should be preloaded during the feature's registration phase.

- **Editor-Only Methods:**
  - `AddAdditionalAssetBundleData()`: Adds data assets to an asset bundle for preloading, applicable only in the editor.
  - `IsDataValid()`: Validates the data, ensuring sources are correctly specified for the client, server, or both.

- **Private Member Variables:**
  - `SourcesToAdd`: An array of `FDataRegistrySourceToAdd`, representing the sources to be added to the registry.
  - `bPreloadInEditor`: A boolean flag indicating if sources should be preloaded in the editor environment.

## Struct: FDataRegistrySourceToAdd
A USTRUCT defining properties for data registry sources to be added. Includes fields such as `RegistryToAddTo`, `AssetPriority`, `bClientSource`, `bServerSource`, `DataTableToAdd`, and `CurveTableToAdd`.

## Detailed Description
This class plays a crucial role in managing data registries in UE5 game features. It leverages a series of overridden methods from its parent class `UGameFeatureAction` to control when and how data sources (like data tables and curve tables) are added to or removed from the game's data registry. The addition and removal are contingent on the game feature's lifecycle events — registering, activating, deactivating, and unregistering.

The class also includes functionality specific to the Unreal Editor, allowing developers to preload data sources for editor sessions and validate the data setup. The struct `FDataRegistrySourceToAdd` aids in specifying the details for each data source, including its type, priority, and intended usage environment (client or server).

The implementation of `UGameFeatureAction_DataRegistrySource` ensures a flexible and dynamic management of data registries, essential for games that rely on modular features and assets.
