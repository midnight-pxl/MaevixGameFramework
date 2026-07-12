// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "CoreData/Types/Settings/MCore_SettingsTypes.h"
#include "MCore_GameSettingsLibrary.generated.h"

class UMCore_DA_SettingDefinition;
class UMCore_PlayerSettingsSave;
class USoundClass;
class USoundMix;

/**
 * Broadcast when one or more settings with bRequiresConfirmation are applied.
 * Carries the affected setting tags for direct C++ subscription without
 * requiring an EventListenerComp.
 */
DECLARE_MULTICAST_DELEGATE_OneParam(
	FOnSettingsConfirmationRequired,
	const TArray<FGameplayTag>&  /* AffectedTags */
);

/**
 * Game settings library orchestrating the DataAsset-driven settings system.
 * Provides typed getters/setters, tag-based lookup, batch operations, and
 * immediate-apply semantics.
 *
 * Each setter writes to committed storage, applies to engine systems (GameUserSettings,
 * CVars, SoundClasses), and saves to disk. Settings with bRequiresConfirmation defer
 * the save until the player confirms via a revert countdown.
 */
UCLASS()
class MAEVIXCORE_API UMCore_GameSettingsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Static delegate for direct C++ subscription to confirmation-required settings events.
	 * Fires when any setting with bRequiresConfirmation is changed without bBypassConfirmation.
	 */
	static FOnSettingsConfirmationRequired OnSettingsConfirmationRequired;

	// ============================================================================
	// TYPED GETTERS
	// ============================================================================

	/** Returns effective float value for a setting (committed, then DataAsset default). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MaevixCore|Settings",
		meta = (WorldContext = "WorldContextObject"))
	static float GetSettingFloat(const UObject* WorldContextObject,
		const UMCore_DA_SettingDefinition* Setting);

	/** Returns effective integer value for a setting (committed, then DataAsset default). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MaevixCore|Settings",
		meta = (WorldContext = "WorldContextObject"))
	static int32 GetSettingInt(const UObject* WorldContextObject,
		const UMCore_DA_SettingDefinition* Setting);

	/** Returns effective boolean value for a setting (committed, then DataAsset default). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MaevixCore|Settings",
		meta = (WorldContext = "WorldContextObject"))
	static bool GetSettingBool(const UObject* WorldContextObject,
		const UMCore_DA_SettingDefinition* Setting);

	// ============================================================================
	// TAG-BASED GETTERS
	// ============================================================================

	/** Returns effective float value by tag (resolved from CoreSettings::SettingsCollections). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MaevixCore|Settings",
		meta = (WorldContext = "WorldContextObject"))
	static float GetSettingFloatByTag(const UObject* WorldContextObject,
		FGameplayTag SettingTag);

	/**
	 * Returns effective integer value by tag (resolved from CoreSettings::SettingsCollections).
	 * Use when the caller has a setting tag but not a definition pointer.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MaevixCore|Settings",
		meta = (WorldContext = "WorldContextObject"))
	static int32 GetSettingIntByTag(const UObject* WorldContextObject,
		FGameplayTag SettingTag);

	/** Returns effective bool value by tag (resolved from CoreSettings::SettingsCollections). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MaevixCore|Settings",
		meta = (WorldContext = "WorldContextObject"))
	static bool GetSettingBoolByTag(const UObject* WorldContextObject,
		FGameplayTag SettingTag);

	// ============================================================================
	// TYPED SETTERS (IMMEDIATE-APPLY, BATCH)
	// ============================================================================

	/**
	 * Set one or more float settings, apply to engine, and save.
	 * All changes are applied in a single pass with one GameUserSettings flush at the end.
	 * If any setting has bRequiresConfirmation and bBypassConfirmation is false,
	 * the values are applied but not saved; a confirmation event is broadcast instead.
	 */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|Settings",
		meta = (WorldContext = "WorldContextObject",
				DisplayName = "Set Setting Float"))
	static void SetSettingFloat(
		const UObject* WorldContextObject,
		const TArray<FMCore_FloatSettingChange>& Changes,
		bool bBypassConfirmation = false);

	/** Set one or more integer settings, apply to engine, and save. */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|Settings",
		meta = (WorldContext = "WorldContextObject",
				DisplayName = "Set Setting Int"))
	static void SetSettingInt(
		const UObject* WorldContextObject,
		const TArray<FMCore_IntSettingChange>& Changes,
		bool bBypassConfirmation = false);

	/** Set one or more boolean settings, apply to engine, and save. */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|Settings",
		meta = (WorldContext = "WorldContextObject",
				DisplayName = "Set Setting Bool"))
	static void SetSettingBool(
		const UObject* WorldContextObject,
		const TArray<FMCore_BoolSettingChange>& Changes,
		bool bBypassConfirmation = false);

	// ============================================================================
	// UTILITIES
	// ============================================================================

	/** Reset a single setting to its DataAsset default value (applied immediately). */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|Settings",
		meta = (WorldContext = "WorldContextObject"))
	static void ResetSettingToDefault(const UObject* WorldContextObject,
		const UMCore_DA_SettingDefinition* Setting);

	/**
	 * Reset all settings across all SettingsCollections to their DataAsset defaults.
	 * Batched with a single GameUserSettings flush per type.
	 */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|Settings",
		meta = (WorldContext = "WorldContextObject"))
	static void ResetAllSettingsToDefault(const UObject* WorldContextObject);

	/**
	 * Reset all settings in a specific category to their DataAsset defaults.
	 * Batched with a single GameUserSettings flush per type.
	 */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|Settings",
		meta = (WorldContext = "WorldContextObject"))
	static void ResetCategoryToDefault(const UObject* WorldContextObject,
		FGameplayTag CategoryTag);

	/** Explicitly save player settings to disk. */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|Settings",
		meta = (WorldContext = "WorldContextObject"))
	static void SavePlayerSettings(const UObject* WorldContextObject);

	/** Reloads player settings from disk, replacing in-memory state, and re-applies all values to the engine. */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|Settings",
		meta = (WorldContext = "WorldContextObject"))
	static void ReloadAndApplyFromDisk(const UObject* WorldContextObject);

	/**
	 * Re-applies every persisted setting to the engine without touching disk. Used by the
	 * boot-time replay path; also called internally by ReloadAndApplyFromDisk after refreshing
	 * in-memory state. Idempotent and safe to call repeatedly. Early-outs on dedicated server.
	 */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|Settings",
		meta = (WorldContext = "WorldContextObject"))
	static void ApplyAllSettingsToEngine(const UObject* WorldContextObject);

