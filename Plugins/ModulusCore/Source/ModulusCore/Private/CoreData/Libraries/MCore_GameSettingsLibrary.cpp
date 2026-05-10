// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreData/Libraries/MCore_GameSettingsLibrary.h"

#include "CoreData/Assets/UI/Themes/MCore_PDA_UITheme_Base.h"
#include "CoreData/Settings/MCore_PlayerSettingsSubsystem.h"
#include "CoreData/Logging/LogModulusSettings.h"
#include "CoreData/Types/Settings/MCore_PlayerSettingsSave.h"
#include "CoreData/Libraries/MCore_EventFunctionLibrary.h"
#include "CoreData/Types/Settings/MCore_DA_SettingDefinition.h"
#include "CoreData/Types/Settings/MCore_DA_SettingsCollection.h"
#include "CoreData/Tags/MCore_SettingsTags.h"
#include "CoreData/DevSettings/MCore_CoreSettings.h"
#include "CoreUI/MCore_UISubsystem.h"

#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Sound/SoundClass.h"
#include "Engine/LocalPlayer.h"
#include "Engine/UserInterfaceSettings.h"
#include "HAL/IConsoleManager.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Types/SlateEnums.h"
#include "Sound/SoundMix.h"
#include "AudioDevice.h"
#include "Algo/Reverse.h"
#include "Scalability.h"

FOnSettingsConfirmationRequired UMCore_GameSettingsLibrary::OnSettingsConfirmationRequired;

// ============================================================================
// CONSOLE VARIABLES
// ============================================================================

static TAutoConsoleVariable<float> CVarMouseSensitivity(
	TEXT("Modulus.Input.MouseSensitivity"),
	1.0f,
	TEXT("Overall mouse sensitivity multiplier (0.1-10.0, default: 1.0)"),
	ECVF_Default
	);

static TAutoConsoleVariable<float> CVarGamepadSensitivity(
	TEXT("Modulus.Input.GamepadSensitivity"),
	1.0f,
	TEXT("Overall gamepad sensitivity multiplier (0.1-10.0, default: 1.0)"),
	ECVF_Default
	);

static TAutoConsoleVariable<float> CVarLookSensitivityX(
	TEXT("Modulus.Input.LookSensitivityX"),
	2.0f,
	TEXT("Horizontal look sensitivity multiplier (0.1-10.0, default: 2.0)"),
	ECVF_Default
	);

static TAutoConsoleVariable<float> CVarLookSensitivityY(
	TEXT("Modulus.Input.LookSensitivityY"),
	2.0f,
	TEXT("Vertical look sensitivity multiplier (0.1-10.0, default: 2.0)"),
	ECVF_Default
	);

static TAutoConsoleVariable<int32> CVarInvertLookX(
	TEXT("Modulus.Input.InvertLookX"),
	0,
	TEXT("Invert horizontal look axis (0=normal, 1=inverted, default: 0)"),
	ECVF_Default
	);

static TAutoConsoleVariable<int32> CVarInvertLookY(
	TEXT("Modulus.Input.InvertLookY"),
	1,
	TEXT("Invert vertical look axis (0=normal, 1=inverted, default: 1)"),
	ECVF_Default
	);

static TAutoConsoleVariable<int32> CVarVibration(
	TEXT("Modulus.Input.Vibration"),
	1,
	TEXT("Controller vibration (0=off, 1=on)"),
	ECVF_Default
	);

static TAutoConsoleVariable<int32> CVarMouseSmoothing(
	TEXT("Modulus.Input.MouseSmoothing"),
	0,
	TEXT("Mouse smoothing (0=off, 1=on)"),
	ECVF_Default
	);

static TAutoConsoleVariable<int32> CVarSubtitles(
	TEXT("Modulus.Audio.Subtitles"),
	1,
	TEXT("Show subtitles (0=off, 1=on)"),
	ECVF_Default
	);

static TAutoConsoleVariable<int32> CVarScreenReader(
	TEXT("Modulus.Accessibility.ScreenReader"),
	0,
	TEXT("Screen reader support (0=off, 1=on)"),
	ECVF_Default
	);

static TAutoConsoleVariable<int32> CVarLargeText(
	TEXT("Modulus.Accessibility.LargeText"),
	0,
	TEXT("Large text mode (0=off, 1=on)"),
	ECVF_Default
	);

// ============================================================================
// INTERNAL HELPER
// ============================================================================

UMCore_PlayerSettingsSave* UMCore_GameSettingsLibrary::GetPlayerSave(const UObject* WorldContextObject)
{
	if (!WorldContextObject) { return nullptr; }

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject,
		EGetWorldErrorMode::LogAndReturnNull);
	if (!World) { return nullptr; }

	const ULocalPlayer* LocalPlayer = World->GetFirstLocalPlayerFromController();
	if (!LocalPlayer) { return nullptr; }

	UMCore_PlayerSettingsSubsystem* SettingsSubsystem = LocalPlayer->GetSubsystem<UMCore_PlayerSettingsSubsystem>();
	if (!SettingsSubsystem) { return nullptr; }

	return SettingsSubsystem->GetPlayerSettings();
}

// ============================================================================
// TYPED GETTERS
// ============================================================================

float UMCore_GameSettingsLibrary::GetSettingFloat(const UObject* WorldContextObject,
	const UMCore_DA_SettingDefinition* Setting)
{
	if (!Setting) { return 0.0f; }

	if (Setting->SettingType != EMCore_SettingType::Slider)
	{
		UE_LOG(LogModulusSettings, Warning,
			TEXT("GameSettingsLibrary::GetSettingFloat: called on non-Slider setting '%s'"),
			*Setting->SettingTag.ToString());
	}

	if (const UMCore_PlayerSettingsSave* Save = GetPlayerSave(WorldContextObject))
	{
		float StoredValue = 0.f;
		if (Save->GetFloatSetting(Setting->GetSaveKey(), StoredValue))
		{
			return StoredValue;
		}
	}

	return Setting->DefaultValue;
}

int32 UMCore_GameSettingsLibrary::GetSettingInt(const UObject* WorldContextObject,
	const UMCore_DA_SettingDefinition* Setting)
{
	if (!Setting) { return 0; }

	if (Setting->SettingType != EMCore_SettingType::Dropdown)
	{
		UE_LOG(LogModulusSettings, Warning,
			TEXT("GameSettingsLibrary::GetSettingInt: called on non-Dropdown setting '%s'"),
			*Setting->SettingTag.ToString());
	}

	if (const UMCore_PlayerSettingsSave* Save = GetPlayerSave(WorldContextObject))
	{
		int32 StoredValue = 0;
		if (Save->GetIntSetting(Setting->GetSaveKey(), StoredValue))
		{
			return StoredValue;
		}
	}

	return Setting->DefaultDropdownIndex;
}

bool UMCore_GameSettingsLibrary::GetSettingBool(const UObject* WorldContextObject,
	const UMCore_DA_SettingDefinition* Setting)
{
	if (!Setting) { return false; }

	if (Setting->SettingType != EMCore_SettingType::Toggle)
	{
		UE_LOG(LogModulusSettings, Warning,
			TEXT("GameSettingsLibrary::GetSettingBool: called on non-Toggle setting '%s'"),
			*Setting->SettingTag.ToString());
	}

	if (const UMCore_PlayerSettingsSave* Save = GetPlayerSave(WorldContextObject))
	{
		bool StoredValue = false;
		if (Save->GetBoolSetting(Setting->GetSaveKey(), StoredValue))
		{
			return StoredValue;
		}
	}

	return Setting->DefaultToggleValue;
}

// ============================================================================
// TEMPLATE HELPERS
// ============================================================================

