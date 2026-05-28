// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CoreUI/Widgets/Settings/MCore_SettingsWidget_Base.h"
#include "MCore_SettingsWidget_Slider.generated.h"

class USlider;
class UCommonTextBlock;
class UMCore_ButtonBase;

/**
 * Settings widget for float/slider-type settings with immediate-apply semantics.
 * Reads MinValue, MaxValue, StepSize, and display format from the bound definition.
 *
 * Requires BindWidget: Slider_Value (USlider), Txt_ValueDisplay (UCommonTextBlock),
 * Btn_StepLeft and Btn_StepRight (UMCore_ButtonBase) for gamepad navigation.
 */
UCLASS(Abstract, Blueprintable, ClassGroup = "MaevixUI", meta = (DisableNativeTick))
class MAEVIXCORE_API UMCore_SettingsWidget_Slider : public UMCore_SettingsWidget_Base
{
	GENERATED_BODY()

protected:
	// ====================================================================
	// BIND WIDGETS
	// ====================================================================

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USlider> Slider_Value;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Txt_ValueDisplay;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UMCore_ButtonBase> Btn_StepLeft;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UMCore_ButtonBase> Btn_StepRight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MaevixCore|Settings|Widget")
	bool bShowStepButtons{true};
	
protected:
	// ====================================================================
	// OVERRIDES
	// ====================================================================

	virtual void OnDefinitionSet_Implementation(const UMCore_DA_SettingDefinition* Definition) override;
	virtual void ApplyTheme_Implementation(UMCore_PDA_UITheme_Base* NewTheme) override;
	virtual FString GetValueAsString_Implementation() const override;
	virtual void ResetToDefault_Implementation() override;
	virtual void RefreshValueFromSettings_Implementation() override;
	virtual void StepLeft_Implementation() override;
	virtual void StepRight_Implementation() override;

	// ====================================================================
	// LIFECYCLE
	// ====================================================================

	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

private:
	// ====================================================================
	// HANDLERS
	// ====================================================================

	UFUNCTION()
	void HandleSliderValueChanged(float RawValue);

	UFUNCTION()
	void HandleStepLeft();

	UFUNCTION()
	void HandleStepRight();

	// ====================================================================
	// HELPERS
	// ====================================================================

	void StepByAmount(float Delta);
	float SnapToStep(float RawValue) const;
	FText FormatValue(float Value) const;
	void ApplyValueToEngine(float SnappedValue);
	void SyncSliderAndDisplay(float SnappedValue);

	// ====================================================================
	// STATE
	// ====================================================================

	/* Race condition guard; prevents HandleSliderValueChanged from firing during programmatic sets */
	bool bIsUpdatingSlider{false};
};
