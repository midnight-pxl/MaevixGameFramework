// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CoreUI/Widgets/Primitives/MCore_ActivatableBase.h"
#include "MCore_SettingsPanel.generated.h"

class UMCore_TabbedContainer;
class UMCore_ButtonBase;
class UMCore_ActionButton;
class UMCore_ConfirmationDialog;
class UMCore_SettingsRevertCountdown;
class UMCore_DA_SettingDefinition;
class UMCore_SettingsWidget_Base;
class UMCore_PDA_UITheme_Base;
class UCommonTextBlock;
class UScrollBox;

/**
 * Data-driven settings panel for auto-generating category tabs
 * and setting widgets from SettingsCollections in
 * UMCore_CoreSettings
 * 
 * Depth-3 tags become main tabs (Settings_Category_Video, etc.)
 * If Depth-4 tags >1 sub-tabbing occurs (Settings_Category_Video_Display, Settings_Category_Video_GraphicQuality, etc.)
 * KeyBinding category creates an inline KeyBindingPanel_Base widget instead of setting rows.
 */
UCLASS()
class MAEVIXCORE_API UMCore_SettingsPanel : public UMCore_ActivatableBase
{
	GENERATED_BODY()
	
public:
 
	/** Re-reads current values from the settings library and updates all setting widgets. */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|Settings")
	void RefreshAllWidgets();
 
protected:
 
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeDestruct() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	virtual void ApplyTheme_Implementation(UMCore_PDA_UITheme_Base* NewTheme) override;
 
	// ============================================================================
	// CONFIGURATION
	// ============================================================================
 
	/** Blueprint class spawned at runtime for nested sub-tab containers. */
	UPROPERTY(EditDefaultsOnly, Category = "MaevixCore|Settings|Panel")
	TSubclassOf<UMCore_TabbedContainer> SubTabContainerClass;

	/**
	 * Confirmation dialog class for reset operations. falls back to
	 * DefaultConfirmationDialogClass in CoreSettings if null
	 */
	UPROPERTY(EditDefaultsOnly, Category = "MaevixCore|Settings|Panel")
	TSubclassOf<UMCore_ConfirmationDialog> ResetConfirmationDialogClass;

	// ============================================================================
	// INPUT ACTIONS
	// ============================================================================

	UPROPERTY(EditDefaultsOnly, Category = "MaevixCore|Settings|Panel|Input Actions")
	FDataTableRowHandle TabNextAction;

	UPROPERTY(EditDefaultsOnly, Category = "MaevixCore|Settings|Panel|Input Actions")
	FDataTableRowHandle TabPrevAction;

	UPROPERTY(EditDefaultsOnly, Category = "MaevixCore|Settings|Panel|Input Actions")
	FDataTableRowHandle SubTabNextAction;

	UPROPERTY(EditDefaultsOnly, Category = "MaevixCore|Settings|Panel|Input Actions")
	FDataTableRowHandle SubTabPrevAction;

	UPROPERTY(EditDefaultsOnly, Category = "MaevixCore|Settings|Panel|Input Actions")
	FDataTableRowHandle RevertSettingsAction;

	UPROPERTY(EditDefaultsOnly, Category = "MaevixCore|Settings|Panel|Input Actions")
	FDataTableRowHandle ResetDefaultsAction;

	UPROPERTY(EditDefaultsOnly, Category = "MaevixCore|Settings|Panel|Input Actions")
	FDataTableRowHandle BackAction;

	// ============================================================================
	// BIND WIDGETS
	// ============================================================================
 
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UMCore_TabbedContainer> TabbedContainer_Main;
 
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Txt_SettingDescription;
 
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UMCore_ButtonBase> Btn_ResetAll;
 
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UMCore_ButtonBase> Btn_ResetCategory;
 
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UMCore_ButtonBase> Btn_Back;

	/** Optional action icon for revert-category input. Auto-resolves platform glyph. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UMCore_ActionButton> Btn_ActionRevert;

	/** Optional action icon for reset-all input. Auto-resolves platform glyph. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UMCore_ActionButton> Btn_ActionReset;

	/** Optional action icon for back input. Auto-resolves platform glyph. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UMCore_ActionButton> Btn_ActionBack;

	// ============================================================================
	// BLUEPRINT EXTENSION POINTS
	// ============================================================================
	
	/** Called after each category page is built and populated with setting widgets */
	UFUNCTION(BlueprintNativeEvent, Category = "MaevixCore|Settings")
	void OnCategoryPageCreated(const FGameplayTag& CategoryTag,
		UScrollBox* PageScrollBox);
	
