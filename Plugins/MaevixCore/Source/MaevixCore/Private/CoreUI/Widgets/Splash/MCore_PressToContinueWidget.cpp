// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/Widgets/Splash/MCore_PressToContinueWidget.h"

#include "CoreData/Libraries/MCore_InputDisplayLibrary.h"

#include "CommonInputSubsystem.h"
#include "CommonTextBlock.h"
#include "Components/Image.h"
#include "Components/WidgetSwitcher.h"
#include "Engine/LocalPlayer.h"
#include "InputCoreTypes.h"

UMCore_PressToContinueWidget::UMCore_PressToContinueWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bShouldFocusOnActivation = true;
}

// ============================================================================
// LIFECYCLE
// ============================================================================

void UMCore_PressToContinueWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	bAccepted = false;

	if (Txt_Prompt)
	{
		Txt_Prompt->SetText(PromptText);
	}

	if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UCommonInputSubsystem* CIS = LocalPlayer->GetSubsystem<UCommonInputSubsystem>())
		{
			InputMethodChangedHandle = CIS->OnInputMethodChangedNative.AddUObject(
				this, &ThisClass::HandleInputMethodChanged);
		}
	}

	if (StartState == EMCore_PressToContinueStartState::Loading)
	{
		EnterLoadingState();
	}
	else
	{
		SetReadyToContinue();
	}
}

void UMCore_PressToContinueWidget::NativeOnDeactivated()
{
	if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UCommonInputSubsystem* CIS = LocalPlayer->GetSubsystem<UCommonInputSubsystem>())
		{
			CIS->OnInputMethodChangedNative.Remove(InputMethodChangedHandle);
		}
	}
	InputMethodChangedHandle.Reset();

	Super::NativeOnDeactivated();
}

// ============================================================================
// PUBLIC API
// ============================================================================

void UMCore_PressToContinueWidget::SetReadyToContinue()
{
	if (bReadyToContinue)
	{
		return;
	}

	bReadyToContinue = true;
	ReadyTimeSeconds = FPlatformTime::Seconds();

	if (WS_Content)
	{
		WS_Content->SetActiveWidgetIndex(1);
	}

	RefreshDismissalIcon();

	K2_OnReadyToContinue();
}

void UMCore_PressToContinueWidget::SetPromptText(const FText& InText)
{
	PromptText = InText;
	if (Txt_Prompt)
	{
		Txt_Prompt->SetText(InText);
	}
}

// ============================================================================
// INPUT
// ============================================================================

FReply UMCore_PressToContinueWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (bAccepted || !bReadyToContinue)
	{
		return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
	}

	if (FPlatformTime::Seconds() - ReadyTimeSeconds < InputDelaySeconds)
	{
		return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
	}

	HandleAccepted();
	return FReply::Handled();
}

FReply UMCore_PressToContinueWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bAcceptMouseInput || bAccepted || !bReadyToContinue)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	if (FPlatformTime::Seconds() - ReadyTimeSeconds < InputDelaySeconds)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	HandleAccepted();
	return FReply::Handled();
}

FReply UMCore_PressToContinueWidget::NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	if (bAccepted || !bReadyToContinue)
	{
		return Super::NativeOnTouchStarted(InGeometry, InGestureEvent);
	}

	if (FPlatformTime::Seconds() - ReadyTimeSeconds < InputDelaySeconds)
	{
		return Super::NativeOnTouchStarted(InGeometry, InGestureEvent);
	}

	HandleAccepted();
	return FReply::Handled();
}

UWidget* UMCore_PressToContinueWidget::NativeGetDesiredFocusTarget() const
{
	return const_cast<UMCore_PressToContinueWidget*>(this);
}

// ============================================================================
// INTERNAL
// ============================================================================

void UMCore_PressToContinueWidget::EnterLoadingState()
{
	bReadyToContinue = false;

	if (WS_Content)
	{
		WS_Content->SetActiveWidgetIndex(0);
	}

	K2_OnEnterLoadingState();
}

void UMCore_PressToContinueWidget::HandleAccepted()
{
	bAccepted = true;
	OnAccepted.Broadcast();
	DeactivateWidget();
}

void UMCore_PressToContinueWidget::RefreshDismissalIcon()
{
	if (!Icon_DismissAction || !bReadyToContinue)
	{
		return;
	}

	const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	UCommonInputSubsystem* CIS = LocalPlayer->GetSubsystem<UCommonInputSubsystem>();
	if (!CIS)
	{
		return;
	}

	const ECommonInputType DeviceType = CIS->GetCurrentInputType();
	const FKey DefaultKey = (DeviceType == ECommonInputType::Gamepad)
		? EKeys::Gamepad_FaceButton_Bottom
		: EKeys::SpaceBar;

	FSlateBrush IconBrush;
	if (UMCore_InputDisplayLibrary::GetIconBrushForKeyByDeviceType(
		LocalPlayer, DefaultKey, DeviceType, IconBrush))
	{
		Icon_DismissAction->SetBrush(IconBrush);
		Icon_DismissAction->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		/* Resolution failed (device has no controller data); collapse so no stale glyph from the
		   previous device lingers. Txt_Prompt still conveys the gate. */
		Icon_DismissAction->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UMCore_PressToContinueWidget::HandleInputMethodChanged(ECommonInputType NewInputType)
{
	RefreshDismissalIcon();
}
