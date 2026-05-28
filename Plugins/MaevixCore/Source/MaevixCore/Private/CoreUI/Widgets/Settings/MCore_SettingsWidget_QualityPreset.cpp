// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/Widgets/Settings/MCore_SettingsWidget_QualityPreset.h"

#include "CoreData/Logging/LogMaevixSettings.h"
#include "CoreData/Settings/MCore_PlayerSettingsSubsystem.h"
#include "CoreData/Types/Settings/MCore_PlayerSettingsSave.h"

#include "Engine/LocalPlayer.h"

int32 UMCore_SettingsWidget_QualityPreset::ResolveDisplayedIndex_Implementation()
{
	const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	UMCore_PlayerSettingsSubsystem* Subsystem = LocalPlayer
		? LocalPlayer->GetSubsystem<UMCore_PlayerSettingsSubsystem>()
		: nullptr;
	UMCore_PlayerSettingsSave* Save = Subsystem ? Subsystem->GetPlayerSettings() : nullptr;

	if (!Save)
	{
		return Super::ResolveDisplayedIndex_Implementation();
	}

	if (!Save->bQualityPresetInitialized)
	{
		UE_LOG(LogMaevixSettings, Warning,
			TEXT("SettingsWidget_QualityPreset::ResolveDisplayedIndex: "
				 "save not yet initialized, falling back to base resolution"));
		return Super::ResolveDisplayedIndex_Implementation();
	}

	const int32 Intent = Save->GetLastSelectedQualityPreset();

	if (Intent >= 0 && Intent <= 3)
	{
		return Intent;
	}

	if (Intent == -1)
	{
		/* Custom; display index 4 (assumes DA's DropdownOptions has 5 entries). */
		return 4;
	}

	UE_LOG(LogMaevixSettings, Warning,
		TEXT("SettingsWidget_QualityPreset::ResolveDisplayedIndex: "
			 "unexpected LastSelectedQualityPreset value %d, clamping to [0, 3]"),
		Intent);
	return FMath::Clamp(Intent, 0, 3);
}
