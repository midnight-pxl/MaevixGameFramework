// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/Widgets/Settings/MCore_SettingsWidget_Slider.h"

#include "CoreData/Types/Settings/MCore_DA_SettingDefinition.h"
#include "CoreData/Types/Settings/MCore_SettingsTypes.h"
#include "CoreData/Libraries/MCore_GameSettingsLibrary.h"
#include "CoreData/Assets/UI/Themes/MCore_PDA_UITheme_Base.h"
#include "CoreData/Logging/LogModulusSettings.h"
#include "CoreUI/Widgets/Primitives/MCore_ButtonBase.h"
#include "CoreData/Libraries/MCore_ThemeLibrary.h"

#include "CommonTextBlock.h"
#include "Components/Slider.h"

// ============================================================================
// LIFECYCLE
// ============================================================================

void UMCore_SettingsWidget_Slider::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Slider_Value)
	{
		Slider_Value->OnValueChanged.AddDynamic(this, &ThisClass::HandleSliderValueChanged);
	}

	const ESlateVisibility StepVisibility = bShowStepButtons ?
		ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	Btn_StepLeft->SetVisibility(StepVisibility);
	Btn_StepLeft->OnButtonClicked.AddDynamic(this, &ThisClass::HandleStepLeft);

	Btn_StepRight->SetVisibility(StepVisibility);
	Btn_StepRight->OnButtonClicked.AddDynamic(this, &ThisClass::HandleStepRight);
}

void UMCore_SettingsWidget_Slider::NativeDestruct()
{
	if (Slider_Value) { Slider_Value->OnValueChanged.RemoveAll(this); }
	if (Btn_StepLeft) { Btn_StepLeft->OnButtonClicked.RemoveAll(this); }
	if (Btn_StepRight) { Btn_StepRight->OnButtonClicked.RemoveAll(this); }

	Super::NativeDestruct();
}

// ============================================================================
// DEFINITION SET
// ============================================================================

void UMCore_SettingsWidget_Slider::OnDefinitionSet_Implementation(
	const UMCore_DA_SettingDefinition* Definition)
{
	if (!Definition || !Slider_Value) { return; }

	const float CurrentValue = UMCore_GameSettingsLibrary::GetSettingFloat(
		GetOwningLocalPlayer(), Definition);

	const float ClampedValue = FMath::Clamp(CurrentValue,
		Definition->MinValue, Definition->MaxValue);

	bIsUpdatingSlider = true;
	Slider_Value->SetMinValue(Definition->MinValue);
	Slider_Value->SetMaxValue(Definition->MaxValue);
	Slider_Value->SetStepSize(0.0f);
	Slider_Value->SetValue(ClampedValue);
	bIsUpdatingSlider = false;

	UE_LOG(LogModulusSettings, Verbose,
		TEXT("SettingsWidget_Slider::OnDefinitionSet: configured range [%.2f, %.2f] step=%.2f, current=%.2f, widget=%s"),
		Definition->MinValue, Definition->MaxValue, Definition->StepSize, ClampedValue, *GetNameSafe(this));

	if (Txt_ValueDisplay)
	{
		Txt_ValueDisplay->SetText(FormatValue(ClampedValue));
	}
}

// ============================================================================
// SLIDER HANDLER
// ============================================================================

void UMCore_SettingsWidget_Slider::HandleSliderValueChanged(float RawValue)
{
	if (bIsUpdatingSlider) { return; }

	const float Snapped = SnapToStep(RawValue);

	if (!FMath::IsNearlyEqual(Snapped, RawValue, KINDA_SMALL_NUMBER))
	{
		bIsUpdatingSlider = true;
		Slider_Value->SetValue(Snapped);
		bIsUpdatingSlider = false;
	}

	ApplyValueToEngine(Snapped);
	SyncSliderAndDisplay(Snapped);
	BroadcastValueChanged();
}

// ============================================================================
// STEP BUTTONS
// ============================================================================

void UMCore_SettingsWidget_Slider::HandleStepLeft()
{
	StepLeft();
}

void UMCore_SettingsWidget_Slider::HandleStepRight()
{
	StepRight();
}

void UMCore_SettingsWidget_Slider::StepLeft_Implementation()
{
	if (!SettingDefinition) { return; }

	StepByAmount(-SettingDefinition->StepSize);
}

void UMCore_SettingsWidget_Slider::StepRight_Implementation()
{
	if (!SettingDefinition) { return; }

	StepByAmount(SettingDefinition->StepSize);
}

void UMCore_SettingsWidget_Slider::StepByAmount(float Delta)
{
	if (!SettingDefinition || !Slider_Value) { return; }

	const float Current = Slider_Value->GetValue();
	const float NewValue = FMath::Clamp(Current + Delta,
		SettingDefinition->MinValue, SettingDefinition->MaxValue);
	const float Snapped = SnapToStep(NewValue);

	bIsUpdatingSlider = true;
	Slider_Value->SetValue(Snapped);
	bIsUpdatingSlider = false;

	ApplyValueToEngine(Snapped);
	SyncSliderAndDisplay(Snapped);
	BroadcastValueChanged();
}

// ============================================================================
// HELPERS
// ============================================================================

