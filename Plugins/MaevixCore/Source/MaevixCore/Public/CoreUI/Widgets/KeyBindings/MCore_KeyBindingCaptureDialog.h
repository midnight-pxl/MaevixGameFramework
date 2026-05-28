// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CoreUI/Widgets/Primitives/MCore_ActivatableBase.h"
#include "MCore_KeyBindingCaptureDialog.generated.h"

class UCommonTextBlock;
class UMCore_ButtonBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCaptureDialogDismissed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCaptureDialogReadyForCapture);

/**
 * Modal capture dialog pushed to the UI layer stack during key binding capture.
 * Displays the action name, slot context, and a prompt that alternates between
 * "Press any key..." and error messages with a cooldown before retry.
 *
 * Requires BindWidget: Txt_ActionName, Txt_SlotContext, Txt_PromptOrError, Btn_Cancel.
 */
UCLASS(Abstract, Blueprintable, ClassGroup = "MaevixUI", meta = (DisableNativeTick))
class MAEVIXCORE_API UMCore_KeyBindingCaptureDialog : public UMCore_ActivatableBase
{
	GENERATED_BODY()

public:
	UMCore_KeyBindingCaptureDialog(const FObjectInitializer& ObjectInitializer);

	// ====================================================================
	// PUBLIC API
	// ====================================================================

	/** Set the dialog display text for this capture session. */
	UFUNCTION(BlueprintCallable, Category = "UI|KeyBinding")
	void ConfigureForCapture(const FText& ActionName, const FText& SlotContext, const FText& PromptText);

	/** Show an error message in the prompt area. Starts the cooldown timer. */
	UFUNCTION(BlueprintCallable, Category = "UI|KeyBinding")
	void ShowCaptureError(const FText& ErrorMessage);

	// ====================================================================
	// DELEGATES
	// ====================================================================

	/** Fires when the user cancels (Cancel button or Back action). */
	UPROPERTY(BlueprintAssignable, Category = "UI|KeyBinding")
	FOnCaptureDialogDismissed OnDialogDismissed;

	/** Fires when the error cooldown expires and the dialog is ready for another capture attempt. */
	UPROPERTY(BlueprintAssignable, Category = "UI|KeyBinding")
	FOnCaptureDialogReadyForCapture OnReadyForCapture;

protected:

	// ====================================================================
	// BIND WIDGETS
	// ====================================================================

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Txt_ActionName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Txt_SlotContext;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Txt_PromptOrError;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UMCore_ButtonBase> Btn_Cancel;

	// ====================================================================
	// CONFIGURATION
	// ====================================================================

	/** How long error messages display before restoring the capture prompt. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|KeyBinding")
	float ErrorDisplayDuration{2.0f};

	/** Input action for back/cancel. Registered on activation, cleaned up on deactivation. */
	UPROPERTY(EditDefaultsOnly, Category = "UI|KeyBinding|Input")
	FDataTableRowHandle BackInputAction;

	// ====================================================================
	// LIFECYCLE
	// ====================================================================

	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeDestruct() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	// ====================================================================
	// THEME
	// ====================================================================

	virtual void ApplyTheme_Implementation(UMCore_PDA_UITheme_Base* NewTheme) override;

private:

	// ====================================================================
	// HANDLERS
	// ====================================================================

	UFUNCTION()
	void HandleCancelClicked();

	UFUNCTION()
	void HandleBackInput(FName ActionName);

	UFUNCTION()
	void HandleErrorCooldownExpired();

	void DismissDialog();

	// ====================================================================
	// STATE
	// ====================================================================

	FTimerHandle ErrorCooldownTimerHandle;
	FText CachedPromptText;
	FMCore_InputActionBindingHandle BackBindingHandle;
};
