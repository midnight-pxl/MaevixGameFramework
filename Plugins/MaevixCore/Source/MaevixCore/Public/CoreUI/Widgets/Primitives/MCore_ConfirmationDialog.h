// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CoreUI/Widgets/Primitives/MCore_ActivatableBase.h"
#include "MCore_ConfirmationDialog.generated.h"

class UCommonTextBlock;
class UMCore_ButtonBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogResult, bool, bConfirmed);

/**
 * Reusable confirmation dialog with Accept/Back input binding support.
 * Auto-wires button delegates and input bindings on activation, defaults focus to Cancel.
 * Broadcasts OnDialogResult and auto-deactivates on resolution.
 *
 * Requires BindWidget: Txt_DialogMessage, Btn_Confirm, Btn_Cancel.
 */
UCLASS(Abstract, Blueprintable, ClassGroup="MaevixUI", meta=(DisableNativeTick))
class MAEVIXCORE_API UMCore_ConfirmationDialog : public UMCore_ActivatableBase
{
	GENERATED_BODY()

public:
	UMCore_ConfirmationDialog(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "UI|Dialog")
	void SetDialogMessage(FText InMessage);

	/** Override confirm and cancel button labels at runtime. */
	UFUNCTION(BlueprintCallable, Category = "UI|Dialog")
	void SetButtonLabels(FText InConfirmText, FText InCancelText);

	/** Fires when dialog resolves. True if confirmed, false if cancelled. */
	UPROPERTY(BlueprintAssignable, Category = "UI|Dialog")
	FOnDialogResult OnDialogResult;

protected:
	// ============================================================================
	// LIFECYCLE
	// ============================================================================

	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeDestruct() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	UPROPERTY(EditDefaultsOnly, Category = "UI|Dialog|Input")
	FDataTableRowHandle AcceptInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Dialog|Input")
	FDataTableRowHandle BackInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Dialog")
	FText ConfirmText;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Dialog")
	FText CancelText;

	// ============================================================================
	// WIDGET BINDINGS
	// ============================================================================

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Txt_DialogMessage;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	TObjectPtr<UMCore_ButtonBase> Btn_Confirm;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	TObjectPtr<UMCore_ButtonBase> Btn_Cancel;

private:
	void ResolveDialog(bool bConfirmed);

	UFUNCTION()
	void HandleConfirmClicked();

	UFUNCTION()
	void HandleCancelClicked();

	UFUNCTION()
	void HandleCancelInput(FName ActionName);

	FMCore_InputActionBindingHandle BackBindingHandle;

	bool bResolved{false};
};