template<typename TChangeStruct, typename TValue>
void UMCore_GameSettingsLibrary::ApplySettingChanges_Internal(
	const UObject* WorldContextObject,
	const TArray<TChangeStruct>& Changes,
	bool bBypassConfirmation,
	TFunctionRef<TValue(const UMCore_DA_SettingDefinition*, TValue)> ClampValue,
	TFunctionRef<void(UMCore_PlayerSettingsSave*, const FString&, TValue)> SetCommitted,
	TFunctionRef<void(const UMCore_DA_SettingDefinition*, TValue)> ApplyToEngine)
{
	if (!WorldContextObject || Changes.IsEmpty()) { return; }

	UMCore_PlayerSettingsSave* Save = GetPlayerSave(WorldContextObject);
	if (!Save)
	{
		UE_LOG(LogModulusSettings, Warning,
			TEXT("GameSettingsLibrary::ApplySettingChanges: failed to get PlayerSettingsSave"));
		return;
	}

	TArray<FGameplayTag> AffectedTags;
	TArray<FGameplayTag> ProcessedTags;
	bool bAnyRequiresConfirmation{false};

	for (const TChangeStruct& Change : Changes)
	{
		if (!Change.Setting)
		{
			UE_LOG(LogModulusSettings, Warning,
				TEXT("GameSettingsLibrary::ApplySettingChanges: null Setting in Changes, skipping"));
			continue;
		}

		const FString Key = Change.Setting->GetSaveKey();
		const TValue ClampedVal = ClampValue(Change.Setting, Change.Value);

		SetCommitted(Save, Key, ClampedVal);
		ApplyToEngine(Change.Setting, ClampedVal);

		if (Change.Setting->bRequiresConfirmation && !bBypassConfirmation)
		{
			bAnyRequiresConfirmation = true;
			AffectedTags.Add(Change.Setting->SettingTag);
		}
		else
		{
			ProcessedTags.Add(Change.Setting->SettingTag);
		}
	}

	if (UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings())
	{
		GameUserSettings->ApplySettings(false);
	}

	if (bAnyRequiresConfirmation)
	{
		/* Stringify tags for the event broadcast payload */
		TArray<FString> AffectedTagStrings;
		AffectedTagStrings.Reserve(AffectedTags.Num());
		for (const FGameplayTag& Tag : AffectedTags)
		{
			AffectedTagStrings.Add(Tag.ToString());
		}

		TMap<FString, FString> EventParams;
		EventParams.Add(TEXT("SettingTags"), FString::Join(AffectedTagStrings, TEXT("|")));

		UMCore_EventFunctionLibrary::BroadcastEvent(
			WorldContextObject,
			MCore_SettingsTags::MCore_Settings_Event_ConfirmationRequired,
			EventParams, EMCore_EventScope::Local);

		OnSettingsConfirmationRequired.Broadcast(AffectedTags);
	}
	else
	{
		SavePlayerSettings(WorldContextObject);
	}

	if (!bBypassConfirmation)
	{
		for (const FGameplayTag& Tag : ProcessedTags)
		{
			BroadcastSettingChanged(WorldContextObject, Tag);
		}
	}
}

template<typename TValue>
TValue UMCore_GameSettingsLibrary::GetSettingByTag_Internal(
	const UObject* WorldContextObject,
	const FGameplayTag& SettingTag,
	TFunctionRef<TValue(const UObject*, const UMCore_DA_SettingDefinition*)> TypedGetter,
	TValue DefaultReturn,
	const TCHAR* FunctionName)
{
	const UMCore_DA_SettingDefinition* Setting = UMCore_CoreSettings::Get()->FindSettingDefinitionByTag(SettingTag);
	if (!Setting)
	{
		UE_LOG(LogModulusSettings, Warning,
			TEXT("GameSettingsLibrary::%s: no definition found for tag '%s' in any collection"),
			FunctionName, *SettingTag.ToString());
		return DefaultReturn;
	}
	return TypedGetter(WorldContextObject, Setting);
}

// ============================================================================
// TAG-BASED GETTERS
// ============================================================================

float UMCore_GameSettingsLibrary::GetSettingFloatByTag(const UObject* WorldContextObject,
	FGameplayTag SettingTag)
{
	return GetSettingByTag_Internal<float>(
		WorldContextObject, SettingTag,
		[](const UObject* WCO, const UMCore_DA_SettingDefinition* S) { return GetSettingFloat(WCO, S); },
		0.f, TEXT("GetSettingFloatByTag"));
}

int32 UMCore_GameSettingsLibrary::GetSettingIntByTag(const UObject* WorldContextObject,
	FGameplayTag SettingTag)
{
	return GetSettingByTag_Internal<int32>(
		WorldContextObject, SettingTag,
		[](const UObject* WCO, const UMCore_DA_SettingDefinition* S) { return GetSettingInt(WCO, S); },
		0, TEXT("GetSettingIntByTag"));
}

bool UMCore_GameSettingsLibrary::GetSettingBoolByTag(const UObject* WorldContextObject,
	FGameplayTag SettingTag)
{
	return GetSettingByTag_Internal<bool>(
		WorldContextObject, SettingTag,
		[](const UObject* WCO, const UMCore_DA_SettingDefinition* S) { return GetSettingBool(WCO, S); },
		false, TEXT("GetSettingBoolByTag"));
}

// ============================================================================
// TYPED SETTERS
// ============================================================================

void UMCore_GameSettingsLibrary::SetSettingFloat(
	const UObject* WorldContextObject,
	const TArray<FMCore_FloatSettingChange>& Changes,
	bool bBypassConfirmation)
{
	ApplySettingChanges_Internal<FMCore_FloatSettingChange, float>(
		WorldContextObject, Changes, bBypassConfirmation,
		[](const UMCore_DA_SettingDefinition* S, float V) {
			return (S->SettingType == EMCore_SettingType::Slider)
				? FMath::Clamp(V, S->MinValue, S->MaxValue) : V;
		},
		[](UMCore_PlayerSettingsSave* Save, const FString& Key, float V) { Save->SetFloatSetting(Key, V); },
		[WorldContextObject](const UMCore_DA_SettingDefinition* S, float V) { ApplySettingToEngine(WorldContextObject, S, V, 0, false); });
}

void UMCore_GameSettingsLibrary::SetSettingInt(
	const UObject* WorldContextObject,
	const TArray<FMCore_IntSettingChange>& Changes,
	bool bBypassConfirmation)
{
	ApplySettingChanges_Internal<FMCore_IntSettingChange, int32>(
		WorldContextObject, Changes, bBypassConfirmation,
		[](const UMCore_DA_SettingDefinition* S, int32 V) {
			return (S->SettingType == EMCore_SettingType::Dropdown && S->DropdownOptions.Num() > 0)
				? FMath::Clamp(V, 0, S->DropdownOptions.Num() - 1) : V;
		},
		[](UMCore_PlayerSettingsSave* Save, const FString& Key, int32 V) { Save->SetIntSetting(Key, V); },
		[WorldContextObject](const UMCore_DA_SettingDefinition* S, int32 V) { ApplySettingToEngine(WorldContextObject, S, 0.f, V, false); });
}

void UMCore_GameSettingsLibrary::SetSettingBool(
	const UObject* WorldContextObject,
	const TArray<FMCore_BoolSettingChange>& Changes,
	bool bBypassConfirmation)
{
	ApplySettingChanges_Internal<FMCore_BoolSettingChange, bool>(
		WorldContextObject, Changes, bBypassConfirmation,
		[](const UMCore_DA_SettingDefinition*, bool V) { return V; },
		[](UMCore_PlayerSettingsSave* Save, const FString& Key, bool V) { Save->SetBoolSetting(Key, V); },
		[WorldContextObject](const UMCore_DA_SettingDefinition* S, bool V) { ApplySettingToEngine(WorldContextObject, S, 0.f, 0, V); });
}

// ============================================================================
// UTILITIES
// ============================================================================

