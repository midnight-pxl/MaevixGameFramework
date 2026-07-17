// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreData/DevSettings/MCore_CoreSettings.h"

#include "CoreData/Assets/UI/Themes/MCore_PDA_UITheme_Base.h"
#include "CoreData/Types/UI/MCore_ThemeTypes.h"
#include "CoreData/Types/Settings/MCore_DA_SettingDefinition.h"
#include "CoreData/Settings/MCore_SettingsCollectionSubsystem.h"
#include "CoreData/Logging/LogMaevixSettings.h"
#include "Sound/SoundMix.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

/* Cache hosts on UMCore_SettingsCollectionSubsystem (UGameInstanceSubsystem). The
   CDO has no world context, so proxies walk GEngine's world contexts to find a
   live Game/PIE world's GameInstance. Cooker/commandlet contexts have no
   GameInstance and are skipped. */
static UMCore_SettingsCollectionSubsystem* MCore_FindRuntimeSubsystem()
{
	if (!GEngine) { return nullptr; }

	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType != EWorldType::Game && Context.WorldType != EWorldType::PIE)
		{
			continue;
		}
		if (!Context.OwningGameInstance) { continue; }
		if (UMCore_SettingsCollectionSubsystem* Subsystem =
			Context.OwningGameInstance->GetSubsystem<UMCore_SettingsCollectionSubsystem>())
		{
			return Subsystem;
		}
	}
	return nullptr;
}

UMCore_CoreSettings::UMCore_CoreSettings()
{
	VolumeMix = TSoftObjectPtr<USoundMix>(FSoftObjectPath(
		TEXT("/MaevixCore/Audio/MCore_VolumeMix.MCore_VolumeMix")));
}

const UMCore_CoreSettings* UMCore_CoreSettings::Get()
{
	return GetDefault<UMCore_CoreSettings>();
}

#if WITH_EDITOR
FText UMCore_CoreSettings::GetSectionText() const
{
	return NSLOCTEXT("MCore_CoreSettings", "SectionText", "Maevix Core");
}

FText UMCore_CoreSettings::GetSectionDescription() const
{
	return NSLOCTEXT("MCore_CoreSettings", "SectionDesc",
		"Configure theme settings and debug options for the Maevix Game Framework.");
}
#endif

bool UMCore_CoreSettings::IsEventLoggingEnabled() const
{
#if WITH_EDITORONLY_DATA
	return bEnableEventSystemLogging;
#else
	return false;
#endif
}

bool UMCore_CoreSettings::IsUIDebugOverlayEnabled() const
{
#if WITH_EDITORONLY_DATA
	return bShowUILayerDebugOverlay;
#else
	return false;
#endif
}

UMCore_PDA_UITheme_Base* UMCore_CoreSettings::GetDefaultTheme() const
{
	if (!IsValidThemeIndex(DefaultThemeIndex))
	{
		return nullptr;
	}

	const FMCore_ThemeEntry& ThemeEntry = AvailableThemes[DefaultThemeIndex];
	if (ThemeEntry.ThemeAsset.IsNull())
	{
		return nullptr;
	}

	return ThemeEntry.ThemeAsset.LoadSynchronous();
}

UMCore_PDA_UITheme_Base* UMCore_CoreSettings::GetDesignTimeTheme()
{
	const UMCore_CoreSettings* Settings = Get();
	return Settings ? Settings->GetDefaultTheme() : nullptr;
}

bool UMCore_CoreSettings::IsValidThemeIndex(int32 Index) const
{
	return AvailableThemes.IsValidIndex(Index);
}

// ============================================================================
// SETTINGS AGGREGATION
// ============================================================================

const TArray<UMCore_DA_SettingsCollection*>& UMCore_CoreSettings::GetAllSettingsCollections() const
{
	if (UMCore_SettingsCollectionSubsystem* Subsystem = MCore_FindRuntimeSubsystem())
	{
		return Subsystem->GetAllSettingsCollections();
	}

	/* No GameInstance available (commandlet / cooker / pre-PIE editor inspection). */
	static const TArray<UMCore_DA_SettingsCollection*> EmptyArray;
	return EmptyArray;
}

UMCore_DA_SettingDefinition* UMCore_CoreSettings::FindSettingDefinitionByTag(
	const FGameplayTag& SettingTag) const
{
	if (UMCore_SettingsCollectionSubsystem* Subsystem = MCore_FindRuntimeSubsystem())
	{
		return Subsystem->FindSettingDefinitionByTag(SettingTag);
	}
	return nullptr;
}

TArray<UMCore_DA_SettingDefinition*> UMCore_CoreSettings::GetSettingsForCategory(
	const FGameplayTag& CategoryTag) const
{
	if (UMCore_SettingsCollectionSubsystem* Subsystem = MCore_FindRuntimeSubsystem())
	{
		return Subsystem->GetSettingsForCategory(CategoryTag);
	}
	return TArray<UMCore_DA_SettingDefinition*>();
}

TArray<FGameplayTag> UMCore_CoreSettings::GetAllSettingsCategories() const
{
	if (UMCore_SettingsCollectionSubsystem* Subsystem = MCore_FindRuntimeSubsystem())
	{
		return Subsystem->GetAllSettingsCategories();
	}
	return TArray<FGameplayTag>();
}

FText UMCore_CoreSettings::GetCategoryDisplayName(const FGameplayTag& CategoryTag) const
{
	if (UMCore_SettingsCollectionSubsystem* Subsystem = MCore_FindRuntimeSubsystem())
	{
		return Subsystem->GetCategoryDisplayName(CategoryTag);
	}
	return FText::GetEmpty();
}

bool UMCore_CoreSettings::HasValidSettingsCollections() const
{
	if (UMCore_SettingsCollectionSubsystem* Subsystem = MCore_FindRuntimeSubsystem())
	{
		return Subsystem->HasValidSettingsCollections();
	}
	return false;
}

void UMCore_CoreSettings::InvalidateCollectionCache()
{
	if (UMCore_SettingsCollectionSubsystem* Subsystem = MCore_FindRuntimeSubsystem())
	{
		Subsystem->InvalidateCollectionCache();
	}
	/* Silent no-op if no subsystem; editor-time edit before any PIE has run. The next
	   PIE start gets a fresh subsystem with empty cache. */
}

#if WITH_EDITOR
void UMCore_CoreSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UMCore_CoreSettings, SettingsCollections))
	{
		InvalidateCollectionCache();
	}
}
#endif
