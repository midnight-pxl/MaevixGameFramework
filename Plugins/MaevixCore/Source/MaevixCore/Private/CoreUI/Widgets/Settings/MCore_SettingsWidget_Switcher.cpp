// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/Widgets/Settings/MCore_SettingsWidget_Switcher.h"

#include "CoreData/Types/Settings/MCore_DA_SettingDefinition.h"
#include "CoreData/Types/Settings/MCore_SettingsTypes.h"
#include "CoreData/Libraries/MCore_GameSettingsLibrary.h"
#include "CoreData/Assets/UI/Themes/MCore_PDA_UITheme_Base.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Algo/Reverse.h"
#include "CoreData/Logging/LogMaevixSettings.h"
#include "CoreUI/Widgets/Primitives/MCore_ButtonBase.h"
#include "CoreData/Libraries/MCore_ThemeLibrary.h"

#include "CommonTextBlock.h"

// ============================================================================
// LIFECYCLE
// ============================================================================

void UMCore_SettingsWidget_Switcher::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_Previous)
	{
		Btn_Previous->OnButtonClicked.AddDynamic(this, &ThisClass::HandlePreviousClicked);
	}

	if (Btn_Next)
	{
		Btn_Next->OnButtonClicked.AddDynamic(this, &ThisClass::HandleNextClicked);
	}
}

void UMCore_SettingsWidget_Switcher::NativeDestruct()
{
	if (Btn_Previous) { Btn_Previous->OnButtonClicked.RemoveAll(this); }
	if (Btn_Next) { Btn_Next->OnButtonClicked.RemoveAll(this); }

	Super::NativeDestruct();
}

// ============================================================================
// DEFINITION SET
// ============================================================================

void UMCore_SettingsWidget_Switcher::OnDefinitionSet_Implementation(
	const UMCore_DA_SettingDefinition* Definition)
{
	if (!Definition)
	{
		return;
	}

	bIsToggleMode = (Definition->SettingType == EMCore_SettingType::Toggle);

	if (bIsToggleMode)
	{
		Options.Reset(2);
		Options.Add(NSLOCTEXT("MaevixCore", "ToggleOff", "Off"));
		Options.Add(NSLOCTEXT("MaevixCore", "ToggleOn", "On"));
	}
	else
	{
		if (Definition->bPopulateFromSupportedResolutions)
		{
			TArray<FIntPoint> Resolutions;
			UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);
			Algo::Reverse(Resolutions);

			Options.Reset(Resolutions.Num());
			for (const FIntPoint& Res : Resolutions)
			{
				Options.Add(FText::FromString(
					FString::Printf(TEXT("%d \u00D7 %d"), Res.X, Res.Y)));
			}
		}
		else
		{
			Options = Definition->DropdownOptions;
		}
	}

	ReadCurrentValue();
	UpdateDisplay();

	UE_LOG(LogMaevixSettings, Verbose,
		TEXT("SettingsWidget_Switcher::OnDefinitionSet: configured %d options, toggle=%s, current=%d, widget=%s"),
		Options.Num(), bIsToggleMode ? TEXT("true") : TEXT("false"), CurrentIndex, *GetNameSafe(this));
}

// ============================================================================
// BUTTON HANDLERS
// ============================================================================

void UMCore_SettingsWidget_Switcher::HandlePreviousClicked()
{
	StepLeft();
}

void UMCore_SettingsWidget_Switcher::HandleNextClicked()
{
	StepRight();
}

// ============================================================================
// STEP OVERRIDES
// ============================================================================

void UMCore_SettingsWidget_Switcher::StepLeft_Implementation()
{
	CycleOption(-1);
}

void UMCore_SettingsWidget_Switcher::StepRight_Implementation()
{
	CycleOption(1);
}

// ============================================================================
// HELPERS
// ============================================================================

void UMCore_SettingsWidget_Switcher::CycleOption(int32 Direction)
{
	if (Options.Num() <= 0)
	{
		return;
	}

	const int32 OptionCount = Options.Num();
	const int32 SelectableCount =
		(SettingDefinition
		 && SettingDefinition->NumSelectableOptions > 0
		 && SettingDefinition->NumSelectableOptions < OptionCount)
			? SettingDefinition->NumSelectableOptions
			: OptionCount;

	if (SelectableCount <= 0) { return; }

	/* If the current displayed index is a non-selectable status (e.g. "Custom" at index 4
	   for QualityPreset), snap to the highest selectable index before stepping so the user
	   navigates within the cycleable range. */
	const int32 BaseIndex = (CurrentIndex >= SelectableCount) ? (SelectableCount - 1) : CurrentIndex;
	CurrentIndex = (BaseIndex + Direction + SelectableCount) % SelectableCount;

	ApplyCurrentValue();
	UpdateDisplay();
}

