// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CommonInputTypeEnum.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "Styling/SlateBrush.h"
#include "MCore_InputDisplayLibrary.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UEnhancedPlayerMappableKeyProfile;
class APlayerController;
class UInputAction;
class UInputMappingContext;
class ULocalPlayer;

/**
 * Enhanced Input display and rebinding helpers.
 * Query current key bindings, retrieve display names, and remap keys via
 * UE's EnhancedInputUserSettings and PlayerMappableKeyProfile systems.
 */
UCLASS()
class MODULUSCORE_API UMCore_InputDisplayLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	// ============================================================================
	// KEY QUERIES
	// ============================================================================

	/** Returns the first bound key for the given input action, or Invalid if unbound. */
	UFUNCTION(BlueprintPure, Category = "Enhanced Input Display")
	static FKey GetCurrentKeyForAction(APlayerController* OwningPlayer, UInputAction* InputAction);

	/** Returns all bound keys for the given input action. */
	UFUNCTION(BlueprintPure, Category = "Enhanced Input Display")
	static TArray<FKey> GetAllKeysForAction(APlayerController* OwningPlayer, UInputAction* InputAction);

	// ============================================================================
	// ACTION METADATA
	// ============================================================================

	/** Returns the display name from PlayerMappableKeySettings, or empty if not configured. */
	UFUNCTION(BlueprintPure, Category = "Enhanced Input Display")
	static FText GetActionDisplayName(APlayerController* OwningPlayer, UInputAction* InputAction);

	/** Returns the display category from PlayerMappableKeySettings, or empty if not configured. */
	UFUNCTION(BlueprintPure, Category = "Enhanced Input Display")
	static FText GetActionDisplayCategory(APlayerController* OwningPlayer, UInputAction* InputAction);

	/** Returns true if the input action has PlayerMappableKeySettings and is marked remappable. */
	UFUNCTION(BlueprintPure, Category = "Enhanced Input Display")
	static bool IsActionRemappable(APlayerController* OwningPlayer, UInputAction* InputAction);

	/** Returns the mapping name from PlayerMappableKeySettings for Data Table display. */
	UFUNCTION(BlueprintPure, Category = "Enhanced Input Display")
	static FName GetActionMappingName(APlayerController* OwningPlayer, UInputAction* InputAction);

	// ============================================================================
	// REBINDING OPERATIONS
	// ============================================================================

	/**
	 * Remap an input action to a new key. Validates that the action is remappable
	 * and handles slot-based binding through the active key profile.
	 */
	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Display")
	static bool RemapActionKey(APlayerController* OwningPlayer,
							  UInputAction* InputAction,
							  FKey NewKey,
							  FText& OutError);

	/** Resets a specific mapping slot for an action to its default binding. */
	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Display")
	static bool ResetActionToDefault(APlayerController* OwningPlayer,
									UInputAction* InputAction,
									UPARAM(DisplayName = "Error Message") FText& OutError);

	// ============================================================================
	// REMAPPABLE ACTION QUERIES
	// ============================================================================

	/** Returns all player-mappable key mappings from the active key profile. */
	UFUNCTION(BlueprintCallable, Category = "Modulus|Input Display")
	static TArray<FPlayerKeyMapping> GetAllRemappableActions(APlayerController* OwningPlayer);

	/** Returns player-mappable key mappings filtered to a specific device type. */
	UFUNCTION(BlueprintCallable, Category = "Modulus|Input Display")
	static TArray<FPlayerKeyMapping> GetRemappableActionsForDevice(
		APlayerController* OwningPlayer, ECommonInputType DeviceType);

	// ============================================================================
	// ICON RESOLUTION
	// ============================================================================

	/** Resolve an FKey to its platform-appropriate icon brush using the current input device. */
	UFUNCTION(BlueprintCallable, Category = "Modulus|Input Display")
	static bool GetIconBrushForKey(const ULocalPlayer* LocalPlayer, FKey Key, FSlateBrush& OutBrush);

	/** Resolve an FKey to its icon brush for a specific device type (for dual-column display). */
	UFUNCTION(BlueprintCallable, Category = "Modulus|Input Display")
	static bool GetIconBrushForKeyByDeviceType(const ULocalPlayer* LocalPlayer, FKey Key,
		ECommonInputType InputType, FSlateBrush& OutBrush);

	// ============================================================================
	// GAMEPAD CONFIG HELPERS
	// ============================================================================

	/** Returns the registered gamepad config names from CommonInput platform settings. */
	UFUNCTION(BlueprintPure, Category = "Modulus|Input Display",
		meta = (WorldContext = "WorldContextObject"))
	static TArray<FName> GetAvailableGamepadConfigs(const UObject* WorldContextObject);

	/** Returns the gamepad name to use for icon resolution, respecting the player's manual override. */
	UFUNCTION(BlueprintPure, Category = "Modulus|Input Display",
		meta = (WorldContext = "WorldContextObject"))
	static FName GetEffectiveGamepadName(const UObject* WorldContextObject,
		const ULocalPlayer* LocalPlayer);

	// ============================================================================
	// SLOT-BASED OPERATIONS
	// ============================================================================

	/** Returns the bound key for a specific action, slot, and device type. */
	UFUNCTION(BlueprintPure, Category = "Modulus|Input Display")
	static FKey GetBoundKeyForSlot(APlayerController* OwningPlayer, UInputAction* InputAction,
		EPlayerMappableKeySlot Slot, bool bGamepad);

	/** Remap an action to a new key in a specific slot. Device derived from NewKey. */
	UFUNCTION(BlueprintCallable, Category = "Modulus|Input Display")
	static bool RemapActionKeyForSlot(APlayerController* OwningPlayer, UInputAction* InputAction,
		FKey NewKey, EPlayerMappableKeySlot Slot, FText& OutError);

	/** Reset an action's binding to default for a specific slot and device type. */
	UFUNCTION(BlueprintCallable, Category = "Modulus|Input Display")
	static bool ResetActionToDefaultForSlot(APlayerController* OwningPlayer, UInputAction* InputAction,
		EPlayerMappableKeySlot Slot, bool bGamepad, FText& OutError);

	// ============================================================================
	// CONTEXT-SCOPED QUERIES
	// ============================================================================

	/** Returns deduplicated remappable actions that belong to the given InputMappingContext. */
	UFUNCTION(BlueprintCallable, Category = "Modulus|Input Display")
	static TArray<FPlayerKeyMapping> GetRemappableActionsForContext(
		APlayerController* OwningPlayer, const UInputMappingContext* MappingContext);

	// ============================================================================
	// BULK RESET OPERATIONS
	// ============================================================================

	/** Reset all key bindings across all actions in the active key profile to defaults. */
	UFUNCTION(BlueprintCallable, Category = "Modulus|Input Display")
	static void ResetAllBindingsToDefault(APlayerController* OwningPlayer);

	/** Reset key bindings for all remappable actions within a specific InputMappingContext. */
	UFUNCTION(BlueprintCallable, Category = "Modulus|Input Display")
	static void ResetBindingsForContext(APlayerController* OwningPlayer,
		const UInputMappingContext* MappingContext);

private:
	/* Resolve Enhanced Input subsystem from PlayerController with validation */
	static UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem(APlayerController* OwningPlayer);

	/* Resolve the active key profile (Subsystem -> UserSettings -> Profile) */
	static UEnhancedPlayerMappableKeyProfile* GetActiveKeyProfile(APlayerController* OwningPlayer);
};