void UMCore_GameSettingsLibrary::ResetSettingToDefault(const UObject* WorldContextObject,
	const UMCore_DA_SettingDefinition* Setting)
{
	if (!Setting) { return; }

	/* const_cast safe: DataAsset from asset registry; const parameter is
	   caller-side guarantee; change struct needs non-const for UPROPERTY serialization */
	UMCore_DA_SettingDefinition* MutableSetting =
		const_cast<UMCore_DA_SettingDefinition*>(Setting);

	switch (Setting->SettingType)
	{
	case EMCore_SettingType::Slider:
		SetSettingFloat(WorldContextObject,
			{ { MutableSetting, Setting->DefaultValue } }, true);
		break;
	case EMCore_SettingType::Toggle:
		SetSettingBool(WorldContextObject,
			{ { MutableSetting, Setting->DefaultToggleValue } }, true);
		break;
	case EMCore_SettingType::Dropdown:
		SetSettingInt(WorldContextObject,
			{ { MutableSetting, Setting->DefaultDropdownIndex } }, true);
		break;
	default:
		break;
	}
}

void UMCore_GameSettingsLibrary::ResetAllSettingsToDefault(const UObject* WorldContextObject)
{
	const TArray<UMCore_DA_SettingsCollection*>& Collections = UMCore_CoreSettings::Get()->GetAllSettingsCollections();
	if (Collections.IsEmpty())
	{
		UE_LOG(LogModulusSettings, Warning,
			TEXT("GameSettingsLibrary::ResetAllSettingsToDefault: no settings collections configured in CoreSettings"));
		return;
	}

	TArray<UMCore_DA_SettingDefinition*> Definitions;
	for (const UMCore_DA_SettingsCollection* Collection : Collections)
	{
		for (const TObjectPtr<UMCore_DA_SettingDefinition>& Setting : Collection->GetAllSettings())
		{
			if (Setting) { Definitions.Add(Setting); }
		}
	}

	ResetDefinitionsToDefault(WorldContextObject, Definitions);
}

/**
 * @param CategoryTag  Must match a registered category (e.g., MCore.Settings.Category.Graphics).
 */
void UMCore_GameSettingsLibrary::ResetCategoryToDefault(const UObject* WorldContextObject,
	FGameplayTag CategoryTag)
{
	if (!CategoryTag.IsValid()) { return; }

	ResetDefinitionsToDefault(WorldContextObject,
		UMCore_CoreSettings::Get()->GetSettingsForCategory(CategoryTag));
}

void UMCore_GameSettingsLibrary::ResetDefinitionsToDefault(const UObject* WorldContextObject,
	const TArray<UMCore_DA_SettingDefinition*>& Definitions)
{
	if (Definitions.IsEmpty()) { return; }

	TArray<FMCore_FloatSettingChange> FloatChanges;
	TArray<FMCore_IntSettingChange> IntChanges;
	TArray<FMCore_BoolSettingChange> BoolChanges;

	for (UMCore_DA_SettingDefinition* Setting : Definitions)
	{
		if (!Setting) { continue; }

		switch (Setting->SettingType)
		{
		case EMCore_SettingType::Slider:
			FloatChanges.Add({ Setting, Setting->DefaultValue });
			break;
		case EMCore_SettingType::Toggle:
			BoolChanges.Add({ Setting, Setting->DefaultToggleValue });
			break;
		case EMCore_SettingType::Dropdown:
			IntChanges.Add({ Setting, Setting->DefaultDropdownIndex });
			break;
		default:
			break;
		}
	}

	if (FloatChanges.Num() > 0) { SetSettingFloat(WorldContextObject, FloatChanges, true); }
	if (IntChanges.Num() > 0) { SetSettingInt(WorldContextObject, IntChanges, true); }
	if (BoolChanges.Num() > 0) { SetSettingBool(WorldContextObject, BoolChanges, true); }
}

void UMCore_GameSettingsLibrary::SavePlayerSettings(const UObject* WorldContextObject)
{
	if (UMCore_PlayerSettingsSave* Save = GetPlayerSave(WorldContextObject))
	{
		Save->SaveSettings();
	}
}

void UMCore_GameSettingsLibrary::ReloadAndApplyFromDisk(const UObject* WorldContextObject)
{
	UMCore_PlayerSettingsSave* CachedSave = GetPlayerSave(WorldContextObject);
	if (!CachedSave)
	{
		UE_LOG(LogModulusSettings, Warning,
			TEXT("GameSettingsLibrary::ReloadAndApplyFromDisk: no cached PlayerSettingsSave available"));
		return;
	}

	const FString SlotName = CachedSave->GetCachedSlotName();
	if (SlotName.IsEmpty())
	{
		UE_LOG(LogModulusSettings, Warning,
			TEXT("GameSettingsLibrary::ReloadAndApplyFromDisk: cached save has no slot name"));
		return;
	}

	UMCore_PlayerSettingsSave* FreshSave = UMCore_PlayerSettingsSave::LoadPlayerSettings(SlotName);
	if (!FreshSave)
	{
		UE_LOG(LogModulusSettings, Warning,
			TEXT("GameSettingsLibrary::ReloadAndApplyFromDisk: failed to load from slot '%s'"), *SlotName);
		return;
	}

	/* Replace in-memory setting maps with on-disk values */
	CachedSave->FloatSettings = FreshSave->FloatSettings;
	CachedSave->IntSettings = FreshSave->IntSettings;
	CachedSave->BoolSettings = FreshSave->BoolSettings;

	ApplyAllSettingsToEngine(WorldContextObject);

	UE_LOG(LogModulusSettings, Log,
		TEXT("GameSettingsLibrary::ReloadAndApplyFromDisk: restored from slot '%s', %d float / %d int / %d bool settings re-applied"),
		*SlotName, CachedSave->FloatSettings.Num(), CachedSave->IntSettings.Num(), CachedSave->BoolSettings.Num());
}

/* Engine-apply half of the load-then-apply pair extracted from ReloadAndApplyFromDisk.
 * Iterates every setting in CoreSettings::SettingsCollections and dispatches the persisted
 * value through ApplySettingToEngine, then flushes UGameUserSettings once at the end.
 * Idempotent; safe to call repeatedly. Early-outs on dedicated server (no audio device,
 * GameUserSettings is a no-op, and all dispatchers warn on missing world context). */
