// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CoreUI/Widgets/Primitives/MCore_ActivatableBase.h"
#include "CommonInputTypeEnum.h"
#include "MCore_PressToContinueWidget.generated.h"

class UCommonTextBlock;
class UImage;
class UWidgetSwitcher;
class UCommonInputSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPressToContinueAccepted);

UENUM(BlueprintType)
enum class EMCore_PressToContinueStartState : uint8
{
	Ready   UMETA(DisplayName="Start Ready",
	              ToolTip="Skip the loading state. Input is accepted after the InputDelaySeconds gate."),
	Loading UMETA(DisplayName="Start Loading",
	              ToolTip="Show loading state until SetReadyToContinue() is called from game code.")
};

/**
 * Gate widget shown before the main menu UI. Captures any keyboard, mouse,
 * gamepad, or touch input once in the Ready state, broadcasts OnAccepted,
 * and deactivates. Pair with vanilla MoviePlayer StartupMovies for the full
 * "splash -> gate -> menu" flow.
 *
 * Two visual states via WS_Content (BindWidgetOptional UWidgetSwitcher):
 *   Layer 0 = Loading (throbber)
 *   Layer 1 = Ready (prompt text + dismissal icon)
 *
 * StartState picks which layer is shown on activation. SetReadyToContinue()
 * advances Loading -> Ready and fires the K2_OnReadyToContinue Blueprint event
 * for WBP-side animation triggers (e.g. pulsing prompt).
 *
 * Optional BindWidget slots: WS_Content, Txt_Prompt, Icon_DismissAction, Img_Background.
 */
UCLASS(Abstract, Blueprintable, ClassGroup="MaevixUI", meta=(DisableNativeTick))
class MAEVIXCORE_API UMCore_PressToContinueWidget : public UMCore_ActivatableBase
{
	GENERATED_BODY()

public:
	UMCore_PressToContinueWidget(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable, Category="MaevixCore|UI")
	FOnPressToContinueAccepted OnAccepted;

	/* Advance from Loading to Ready. No-op if already Ready. Fires K2_OnReadyToContinue. */
	UFUNCTION(BlueprintCallable, Category="MaevixCore|UI")
	void SetReadyToContinue();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="MaevixCore|UI")
	bool IsReadyToContinue() const { return bReadyToContinue; }

	UFUNCTION(BlueprintCallable, Category="MaevixCore|UI")
	void SetPromptText(const FText& InText);

protected:
	/* Switcher layers must be ordered: 0 = Loading (throbber), 1 = Ready (prompt). */
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<UWidgetSwitcher> WS_Content;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> Txt_Prompt;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<UImage> Icon_DismissAction;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<UImage> Img_Background;

	/* Initial visual state on activation. Default Ready preserves the simple "press any key" case. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MaevixCore|UI|PressToContinue")
	EMCore_PressToContinueStartState StartState{EMCore_PressToContinueStartState::Ready};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MaevixCore|UI|PressToContinue")
	FText PromptText{NSLOCTEXT("MaevixCore", "PressAnyKeyToContinue", "Press any key to continue")};

	/*
	 * Anti-bounce window measured from the Loading->Ready transition (or from activation
	 * if StartState=Ready). Prevents accidental dismiss from key holdovers during the swap.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MaevixCore|UI|PressToContinue",
		meta=(ClampMin="0.0", ClampMax="2.0"))
	float InputDelaySeconds{0.5f};

	/* When false, only keyboard/gamepad/touch dismiss. Mouse clicks are ignored. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MaevixCore|UI|PressToContinue")
	bool bAcceptMouseInput{true};

	/* Fired when the widget enters the Loading state on activation. WBP-side animation hook. */
	UFUNCTION(BlueprintImplementableEvent, Category="MaevixCore|UI|PressToContinue",
		meta=(DisplayName="On Enter Loading State"))
	void K2_OnEnterLoadingState();

	/* Fired when the widget enters the Ready state. WBP-side animation hook (e.g. pulsing prompt). */
	UFUNCTION(BlueprintImplementableEvent, Category="MaevixCore|UI|PressToContinue",
		meta=(DisplayName="On Ready To Continue"))
	void K2_OnReadyToContinue();

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;

	virtual UWidget* NativeGetDesiredFocusTarget() const override;

private:
	void EnterLoadingState();
	void HandleAccepted();
	void RefreshDismissalIcon();

	void HandleInputMethodChanged(ECommonInputType NewInputType);

	bool bReadyToContinue{false};
	bool bAccepted{false};
	double ReadyTimeSeconds{0.0};
	FDelegateHandle InputMethodChangedHandle;
};
