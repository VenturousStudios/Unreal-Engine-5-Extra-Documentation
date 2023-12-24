---
layout: default
title: Action: Add Cheats
parent: GameFeatures
nav_order: 1
---
# Add Cheats Action

## Description

The UGameFeatureAction_AddCheats class in Unreal Engine 5 is a specialized class designed to augment game features by integrating cheat functionalities. This class is part of the game feature plugin system, which allows developers to modularly add or modify features in a game. Specifically, UGameFeatureAction_AddCheats focuses on extending the game's cheat system through the addition of custom cheat manager extensions.

Key aspects of this class include:

Cheat Manager Extensions Integration: The primary purpose of this class is to add custom cheat manager extensions to the game. These extensions allow for the implementation of new cheat commands or functionalities that can be used during game development for testing or debugging purposes.

Game Feature Plugin Enhancement: As part of the game feature plugin architecture, this class provides a way to dynamically augment a game's capabilities, particularly in terms of cheat-related features, without altering the core game code.

Activation and Deactivation Handling: The class manages the lifecycle of cheat extensions by properly handling their activation when a game feature is turned on and ensuring clean deactivation and cleanup when the feature is turned off.

Asynchronous Loading Support: It supports the asynchronous loading of cheat managers, enabling efficient management of resources and smoother gameplay experience, particularly when dealing with large or complex game features.

Editor-Level Data Validation: In the Unreal Engine editor environment, the class includes functionality to validate the data related to cheat managers, ensuring that the setup is correct and ready for use within the game.

Modular and Reusable Design: The design of the class aligns with Unreal Engine's emphasis on modular and reusable components, making it easier for developers to implement and manage cheat-related features across different projects or game modules.

## Key Components and Functionalities

1. Cheat Manager Extensions Management
Description
This component is responsible for adding and removing cheat manager extensions. It enhances the game's cheat manager by dynamically integrating additional functionalities.

Functionalities
Add Extensions: When a game feature is activated, it registers a callback for cheat manager creation and adds specified cheat manager extensions.
Remove Extensions: Upon deactivation of the game feature, it removes all added extensions from the cheat manager and cleans up references.
2. Game Feature Activation and Deactivation
Description
Overrides methods from UGameFeatureAction to handle specific logic when a game feature is activated or deactivated.

Functionalities
OnGameFeatureActivating: Sets the bIsActive flag to true and registers for cheat manager creation.
OnGameFeatureDeactivating: Unregisters from cheat manager creation, removes extensions, empties the list of spawned cheat managers, and sets bIsActive to false.
3. Cheat Manager Creation Handling
Description
Provides a mechanism to respond when a cheat manager is created, allowing the class to integrate extensions seamlessly.

Functionalities
OnCheatManagerCreated: Invoked when a cheat manager is created. It cleans out stale pointers and adds extensions to the newly created cheat manager.
4. Data Validation (Editor-Only)
Description
Implements a validation system in the editor to ensure the integrity and correctness of the cheat manager extensions data.

Functionalities
IsDataValid: Checks each entry in the CheatManagers array for validity, reporting errors in the editor for any null entries.
5. Extension Instantiation
Description
Handles the instantiation of cheat manager extensions and their addition to the game's cheat manager.

Functionalities
SpawnCheatManagerExtension: Creates a new instance of a cheat manager extension and adds it to the cheat manager. It ensures compatibility between the cheat manager and the extension class.
Additional Notes
Asynchronous Loading: Optionally supports asynchronous loading of cheat manager extensions, enhancing performance and reducing loading times.
Editor and Runtime Usage: Designed for both editor and runtime environments in Unreal Engine, with certain functionalities (like data validation) exclusive to the editor.
Dynamic Extension Management: Facilitates a dynamic and flexible approach to extending cheat functionalities, aligning with the modular design principles of Unreal Engine.