void UMCore_GameSettingsLibrary::ApplyAllSettingsToEngine(const UObject* WorldContextObject)
{
	if (IsRunningDedicatedServer()) { return; }

	UMCore_PlayerSettingsSave* CachedSave = GetPlayerSave(WorldContextObject);
	if (!CachedSave)
	{
		UE_LOG(LogModulusSettings, Warning,
			TEXT("GameSettingsLibrary::ApplyAllSettingsToEngine: no cached PlayerSettingsSave available"));
		return;
	}

	const UMCore_CoreSettings* CoreSettings = UMCore_CoreSettings::Get();
	if (!CoreSettings)
	{
		UE_LOG(LogModulusSettings, Warning,
			TEXT("GameSettingsLibrary::ApplyAllSettingsToEngine: CoreSettings unavailable"));
		return;
	}

	/* Snapshot the user's preset intent before iteration. State replay through
	   ApplyViaNamedSetter is not user input; child-scalability writes flip the
	   save's LastSelectedQualityPreset to -1 (Custom) via MarkQualityPresetCustom.
	   Caching here lets the OverallScalabilityLevel short-circuit below read the
	   pre-iteration value, and lets the post-loop restore reverse any clobber. */
	const int32 PreservedQualityPreset = CachedSave->GetLastSelectedQualityPreset();

	const TArray<UMCore_DA_SettingsCollection*>& Collections = CoreSettings->GetAllSettingsCollections();
	for (const UMCore_DA_SettingsCollection* Collection : Collections)
	{
		if (!Collection) { continue; }

		for (const TObjectPtr<UMCore_DA_SettingDefinition>& Definition : Collection->GetAllSettings())
		{
			if (!Definition) { continue; }

			/* Custom intent; skip QualityPreset apply so individual scalability DAs drive engine state.
			   Without this guard, the cascade in ApplyViaNamedSetter would overwrite just-loaded
			   individual save values with engine state matching the saved preset value. The read uses
			   the pre-iteration snapshot rather than the live save, because earlier iterations may
			   have flipped the save's value to -1 as a side effect of child writes. */
			static const FName OverallScalabilityProp(TEXT("OverallScalabilityLevel"));
			if (Definition->NamedSetter == OverallScalabilityProp
				&& PreservedQualityPreset == -1)
			{
				continue;
			}

			switch (Definition->SettingType)
			{
			case EMCore_SettingType::Slider:
				ApplySettingToEngine(WorldContextObject, Definition,
					GetSettingFloat(WorldContextObject, Definition), 0, false);
				break;
			case EMCore_SettingType::Dropdown:
				ApplySettingToEngine(WorldContextObject, Definition,
					0.0f, GetSettingInt(WorldContextObject, Definition), false);
				break;
			case EMCore_SettingType::Toggle:
				ApplySettingToEngine(WorldContextObject, Definition,
					0.0f, 0, GetSettingBool(WorldContextObject, Definition));
				break;
			default:
				break;
			}
		}
	}

	if (UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings())
	{
		GameUserSettings->ApplySettings(false);
	}

	/* Restore user intent if iteration mutated it. The dispatcher's MarkQualityPresetCustom
	   fire-paths are correct for slider commits but wrong for replay; this restore reverses
	   the side effect without touching the dispatcher. Guarded write so the save remains
	   untouched in the common case (no scalability DAs, or intent already -1 pre-iteration). */
	if (CachedSave->GetLastSelectedQualityPreset() != PreservedQualityPreset)
	{
		CachedSave->SetLastSelectedQualityPreset(PreservedQualityPreset);
	}
}

// ============================================================================
// ENGINE APPLY DISPATCHER
// ============================================================================

void UMCore_GameSettingsLibrary::ApplySettingToEngine(const UObject* WorldContextObject,
	const UMCore_DA_SettingDefinition* Setting, float FloatValue, int32 IntValue, bool BoolValue)
{
	if (!Setting) { return; }

	/* Phase 1; GameUserSettings (three-bucket dispatcher) */
	if (!Setting->NamedSetter.IsNone())
	{
		ApplyViaNamedSetter(Setting->NamedSetter, FloatValue, IntValue, BoolValue, WorldContextObject, Setting);
	}

	/* Phase 2; Console Variables */
	if (!Setting->ConsoleVariable.IsNone())
	{
		switch (Setting->SettingType)
		{
		case EMCore_SettingType::Slider:
			ApplyToConsoleVariable(Setting->ConsoleVariable, FloatValue);
			break;
		case EMCore_SettingType::Toggle:
			ApplyToConsoleVariable(Setting->ConsoleVariable, BoolValue);
			break;
		case EMCore_SettingType::Dropdown:
			ApplyToConsoleVariable(Setting->ConsoleVariable, IntValue);
			break;
		default:
			break;
		}
	}

	/* Phase 3; Sound Class volume (Slider only) */
	if (!Setting->SoundClass.IsNull() && Setting->SettingType == EMCore_SettingType::Slider)
	{
		ApplyToSoundClass(WorldContextObject, Setting->SoundClass, FloatValue);
	}

	/* Phase 4; SoundMix push/pop (Toggle only) */
	if (!Setting->PushedSoundMix.IsNull() && Setting->SettingType == EMCore_SettingType::Toggle)
	{
		ApplyToSoundMix(WorldContextObject, Setting->PushedSoundMix,
			Setting->GetSaveKey(), BoolValue);
	}

	/* Phase 5; Color Vision Deficiency (Slate renderer, client-only) */
	if (Setting->ColorVisionRole != EMCore_ColorVisionRole::None)
	{
		ApplyToColorVisionDeficiency(Setting, IntValue, FloatValue);
	}
}

// ============================================================================
// GAME USER SETTINGS (TWO-BUCKET DISPATCHER)
// ============================================================================

/* Two-bucket dispatcher for engine setter targets. Resolution order is
 * Bucket 3 -> Bucket 1 -> not-found warning. Bucket 3 runs first so that
 * translation-key / paired-param / non-GameUserSettings targets take precedence over
 * blind reflection. The FName must be the literal engine name; do not
 * invent or translate keys.
 *
 * Bucket 3; function-dispatch for irreducible operations: cascades, paired
 *            parameters, non-GameUserSettings targets, and ScalabilityQuality struct
 *            members. Each entry writes the engine state directly and (for
 *            scalability children) calls Scalability::SetQualityLevels to
 *            push struct values onto the live sg.* CVars. Examples:
 *            OverallScalabilityLevel (preset cascade), TextureQuality and
 *            siblings (struct field + cascade), EnableHDRDisplayOutput
 *            (paired bool+int), DisplayGamma (GEngine target),
 *            ApplicationScale (UUserInterfaceSettings target).
 *
 * Bucket 1; top-level UPROPERTY on UGameUserSettings, written via FProperty
 *            reflection. Example: bUseVSync, AudioQualityLevel, FrameRateLimit.
 *
 * Why no struct-reflection bucket: FQualityLevels reflection via
 * FStructProperty + FindPropertyByName silently fails to write under UE 5.6
 * (the namespaced struct doesn't resolve through the property chain), and
 * even when the write would succeed it doesn't push to the sg.* CVars
 * without a follow-up Scalability::SetQualityLevels call. Explicit per-member
 * dispatch is reliable and matches Lyra's pattern.
 *
 * Returns true if the dispatch landed in any bucket, false (with warning) if
 * the FName matched none of them. */
