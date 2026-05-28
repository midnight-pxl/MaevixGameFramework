// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CoreUI/Widgets/Settings/MCore_SettingsWidget_Base.h"
#include "MCore_SettingsWidget_Switcher.generated.h"

class UMCore_ButtonBase;

/**
 * Inline option cycling widget for Toggle and Dropdown setting types with
 * immediate-apply semantics: every cycle writes to the settings system immediately.
 * Toggle mode uses hardcoded Off/On options; Dropdown copies labels from the definition.
 *
 * Requires BindWidget: Btn_Previous, Btn_Next (UMCore_ButtonBase),
 * Txt_CurrentOption (UCommonTextBlock).
 */
UCLASS(Abstract, Blueprintable, ClassGroup = "MaevixUI", meta = (DisableNativeTick))
class MAEVIXCORE_API UMCore_SettingsWidget_Switcher : public UMCore_SettingsWidget_Base
{
	GENERATED_BODY()

protected:
	// ====================================================================
	// BIND WIDGETS
	// ====================================================================

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UMCore_ButtonBase> Btn_Previous;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Txt_CurrentOption;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UMCore_ButtonBase> Btn_Next;

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

	/** Returns the index that should currently be displayed.
	 *  Default reads the setting's save-key int via GameSettingsLibrary.
	 *  Override for widgets where display state diverges from save (e.g. QualityPreset Custom). */
	UFUNCTION(BlueprintNativeEvent, Category = "MaevixCore|Settings")
	int32 ResolveDisplayedIndex();
	virtual int32 ResolveDisplayedIndex_Implementation();

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
	void HandlePreviousClicked();

	UFUNCTION()
	void HandleNextClicked();

	// ====================================================================
	// HELPERS
	// ====================================================================

	void CycleOption(int32 Direction);
	void UpdateDisplay();
	void ApplyCurrentValue();
	void ReadCurrentValue();

	// ====================================================================
	// STATE
	// ====================================================================

	TArray<FText> Options;
	int32 CurrentIndex{0};
	bool bIsToggleMode{false};
};
