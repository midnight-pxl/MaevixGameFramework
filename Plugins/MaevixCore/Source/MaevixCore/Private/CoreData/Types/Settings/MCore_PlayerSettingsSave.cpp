// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved.

#include "CoreData/Types/Settings/MCore_PlayerSettingsSave.h"

#include "CoreData/Logging/LogMaevixSettings.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/UserInterfaceSettings.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"

UMCore_PlayerSettingsSave::UMCore_PlayerSettingsSave()
{
}

// ============================================================================
// GENERIC ACCESSORS
// ============================================================================

bool UMCore_PlayerSettingsSave::GetFloatSetting(const FString& Key, float& OutValue) const
{
	if (const float* FoundSetting = FloatSettings.Find(Key))
	{
		OutValue = *FoundSetting;
		return true;
	}
	return false;
}

void UMCore_PlayerSettingsSave::SetFloatSetting(const FString& Key, float Value)
{
	FloatSettings.Add(Key, Value);
}

bool UMCore_PlayerSettingsSave::GetIntSetting(const FString& Key, int32& OutValue) const
{
	if (const int32* FoundSetting = IntSettings.Find(Key))
	{
		OutValue = *FoundSetting;
		return true;
	}
	return false;
}

void UMCore_PlayerSettingsSave::SetIntSetting(const FString& Key, int32 Value)
{
	IntSettings.Add(Key, Value);
}

bool UMCore_PlayerSettingsSave::GetBoolSetting(const FString& Key, bool& OutValue) const
{
	if (const bool* FoundSetting = BoolSettings.Find(Key))
	{
		OutValue = *FoundSetting;
		return true;
	}
	return false;
}

void UMCore_PlayerSettingsSave::SetBoolSetting(const FString& Key, bool Value)
{
	BoolSettings.Add(Key, Value);
}

// ============================================================================
// PERSISTENCE
// ============================================================================

void UMCore_PlayerSettingsSave::SaveSettings()
{
	UGameplayStatics::SaveGameToSlot(this, CachedSlotName, 0);
	UE_LOG(LogMaevixSettings, Log, TEXT("PlayerSettingsSave::SaveSettings: saved to slot '%s'"), *CachedSlotName);
}

UMCore_PlayerSettingsSave* UMCore_PlayerSettingsSave::LoadPlayerSettings(const FString& SlotName)
{
	UMCore_PlayerSettingsSave* Settings = nullptr;

	bool bExistingSave{false};

	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		if (USaveGame* LoadedSave = UGameplayStatics::LoadGameFromSlot(SlotName, 0))
		{
			Settings = Cast<UMCore_PlayerSettingsSave>(LoadedSave);
			if (!Settings)
			{
				UE_LOG(LogMaevixSettings, Warning,
					TEXT("PlayerSettingsSave::LoadPlayerSettings: save existed in slot '%s' but cast to UMCore_PlayerSettingsSave failed"),
					*SlotName);
			}
			else
			{
				bExistingSave = true;
			}
		}
	}

	if (!Settings)
	{
		Settings = Cast<UMCore_PlayerSettingsSave>(
			UGameplayStatics::CreateSaveGameObject(UMCore_PlayerSettingsSave::StaticClass()));
		UE_LOG(LogMaevixSettings, Log,
			TEXT("PlayerSettingsSave::LoadPlayerSettings: created fresh settings for slot '%s'"),
			*SlotName);
	}
	else
	{
		UE_LOG(LogMaevixSettings, Log,
			TEXT("PlayerSettingsSave::LoadPlayerSettings: loaded existing settings from slot '%s'"),
			*SlotName);
	}

	Settings->CachedSlotName = SlotName;
	Settings->ValidateSettings();

	UE_LOG(LogMaevixSettings, Log,
		TEXT("PlayerSettingsSave::LoadPlayerSettings: loaded from slot '%s' (existing=%s)"),
		*SlotName, bExistingSave ? TEXT("true") : TEXT("false"));
	return Settings;
}