bool UMCore_GameSettingsLibrary::ApplyViaNamedSetter(const FName& SetterName,
	float FloatValue, int32 IntValue, bool bBoolValue,
	const UObject* WorldContextObject,
	const UMCore_DA_SettingDefinition* Definition)
{
	if (SetterName.IsNone()) { return false; }

	UE_LOG(LogModulusSettings, Verbose,
		TEXT("GameSettingsLibrary::ApplyViaNamedSetter: dispatch '%s'"), *SetterName.ToString());

	/* Theme apply; does not depend on UGameUserSettings; resolves the target
	 * theme asset from Definition->ThemeOptions and hands off to UISubsystem. */
	if (SetterName == TEXT("MCore.SetActiveTheme"))
	{
		if (!Definition || !Definition->ThemeOptions.IsValidIndex(IntValue))
		{
			UE_LOG(LogModulusSettings, Warning,
				TEXT("GameSettingsLibrary::ApplyViaNamedSetter: MCore.SetActiveTheme: invalid index %d or missing Definition"),
				IntValue);
			return false;
		}

		UMCore_PDA_UITheme_Base* Theme =
			Definition->ThemeOptions[IntValue].ThemeAsset.LoadSynchronous();
		if (!Theme)
		{
			UE_LOG(LogModulusSettings, Warning,
				TEXT("GameSettingsLibrary::ApplyViaNamedSetter: MCore.SetActiveTheme: asset failed to load at index %d"),
				IntValue);
			return false;
		}

		ULocalPlayer* LocalPlayer = WorldContextObject
			? GEngine->GetFirstGamePlayer(WorldContextObject->GetWorld())
			: nullptr;
		UMCore_UISubsystem* UISubsystem = LocalPlayer
			? LocalPlayer->GetSubsystem<UMCore_UISubsystem>()
			: nullptr;
		if (!UISubsystem)
		{
			UE_LOG(LogModulusSettings, Warning,
				TEXT("GameSettingsLibrary::ApplyViaNamedSetter: MCore.SetActiveTheme: no UISubsystem for context"));
			return false;
		}

		UISubsystem->SetActiveTheme(Theme);
		return true;
	}

	// Editor-unsafe keys: these mutate the host process's window/display
	// state when invoked in PIE, which freezes/destabilizes the editor.
	// The DA still writes to the save slot and the confirmation modal still
	// fires; only the engine-side side-effect is suppressed in editor.
	// Re-test these paths in a packaged build to verify end-to-end behavior.
	//
	// Developers can opt in to applying these in PIE by setting
	// UMCore_CoreSettings::bApplyDisplaySettingsInPIE = true.
	static const TSet<FName> EditorUnsafeKeys = {
		TEXT("ScreenResolution"),
		TEXT("FullscreenMode"),
		TEXT("bUseHDRDisplayOutput"),
		TEXT("HDRDisplayOutputNits")
	};

	static const TSet<FName> EditorUnsafeScalabilityKeys = {
		TEXT("OverallScalabilityLevel"),
		TEXT("TextureQuality"),
		TEXT("ShadowQuality"),
		TEXT("AntiAliasingQuality"),
		TEXT("PostProcessQuality"),
		TEXT("ViewDistanceQuality"),
		TEXT("FoliageQuality"),
		TEXT("ShadingQuality"),
		TEXT("EffectsQuality"),
		TEXT("GlobalIlluminationQuality"),
		TEXT("ReflectionQuality"),
		TEXT("LandscapeQuality"),
		TEXT("ResolutionQuality"),
		TEXT("bUseDynamicResolution")
	};

	if (GIsEditor && !IsRunningGame() && EditorUnsafeKeys.Contains(SetterName))
	{
		const UMCore_CoreSettings* CoreSettings = UMCore_CoreSettings::Get();
		if (CoreSettings && !CoreSettings->bApplyDisplaySettingsInPIE)
		{
			UE_LOG(LogModulusSettings, Verbose,
				TEXT("GameSettingsLibrary::ApplyViaNamedSetter: skipping editor-unsafe key '%s' in PIE (set CoreSettings::bApplyDisplaySettingsInPIE=true to override)"),
				*SetterName.ToString());
			return true; // Treat as handled; save-path proceeds, engine call suppressed
		}
	}

	if (GIsEditor && !IsRunningGame() && EditorUnsafeScalabilityKeys.Contains(SetterName))
	{
		const UMCore_CoreSettings* CoreSettings = UMCore_CoreSettings::Get();
		if (CoreSettings && !CoreSettings->bApplyScalabilitySettingsInPIE)
		{
			UE_LOG(LogModulusSettings, Verbose,
				TEXT("GameSettingsLibrary::ApplyViaNamedSetter: skipping editor-unsafe scalability key '%s' in PIE (set CoreSettings::bApplyScalabilitySettingsInPIE=true to override)"),
				*SetterName.ToString());
			
			if (SetterName != TEXT("OverallScalabilityLevel"))
			{
				MarkQualityPresetCustom(GetPlayerSave(WorldContextObject));
				UMCore_EventFunctionLibrary::BroadcastSimpleEvent(
					WorldContextObject,
					MCore_SettingsTags::MCore_Settings_Event_ExternalValueChange,
					EMCore_EventScope::Local);
			}
			return true; // Treat as handled; save-path proceeds, engine call suppressed
		}
	}

	UGameUserSettings* GameUserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr;

	// ============================================================
	// Bucket 3; function-dispatch for irreducible operations.
	// ============================================================
	if (SetterName == TEXT("OverallScalabilityLevel"))
	{
		if (!GameUserSettings) { return false; }
		UMCore_PlayerSettingsSave* Save = GetPlayerSave(WorldContextObject);

		GameUserSettings->SetOverallScalabilityLevel(IntValue);

		/* Cascade engine values back to individual save keys so subsequent reloads
		   and per-widget reads reflect what the preset just applied. */
		CascadeScalabilityValuesToSave(Save);
		if (Save) { Save->SetLastSelectedQualityPreset(FMath::Clamp(IntValue, 0, 3)); }

		/* Notify subscribed widgets to refresh from save. */
		UMCore_EventFunctionLibrary::BroadcastSimpleEvent(
			WorldContextObject,
			MCore_SettingsTags::MCore_Settings_Event_ExternalValueChange,
			EMCore_EventScope::Local);
		return true;
	}

	/* ScalabilityQuality child setters. Each writes the FQualityLevels member directly
	   and calls Scalability::SetQualityLevels to push the struct values onto the sg.* CVars
	   (mirrors what UGameUserSettings::ApplyNonResolutionSettings does internally). Without
	   the cascade call the field write would persist to ini but never affect the running
	   session. Each commit flips the saved preset to Custom and broadcasts so the
	   QualityPreset widget refreshes. Listed alphabetically. */
	if (SetterName == TEXT("AntiAliasingQuality"))
	{
		if (!GameUserSettings) { return false; }
		GameUserSettings->ScalabilityQuality.AntiAliasingQuality = FMath::Clamp(IntValue, 0, 3);
		Scalability::SetQualityLevels(GameUserSettings->ScalabilityQuality);

		MarkQualityPresetCustom(GetPlayerSave(WorldContextObject));
		UMCore_EventFunctionLibrary::BroadcastSimpleEvent(
			WorldContextObject,
			MCore_SettingsTags::MCore_Settings_Event_ExternalValueChange,
			EMCore_EventScope::Local);
		return true;
	}
	if (SetterName == TEXT("EffectsQuality"))
	{
		if (!GameUserSettings) { return false; }
		GameUserSettings->ScalabilityQuality.EffectsQuality = FMath::Clamp(IntValue, 0, 3);
		Scalability::SetQualityLevels(GameUserSettings->ScalabilityQuality);

		MarkQualityPresetCustom(GetPlayerSave(WorldContextObject));
		UMCore_EventFunctionLibrary::BroadcastSimpleEvent(
			WorldContextObject,
			MCore_SettingsTags::MCore_Settings_Event_ExternalValueChange,
			EMCore_EventScope::Local);
		return true;
	}
	if (SetterName == TEXT("FoliageQuality"))
	{
		if (!GameUserSettings) { return false; }
		GameUserSettings->ScalabilityQuality.FoliageQuality = FMath::Clamp(IntValue, 0, 3);
		Scalability::SetQualityLevels(GameUserSettings->ScalabilityQuality);

		MarkQualityPresetCustom(GetPlayerSave(WorldContextObject));
		UMCore_EventFunctionLibrary::BroadcastSimpleEvent(
			WorldContextObject,
			MCore_SettingsTags::MCore_Settings_Event_ExternalValueChange,
			EMCore_EventScope::Local);
		return true;
	}
	if (SetterName == TEXT("GlobalIlluminationQuality"))
	{
		if (!GameUserSettings) { return false; }
		GameUserSettings->ScalabilityQuality.GlobalIlluminationQuality = FMath::Clamp(IntValue, 0, 3);
		Scalability::SetQualityLevels(GameUserSettings->ScalabilityQuality);

		MarkQualityPresetCustom(GetPlayerSave(WorldContextObject));
		UMCore_EventFunctionLibrary::BroadcastSimpleEvent(
			WorldContextObject,
			MCore_SettingsTags::MCore_Settings_Event_ExternalValueChange,
			EMCore_EventScope::Local);
		return true;
	}
	if (SetterName == TEXT("LandscapeQuality"))
	{
		if (!GameUserSettings) { return false; }
		GameUserSettings->ScalabilityQuality.LandscapeQuality = FMath::Clamp(IntValue, 0, 3);
		Scalability::SetQualityLevels(GameUserSettings->ScalabilityQuality);

		MarkQualityPresetCustom(GetPlayerSave(WorldContextObject));
		UMCore_EventFunctionLibrary::BroadcastSimpleEvent(
			WorldContextObject,
			MCore_SettingsTags::MCore_Settings_Event_ExternalValueChange,
			EMCore_EventScope::Local);
		return true;
	}
	if (SetterName == TEXT("PostProcessQuality"))
	{
		if (!GameUserSettings) { return false; }
		GameUserSettings->ScalabilityQuality.PostProcessQuality = FMath::Clamp(IntValue, 0, 3);
		Scalability::SetQualityLevels(GameUserSettings->ScalabilityQuality);

		MarkQualityPresetCustom(GetPlayerSave(WorldContextObject));
		UMCore_EventFunctionLibrary::BroadcastSimpleEvent(
			WorldContextObject,
			MCore_SettingsTags::MCore_Settings_Event_ExternalValueChange,
			EMCore_EventScope::Local);
		return true;
	}
	if (SetterName == TEXT("ReflectionQuality"))
	{
		if (!GameUserSettings) { return false; }
		GameUserSettings->ScalabilityQuality.ReflectionQuality = FMath::Clamp(IntValue, 0, 3);
		Scalability::SetQualityLevels(GameUserSettings->ScalabilityQuality);

		MarkQualityPresetCustom(GetPlayerSave(WorldContextObject));
		UMCore_EventFunctionLibrary::BroadcastSimpleEvent(
			WorldContextObject,
			MCore_SettingsTags::MCore_Settings_Event_ExternalValueChange,
			EMCore_EventScope::Local);
		return true;
	}
	if (SetterName == TEXT("ShadingQuality"))
	{
		if (!GameUserSettings) { return false; }
		GameUserSettings->ScalabilityQuality.ShadingQuality = FMath::Clamp(IntValue, 0, 3);
		Scalability::SetQualityLevels(GameUserSettings->ScalabilityQuality);

		MarkQualityPresetCustom(GetPlayerSave(WorldContextObject));
		UMCore_EventFunctionLibrary::BroadcastSimpleEvent(
			WorldContextObject,
			MCore_SettingsTags::MCore_Settings_Event_ExternalValueChange,
			EMCore_EventScope::Local);
		return true;
	}
	if (SetterName == TEXT("ShadowQuality"))
	{
		if (!GameUserSettings) { return false; }
		GameUserSettings->ScalabilityQuality.ShadowQuality = FMath::Clamp(IntValue, 0, 3);
		Scalability::SetQualityLevels(GameUserSettings->ScalabilityQuality);

		MarkQualityPresetCustom(GetPlayerSave(WorldContextObject));
		UMCore_EventFunctionLibrary::BroadcastSimpleEvent(
			WorldContextObject,
			MCore_SettingsTags::MCore_Settings_Event_ExternalValueChange,
			EMCore_EventScope::Local);
		return true;
	}
	if (SetterName == TEXT("TextureQuality"))
	{
		if (!GameUserSettings) { return false; }
		GameUserSettings->ScalabilityQuality.TextureQuality = FMath::Clamp(IntValue, 0, 3);
		Scalability::SetQualityLevels(GameUserSettings->ScalabilityQuality);

		MarkQualityPresetCustom(GetPlayerSave(WorldContextObject));
		UMCore_EventFunctionLibrary::BroadcastSimpleEvent(
			WorldContextObject,
			MCore_SettingsTags::MCore_Settings_Event_ExternalValueChange,
			EMCore_EventScope::Local);
		return true;
	}
	if (SetterName == TEXT("ViewDistanceQuality"))
	{
		if (!GameUserSettings) { return false; }
		GameUserSettings->ScalabilityQuality.ViewDistanceQuality = FMath::Clamp(IntValue, 0, 3);
		Scalability::SetQualityLevels(GameUserSettings->ScalabilityQuality);

		MarkQualityPresetCustom(GetPlayerSave(WorldContextObject));
		UMCore_EventFunctionLibrary::BroadcastSimpleEvent(
			WorldContextObject,
			MCore_SettingsTags::MCore_Settings_Event_ExternalValueChange,
			EMCore_EventScope::Local);
		return true;
	}
	if (SetterName == TEXT("ScreenResolution"))
	{
		// TODO: IntValue is currently the index into the descending-sorted
		// supported-resolutions list. If a path supplying packed FIntPoint
		// X/Y becomes available, decode here and call SetScreenResolution
		// with the FIntPoint directly.
		if (!GameUserSettings) { return false; }

		TArray<FIntPoint> Resolutions;
		UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);
		Algo::Reverse(Resolutions);

		if (Resolutions.IsValidIndex(IntValue))
		{
			GameUserSettings->SetScreenResolution(Resolutions[IntValue]);
		}
		else
		{
			UE_LOG(LogModulusSettings, Warning,
				TEXT("GameSettingsLibrary::ApplyViaNamedSetter: resolution index %d out of range (%d available)"),
				IntValue, Resolutions.Num());
		}
		return true;
	}
	if (SetterName == TEXT("bUseHDRDisplayOutput"))
	{
		/* Paired with HDRDisplayOutputNits. Read the other axis from current
		   GameUserSettings state so EnableHDRDisplayOutput receives both params. */
		if (!GameUserSettings) { return false; }
		const int32 CurrentNits = GameUserSettings->GetCurrentHDRDisplayNits();
		GameUserSettings->EnableHDRDisplayOutput(bBoolValue, CurrentNits > 0 ? CurrentNits : 1000);
		return true;
	}
	if (SetterName == TEXT("HDRDisplayOutputNits"))
	{
		if (!GameUserSettings) { return false; }
		GameUserSettings->EnableHDRDisplayOutput(GameUserSettings->IsHDREnabled(), IntValue);
		return true;
	}
	if (SetterName == TEXT("FullscreenMode"))
	{
		if (!GameUserSettings) { return false; }
		GameUserSettings->SetFullscreenMode(EWindowMode::ConvertIntToWindowMode(IntValue));
		return true;
	}
	if (SetterName == TEXT("DisplayGamma"))
	{
		if (GEngine)
		{
			GEngine->DisplayGamma = FloatValue;
			UE_LOG(LogModulusSettings, Log,
				TEXT("GameSettingsLibrary::ApplyViaNamedSetter: DisplayGamma=%.3f"), FloatValue);
		}
		return true;
	}
	if (SetterName == TEXT("ApplicationScale"))
	{
		GetMutableDefault<UUserInterfaceSettings>()->ApplicationScale = FloatValue;
		UE_LOG(LogModulusSettings, Log,
			TEXT("GameSettingsLibrary::ApplyViaNamedSetter: ApplicationScale=%.3f"), FloatValue);
		return true;
	}

	if (!GameUserSettings) { return false; }

	// ============================================================
	// Bucket 1; top-level UPROPERTY on UGameUserSettings via reflection.
	// ============================================================
	if (FProperty* Prop = GameUserSettings->GetClass()->FindPropertyByName(SetterName))
	{
		return WriteReflectedProperty(Prop, GameUserSettings, FloatValue, IntValue, bBoolValue);
	}

	UE_LOG(LogModulusSettings, Warning,
		TEXT("GameSettingsLibrary::ApplyViaNamedSetter: '%s' not found in any bucket "
			 "(top-level UPROPERTY on %s or function-dispatch table)"),
		*SetterName.ToString(), *GameUserSettings->GetClass()->GetName());
	return false;
}

