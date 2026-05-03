// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/Widgets/Settings/MCore_SettingsWidget_Base.h"

#include "CoreData/Types/Settings/MCore_DA_SettingDefinition.h"
#include "CoreData/Assets/UI/Themes/MCore_PDA_UITheme_Base.h"
#include "CoreData/Logging/LogModulusSettings.h"
#include "CoreData/Tags/MCore_SettingsTags.h"
#include "CoreData/Types/Events/MCore_EventData.h"
#include "CoreEvents/MCore_LocalEventSubsystem.h"
#include "CoreUI/MCore_UISubsystem.h"
#include "CoreData/DevSettings/MCore_CoreSettings.h"
#include "CoreData/Libraries/MCore_ThemeLibrary.h"

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
			TEXT("SettingsWidget_Base::InitFromDefinition -- null definition passed, widget=%s"), *GetNameSafe(this));
		return;
	}

	if (!InDefinition->IsValid())
	{
		UE_LOG(LogModulusSettings, Warning,
			TEXT("SettingsWidget_Base::InitFromDefinition -- definition '%s' failed validation, widget=%s"),
			*InDefinition->GetName(), *GetNameSafe(this));
		return;
	}

	SettingDefinition = InDefinition;

	if (Txt_SettingName)
	{
		Txt_SettingName->SetText(InDefinition->DisplayName);
	}

	UE_LOG(LogModulusSettings, Verbose,
		TEXT("SettingsWidget_Base::InitFromDefinition -- initialized from definition: %s (%s), widget=%s"),
		*InDefinition->DisplayName.ToString(),
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
		TEXT("SettingsWidget_Base::ResetToDefault -- not overridden, widget will not reset, widget=%s"),
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
			TEXT("SettingsWidget_Base::BroadcastValueChanged -- value changed: %s = %s, widget=%s"),
			*SettingDefinition->SettingTag.ToString(),
			*ValueStr,
			*GetNameSafe(this));
	}
}

// ============================================================================
// THEME
// ============================================================================

void UMCore_SettingsWidget_Base::ApplyTheme_Implementation(UMCore_PDA_UITheme_Base* NewTheme)
{
	if (NewTheme)
	{
		UMCore_ThemeLibrary::ApplyTextStyleFromTheme(
			GetOwningLocalPlayer(), Txt_SettingName, NewTheme->LabelTextStyle);
	}

	K2_OnThemeApplied(NewTheme);
}

void UMCore_SettingsWidget_Base::HandleThemeChanged(UMCore_PDA_UITheme_Base* NewTheme)
{
	CachedTheme = NewTheme;
	ApplyTheme(NewTheme);
}

void UMCore_SettingsWidget_Base::BindThemeDelegate()
{
	if (bThemeDelegateBound) { return; }

	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!LocalPlayer) { return; }

	UMCore_UISubsystem* UI = LocalPlayer->GetSubsystem<UMCore_UISubsystem>();
	if (!UI) { return; }

	UI->OnThemeChanged.AddDynamic(this, &UMCore_SettingsWidget_Base::HandleThemeChanged);
	bThemeDelegateBound = true;
}

void UMCore_SettingsWidget_Base::UnbindThemeDelegate()
{
	if (!bThemeDelegateBound) { return; }

	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!LocalPlayer)
	{
		bThemeDelegateBound = false;
		return;
	}

	UMCore_UISubsystem* UI = LocalPlayer->GetSubsystem<UMCore_UISubsystem>();
	if (UI)
	{
		UI->OnThemeChanged.RemoveDynamic(this, &UMCore_SettingsWidget_Base::HandleThemeChanged);
	}

	bThemeDelegateBound = false;
}

// ============================================================================
// LIFECYCLE
// ============================================================================

void UMCore_SettingsWidget_Base::NativePreConstruct()
{
	Super::NativePreConstruct();
	ApplyTheme(UMCore_CoreSettings::GetDesignTimeTheme());
}

void UMCore_SettingsWidget_Base::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BindThemeDelegate();

	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UMCore_UISubsystem* UI = LocalPlayer->GetSubsystem<UMCore_UISubsystem>())
		{
			ApplyTheme(UI->GetActiveTheme());
		}

		if (UMCore_LocalEventSubsystem* LES = LocalPlayer->GetSubsystem<UMCore_LocalEventSubsystem>())
		{
			EventSubscriptionHandle = LES->OnLocalEventBroadcast.AddUObject(
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
			if (UMCore_LocalEventSubsystem* LES = LocalPlayer->GetSubsystem<UMCore_LocalEventSubsystem>())
			{
				LES->OnLocalEventBroadcast.Remove(EventSubscriptionHandle);
			}
		}
		EventSubscriptionHandle.Reset();
	}

	UnbindThemeDelegate();
	Super::NativeDestruct();
}

void UMCore_SettingsWidget_Base::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	bIsRowMouseOver = true;
	UpdateHighlightState();
	BroadcastFocusedIfValid();
}

void UMCore_SettingsWidget_Base::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	bIsRowMouseOver = false;
	UpdateHighlightState();
}

void UMCore_SettingsWidget_Base::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);
	bIsRowInFocusPath = true;
	UpdateHighlightState();
	BroadcastFocusedIfValid();
}

void UMCore_SettingsWidget_Base::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);
	bIsRowInFocusPath = false;
	UpdateHighlightState();
}

void UMCore_SettingsWidget_Base::BroadcastFocusedIfValid()
{
	if (const UMCore_DA_SettingDefinition* Def = GetSettingDefinition())
	{
		OnSettingFocused.Broadcast(Def->SettingTag, Def->Description);
	}
}

void UMCore_SettingsWidget_Base::UpdateHighlightState()
{
	if (Highlight)
	{
		const ESlateVisibility NewVisibility = (bIsRowMouseOver || bIsRowInFocusPath)
			? ESlateVisibility::HitTestInvisible
			: ESlateVisibility::Hidden;
		Highlight->SetVisibility(NewVisibility);
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
