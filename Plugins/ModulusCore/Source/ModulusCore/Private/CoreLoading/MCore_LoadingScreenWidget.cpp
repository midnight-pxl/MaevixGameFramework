// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreLoading/MCore_LoadingScreenWidget.h"

#include "CommonInputSubsystem.h"
#include "CommonTextBlock.h"
#include "Components/Image.h"
#include "Components/Widget.h"
#include "CoreData/DevSettings/MCore_CoreSettings.h"
#include "CoreData/Libraries/MCore_InputDisplayLibrary.h"
#include "CoreData/Logging/LogModulusLoading.h"
#include "CoreLoading/MCore_LoadingScreenSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"

// ============================================================================
// INITIALIZATION
// ============================================================================

void UMCore_LoadingScreenWidget::Initialize(const FMCore_LoadingScreenEntry& Entry)
{
	if (Image_Background && !Entry.BackgroundImage.IsNull())
	{
		/* Subsystem has already synchronously loaded this texture before handing the entry over. */
		if (UTexture2D* Texture = Entry.BackgroundImage.Get())
		{
			Image_Background->SetBrushFromTexture(Texture);
		}
	}

	if (Txt_Tip && !Entry.TipText.IsEmpty())
	{
		Txt_Tip->SetText(Entry.TipText);
	}

	/* Prompt stays hidden until HandleLoadingComplete fires post-load. */
	if (Prompt_Continue)
	{
		Prompt_Continue->SetVisibility(ESlateVisibility::Collapsed);
	}

	K2_OnLoadingScreenInitialized(Entry);
}

// ============================================================================
// USERWIDGET OVERRIDES
// ============================================================================

void UMCore_LoadingScreenWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(LocalPlayer))
		{
			InputMethodChangedHandle = InputSubsystem->OnInputMethodChangedNative.AddUObject(
				this, &ThisClass::HandleInputMethodChanged);
		}
	}
}

void UMCore_LoadingScreenWidget::NativeDestruct()
{
	if (InputMethodChangedHandle.IsValid())
	{
		if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
		{
			if (UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(LocalPlayer))
			{
				InputSubsystem->OnInputMethodChangedNative.Remove(InputMethodChangedHandle);
			}
		}
		InputMethodChangedHandle.Reset();
	}

	Super::NativeDestruct();
}

FReply UMCore_LoadingScreenWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (bAwaitingInput)
	{
		OnDismissalInputReceived();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UMCore_LoadingScreenWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bAwaitingInput)
	{
		OnDismissalInputReceived();
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UMCore_LoadingScreenWidget::NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	if (bAwaitingInput)
	{
		OnDismissalInputReceived();
		return FReply::Handled();
	}
	return Super::NativeOnTouchStarted(InGeometry, InGestureEvent);
}

// ============================================================================
// DISMISSAL LIFECYCLE
// ============================================================================

void UMCore_LoadingScreenWidget::HandleLoadingComplete()
{
	OnLoadingComplete();
}

void UMCore_LoadingScreenWidget::OnLoadingComplete_Implementation()
{
	if (Prompt_Continue)
	{
		Prompt_Continue->SetVisibility(ESlateVisibility::Visible);
	}

	RefreshDismissalIcon();

	SetKeyboardFocus();
	bAwaitingInput = true;
}

void UMCore_LoadingScreenWidget::OnDismissalInputReceived_Implementation()
{
	/* SP / co-op default. Override (BP or C++) for server-authoritative ready
	   barriers: do not call DismissLoadingScreen here; route through your RPC. */
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMCore_LoadingScreenSubsystem* Subsystem = GI->GetSubsystem<UMCore_LoadingScreenSubsystem>())
		{
			Subsystem->DismissLoadingScreen();
		}
	}
}

// ============================================================================
// PROMPT HELPERS
// ============================================================================

void UMCore_LoadingScreenWidget::SetPromptText(const FText& InText)
{
	if (Txt_DismissPrompt)
	{
		Txt_DismissPrompt->SetText(InText);
	}
}

void UMCore_LoadingScreenWidget::SetPromptIconVisible(bool bVisible)
{
	if (Icon_DismissAction)
	{
		Icon_DismissAction->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UMCore_LoadingScreenWidget::SetAwaitingDismissalInput(bool bAwaiting)
{
	bAwaitingInput = bAwaiting;
}

// ============================================================================
// ICON
// ============================================================================

void UMCore_LoadingScreenWidget::RefreshDismissalIcon()
{
	const UMCore_CoreSettings* Settings = UMCore_CoreSettings::Get();
	if (!Settings || Settings->LoadingDismissalAction.IsNull())
	{
		SetPromptIconVisible(false);
		return;
	}

	UInputAction* Action = Settings->LoadingDismissalAction.LoadSynchronous();
	if (!Action)
	{
		SetPromptIconVisible(false);
		return;
	}

	const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	APlayerController* PC = LocalPlayer ? LocalPlayer->GetPlayerController(GetWorld()) : nullptr;
	if (!PC)
	{
		/* PostLoadMapWithWorld guarantees a PC in nearly all cases, but log + bail
		   if we slipped through during teardown. */
		UE_LOG(LogModulusLoading, Verbose,
			TEXT("LoadingScreenWidget::RefreshDismissalIcon: no PlayerController, deferring icon refresh"));
		SetPromptIconVisible(false);
		return;
	}

	const FKey Key = UMCore_InputDisplayLibrary::GetCurrentKeyForAction(PC, Action);
	if (!Key.IsValid())
	{
		SetPromptIconVisible(false);
		return;
	}

	FSlateBrush Brush;
	if (UMCore_InputDisplayLibrary::GetIconBrushForKey(LocalPlayer, Key, Brush) && Icon_DismissAction)
	{
		Icon_DismissAction->SetBrush(Brush);
		SetPromptIconVisible(true);
	}
	else
	{
		SetPromptIconVisible(false);
	}
}

void UMCore_LoadingScreenWidget::HandleInputMethodChanged(ECommonInputType NewInputType)
{
	if (bAwaitingInput)
	{
		RefreshDismissalIcon();
	}
}
