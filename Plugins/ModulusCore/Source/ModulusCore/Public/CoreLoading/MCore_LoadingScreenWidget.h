// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CommonInputTypeEnum.h"
#include "CoreData/Types/Loading/MCore_LoadingScreenTypes.h"
#include "MCore_LoadingScreenWidget.generated.h"

class UImage;
class UCommonTextBlock;
class UWidget;

/**
 * Base widget for level-transition loading screens. Hosted by UE's MoviePlayer
 * via TakeWidget() during PreLoadMap, so it renders as a Slate widget while the
 * game thread is blocked on the level swap.
 *
 * Derives from UUserWidget (not UMCore_ActivatableBase) because the CommonUI
 * activation pipeline does not run during PreLoadMap, so ActivatableBase's
 * lifecycle protections add no value here. The Blueprint subclass must be
 * cookable (referenced as TSoftClassPtr in MCore_CoreSettings).
 *
 * Optional bound widgets receive default content from Initialize(); add your
 * own logic via K2_OnLoadingScreenInitialized.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class MODULUSCORE_API UMCore_LoadingScreenWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// ============================================================================
	// BOUND WIDGETS
	// ============================================================================

	UPROPERTY(BlueprintReadOnly, Transient, meta=(BindWidgetOptional))
	TObjectPtr<UImage> Image_Background;

	UPROPERTY(BlueprintReadOnly, Transient, meta=(BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> Txt_Tip;

	/** Container for the dismiss prompt (icon + text). Toggled visible by OnLoadingComplete. */
	UPROPERTY(BlueprintReadOnly, Transient, meta=(BindWidgetOptional))
	TObjectPtr<UWidget> Prompt_Continue;

	UPROPERTY(BlueprintReadOnly, Transient, meta=(BindWidgetOptional))
	TObjectPtr<UImage> Icon_DismissAction;

	UPROPERTY(BlueprintReadOnly, Transient, meta=(BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> Txt_DismissPrompt;

public:
	// ============================================================================
	// INITIALIZATION
	// ============================================================================

	/**
	 * Applies entry data to optional bound widgets and dispatches the Blueprint event.
	 * Called by UMCore_LoadingScreenSubsystem before the widget is handed to MoviePlayer.
	 */
	UFUNCTION(BlueprintCallable, Category="Modulus|Loading")
	void Initialize(const FMCore_LoadingScreenEntry& Entry);

	/** Blueprint hook fired after Initialize applies the default content. */
	UFUNCTION(BlueprintImplementableEvent, Category="Modulus|Loading",
		meta=(DisplayName="On Loading Screen Initialized"))
	void K2_OnLoadingScreenInitialized(const FMCore_LoadingScreenEntry& Entry);

	// ============================================================================
	// DISMISSAL LIFECYCLE
	// ============================================================================

	/**
	 * Called by UMCore_LoadingScreenSubsystem when PostLoadMapWithWorld fires
	 * and dismissal mode is InputRequired. Sets prompt visibility, refreshes
	 * the input icon, claims keyboard focus, and enables native input handling.
	 */
	UFUNCTION(BlueprintCallable, Category="Modulus|Loading")
	void HandleLoadingComplete();

	/**
	 * Default content placement after load completes. Override to customize
	 * prompt visibility, icon refresh, or focus behavior.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Modulus|Loading",
		meta=(DisplayName="On Loading Complete"))
	void OnLoadingComplete();

	/**
	 * Fired when the player provides dismissal input. Default implementation calls
	 * Subsystem->DismissLoadingScreen(). Override (BP or C++) for server-authoritative
	 * ready barriers: send your RPC, call SetAwaitingDismissalInput(false), update
	 * prompt via SetPromptText, and do NOT call DismissLoadingScreen here. Your
	 * client RPC handler should call Subsystem->DismissLoadingScreen() when the
	 * server signals dismissal.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Modulus|Loading",
		meta=(DisplayName="On Dismissal Input Received"))
	void OnDismissalInputReceived();

	// ============================================================================
	// PROMPT HELPERS
	// ============================================================================

	UFUNCTION(BlueprintCallable, Category="Modulus|Loading")
	void SetPromptText(const FText& InText);

	UFUNCTION(BlueprintCallable, Category="Modulus|Loading")
	void SetPromptIconVisible(bool bVisible);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Modulus|Loading")
	bool IsAwaitingDismissalInput() const { return bAwaitingInput; }

	UFUNCTION(BlueprintCallable, Category="Modulus|Loading")
	void SetAwaitingDismissalInput(bool bAwaiting);

protected:
	// ============================================================================
	// USERWIDGET OVERRIDES
	// ============================================================================

	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;

private:
	// ============================================================================
	// INTERNAL
	// ============================================================================

	void RefreshDismissalIcon();
	void HandleInputMethodChanged(ECommonInputType NewInputType);

	bool bAwaitingInput{false};
	FDelegateHandle InputMethodChangedHandle;
};