void UMCore_SettingsWidget_Switcher::UpdateDisplay()
{
	if (Txt_CurrentOption && Options.IsValidIndex(CurrentIndex))
	{
		Txt_CurrentOption->SetText(Options[CurrentIndex]);
	}
}

void UMCore_SettingsWidget_Switcher::ApplyCurrentValue()
{
	if (!SettingDefinition)
	{
		return;
	}

	UMCore_DA_SettingDefinition* MutableDef =
		const_cast<UMCore_DA_SettingDefinition*>(SettingDefinition.Get());

	if (bIsToggleMode)
	{
		TArray<FMCore_BoolSettingChange> Changes;
		Changes.Add({MutableDef, CurrentIndex == 1});
		UMCore_GameSettingsLibrary::SetSettingBool(GetOwningLocalPlayer(), Changes, false);
	}
	else
	{
		TArray<FMCore_IntSettingChange> Changes;
		Changes.Add({MutableDef, CurrentIndex});
		UMCore_GameSettingsLibrary::SetSettingInt(GetOwningLocalPlayer(), Changes, false);
	}

	BroadcastValueChanged();
}

void UMCore_SettingsWidget_Switcher::ReadCurrentValue()
{
	if (!SettingDefinition) { return; }

	if (bIsToggleMode)
	{
		const bool BoolVal = UMCore_GameSettingsLibrary::GetSettingBool(
			GetOwningLocalPlayer(), SettingDefinition.Get());
		CurrentIndex = BoolVal ? 1 : 0;
	}
	else
	{
		CurrentIndex = ResolveDisplayedIndex();
	}

	if (Options.Num() > 0)
	{
		CurrentIndex = FMath::Clamp(CurrentIndex, 0, Options.Num() - 1);
	}
	else
	{
		CurrentIndex = 0;
	}
}

int32 UMCore_SettingsWidget_Switcher::ResolveDisplayedIndex_Implementation()
{
	if (!SettingDefinition) { return 0; }
	return UMCore_GameSettingsLibrary::GetSettingInt(
		GetOwningLocalPlayer(), SettingDefinition.Get());
}

// ============================================================================
// VALUE INTERFACE OVERRIDES
// ============================================================================

FString UMCore_SettingsWidget_Switcher::GetValueAsString_Implementation() const
{
	if (Options.IsValidIndex(CurrentIndex))
	{
		return Options[CurrentIndex].ToString();
	}

	return FString();
}

void UMCore_SettingsWidget_Switcher::ResetToDefault_Implementation()
{
	if (!SettingDefinition) { return; }
	
	UMCore_GameSettingsLibrary::ResetSettingToDefault(
		GetOwningLocalPlayer(), SettingDefinition.Get());

	ReadCurrentValue();
	UpdateDisplay();

	UE_LOG(LogMaevixSettings, Verbose,
		TEXT("SettingsWidget_Switcher::ResetToDefault: reset to index %d, widget=%s"),
		CurrentIndex, *GetNameSafe(this));
}

void UMCore_SettingsWidget_Switcher::RefreshValueFromSettings_Implementation()
{
	if (!SettingDefinition) { return; }
	
	ReadCurrentValue();
	UpdateDisplay();
}

// ============================================================================
// THEME
// ============================================================================

void UMCore_SettingsWidget_Switcher::ApplyTheme_Implementation(UMCore_PDA_UITheme_Base* NewTheme)
{
	Super::ApplyTheme_Implementation(NewTheme);

	if (!NewTheme) { return; }
	
	UMCore_ThemeLibrary::ApplyTextStyleFromTheme(
		GetOwningLocalPlayer(), Txt_CurrentOption, NewTheme->ValueTextStyle);
	
	const TSubclassOf<UCommonButtonStyle> ArrowStyle =
		UMCore_ThemeLibrary::ResolveButtonStyle(
			NewTheme->GhostButtonStyle, NewTheme->SecondaryButtonStyle);

	if (ArrowStyle)
	{
		Btn_Previous->SetButtonStyleOverride(ArrowStyle);
		Btn_Next->SetButtonStyleOverride(ArrowStyle);
	}
}
