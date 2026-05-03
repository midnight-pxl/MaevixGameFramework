// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

/**
 * MCore_DA_SettingDefinition.h
 *
 * DataAsset defining a single game setting: type, value range, defaults,
 * category, and how it applies to engine systems (GameUserSettings, CVars,
 * SoundClasses).
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "MCore_SettingsTypes.h"
#include "MCore_DA_SettingDefinition.generated.h"

class UInputAction;
class UInputMappingContext;
class USoundClass;
class USoundMix;
class UMCore_SettingsWidget_Base;

/* How slider values display in the UI */
UENUM(BlueprintType)
enum class EMCore_SliderDisplayFormat : uint8
{
	/* Show raw value: 0.75 */
	RawValue,
	/* Show percentage: 75% */
	Percentage,
	/* Show integer: 75 */
	WholeNumber
};

/* Role this setting plays in driving the Slate color-vision-deficiency filter.
   Author two DAs — a Dropdown for Type and a Slider for Severity — both with
   this field non-None. The library caches Type + Severity across invocations
   so either DA's apply re-issues a SetColorVisionDeficiencyType call with the
   current value of both axes. */
UENUM(BlueprintType)
enum class EModulusColorVisionRole : uint8
{
	None               UMETA(DisplayName = "None"),
	DeficiencyType     UMETA(DisplayName = "Deficiency Type (Dropdown)"),
	DeficiencySeverity UMETA(DisplayName = "Deficiency Severity (Slider)")
};

/**
 * DataAsset defining a single game setting with type, value range, defaults, and engine apply targets.
 * Supports Toggle, Slider, and Dropdown types with optional confirmation countdown.
 *
 * Create in Content Browser; reference from UMCore_DA_SettingsCollection.
 */
UCLASS(BlueprintType, Const)
class MODULUSCORE_API UMCore_DA_SettingDefinition : public UDataAsset
{
	GENERATED_BODY()

public:

	// ============================================================================
	// IDENTITY
	// ============================================================================

	/* Unique gameplay tag identifying this setting (e.g. Settings.Graphics.ShadowQuality) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setting|Identity",
		meta = (Categories = "Settings"))
	FGameplayTag SettingTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setting|Identity")
	FText DisplayName;

	/* Optional tooltip/description */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setting|Identity")
	FText Description;

	// ============================================================================
	// TYPE & VALUES
	// ============================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setting|Type")
	EMCore_SettingType SettingType = EMCore_SettingType::Toggle;

