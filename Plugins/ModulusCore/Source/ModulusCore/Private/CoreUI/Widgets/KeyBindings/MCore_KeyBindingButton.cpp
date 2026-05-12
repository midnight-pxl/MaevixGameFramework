// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/Widgets/KeyBindings/MCore_KeyBindingButton.h"

#include "CoreUI/Widgets/Primitives/MCore_ButtonBase.h"
#include "CoreData/Libraries/MCore_InputDisplayLibrary.h"
#include "CoreData/Logging/LogModulusUI.h"

#include "CommonInputSubsystem.h"
#include "CommonInputTypeEnum.h"
#include "Components/Image.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/IInputProcessor.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"

// ============================================================================
// INPUT PREPROCESSOR
// ============================================================================

class FMCore_KeyCaptureProcessor : public IInputProcessor
{
public:
	FMCore_KeyCaptureProcessor(UMCore_KeyBindingButton* InOwner, bool bInGamepadOnly)
		: Owner(InOwner)
		, bGamepadOnly(bInGamepadOnly)
	{
	}

	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp,
		TSharedRef<ICursor> Cursor) override
	{
	}

	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp,
		const FKeyEvent& InKeyEvent) override
	{
		if (!Owner.IsValid()) { return false; }

		const FKey Key = InKeyEvent.GetKey();

		if (Key == EKeys::Escape)
		{
			Owner->OnCaptureCancelled();
			return true;
		}

		/* Filter by device type */
		if (bGamepadOnly && !Key.IsGamepadKey()) { return false; }
		if (!bGamepadOnly && Key.IsGamepadKey()) { return false; }

		Owner->OnKeyCaptured(Key);
		return true;
	}

	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp,
		const FPointerEvent& MouseEvent) override
	{
		if (!Owner.IsValid()) { return false; }

		const FKey Key = MouseEvent.GetEffectingButton();

		/* Left click cancels capture and passes through to UI */
		if (Key == EKeys::LeftMouseButton)
		{
			Owner->OnCaptureCancelled();
			return false;
		}

		/* Gamepad buttons don't capture mouse */
		if (bGamepadOnly) { return false; }

		/* Right click, middle click, side buttons are valid KBM bindings */
		Owner->OnKeyCaptured(Key);
		return true;
	}

	virtual const TCHAR* GetDebugName() const override
	{
		return TEXT("MCore_KeyCaptureProcessor");
	}

private:
	TWeakObjectPtr<UMCore_KeyBindingButton> Owner;
	bool bGamepadOnly;
};

// ============================================================================
// LIFECYCLE
// ============================================================================

void UMCore_KeyBindingButton::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (UnboundText.IsEmpty())
	{
		UnboundText = NSLOCTEXT("ModulusCore", "Unbound", "None");
	}

	if (Btn_Capture)
	{
		Btn_Capture->OnButtonClicked.AddDynamic(this, &ThisClass::HandleButtonPressed);
	}

	if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UCommonInputSubsystem* CIS = LocalPlayer->GetSubsystem<UCommonInputSubsystem>())
		{
			CIS->OnInputMethodChangedNative.AddUObject(this, &ThisClass::HandleInputMethodChanged);
		}
	}
}

void UMCore_KeyBindingButton::NativeDestruct()
{
	/* Clean up preprocessor if capture was interrupted */
	if (KeyCaptureProcessor.IsValid() && FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(KeyCaptureProcessor);
		KeyCaptureProcessor.Reset();
	}

	if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UCommonInputSubsystem* CIS = LocalPlayer->GetSubsystem<UCommonInputSubsystem>())
		{
			CIS->OnInputMethodChangedNative.RemoveAll(this);
		}
	}

	if (Btn_Capture)
	{
		Btn_Capture->OnButtonClicked.RemoveAll(this);
	}

	Super::NativeDestruct();
}

// ============================================================================
// PUBLIC API
// ============================================================================

void UMCore_KeyBindingButton::InitForSlot(APlayerController* OwningPlayer, UInputAction* Action,
	EPlayerMappableKeySlot InSlot, bool bInIsGamepad)
{
	this->PlayerRef = OwningPlayer;
	BoundAction = Action;
	Slot = InSlot;
	bIsGamepad = bInIsGamepad;

	RefreshKeyDisplay();
}