void UMCore_PlayerSettingsSave::LoadPlayerSettingsAsync(const FString& SlotName, FOnPlayerSettingsLoaded OnLoaded)
{
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		UMCore_PlayerSettingsSave* NewSettings = Cast<UMCore_PlayerSettingsSave>(
			UGameplayStatics::CreateSaveGameObject(UMCore_PlayerSettingsSave::StaticClass()));
		NewSettings->CachedSlotName = SlotName;
		OnLoaded.ExecuteIfBound(NewSettings);
		return;
	}

	FAsyncLoadGameFromSlotDelegate AsyncLoadGameDelegate;
	AsyncLoadGameDelegate.BindLambda([OnLoaded, SlotName](const FString& LoadedSlot, const int32 UserIndex, USaveGame* LoadedSave)
	{
		UMCore_PlayerSettingsSave* Settings = nullptr;

		if (LoadedSave)
		{
			Settings = Cast<UMCore_PlayerSettingsSave>(LoadedSave);
			if (!Settings)
			{
				UE_LOG(LogMaevixSettings, Warning,
					TEXT("PlayerSettingsSave::LoadPlayerSettingsAsync: save existed in slot '%s' but cast to UMCore_PlayerSettingsSave failed"),
					*SlotName);
			}
		}

		if (!Settings)
		{
			Settings = Cast<UMCore_PlayerSettingsSave>(
				UGameplayStatics::CreateSaveGameObject(UMCore_PlayerSettingsSave::StaticClass()));
		}

		Settings->CachedSlotName = SlotName;
		Settings->ValidateSettings();
		OnLoaded.ExecuteIfBound(Settings);
	});

	UGameplayStatics::AsyncLoadGameFromSlot(SlotName, 0, AsyncLoadGameDelegate);
}

// ============================================================================
// FRAMEWORK CONVENIENCE
// ============================================================================

void UMCore_PlayerSettingsSave::SetUIScale(float NewScale)
{
	UIScale = FMath::Clamp(NewScale, 0.5f, 3.0f);
	ApplyUIScale();
}

void UMCore_PlayerSettingsSave::SetTooltipDelay(int32 DelayMs)
{
	TooltipDelayMs = FMath::Max(0, DelayMs);
}

void UMCore_PlayerSettingsSave::SetCategoryCollapsed(const FGameplayTag& CategoryTag, bool bCollapsed)
{
	if (!CategoryTag.IsValid())
	{
		return;
	}

	if (bCollapsed)
	{
		CollapsedCategories.Add(CategoryTag);
	}
	else
	{
		CollapsedCategories.Remove(CategoryTag);
	}
}

bool UMCore_PlayerSettingsSave::IsCategoryCollapsed(const FGameplayTag& CategoryTag) const
{
	return CategoryTag.IsValid() && CollapsedCategories.Contains(CategoryTag);
}

void UMCore_PlayerSettingsSave::SetLastSelectedCategory(const FGameplayTag& CategoryTag)
{
	LastSelectedCategory = CategoryTag;
}

void UMCore_PlayerSettingsSave::SetLastSelectedQualityPreset(int32 NewValue)
{
	LastSelectedQualityPreset = NewValue;
}

void UMCore_PlayerSettingsSave::ValidateSettings()
{
	UIScale = FMath::Clamp(UIScale, 0.5f, 3.0f);
	TooltipDelayMs = FMath::Max(0, TooltipDelayMs);

	/* Backfill QualityPreset intent on first load. The flag disambiguates fresh saves
	   (or pre-feature saves) from explicit Custom intent set later by the user. */
	if (!bQualityPresetInitialized)
	{
		if (GEngine)
		{
			if (UGameUserSettings* GameUserSettings = GEngine->GetGameUserSettings())
			{
				const int32 Single = GameUserSettings->ScalabilityQuality.GetSingleQualityLevel();
				if (Single >= 0 && Single <= 3)
				{
					LastSelectedQualityPreset = Single;
				}
				/* else: stays -1 (genuinely Custom or beyond exposed range) */
			}
		}
		bQualityPresetInitialized = true;
	}
}

// ============================================================================
// PRIVATE
// ============================================================================

void UMCore_PlayerSettingsSave::ApplyUIScale()
{
	if (!GEngine) { return; }

	UUserInterfaceSettings* UISettings = GetMutableDefault<UUserInterfaceSettings>();
	if (!UISettings) { return; }

	UISettings->ApplicationScale = UIScale;
}