bool UMCore_GameSettingsLibrary::WriteReflectedProperty(FProperty* Prop, void* Container,
	float FloatValue, int32 IntValue, bool bBoolValue)
{
	if (!Prop || !Container) { return false; }

	if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop))
	{
		FloatProp->SetPropertyValue_InContainer(Container, FloatValue);
		return true;
	}
	if (FDoubleProperty* DoubleProp = CastField<FDoubleProperty>(Prop))
	{
		DoubleProp->SetPropertyValue_InContainer(Container, static_cast<double>(FloatValue));
		return true;
	}
	if (FIntProperty* IntProp = CastField<FIntProperty>(Prop))
	{
		IntProp->SetPropertyValue_InContainer(Container, IntValue);
		return true;
	}
	if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop))
	{
		BoolProp->SetPropertyValue_InContainer(Container, bBoolValue);
		return true;
	}
	if (FByteProperty* ByteProp = CastField<FByteProperty>(Prop))
	{
		ByteProp->SetPropertyValue_InContainer(Container, static_cast<uint8>(IntValue));
		return true;
	}

	UE_LOG(LogModulusSettings, Warning,
		TEXT("GameSettingsLibrary::WriteReflectedProperty: '%s' has unsupported type '%s' "
			 "(attempted float=%.3f int=%d bool=%d)"),
		*Prop->GetName(), *Prop->GetCPPType(),
		FloatValue, IntValue, bBoolValue ? 1 : 0);
	return false;
}