	/* How this slider value displays in the UI */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setting|Slider",
		meta = (EditCondition = "SettingType == EMCore_SettingType::Slider",
				EditConditionHides))
	EMCore_SliderDisplayFormat SliderDisplayFormat = EMCore_SliderDisplayFormat::RawValue;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setting|Slider",
		meta = (EditCondition = "SettingType == EMCore_SettingType::Slider",
				EditConditionHides, ClampMin = "0.0"))
	float MinValue{0.0f};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setting|Slider",
		meta = (EditCondition = "SettingType == EMCore_SettingType::Slider",
				EditConditionHides, ClampMin = "0.0"))
	float MaxValue{1.0f};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setting|Slider",
		meta = (EditCondition = "SettingType == EMCore_SettingType::Slider",
				EditConditionHides))
	float StepSize{0.02f};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setting|Slider",
		meta = (EditCondition = "SettingType == EMCore_SettingType::Slider",
				EditConditionHides))
	float DefaultValue{0.5f};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setting|Toggle",
		meta = (EditCondition = "SettingType == EMCore_SettingType::Toggle",
				EditConditionHides))
	bool DefaultToggleValue{false};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setting|Dropdown",
		meta = (EditCondition = "SettingType == EMCore_SettingType::Dropdown",
				EditConditionHides))
	TArray<FText> DropdownOptions;

	/** If > 0, user can only cycle indices 0..NumSelectableOptions-1. Higher indices are
	 *  display-only (e.g. "Custom" status). 0 means all DropdownOptions are selectable. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting|Dropdown",
		meta = (EditCondition = "SettingType == EMCore_SettingType::Dropdown",
				EditConditionHides, ClampMin = "0"))
	int32 NumSelectableOptions{0};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setting|Dropdown",
		meta = (EditCondition = "SettingType == EMCore_SettingType::Dropdown",
				EditConditionHides, ClampMin = "0"))
	int32 DefaultDropdownIndex{0};

	// ============================================================================
	// CATEGORIZATION
	// ============================================================================

	/* Category tag for grouping (e.g. Settings.Category.Graphics) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setting|Category",
		meta = (Categories = "Settings.Category"))
	FGameplayTag CategoryTag;

	/**
	 * Optional visual grouping label within a settings page.
	 * Settings sharing the same SectionName under a sub-category are rendered
	 * under a shared section header. Empty means no section header.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting|Category")
	FText SectionName;

	// ============================================================================
	// APPLY CONFIGURATION
	// ============================================================================

	/* Engine setter target. Routes via the three-bucket dispatcher in
	 * MCore_GameSettingsLibrary::ApplyViaNamedSetter:
	 *   Bucket 1: Top-level UPROPERTY on UGameUserSettings (e.g. bUseVSync,
	 *             FullscreenMode, AudioQualityLevel, FrameRateLimit).
	 *   Bucket 2: Member of UGameUserSettings::ScalabilityQuality struct
	 *             (e.g. TextureQuality, EffectsQuality, ReflectionQuality).
	 *   Bucket 3: Function-dispatch for irreducible operations
	 *             (OverallScalabilityLevel, ScreenResolution,
	 *              EnableHDRDisplayOutput, DisplayGamma, ApplicationScale).
	 *
	 * Use the literal engine name. Do not translate or invent keys. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setting|Apply",
		meta = (DisplayName = "Named Setter"))
	FName NamedSetter = NAME_None;

	/** Optional override of the widget class to instantiate for this setting.
	 *  Null = use the type-driven default from CoreSettings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setting|Apply",
		meta = (DisplayName = "Widget Class Override"))
	TSubclassOf<UMCore_SettingsWidget_Base> WidgetClassOverride;

	/* Console variable to write to (empty = none) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setting|Apply")
	FName ConsoleVariable;

	/* Sound class to adjust volume on (Audio settings) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setting|Apply",
		meta = (EditCondition = "SettingType == EMCore_SettingType::Slider",
		        EditConditionHides))
	TSoftObjectPtr<USoundClass> SoundClass;

	/* When true, widget queries UKismetSystemLibrary::GetSupportedFullscreenResolutions
	   at runtime instead of using DA-authored DropdownOptions */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setting|Apply",
		meta = (EditCondition = "SettingType == EMCore_SettingType::Dropdown",
		        EditConditionHides))
	bool bPopulateFromSupportedResolutions = false;

	/* Drives UWidgetBlueprintLibrary::SetColorVisionDeficiencyType at the Slate
	   renderer layer. Default None = inert. Pair two DAs: a Dropdown for Type
	   (values 0-3 = Normal/Deuteranope/Protanope/Tritanope) and a Slider for
	   Severity (0.0-10.0). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setting|Apply")
	EModulusColorVisionRole ColorVisionRole = EModulusColorVisionRole::None;

	/* SoundMix pushed when toggle is ON, popped when OFF.
	   Library tracks matched push/pop pairs per setting save key */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setting|Apply",
		meta = (EditCondition = "SettingType == EMCore_SettingType::Toggle",
		        EditConditionHides))
	TSoftObjectPtr<USoundMix> PushedSoundMix;

	// ============================================================================
	// BEHAVIOR
	// ============================================================================

	/**
	 * If true, applying this setting shows UMCore_SettingsRevertCountdown.
	 * Use for destructive settings (resolution, display mode) that may render
	 * the screen unreadable. The countdown auto-reverts if not confirmed.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Setting|Behavior",
		meta = (DisplayName = "Requires Confirmation"))
	bool bRequiresConfirmation = false;

	// ============================================================================
	// METHODS
	// ============================================================================

	/** Returns the save key derived from SettingTag (dots replaced with underscores). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ModulusCore|Settings")
	FString GetSaveKey() const;

	/** Validates this definition. Returns false if SettingTag/DisplayName is missing or values are out of range. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ModulusCore|Settings")
	bool IsValid() const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
