// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreData/Settings/MCore_PlayerSettingsSubsystem.h"

#include "CoreData/Types/Settings/MCore_PlayerSettingsSave.h"
#include "CoreData/Libraries/MCore_GameSettingsLibrary.h"
#include "CoreData/Tags/MCore_SettingsTags.h"
#include "CoreData/Logging/LogModulusSettings.h"

#include "Engine/LocalPlayer.h"

void UMCore_PlayerSettingsSubsystem::Deinitialize()
{
	if (CachedPlayerSettings)
	{
		CachedPlayerSettings->SaveSettings();
		CachedPlayerSettings = nullptr;
	}

	Super::Deinitialize();
}

void UMCore_PlayerSettingsSubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
	Super::PlayerControllerChanged(NewPlayerController);

	if (!NewPlayerController || bBootApplyDone) { return; }

	bBootApplyDone = true;
	UMCore_GameSettingsLibrary::ApplyAllSettingsToEngine(this);

	UE_LOG(LogModulusSettings, Log,
		TEXT("PlayerSettingsSubsystem::PlayerControllerChanged: boot-time apply complete"));
}

FString UMCore_PlayerSettingsSubsystem::GetSettingsSaveSlotName_Implementation() const
{
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	const int32 PlayerIndex = LocalPlayer ? LocalPlayer->GetControllerId() : 0;
	return FString::Printf(TEXT("MCore_PlayerSettings_%d"), PlayerIndex);
}

UMCore_PlayerSettingsSave* UMCore_PlayerSettingsSubsystem::GetPlayerSettings()
{
	if (!CachedPlayerSettings)
	{
		CachedPlayerSettings = UMCore_PlayerSettingsSave::LoadPlayerSettings(GetSettingsSaveSlotName());

		UE_LOG(LogModulusSettings, Log,
			TEXT("PlayerSettingsSubsystem::GetPlayerSettings: loaded from slot '%s'"),
			*CachedPlayerSettings->GetCachedSlotName());
	}

	return CachedPlayerSettings;
}

int32 UMCore_PlayerSettingsSubsystem::GetActiveTextSizeIndex() const
{
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer) { return 0; }

	const int32 RawIndex = UMCore_GameSettingsLibrary::GetSettingIntByTag(
		LocalPlayer, MCore_SettingsTags::MCore_Settings_Accessibility_UITextSize);
	return FMath::Max(RawIndex, 0);
}
