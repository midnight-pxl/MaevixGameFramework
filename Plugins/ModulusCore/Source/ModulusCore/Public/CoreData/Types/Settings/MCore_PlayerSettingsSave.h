// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/SaveGame.h"
#include "MCore_PlayerSettingsSave.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnPlayerSettingsLoaded, UMCore_PlayerSettingsSave*, PlayerSettings);

/**
 * Player settings save game using immediate-apply model.
 * Stores framework UI state (scale, tooltip delay) and generic typed setting values
 * written by UMCore_GameSettingsLibrary's setters.
 *
 * Access via UMCore_PlayerSettingsSubsystem::GetPlayerSettings(). Modify via GameSettingsLibrary (preferred).
 */
UCLASS()
class MODULUSCORE_API UMCore_PlayerSettingsSave : public USaveGame
{
	GENERATED_BODY()

	public:
    UMCore_PlayerSettingsSave();

	// ========================================================================
	// FRAMEWORK UI STATE
	// ========================================================================

    /** UI scale multiplier (0.5 to 3.0) */
    UPROPERTY(SaveGame)
    float UIScale{1.0f};

    /** Tooltip hover delay in milliseconds */
    UPROPERTY(SaveGame)
    int32 TooltipDelayMs{500};

    /** Last settings tab the player had open (restored on re-open) */
    UPROPERTY(SaveGame)
    FGameplayTag LastSelectedCategory;

    UPROPERTY(SaveGame)
    TSet<FGameplayTag> CollapsedCategories;

    UPROPERTY(SaveGame)
    bool bHasSeenWelcomeScreen{false};

    /** Index into the available gamepad controller configs. 0 = Auto-Detect. */
    UPROPERTY(SaveGame, BlueprintReadWrite, Category = "ModulusCore|Settings")
    int32 GamepadIconSetIndex = 0;

	/** Last value the user explicitly selected for the QualityPreset (OverallScalabilityLevel).
	   -1 means Custom (user has tweaked individual scalability settings). 0..3 are preset levels. */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "ModulusCore|Settings|QualityPreset")
	int32 LastSelectedQualityPreset{-1};

	/** Disambiguates first-load (-1 = uninitialized, will be backfilled from engine state) from
	   explicitly-Custom (-1 chosen because user tweaked an individual scalability). */
	UPROPERTY(SaveGame)
	bool bQualityPresetInitialized{false};

	/** Soft path to the user's active UMCore_PDA_UITheme_Base. Empty path =
	   no override (UISubsystem falls back to project default). FSoftObjectPath
	   survives asset moves/renames better than an FName ID. */
	UPROPERTY(SaveGame)
	FSoftObjectPath ActiveThemePath;

	// ========================================================================
	// GENERIC SETTING STORAGE
	// ========================================================================

	UPROPERTY(SaveGame)
	TMap<FString, float> FloatSettings;

	UPROPERTY(SaveGame)
	TMap<FString, int32> IntSettings;

	UPROPERTY(SaveGame)
	TMap<FString, bool> BoolSettings;

	// ========================================================================
	// GENERIC ACCESSORS
	// ========================================================================

	/** Returns true and populates OutValue if a float value exists for the given key. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ModulusCore|Settings")
	bool GetFloatSetting(const FString& Key, float& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "ModulusCore|Settings")
	void SetFloatSetting(const FString& Key, float Value);

	/** Returns true and populates OutValue if an int value exists for the given key. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ModulusCore|Settings")
	bool GetIntSetting(const FString& Key, int32& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "ModulusCore|Settings")
	void SetIntSetting(const FString& Key, int32 Value);

	/** Returns true and populates OutValue if a bool value exists for the given key. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ModulusCore|Settings")
	bool GetBoolSetting(const FString& Key, bool& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "ModulusCore|Settings")
	void SetBoolSetting(const FString& Key, bool Value);

	// ========================================================================
	// PERSISTENCE
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "ModulusCore|Settings")
	void SaveSettings();

	/**
	 * Load player settings from disk (synchronous).
	 * Returns existing save if found, or creates new instance with defaults.
	 * Caches the slot name on the returned object for use by SaveSettings().
	 */
	UFUNCTION(BlueprintCallable, Category = "ModulusCore|Settings")
	static UMCore_PlayerSettingsSave* LoadPlayerSettings(const FString& SlotName);

	/**
	 * Load player settings from disk (asynchronous).
	 * Falls back to synchronous creation if no save file exists.
	 * Caches the slot name on the returned object for use by SaveSettings().
	 */
	UFUNCTION(BlueprintCallable, Category = "ModulusCore|Settings")
	static void LoadPlayerSettingsAsync(const FString& SlotName, FOnPlayerSettingsLoaded OnLoaded);

	const FString& GetCachedSlotName() const { return CachedSlotName; }

	// ========================================================================
	// FRAMEWORK CONVENIENCE
	// ========================================================================

	/** Set UI scale (clamped 0.5-3.0) and apply immediately. */
	UFUNCTION(BlueprintCallable, Category = "ModulusCore|Settings")
	void SetUIScale(float NewScale);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ModulusCore|Settings")
	float GetUIScale() const { return UIScale; }

	UFUNCTION(BlueprintCallable, Category = "ModulusCore|Settings")
	void SetTooltipDelay(int32 DelayMs);

	UFUNCTION(BlueprintCallable, Category = "ModulusCore|Settings")
	void SetCategoryCollapsed(const FGameplayTag& CategoryTag, bool bCollapsed);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ModulusCore|Settings")
	bool IsCategoryCollapsed(const FGameplayTag& CategoryTag) const;

	UFUNCTION(BlueprintCallable, Category = "ModulusCore|Settings")
	void SetLastSelectedCategory(const FGameplayTag& CategoryTag);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ModulusCore|Settings")
	int32 GetLastSelectedQualityPreset() const { return LastSelectedQualityPreset; }

	/** Sets the last user-selected QualityPreset intent. Caller is responsible for persistence. */
	UFUNCTION(BlueprintCallable, Category = "ModulusCore|Settings")
	void SetLastSelectedQualityPreset(int32 NewValue);

	/**
	 * Validate all stored values and fix any out-of-range data.
	 *
	 * Blueprint Usage:
	 *   Called automatically after load. Can be called manually after bulk edits.
	 */
	UFUNCTION(BlueprintCallable, Category = "ModulusCore|Settings")
	void ValidateSettings();

private:
	FString CachedSlotName;

	void ApplyUIScale();
};
