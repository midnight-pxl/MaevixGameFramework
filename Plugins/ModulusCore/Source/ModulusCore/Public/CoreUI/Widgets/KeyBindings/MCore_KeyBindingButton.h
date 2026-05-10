// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "MCore_KeyBindingButton.generated.h"

class UMCore_ButtonBase;
class UInputAction;
class APlayerController;
class ULocalPlayer;
class IInputProcessor;
class FMCore_KeyCaptureProcessor;

UENUM(BlueprintType)
enum class EMCore_KeyBindingButtonState : uint8
{
	Bound,
	Unbound,
	Capturing
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRebindComplete, bool, bSuccess, FText, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCaptureStateChanged, UMCore_KeyBindingButton*, Button, bool, bCapturing);

/**
 * Single key binding slot using ButtonBase as the display surface.
 * Uses a Slate IInputProcessor to intercept keys before CommonUI's action router
 * can consume them during capture mode.
 *
 * Requires BindWidget: Btn_Capture.
 */
UCLASS(Abstract, Blueprintable, ClassGroup = "ModulusUI", meta = (DisableNativeTick))
class MODULUSCORE_API UMCore_KeyBindingButton : public UCommonUserWidget
{
	GENERATED_BODY()

	friend class FMCore_KeyCaptureProcessor;

public:
	// ====================================================================
	// PUBLIC API
	// ====================================================================

	/** Configure this button for a specific action, slot, and device type. */
	UFUNCTION(BlueprintCallable, Category = "UI|KeyBinding")
	void InitForSlot(APlayerController* OwningPlayer, UInputAction* Action,
		EPlayerMappableKeySlot InSlot, bool bInIsGamepad);

	/** Re-read the current binding and update visuals via ButtonBase. */
	UFUNCTION(BlueprintCallable, Category = "UI|KeyBinding")
	void RefreshKeyDisplay();

	/** Begin listening for a key press. Registers the input preprocessor. */
	UFUNCTION(BlueprintCallable, Category = "UI|KeyBinding")
	void EnterCaptureMode();

	/** Stop listening and restore display state. */
	UFUNCTION(BlueprintCallable, Category = "UI|KeyBinding")
	void ExitCaptureMode();

	UFUNCTION(BlueprintPure, Category = "UI|KeyBinding")
	bool IsGamepadSlot() const { return bIsGamepad; }

	UFUNCTION(BlueprintPure, Category = "UI|KeyBinding")
	UInputAction* GetInputAction() const { return BoundAction; }

	UFUNCTION(BlueprintPure, Category = "UI|KeyBinding")
	EPlayerMappableKeySlot GetSlot() const { return Slot; }

	UPROPERTY(BlueprintAssignable, Category = "UI|KeyBinding")
	FOnRebindComplete OnRebindComplete;

	UPROPERTY(BlueprintAssignable, Category = "UI|KeyBinding")
	FOnCaptureStateChanged OnCaptureStateChanged;

	UPROPERTY(BlueprintReadOnly, Category = "UI|KeyBinding")
	EMCore_KeyBindingButtonState CurrentState = EMCore_KeyBindingButtonState::Unbound;

protected:
	// ====================================================================
	// BIND WIDGETS
	// ====================================================================

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UMCore_ButtonBase> Btn_Capture;

	// ====================================================================
	// CONFIGURATION
	// ====================================================================

	/** Text shown when no key is bound for this slot. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|KeyBinding")
	FText UnboundText;

	// ====================================================================
	// HANDLERS
	// ====================================================================

	UFUNCTION()
	void HandleButtonPressed();
	void AttemptRebind(FKey NewKey);

	// ====================================================================
	// INPUT PROCESSOR CALLBACKS
	// ====================================================================

	void OnKeyCaptured(FKey NewKey);
	void OnCaptureCancelled();

	// ====================================================================
	// LIFECYCLE
	// ====================================================================

	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

private:
	// ====================================================================
	// STATE
	// ====================================================================

	TWeakObjectPtr<APlayerController> PlayerRef;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> BoundAction;

	EPlayerMappableKeySlot Slot = EPlayerMappableKeySlot::First;
	bool bIsGamepad{false};

	TSharedPtr<IInputProcessor> KeyCaptureProcessor;
};