// ============================================================================
// QUALITY PRESET CASCADE / INTENT
// ============================================================================

void UMCore_GameSettingsLibrary::CascadeScalabilityValuesToSave(UMCore_PlayerSettingsSave* Save)
{
	if (!Save) { return; }

	UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings();
	if (!GameUserSettings) { return; }

	const UMCore_CoreSettings* CoreSettings = UMCore_CoreSettings::Get();
	if (!CoreSettings) { return; }

	static const FName Name_TextureQuality(TEXT("TextureQuality"));
	static const FName Name_ShadowQuality(TEXT("ShadowQuality"));
	static const FName Name_AntiAliasingQuality(TEXT("AntiAliasingQuality"));
	static const FName Name_PostProcessQuality(TEXT("PostProcessQuality"));
	static const FName Name_ViewDistanceQuality(TEXT("ViewDistanceQuality"));
	static const FName Name_FoliageQuality(TEXT("FoliageQuality"));
	static const FName Name_EffectsQuality(TEXT("EffectsQuality"));
	static const FName Name_ShadingQuality(TEXT("ShadingQuality"));
	static const FName Name_GlobalIlluminationQuality(TEXT("GlobalIlluminationQuality"));
	static const FName Name_ReflectionQuality(TEXT("ReflectionQuality"));

	auto ReadMember = [GameUserSettings](const FName& Member, int32& OutValue) -> bool
	{
		if      (Member == Name_TextureQuality)            { OutValue = GameUserSettings->ScalabilityQuality.TextureQuality;            return true; }
		else if (Member == Name_ShadowQuality)             { OutValue = GameUserSettings->ScalabilityQuality.ShadowQuality;             return true; }
		else if (Member == Name_AntiAliasingQuality)       { OutValue = GameUserSettings->ScalabilityQuality.AntiAliasingQuality;       return true; }
		else if (Member == Name_PostProcessQuality)        { OutValue = GameUserSettings->ScalabilityQuality.PostProcessQuality;        return true; }
		else if (Member == Name_ViewDistanceQuality)       { OutValue = GameUserSettings->ScalabilityQuality.ViewDistanceQuality;       return true; }
		else if (Member == Name_FoliageQuality)            { OutValue = GameUserSettings->ScalabilityQuality.FoliageQuality;            return true; }
		else if (Member == Name_EffectsQuality)            { OutValue = GameUserSettings->ScalabilityQuality.EffectsQuality;            return true; }
		else if (Member == Name_ShadingQuality)            { OutValue = GameUserSettings->ScalabilityQuality.ShadingQuality;            return true; }
		else if (Member == Name_GlobalIlluminationQuality) { OutValue = GameUserSettings->ScalabilityQuality.GlobalIlluminationQuality; return true; }
		else if (Member == Name_ReflectionQuality)         { OutValue = GameUserSettings->ScalabilityQuality.ReflectionQuality;         return true; }
		return false;
	};

	const TArray<UMCore_DA_SettingsCollection*>& Collections = CoreSettings->GetAllSettingsCollections();
	for (const UMCore_DA_SettingsCollection* Collection : Collections)
	{
		if (!Collection) { continue; }
		for (const TObjectPtr<UMCore_DA_SettingDefinition>& Definition : Collection->GetAllSettings())
		{
			if (!Definition) { continue; }
			int32 EngineValue = 0;
			if (ReadMember(Definition->NamedSetter, EngineValue))
			{
				Save->SetIntSetting(Definition->GetSaveKey(), EngineValue);
			}
		}
	}
}

void UMCore_GameSettingsLibrary::MarkQualityPresetCustom(UMCore_PlayerSettingsSave* Save)
{
	if (Save) { Save->SetLastSelectedQualityPreset(-1); }
}

// ============================================================================
// CONSOLE VARIABLES
// ============================================================================

void UMCore_GameSettingsLibrary::ApplyToConsoleVariable(const FName& CVarName, float Value)
{
	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*CVarName.ToString());
	if (CVar)
	{
		CVar->Set(Value, ECVF_SetByCode);
	}
	else
	{
		UE_LOG(LogModulusSettings, Warning,
			TEXT("GameSettingsLibrary::ApplyToConsoleVariable: console variable '%s' not found"), *CVarName.ToString());
	}
}

void UMCore_GameSettingsLibrary::ApplyToConsoleVariable(const FName& CVarName, int32 Value)
{
	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*CVarName.ToString());
	if (CVar)
	{
		CVar->Set(Value, ECVF_SetByCode);
	}
	else
	{
		UE_LOG(LogModulusSettings, Warning,
			TEXT("GameSettingsLibrary::ApplyToConsoleVariable: console variable '%s' not found"), *CVarName.ToString());
	}
}

void UMCore_GameSettingsLibrary::ApplyToConsoleVariable(const FName& CVarName, bool Value)
{
	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*CVarName.ToString());
	if (CVar)
	{
		CVar->Set(Value, ECVF_SetByCode);
	}
	else
	{
		UE_LOG(LogModulusSettings, Warning,
			TEXT("GameSettingsLibrary::ApplyToConsoleVariable: console variable '%s' not found"), *CVarName.ToString());
	}
}

// ============================================================================
// SOUND CLASS
// ============================================================================

namespace
{
	/* File-scope cache of the most recent slider value committed per SoundClass.
	 * Drives the parent-chain product cascade in ApplyToSoundClass; every commit
	 * walks the cache and re-pushes Product(self * cached ancestors) for every
	 * tracked class, so a Master adjustment correctly propagates to all
	 * descendant categories without clobbering their independently-set values.
	 * Weak pointers so cache entries don't extend SoundClass lifetimes; stale
	 * entries are skipped at walk time. */
	TMap<TWeakObjectPtr<USoundClass>, float> GMCore_VolumeCache;

	constexpr int32 GMCore_VolumeWalkMaxDepth = 16;
}

/* Applies a volume slider commit to a SoundClass via the SoundMix override
 * pathway. Direct mutation of USoundClass::Properties.Volume is silently
 * ignored by AudioDevice for already-playing sources (see AudioDevice.cpp
 * playback-time resolution); the engine canonical path is to push a SoundMix
 * with class adjusters and then override the Volume on a per-class basis.
 *
 * The SoundMix asset is configured on UMCore_CoreSettings::VolumeMix
 * (defaults to MCore_VolumeMix shipped with the plugin). Projects can author
 * a custom SoundMix and point CoreSettings at it; see the README section
 * "Customizing the Volume Mix".
 *
 * Parent-chain cascade: SetSoundMixClassOverride with bApplyToChildren=true
 * last-write-wins clobbers per-category overrides; with false, parent volumes
 * never propagate. We keep a per-class cache of the last committed value and,
 * on every commit, re-push Product(self * cached ancestors via ParentClass)
 * for every tracked class. Idempotent: classes whose product is unchanged
 * end up overriding to the same value they already had. Depth-bailed at 16
 * to defend against malformed (cyclic) ParentClass hierarchies; the engine
 * has no cycle guard of its own. */
