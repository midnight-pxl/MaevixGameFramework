// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/Widgets/Settings/MCore_SettingsWidget_Base.h"

#include "CoreData/Types/Settings/MCore_DA_SettingDefinition.h"
#include "CoreData/Logging/LogModulusSettings.h"
#include "CoreData/Tags/MCore_SettingsTags.h"
#include "CoreData/Types/Events/MCore_EventData.h"
#include "CoreEvents/MCore_LocalEventSubsystem.h"

#include "Engine/LocalPlayer.h"
#include "CommonTextBlock.h"

// ============================================================================
// INITIALIZATION
// ============================================================================

void UMCore_SettingsWidget_Base::InitFromDefinition(const UMCore_DA_SettingDefinition* InDefinition)
{
	if (!InDefinition)
	{
		UE_LOG(LogModulusSettings, Warning,
			TEXT("SettingsWidget_Base::InitFromDefinition: null definition passed, widget=%s"), *GetNameSafe(this));
		return;
	}

	if (!InDefinition->IsValid())
	{
		UE_LOG(LogModulusSettings, Warning,
			TEXT("SettingsWidget_Base::InitFromDefinition: definition '%s' failed validation, widget=%s"),
			*InDefinition->GetName(), *GetNameSafe(this));
		return;
	}

	SettingDefinition = InDefinition;

	if (Txt_Label)
	{
		Txt_Label->SetText(InDefinition->SettingDisplayName);
	}

	UE_LOG(LogModulusSettings, Verbose,
		TEXT("SettingsWidget_Base::InitFromDefinition: initialized from definition: %s (%s), widget=%s"),
		*InDefinition->SettingDisplayName.ToString(),
		*InDefinition->SettingTag.ToString(),
		*GetNameSafe(this));

	OnDefinitionSet(InDefinition);
}

FGameplayTag UMCore_SettingsWidget_Base::GetSettingTag() const
{
	return SettingDefinition ? SettingDefinition->SettingTag : FGameplayTag();
}

// ============================================================================
// VALUE INTERFACE (DEFAULTS)
// ============================================================================

void UMCore_SettingsWidget_Base::ResetToDefault_Implementation()
{
	UE_LOG(LogModulusSettings, Warning,
		TEXT("SettingsWidget_Base::ResetToDefault: not overridden, widget will not reset, widget=%s"),
		*GetNameSafe(this));
}

void UMCore_SettingsWidget_Base::RefreshValueFromSettings_Implementation()
{
	/* No-op. Derived classes fetch their values and update visuals */
}

FString UMCore_SettingsWidget_Base::GetValueAsString_Implementation() const
{
	return TEXT("(not implemented)");
}

void UMCore_SettingsWidget_Base::StepLeft_Implementation()
{
}

void UMCore_SettingsWidget_Base::StepRight_Implementation()
{
}

// ============================================================================
// SUBCLASS SUPPORT
// ============================================================================

void UMCore_SettingsWidget_Base::OnDefinitionSet_Implementation(
	const UMCore_DA_SettingDefinition* Definition)
{
}

void UMCore_SettingsWidget_Base::BroadcastValueChanged()
{
	if (SettingDefinition)
	{
		const FString ValueStr = GetValueAsString();
		OnSettingValueChanged.Broadcast(SettingDefinition->SettingTag, ValueStr);

		UE_LOG(LogModulusSettings, Verbose,
			TEXT("SettingsWidget_Base::BroadcastValueChanged: value changed: %s = %s, widget=%s"),
			*SettingDefinition->SettingTag.ToString(),
			*ValueStr,
			*GetNameSafe(this));
	}
}

// ============================================================================
// LIFECYCLE
// ============================================================================

void UMCore_SettingsWidget_Base::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UMCore_LocalEventSubsystem* LocalEventSubsystem = LocalPlayer->GetSubsystem<UMCore_LocalEventSubsystem>())
		{
			EventSubscriptionHandle = LocalEventSubsystem->OnLocalEventBroadcast.AddUObject(
				this, &UMCore_SettingsWidget_Base::HandleLocalEvent);
		}
	}
}

void UMCore_SettingsWidget_Base::NativeDestruct()
{
	if (EventSubscriptionHandle.IsValid())
	{
		if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
		{
			if (UMCore_LocalEventSubsystem* LocalEventSubsystem = LocalPlayer->GetSubsystem<UMCore_LocalEventSubsystem>())
			{
				LocalEventSubsystem->OnLocalEventBroadcast.Remove(EventSubscriptionHandle);
			}
		}
		EventSubscriptionHandle.Reset();
	}

	Super::NativeDestruct();
}

// ============================================================================
// ROW HOOKS
// ============================================================================

void UMCore_SettingsWidget_Base::NotifyRowFocusGained()
{
	BroadcastFocusedIfValid();
}

void UMCore_SettingsWidget_Base::BroadcastFocusedIfValid()
{
	if (const UMCore_DA_SettingDefinition* Def = GetSettingDefinition())
	{
		OnSettingFocused.Broadcast(Def->SettingTag, Def->Description);
	}
}

void UMCore_SettingsWidget_Base::HandleLocalEvent(const FMCore_EventData& EventData)
{
	if (EventData.EventTag.MatchesTagExact(MCore_SettingsTags::MCore_Settings_Event_ExternalValueChange)
		&& SettingDefinition != nullptr)
	{
		RefreshValueFromSettings();
	}
}
