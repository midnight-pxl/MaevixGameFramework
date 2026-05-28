// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CoreUI/Widgets/Primitives/MCore_RowBase.h"
#include "MCore_KeyBindingRow.generated.h"

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
 * Inherits Txt_Label and HighlightBorder from UMCore_RowBase. WBPs may
 * optionally bind Txt_Label for the action display name; if unbound, the
 * action label silently no-ops.
 */
UCLASS(Abstract, Blueprintable, ClassGroup = "MaevixUI", meta = (DisableNativeTick))
class MAEVIXCORE_API UMCore_KeyBindingRow : public UMCore_RowBase
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

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UMCore_KeyBindingButton> Btn_KBM_Primary;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UMCore_KeyBindingButton> Btn_KBM_Secondary;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UMCore_KeyBindingButton> Btn_Gamepad_Primary;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UMCore_KeyBindingButton> Btn_Gamepad_Secondary;

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
