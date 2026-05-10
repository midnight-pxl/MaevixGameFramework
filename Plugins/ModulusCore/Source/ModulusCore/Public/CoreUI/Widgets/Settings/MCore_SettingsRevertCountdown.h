// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CoreUI/Widgets/Primitives/MCore_ActivatableBase.h"
#include "GameplayTagContainer.h"
#include "MCore_SettingsRevertCountdown.generated.h"

class UCommonTextBlock;
class UMCore_ButtonBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountdownResult, bool, bConfirmed);

/**
 * Countdown modal for confirmation-required settings.
 * Caller pushes this to the Modal layer and calls StartCountdown with the affected
 * tags and their previous values. The widget handles confirm, revert, and auto-revert
 * on timeout via the existing batch setters with bBypassConfirmation=true.
 *
 * Requires BindWidget: Txt_Message, Txt_Countdown, Btn_Confirm, Btn_Revert.
 */
UCLASS(Abstract, Blueprintable, ClassGroup="ModulusUI", meta=(DisableNativeTick))
class MODULUSCORE_API UMCore_SettingsRevertCountdown : public UMCore_ActivatableBase
{
	GENERATED_BODY()

public:
	UMCore_SettingsRevertCountdown(const FObjectInitializer& ObjectInitializer);

	/**
	 * Begin the countdown. Stores affected settings and starts a 1-second looping timer.
	 * Duration is read from CoreSettings::ConfirmationRevertDelay.
	 */
	UFUNCTION(BlueprintCallable, Category = "UI|Settings")
	void StartCountdown(const TArray<FGameplayTag>& AffectedTags);

	/** Fires when the countdown resolves. True = user confirmed, false = reverted or timed out. */
	UPROPERTY(BlueprintAssignable, Category = "UI|Settings")
	FOnCountdownResult OnCountdownResult;

	/** Marks this countdown as resolved without firing the auto-revert fallback.
	 *  Call before DeactivateWidget() when replacing this countdown with a merged
	 *  successor inside the debounce window. */
	void SuppressFallbackForReplacement();

protected:
	// ============================================================================
	// LIFECYCLE
	// ============================================================================

	virtual void NativeOnInitialized() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeDestruct() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	// ============================================================================
	// WIDGET BINDINGS
	// ============================================================================

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Txt_Message;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Txt_Countdown;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	TObjectPtr<UMCore_ButtonBase> Btn_Confirm;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	TObjectPtr<UMCore_ButtonBase> Btn_Revert;

private:
	void ResolveCountdown(bool bConfirmed);

	/* Looks up each pending tag's definition and calls the appropriate batch setter */
	void ApplySettings(bool bConfirmed);

	UFUNCTION()
	void HandleConfirmClicked();

	UFUNCTION()
	void HandleRevertClicked();

	UFUNCTION()
	void HandleCountdownTick();

	TArray<FGameplayTag> PendingTags;
	FTimerHandle CountdownTimerHandle;
	float RemainingSeconds{0.f};
	float TotalSeconds{0.f};
	bool bResolved{false};
};