private:
	// ============================================================================
	// INTERNAL HELPERS
	// ============================================================================

	static UMCore_PlayerSettingsSave* GetPlayerSave(const UObject* WorldContextObject);

	static void BroadcastSettingChanged(const UObject* WorldContextObject,
		const FGameplayTag& SettingTag);

	template<typename TChangeStruct, typename TValue>
	static void ApplySettingChanges_Internal(
		const UObject* WorldContextObject,
		const TArray<TChangeStruct>& Changes,
		bool bBypassConfirmation,
		TFunctionRef<TValue(const UMCore_DA_SettingDefinition*, TValue)> ClampValue,
		TFunctionRef<void(UMCore_PlayerSettingsSave*, const FString&, TValue)> SetCommitted,
		TFunctionRef<void(const UMCore_DA_SettingDefinition*, TValue)> ApplyToEngine);

	template<typename TValue>
	static TValue GetSettingByTag_Internal(
		const UObject* WorldContextObject,
		const FGameplayTag& SettingTag,
		TFunctionRef<TValue(const UObject*, const UMCore_DA_SettingDefinition*)> TypedGetter,
		TValue DefaultReturn,
		const TCHAR* FunctionName);

	// ============================================================================
	// ENGINE APPLY HELPERS
	// ============================================================================

	static void ApplySettingToEngine(const UObject* WorldContextObject,
		const UMCore_DA_SettingDefinition* Setting,
		float FloatValue, int32 IntValue, bool BoolValue);

	/* Definition is optional but required for any NamedSetter that needs back-
	 * reference to its origin DA (e.g., MCore.SetActiveTheme reads ThemeOptions
	 * from Definition). Passing nullptr is safe for legacy buckets. */
	static bool ApplyViaNamedSetter(const FName& SetterName,
		float FloatValue, int32 IntValue, bool bBoolValue,
		const UObject* WorldContextObject,
		const UMCore_DA_SettingDefinition* Definition = nullptr);

	/* Type-aware reflection write. Three-value signature mirrors
	 * ApplyViaNamedSetter: each typed cast picks the value matching its
	 * property type. Returns true on a successful typed write, false (with
	 * a specific warning naming the property, its actual GetCPPType(), and
	 * which typed value was attempted) on type mismatch. */
	static bool WriteReflectedProperty(FProperty* Prop, void* Container,
		float FloatValue, int32 IntValue, bool bBoolValue);

	/* Reads each ScalabilityQuality member from GameUserSettings and writes its value to the matching
	 * DA's save key (Definition->NamedSetter matched against 10 known names). */
	static void CascadeScalabilityValuesToSave(UMCore_PlayerSettingsSave* Save);

	// Sets LastSelectedQualityPreset to -1 (Custom) on the given save. No-op if Save is null.
	static void MarkQualityPresetCustom(UMCore_PlayerSettingsSave* Save);

	static void ApplyToConsoleVariable(const FName& CVarName, float Value);
	static void ApplyToConsoleVariable(const FName& CVarName, int32 Value);
	static void ApplyToConsoleVariable(const FName& CVarName, bool Value);

	static void ApplyToSoundClass(const UObject* WorldContextObject,
		const TSoftObjectPtr<USoundClass>& SoundClassRef, float Volume);

	/* Idempotent. Pushes the configured volume mix to the active audio
	 * device if not already active. Called from ApplyToSoundClass on each
	 * slider commit; no-ops after first push. */
	static void EnsureVolumeMixActive(const UObject* WorldContextObject, USoundMix* VolumeMix);

	static void ApplyToSoundMix(const UObject* WorldContextObject,
		TSoftObjectPtr<USoundMix> SoundMixRef,
		const FString& SaveKey, bool bDesiredActive);

	static bool ApplyToColorVisionDeficiency(const UMCore_DA_SettingDefinition* Definition,
		int32 IntValue, float FloatValue);

	static void ResetDefinitionsToDefault(const UObject* WorldContextObject,
		const TArray<UMCore_DA_SettingDefinition*>& Definitions);
};