void UMCore_GameSettingsLibrary::ApplyToSoundClass(
	const UObject* WorldContextObject,
	const TSoftObjectPtr<USoundClass>& SoundClassRef,
	float Volume)
{
	USoundClass* LoadedClass = SoundClassRef.LoadSynchronous();
	if (!LoadedClass)
	{
		UE_LOG(LogModulusSettings, Warning,
			TEXT("GameSettingsLibrary::ApplyToSoundClass: SoundClass failed to load (ref '%s')"),
			*SoundClassRef.ToString());
		return;
	}

	const UMCore_CoreSettings* CoreSettings = GetDefault<UMCore_CoreSettings>();
	USoundMix* VolumeMix = CoreSettings ? CoreSettings->VolumeMix.LoadSynchronous() : nullptr;
	if (!VolumeMix)
	{
		UE_LOG(LogModulusSettings, Warning,
			TEXT("GameSettingsLibrary::ApplyToSoundClass: VolumeMix not configured in MCore_CoreSettings"));
		return;
	}

	EnsureVolumeMixActive(WorldContextObject, VolumeMix);

	const float ClampedVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	GMCore_VolumeCache.Add(LoadedClass, ClampedVolume);

	for (const TPair<TWeakObjectPtr<USoundClass>, float>& Entry : GMCore_VolumeCache)
	{
		USoundClass* TrackedClass = Entry.Key.Get();
		if (!TrackedClass) { continue; }

		float Product = Entry.Value;
		USoundClass* Ancestor = TrackedClass->ParentClass;
		int32 Depth = 0;
		while (Ancestor && Depth < GMCore_VolumeWalkMaxDepth)
		{
			if (const float* AncestorVolume = GMCore_VolumeCache.Find(Ancestor))
			{
				Product *= *AncestorVolume;
			}
			Ancestor = Ancestor->ParentClass;
			++Depth;
		}

		UGameplayStatics::SetSoundMixClassOverride(
			WorldContextObject,
			VolumeMix,
			TrackedClass,
			FMath::Clamp(Product, 0.0f, 1.0f),
			1.0f,    /* PitchAdjuster */
			0.5f,    /* FadeInTime */
			false);  /* bApplyToChildren */
	}

	UE_LOG(LogModulusSettings, Log,
		TEXT("GameSettingsLibrary::ApplyToSoundClass: committed %s = %.3f, %d cached classes re-pushed"),
		*LoadedClass->GetName(), ClampedVolume, GMCore_VolumeCache.Num());
}

void UMCore_GameSettingsLibrary::EnsureVolumeMixActive(
	const UObject* WorldContextObject, USoundMix* VolumeMix)
{
	if (!VolumeMix || !WorldContextObject || !GEngine) { return; }

	UE_LOG(LogModulusSettings, Verbose,
		TEXT("GameSettingsLibrary::EnsureVolumeMixActive: pushing VolumeMix '%s'"),
		*VolumeMix->GetName());

	UGameplayStatics::PushSoundMixModifier(WorldContextObject, VolumeMix);
}

// ============================================================================
// SOUND MIX
// ============================================================================

void UMCore_GameSettingsLibrary::ApplyToSoundMix(const UObject* WorldContextObject,
	TSoftObjectPtr<USoundMix> SoundMixRef, const FString& SaveKey, bool bDesiredActive)
{
	static TMap<FString, bool> PushedState;
	static Audio::FDeviceId LastSeenDeviceId = INDEX_NONE;
	
	if (!WorldContextObject) { return; }
	
	if (UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(
		WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr)
	{
		if (FAudioDeviceHandle DeviceHandle = World->GetAudioDevice())
		{
			const Audio::FDeviceId CurrentDeviceId = DeviceHandle->DeviceID;
			if (CurrentDeviceId != LastSeenDeviceId)
			{
				PushedState.Reset();
				LastSeenDeviceId = CurrentDeviceId;
			}
		}
	}
	
	const bool* ExistingState = PushedState.Find(SaveKey);
	if (ExistingState && *ExistingState == bDesiredActive) { return; }
	
	USoundMix* Mix = SoundMixRef.LoadSynchronous();
	if (!Mix)
	{
		UE_LOG(LogModulusSettings, Warning,
			TEXT("GameSettingsLibrary::ApplyToSoundMix: failed to load SoundMix '%s'"),
			*SoundMixRef.ToString());
		return;
	}

	if (bDesiredActive)
	{
		UGameplayStatics::PushSoundMixModifier(WorldContextObject, Mix);
	}
	else
	{
		UGameplayStatics::PopSoundMixModifier(WorldContextObject, Mix);
	}

	PushedState.Add(SaveKey, bDesiredActive);

	UE_LOG(LogModulusSettings, Verbose,
		TEXT("GameSettingsLibrary::ApplyToSoundMix: SoundMix '%s' %s (key: %s)"),
		*Mix->GetName(), bDesiredActive ? TEXT("pushed") : TEXT("popped"), *SaveKey);
}

// ============================================================================
// COLOR VISION DEFICIENCY
// ============================================================================

bool UMCore_GameSettingsLibrary::ApplyToColorVisionDeficiency(
	const UMCore_DA_SettingDefinition* Definition, int32 IntValue, float FloatValue)
{
	if (!Definition || Definition->ColorVisionRole == EMCore_ColorVisionRole::None)
	{
		return false;
	}

	/* One Slate-wide (Type, Severity) slot; bare statics, no save-key keying.
	   ReloadAndApplyFromDisk fires both role DAs at init so both caches converge. */
	static int32 CachedType = 0;       /* 0 == NormalVision */
	static float CachedSeverity = 0.f;

	switch (Definition->ColorVisionRole)
	{
	case EMCore_ColorVisionRole::DeficiencyType:
		if (Definition->SettingType != EMCore_SettingType::Dropdown)
		{
			UE_LOG(LogModulusSettings, Warning,
				TEXT("GameSettingsLibrary::ApplyToColorVisionDeficiency: "
					 "DeficiencyType role requires Dropdown SettingType (setting: %s)"),
				*Definition->GetName());
			return false;
		}
		CachedType = FMath::Clamp(IntValue, 0, 3);
		break;

	case EMCore_ColorVisionRole::DeficiencySeverity:
		if (Definition->SettingType != EMCore_SettingType::Slider)
		{
			UE_LOG(LogModulusSettings, Warning,
				TEXT("GameSettingsLibrary::ApplyToColorVisionDeficiency: "
					 "DeficiencySeverity role requires Slider SettingType (setting: %s)"),
				*Definition->GetName());
			return false;
		}
		CachedSeverity = FMath::Clamp(FloatValue, 0.f, 10.f);
		break;

	default:
		return false;
	}

	UWidgetBlueprintLibrary::SetColorVisionDeficiencyType(
		static_cast<EColorVisionDeficiency>(CachedType),
		CachedSeverity,
		/*bCorrectDeficiency=*/true,
		/*bShowCorrectionWithDeficiency=*/false);

	UE_LOG(LogModulusSettings, Verbose,
		TEXT("GameSettingsLibrary::ApplyToColorVisionDeficiency: Type=%d Severity=%.2f"),
		CachedType, CachedSeverity);

	return true;
}

// ============================================================================
// EVENT BROADCASTING
// ============================================================================

void UMCore_GameSettingsLibrary::BroadcastSettingChanged(const UObject* WorldContextObject,
	const FGameplayTag& SettingTag)
{
	if (!SettingTag.IsValid() || !WorldContextObject) { return; }

	UMCore_EventFunctionLibrary::BroadcastSimpleEvent(WorldContextObject,
		SettingTag, EMCore_EventScope::Local);
}