float UMCore_SettingsWidget_Slider::SnapToStep(float RawValue) const
{
	if (!SettingDefinition || SettingDefinition->StepSize <= 0.f) { return RawValue; }

	const float Snapped = FMath::RoundToFloat(
		(RawValue - SettingDefinition->MinValue) / SettingDefinition->StepSize)
		* SettingDefinition->StepSize + SettingDefinition->MinValue;

	return FMath::Clamp(Snapped,
		SettingDefinition->MinValue, SettingDefinition->MaxValue);
}

FText UMCore_SettingsWidget_Slider::FormatValue(float Value) const
{
	if (!SettingDefinition) { return FText::GetEmpty(); }

	switch (SettingDefinition->SliderDisplayFormat)
	{
	case EMCore_SliderDisplayFormat::Percentage:
		{
			const float Range = SettingDefinition->MaxValue - SettingDefinition->MinValue;
			if (FMath::IsNearlyZero(Range))
			{
				return FText::FromString(TEXT("0%"));
			}
			
			const int32 Percent = FMath::RoundToInt(
				(Value - SettingDefinition->MinValue) / Range * 100.0f);
			
			return FText::FromString(FString::Printf(TEXT("%d%%"), Percent));
		}

	case EMCore_SliderDisplayFormat::WholeNumber:
		return FText::FromString(FString::Printf(TEXT("%d"),
			FMath::RoundToInt(Value)));

	case EMCore_SliderDisplayFormat::RawValue:
	default:
		return FText::FromString(FString::Printf(TEXT("%.2f"), Value));
	}
}

void UMCore_SettingsWidget_Slider::ApplyValueToEngine(float SnappedValue)
{
	if (!SettingDefinition) { return; }

	TArray<FMCore_FloatSettingChange> Changes;
	Changes.Add({const_cast<UMCore_DA_SettingDefinition*>(SettingDefinition.Get()), SnappedValue});
	UMCore_GameSettingsLibrary::SetSettingFloat(GetOwningLocalPlayer(), Changes, false);
}

void UMCore_SettingsWidget_Slider::SyncSliderAndDisplay(float SnappedValue)
{
	if (Txt_ValueDisplay)
	{
		Txt_ValueDisplay->SetText(FormatValue(SnappedValue));
	}
}

// ============================================================================
// VALUE INTERFACE OVERRIDES
// ============================================================================

FString UMCore_SettingsWidget_Slider::GetValueAsString_Implementation() const
{
	if (!Slider_Value) { return TEXT("0"); }

	return FormatValue(Slider_Value->GetValue()).ToString();
}

void UMCore_SettingsWidget_Slider::ResetToDefault_Implementation()
{
	if (!SettingDefinition || !Slider_Value)
	{
		return;
	}

	const float DefaultVal = SettingDefinition->DefaultValue;

	bIsUpdatingSlider = true;
	Slider_Value->SetValue(DefaultVal);
	bIsUpdatingSlider = false;

	ApplyValueToEngine(DefaultVal);
	SyncSliderAndDisplay(DefaultVal);

	UE_LOG(LogModulusSettings, Verbose,
		TEXT("SettingsWidget_Slider::ResetToDefault: reset to %.2f, widget=%s"),
		DefaultVal, *GetNameSafe(this));
}

void UMCore_SettingsWidget_Slider::RefreshValueFromSettings_Implementation()
{
	if (!SettingDefinition || !Slider_Value) { return; }
	
	const float CurrentValue = UMCore_GameSettingsLibrary::GetSettingFloat(
		GetOwningLocalPlayer(), SettingDefinition);
	
	const float ClampedValue = FMath::Clamp(CurrentValue,
		SettingDefinition->MinValue, SettingDefinition->MaxValue);
	
	bIsUpdatingSlider = true;
	Slider_Value->SetValue(ClampedValue);
	bIsUpdatingSlider = false;
	
	if (Txt_ValueDisplay)
	{
		Txt_ValueDisplay->SetText(FormatValue(ClampedValue));
	}
}

// ============================================================================
// THEME
// ============================================================================

void UMCore_SettingsWidget_Slider::ApplyTheme_Implementation(UMCore_PDA_UITheme_Base* NewTheme)
{
	Super::ApplyTheme_Implementation(NewTheme);

	if (!NewTheme)
	{
		return;
	}

	UMCore_ThemeLibrary::ApplyTextStyleFromTheme(
		GetOwningLocalPlayer(), Txt_ValueDisplay, NewTheme->ValueTextStyle);
	
	if (Slider_Value && NewTheme->SliderStyle)
	{
		Slider_Value->SetWidgetStyle(
			UMCore_ThemeLibrary::BuildSliderStyle(
				NewTheme->SliderStyle, Slider_Value->GetWidgetStyle()));
	}
	
	const TSubclassOf<UCommonButtonStyle> StepButtonStyle =
		UMCore_ThemeLibrary::ResolveButtonStyle(
			NewTheme->GhostButtonStyle, NewTheme->SecondaryButtonStyle);
	
	if (StepButtonStyle)
	{
		Btn_StepLeft->SetButtonStyleOverride(StepButtonStyle);
		Btn_StepRight->SetButtonStyleOverride(StepButtonStyle);
	}
}