void UMCore_KeyBindingButton::RefreshKeyDisplay()
{
	if (!Btn_Capture) { return; }

	APlayerController* Player = PlayerRef.Get();
	if (!Player || !BoundAction)
	{
		Btn_Capture->SetButtonText(UnboundText);
		Btn_Capture->SetDisplayMode(EMCore_ButtonDisplayMode::TextOnly);
		CurrentState = EMCore_KeyBindingButtonState::Unbound;
		return;
	}

	const FKey BoundKey = UMCore_InputDisplayLibrary::GetBoundKeyForSlot(
		Player, BoundAction, Slot, bIsGamepad);

	if (BoundKey.IsValid())
	{
		const ULocalPlayer* LocalPlayer = Player->GetLocalPlayer();
		const ECommonInputType DeviceType = bIsGamepad
			? ECommonInputType::Gamepad
			: ECommonInputType::MouseAndKeyboard;

		FSlateBrush IconBrush;
		const bool bHasIcon = LocalPlayer && UMCore_InputDisplayLibrary::GetIconBrushForKeyByDeviceType(
			LocalPlayer, BoundKey, DeviceType, IconBrush);

		if (bHasIcon)
		{
			Btn_Capture->SetButtonIconBrush(IconBrush);
			Btn_Capture->SetDisplayMode(EMCore_ButtonDisplayMode::IconOnly);
		}
		else
		{
			Btn_Capture->SetButtonText(BoundKey.GetDisplayName());
			Btn_Capture->SetDisplayMode(EMCore_ButtonDisplayMode::TextOnly);
		}

		CurrentState = EMCore_KeyBindingButtonState::Bound;
	}
	else
	{
		Btn_Capture->SetButtonText(UnboundText);
		Btn_Capture->SetDisplayMode(EMCore_ButtonDisplayMode::TextOnly);
		CurrentState = EMCore_KeyBindingButtonState::Unbound;
	}
}

// ============================================================================
// CAPTURE MODE
// ============================================================================

void UMCore_KeyBindingButton::EnterCaptureMode()
{
	if (CurrentState == EMCore_KeyBindingButtonState::Capturing)
	{
		UE_LOG(LogModulusUI, Warning,
			TEXT("KeyBindingButton::EnterCaptureMode: %s is already in capture mode"),
			*GetNameSafe(this));
		return;
	}

	UE_LOG(LogModulusUI, Verbose,
		TEXT("KeyBindingButton::EnterCaptureMode: %s entering capture mode"),
		*GetNameSafe(this));

	CurrentState = EMCore_KeyBindingButtonState::Capturing;
	OnCaptureStateChanged.Broadcast(this, true);

	if (FSlateApplication::IsInitialized())
	{
		KeyCaptureProcessor = MakeShared<FMCore_KeyCaptureProcessor>(this, bIsGamepad);
		FSlateApplication::Get().RegisterInputPreProcessor(KeyCaptureProcessor, 0);
	}
}

void UMCore_KeyBindingButton::ExitCaptureMode()
{
	UE_LOG(LogModulusUI, Verbose,
		TEXT("KeyBindingButton::ExitCaptureMode: %s exiting capture mode"),
		*GetNameSafe(this));

	if (KeyCaptureProcessor.IsValid() && FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(KeyCaptureProcessor);
		KeyCaptureProcessor.Reset();
	}

	OnCaptureStateChanged.Broadcast(this, false);
	RefreshKeyDisplay();
}

// ============================================================================
// HANDLERS
// ============================================================================

void UMCore_KeyBindingButton::HandleButtonPressed()
{
	if (CurrentState != EMCore_KeyBindingButtonState::Capturing)
	{
		EnterCaptureMode();
	}
}

void UMCore_KeyBindingButton::AttemptRebind(FKey NewKey)
{
	APlayerController* Player = PlayerRef.Get();
	if (!Player || !BoundAction)
	{
		UE_LOG(LogModulusUI, Warning,
			TEXT("KeyBindingButton::AttemptRebind: %s rebind failed, invalid state (OwningPlayer=%s, Action=%s)"),
			*GetNameSafe(this), *GetNameSafe(Player), *GetNameSafe(BoundAction.Get()));
		ExitCaptureMode();
		OnRebindComplete.Broadcast(false, FText::FromString(TEXT("Invalid state")));
		return;
	}

	UE_LOG(LogModulusUI, Verbose,
		TEXT("KeyBindingButton::AttemptRebind: %s attempting rebind to [%s]"),
		*GetNameSafe(this), *NewKey.ToString());

	FText OutError;
	const bool bSuccess = UMCore_InputDisplayLibrary::RemapActionKeyForSlot(
		Player, BoundAction, NewKey, Slot, OutError);

	if (!bSuccess)
	{
		UE_LOG(LogModulusUI, Warning, TEXT("KeyBindingButton::AttemptRebind: %s rebind to [%s] failed: %s"),
			*GetNameSafe(this), *NewKey.ToString(), *OutError.ToString());
	}

	ExitCaptureMode();
	OnRebindComplete.Broadcast(bSuccess, OutError);
}

void UMCore_KeyBindingButton::OnKeyCaptured(FKey NewKey)
{
	UE_LOG(LogModulusUI, Verbose,
		TEXT("KeyBindingButton::OnKeyCaptured: %s captured key [%s]"),
		*GetNameSafe(this), *NewKey.ToString());
	AttemptRebind(NewKey);
}

void UMCore_KeyBindingButton::OnCaptureCancelled()
{
	ExitCaptureMode();
}

void UMCore_KeyBindingButton::HandleInputMethodChanged(ECommonInputType NewInputType)
{
	RefreshKeyDisplay();
}
