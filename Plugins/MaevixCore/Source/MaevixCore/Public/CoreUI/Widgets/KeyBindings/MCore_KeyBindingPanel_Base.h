// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "MCore_KeyBindingPanel_Base.generated.h"

class UScrollBox;
class UMCore_TabbedContainer;
class UMCore_ButtonBase;
class UMCore_KeyBindingRow;
class UMCore_KeyBindingButton;
class UMCore_KeyBindingCaptureDialog;
class UMCore_ConfirmationDialog;
class UMCore_PDA_UITheme_Base;
class UCommonTextBlock;
class UInputAction;
class UInputMappingContext;
class APlayerController;

/**
 * Key binding panel that populates a TabbedContainer with one tab per InputMappingContext.
 * Each tab contains category headers and binding rows in a ScrollBox.
 * Falls back to a single flat page if no KeyBindingContexts are configured in CoreSettings.
 * Designed to be hosted inline in the Settings Panel's KeyBinding category tab.
 *
 * Requires BindWidget: TabbedContainer_Contexts.
 */
UCLASS(Abstract, Blueprintable)
class MAEVIXCORE_API UMCore_KeyBindingPanel_Base : public UCommonUserWidget
{
	GENERATED_BODY()

public:

	// ============================================================================
	// PUBLIC API
	// ============================================================================

	/** Query all remappable actions, group by IMC and category, and spawn rows into tabs. */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|UI|KeyBinding")
	void PopulateBindings(APlayerController* OwningPlayer);

	/** Refresh all spawned rows to reflect current bindings. */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|UI|KeyBinding")
	void RefreshAllRows();

	// ============================================================================
	// BLUEPRINT HOOKS
	// ============================================================================

	/** Fires after each IMC tab is created and registered in the TabbedContainer. */
	UFUNCTION(BlueprintNativeEvent, Category = "MaevixCore|UI|KeyBinding")
	void OnContextTabCreated(FName TabID, UWidget* PageWidget, const FText& ContextDisplayName);
	virtual void OnContextTabCreated_Implementation(FName TabID, UWidget* PageWidget, const FText& ContextDisplayName) {}

	/** Fires after each category header is added to a tab's ScrollBox. */
	UFUNCTION(BlueprintNativeEvent, Category = "MaevixCore|UI|KeyBinding")
	void OnCategoryHeaderCreated(const FText& CategoryDisplayName, UWidget* HeaderWidget);
	virtual void OnCategoryHeaderCreated_Implementation(const FText& CategoryDisplayName, UWidget* HeaderWidget) {}

	/** Fires after each row is created and added to a tab's ScrollBox. */
	UFUNCTION(BlueprintNativeEvent, Category = "MaevixCore|UI|KeyBinding")
	void OnRowCreated(UMCore_KeyBindingRow* Row);
	virtual void OnRowCreated_Implementation(UMCore_KeyBindingRow* Row) {}

protected:

	// ============================================================================
	// BIND WIDGETS
	// ============================================================================

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UMCore_TabbedContainer> TabbedContainer_Bindings;

	/** Optional. When bound, drives the reset-all-bindings confirmation flow. Absent = no reset-all UI. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UMCore_ButtonBase> Btn_ResetAllBindings;

	/**
	 * Optional. When bound, drives the reset-category confirmation flow for the active IMC tab.
	 * Absent = no per-category reset UI.
	 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UMCore_ButtonBase> Btn_ResetCategory;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MaevixCore|UI|KeyBinding")
	TSubclassOf<UMCore_KeyBindingRow> KeyBindingRowClass;

	/** Dialog class pushed to modal layer during key capture. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MaevixCore|UI|KeyBinding")
	TSubclassOf<UMCore_KeyBindingCaptureDialog> CaptureDialogClass;

	/** Text shown during KBM key capture. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MaevixCore|UI|KeyBinding")
	FText CapturePromptText;

	/** Text shown during gamepad button capture. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MaevixCore|UI|KeyBinding")
	FText CapturePromptGamepadText;
	
	/**
	 * Confirmation dialog class for reset operations. Falls back to
	 * CoreSettings::DefaultConfirmationDialogClass if null.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MaevixCore|UI|KeyBinding")
	TSubclassOf<UMCore_ConfirmationDialog> ResetConfirmationDialogClass;

	// ============================================================================
	// THEME
	// ============================================================================

	/** Apply theme styling to this widget. Override for custom theme handling. */
	UFUNCTION(BlueprintNativeEvent, Category = "MaevixCore|Theme")
	void ApplyTheme(UMCore_PDA_UITheme_Base* NewTheme);

	UFUNCTION(BlueprintImplementableEvent, Category = "MaevixCore|Theme",
		meta = (DisplayName = "On Theme Applied"))
	void K2_OnThemeApplied(UMCore_PDA_UITheme_Base* Theme);

	UPROPERTY(Transient)
	mutable TWeakObjectPtr<UMCore_PDA_UITheme_Base> CachedTheme;

	// ============================================================================
	// LIFECYCLE
	// ============================================================================

	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

private:

	// ============================================================================
	// PANEL BUILD
	// ============================================================================

	UScrollBox* BuildContextPage(APlayerController* OwningPlayer, const UInputMappingContext* MappingContext);
	UScrollBox* BuildFallbackPage(APlayerController* OwningPlayer);
	UCommonTextBlock* CreateThemedCategoryHeader(const FText& CategoryDisplayName);

	// ============================================================================
	// TAB CALLBACKS
	// ============================================================================

	UFUNCTION()
	void HandleContextTabSelected(FName TabID);

	// ============================================================================
	// RESET HANDLERS
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
	void HandleRowRebindCompleted();

	UFUNCTION()
	void HandleRowCaptureStateChanged(UMCore_KeyBindingButton* Button, bool bCapturing);

	UFUNCTION()
	void HandleRowRebindResult(bool bSuccess, FText ErrorMessage);

	UFUNCTION()
	void HandleCaptureDialogDismissed();

	UFUNCTION()
	void HandleCaptureDialogReadyForCapture();

	// ============================================================================
	// THEME INTERNALS
	// ============================================================================

	void BindThemeDelegate();
	void UnbindThemeDelegate();

	UFUNCTION()
	void HandleThemeChanged(UMCore_PDA_UITheme_Base* NewTheme);

	bool bThemeDelegateBound{false};

	// ============================================================================
	// DIALOG HELPERS
	// ============================================================================

	void DismissActiveCaptureDialog();
	void DismissActiveConfirmationDialog();

	// ============================================================================
	// STATE
	// ============================================================================

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMCore_KeyBindingRow>> AllRows;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UCommonTextBlock>> SpawnedHeaders;

	TWeakObjectPtr<UMCore_ConfirmationDialog> PendingConfirmationDialog;

	TWeakObjectPtr<UMCore_KeyBindingCaptureDialog> ActiveCaptureDialog;

	TWeakObjectPtr<UMCore_KeyBindingButton> ActiveCaptureButton;

	FName ActiveContextTabID;
};
