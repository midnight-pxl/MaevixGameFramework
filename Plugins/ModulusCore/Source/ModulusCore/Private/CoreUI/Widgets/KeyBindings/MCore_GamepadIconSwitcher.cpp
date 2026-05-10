// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/Widgets/KeyBindings/MCore_GamepadIconSwitcher.h"

#include "CoreUI/Widgets/Primitives/MCore_ButtonBase.h"
#include "CoreData/Settings/MCore_PlayerSettingsSubsystem.h"
#include "CoreData/Types/Settings/MCore_PlayerSettingsSave.h"
#include "CoreData/Libraries/MCore_InputDisplayLibrary.h"
#include "CoreData/Libraries/MCore_EventFunctionLibrary.h"
#include "CoreData/Tags/MCore_SettingsTags.h"
#include "CoreData/Logging/LogModulusUI.h"

#include "CommonTextBlock.h"
#include "CommonInputBaseTypes.h"

#define LOCTEXT_NAMESPACE "ModulusCoreUI"

// ============================================================================
// LIFECYCLE
// ============================================================================

void UMCore_GamepadIconSwitcher::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_Previous)
	{
		Btn_Previous->OnButtonClicked.AddDynamic(this, &ThisClass::HandlePrevious);
	}
	if (Btn_Next)
	{
		Btn_Next->OnButtonClicked.AddDynamic(this, &ThisClass::HandleNext);
	}

	RefreshDisplay();
}

void UMCore_GamepadIconSwitcher::NativeDestruct()
{
	if (Btn_Previous) { Btn_Previous->OnButtonClicked.RemoveAll(this); }
	if (Btn_Next) { Btn_Next->OnButtonClicked.RemoveAll(this); }

	Super::NativeDestruct();
}

// ============================================================================
// PUBLIC API
// ============================================================================

void UMCore_GamepadIconSwitcher::RefreshDisplay()
{
	AvailableOptions.Reset();
	AvailableOptions.Add(LOCTEXT("AutoDetect", "Auto-Detect"));

	/* Pull registered gamepad configs and build display names */
	TArray<FName> Configs = UMCore_InputDisplayLibrary::GetAvailableGamepadConfigs(this);

	UCommonInputPlatformSettings* PlatformSettings = UCommonInputPlatformSettings::Get();
	for (const FName& ConfigName : Configs)
	{
		FText DisplayName = FText::FromName(ConfigName);

		/* Use the controller data asset's display name if available */
		if (PlatformSettings)
		{
			TArray<const UCommonInputBaseControllerData*> DataAssets =
				PlatformSettings->GetControllerDataForInputType(
					ECommonInputType::Gamepad, ConfigName);

			for (const UCommonInputBaseControllerData* Data : DataAssets)
			{
				if (Data && !Data->GamepadDisplayName.IsEmpty())
				{
					DisplayName = Data->GamepadDisplayName;
					break;
				}
			}
		}

		AvailableOptions.Add(DisplayName);
	}

	/* Read saved index */
	const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	UMCore_PlayerSettingsSubsystem* SettingsSubsystem = LocalPlayer ? LocalPlayer->GetSubsystem<UMCore_PlayerSettingsSubsystem>() : nullptr;
	if (SettingsSubsystem)
	{
		if (UMCore_PlayerSettingsSave* Save = SettingsSubsystem->GetPlayerSettings())
		{
			CurrentIndex = Save->GamepadIconSetIndex;
		}
	}

	CurrentIndex = FMath::Clamp(CurrentIndex, 0, AvailableOptions.Num() - 1);

	if (Txt_PlatformName && AvailableOptions.IsValidIndex(CurrentIndex))
	{
		Txt_PlatformName->SetText(AvailableOptions[CurrentIndex]);
	}
}

// ============================================================================
// HANDLERS
// ============================================================================

void UMCore_GamepadIconSwitcher::HandlePrevious()
{
	if (AvailableOptions.Num() <= 0) { return; }

	CurrentIndex = (CurrentIndex - 1 + AvailableOptions.Num()) % AvailableOptions.Num();
	ApplySelection();
}

void UMCore_GamepadIconSwitcher::HandleNext()
{
	if (AvailableOptions.Num() <= 0) { return; }

	CurrentIndex = (CurrentIndex + 1) % AvailableOptions.Num();
	ApplySelection();
}

void UMCore_GamepadIconSwitcher::ApplySelection()
{
	const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	UMCore_PlayerSettingsSubsystem* SettingsSubsystem = LocalPlayer ? LocalPlayer->GetSubsystem<UMCore_PlayerSettingsSubsystem>() : nullptr;
	if (SettingsSubsystem)
	{
		if (UMCore_PlayerSettingsSave* Save = SettingsSubsystem->GetPlayerSettings())
		{
			Save->GamepadIconSetIndex = CurrentIndex;
			Save->SaveSettings();
		}
	}

	if (Txt_PlatformName && AvailableOptions.IsValidIndex(CurrentIndex))
	{
		Txt_PlatformName->SetText(AvailableOptions[CurrentIndex]);
	}

	UMCore_EventFunctionLibrary::BroadcastSimpleEvent(
		this,
		MCore_SettingsTags::MCore_Settings_Event_GamepadIconSetChanged,
		EMCore_EventScope::Local);

	UE_LOG(LogModulusUI, Log,
		TEXT("GamepadIconSwitcher::ApplySelection: gamepad icon set changed to index %d, widget=%s"),
		CurrentIndex, *GetNameSafe(this));
}

#undef LOCTEXT_NAMESPACE