	/** Called after the full panel build completes. All tabs + pages ready */
	UFUNCTION(BlueprintNativeEvent, Category = "MaevixCore|Settings")
	void OnPanelBuildComplete();
 
private:
 
	// ============================================================================
	// PANEL BUILD
	// ============================================================================
 
	void BuildPanel();

	UMCore_TabbedContainer* BuildTabbedPage(
		const FGameplayTag& ParentTag, const TArray<FGameplayTag>& ChildTags);

	void PopulatePage(UScrollBox* ScrollBox, const FGameplayTag& CategoryTag);
 
	UMCore_SettingsWidget_Base* CreateSettingWidget(const UMCore_DA_SettingDefinition* Definition);
 
	// ============================================================================
	// TAB CALLBACKS
	// ============================================================================
 
	UFUNCTION()
	void HandleMainTabSelected(FName TabID);
 
	UFUNCTION()
	void HandleSubTabSelected(FName TabID);
 
	void FocusFirstWidgetInActivePage();
 
	// ============================================================================
	// INPUT ACTION HANDLERS
	// ============================================================================

	UFUNCTION()
	void HandleTabNextInput(FName ActionName);

	UFUNCTION()
	void HandleTabPrevInput(FName ActionName);

	UFUNCTION()
	void HandleSubTabNextInput(FName ActionName);

	UFUNCTION()
	void HandleSubTabPrevInput(FName ActionName);

	UFUNCTION()
	void HandleResetCategoryInput(FName ActionName);

	UFUNCTION()
	void HandleResetAllInput(FName ActionName);

	// ============================================================================
	// ACTION BAR
	// ============================================================================

	UFUNCTION()
	void HandleResetAllClicked();

	UFUNCTION()
	void HandleResetAllConfirmResult(bool bConfirmed);

	UFUNCTION()
	void HandleResetCategoryClicked();

	UFUNCTION()
	void HandleResetCategoryConfirmResult(bool bConfirmed);

	UFUNCTION()
	void HandleBackClicked();
 
	// ============================================================================
	// SETTING FOCUS
	// ============================================================================

	UFUNCTION()
	void HandleSettingFocused(FGameplayTag SettingTag, FText Description);
 
	// ============================================================================
	// INTERNAL STATE
	// ============================================================================
 
	/**
	 * Maps single-page tab IDs and sub-tab IDs to their depth-4 leaf category tag.
	 * Sub-tabbed main tab IDs are NOT in this map,
	 * query their nested container for Parent instead.
	 */
	TMap<FName, FGameplayTag> TabIDToLeafTag;

	/** Maps main tab IDs to their sub-tab container (only for multi-subcategory tabs). */
	UPROPERTY()
	TMap<FName, TObjectPtr<UMCore_TabbedContainer>> MainTabToSubContainer;
 
	FGameplayTag ActiveLeafCategory;
 
	UPROPERTY()
	TArray<TObjectPtr<UMCore_SettingsWidget_Base>> AllSettingWidgets;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UCommonTextBlock>> SpawnedSectionHeaders;

	UPROPERTY()
	TArray<TObjectPtr<UMCore_TabbedContainer>> SubTabContainers;

	TWeakObjectPtr<UMCore_ConfirmationDialog> PendingConfirmationDialog;

	TWeakObjectPtr<UMCore_SettingsRevertCountdown> ActiveRevertCountdown;
	
	/** Tags accumulated during the debounce window. Merged into the countdown when it spawns. */
	TArray<FGameplayTag> PendingConfirmationTags;
	
	/** Debounce timer; resets on each new confirmation-required change. */
	FTimerHandle ConfirmationDebounceTimer;
	
	/** Spawns the countdown modal with all accumulated PendingConfirmationTags. */
	void SpawnRevertCountdown();

	void HandleConfirmationRequired(const TArray<FGameplayTag>& AffectedTags);

	UFUNCTION()
	void HandleCountdownResult(bool bConfirmed);

	bool bNeedsFullRebuild{false};
};
