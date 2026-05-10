// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "MCore_KeyBindingRow.generated.h"

class UCommonTextBlock;
class UMCore_KeyBindingButton;
class UInputAction;
class APlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRowRebindCompleted);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRowCaptureStateChanged,
	UMCore_KeyBindingButton*, Button, bool, bCapturing);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRowRebindResult,
	bool, bSuccess, FText, ErrorMessage);

/**
 * Single action row with KBM and Gamepad binding slots.
 * Owns 4 KeyBindingButtons and routes capture state and rebind results
 * upward to the owning panel.
 *
 * Requires BindWidget: Txt_ActionName, Btn_KBM_Primary, Btn_KBM_Secondary,
 * Btn_Gamepad_Primary, Btn_Gamepad_Secondary.
 */
UCLASS(Abstract, Blueprintable, ClassGroup = "ModulusUI", meta = (DisableNativeTick))
class MODULUSCORE_API UMCore_KeyBindingRow : public UCommonUserWidget
{
	GENERATED_BODY()

public:

	// ====================================================================
	// PUBLIC API
	// ====================================================================

	/** Configure this row for a specific action. Sets display name and inits all buttons. */
	UFUNCTION(BlueprintCallable, Category = "UI|KeyBinding")
	void InitFromAction(APlayerController* OwningPlayer, UInputAction* Action, bool bShowSecondary);

	/** Re-read current bindings and update all 4 buttons. */
	UFUNCTION(BlueprintCallable, Category = "UI|KeyBinding")
	void RefreshDisplay();

	UFUNCTION(BlueprintPure, Category = "UI|KeyBinding")
	UInputAction* GetInputAction() const { return BoundAction; }

	/** Fires after any button completes a rebind attempt. Panel binds to this for cross-row refresh. */
	UPROPERTY(BlueprintAssignable, Category = "UI|KeyBinding")
	FOnRowRebindCompleted OnRowRebindCompleted;

	/** Fires when a button enters or exits capture mode. Panel binds for centralized prompt display. */
	UPROPERTY(BlueprintAssignable, Category = "UI|KeyBinding")
	FOnRowCaptureStateChanged OnRowCaptureStateChanged;

	/** Fires after a rebind attempt with success/failure status. Panel binds for centralized error display. */
	UPROPERTY(BlueprintAssignable, Category = "UI|KeyBinding")
	FOnRowRebindResult OnRowRebindResult;

protected:

	// ====================================================================
	// BIND WIDGETS
	// ====================================================================

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Txt_ActionName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UMCore_KeyBindingButton> Btn_KBM_Primary;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UMCore_KeyBindingButton> Btn_KBM_Secondary;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UMCore_KeyBindingButton> Btn_Gamepad_Primary;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UMCore_KeyBindingButton> Btn_Gamepad_Secondary;

	/** Optional. When supplied by the WBP, this widget is shown while the row is hovered or in the focus path, hidden otherwise. Set HitTestInvisible to avoid stealing hover events. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> Highlight;

	// ====================================================================
	// LIFECYCLE
	// ====================================================================

	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	//~ Begin UUserWidget interface
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;
	//~ End UUserWidget interface

	/** Updates Highlight visibility based on current hover and focus-path state. Safe to call when Highlight is unbound. */
	void UpdateHighlightState();

	bool bIsRowMouseOver = false;
	bool bIsRowInFocusPath = false;

private:

	// ====================================================================
	// HANDLERS
	// ====================================================================

	UFUNCTION()
	void HandleRebindComplete(bool bSuccess, FText ErrorMessage);

	UFUNCTION()
	void HandleCaptureStateChanged(UMCore_KeyBindingButton* Button, bool bCapturing);

	// ====================================================================
	// STATE
	// ====================================================================

	TWeakObjectPtr<APlayerController> PlayerRef;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> BoundAction;
};
