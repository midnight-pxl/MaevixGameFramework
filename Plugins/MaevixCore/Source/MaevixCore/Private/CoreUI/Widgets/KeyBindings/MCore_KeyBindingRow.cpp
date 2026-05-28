// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/Widgets/KeyBindings/MCore_KeyBindingRow.h"

#include "CoreUI/Widgets/KeyBindings/MCore_KeyBindingButton.h"
#include "CoreData/Libraries/MCore_InputDisplayLibrary.h"
#include "CoreData/Logging/LogMaevixUI.h"

#include "InputAction.h"
#include "CommonTextBlock.h"

// ============================================================================
// LIFECYCLE
// ============================================================================

void UMCore_KeyBindingRow::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_KBM_Primary)
	{
		Btn_KBM_Primary->OnRebindComplete.AddDynamic(this, &ThisClass::HandleRebindComplete);
		Btn_KBM_Primary->OnCaptureStateChanged.AddDynamic(this, &ThisClass::HandleCaptureStateChanged);
	}
	if (Btn_KBM_Secondary)
	{
		Btn_KBM_Secondary->OnRebindComplete.AddDynamic(this, &ThisClass::HandleRebindComplete);
		Btn_KBM_Secondary->OnCaptureStateChanged.AddDynamic(this, &ThisClass::HandleCaptureStateChanged);
	}
	if (Btn_Gamepad_Primary)
	{
		Btn_Gamepad_Primary->OnRebindComplete.AddDynamic(this, &ThisClass::HandleRebindComplete);
		Btn_Gamepad_Primary->OnCaptureStateChanged.AddDynamic(this, &ThisClass::HandleCaptureStateChanged);
	}
	if (Btn_Gamepad_Secondary)
	{
		Btn_Gamepad_Secondary->OnRebindComplete.AddDynamic(this, &ThisClass::HandleRebindComplete);
		Btn_Gamepad_Secondary->OnCaptureStateChanged.AddDynamic(this, &ThisClass::HandleCaptureStateChanged);
	}
}

void UMCore_KeyBindingRow::NativeDestruct()
{
	if (Btn_KBM_Primary)
	{
		Btn_KBM_Primary->OnRebindComplete.RemoveAll(this);
		Btn_KBM_Primary->OnCaptureStateChanged.RemoveAll(this);
	}
	if (Btn_KBM_Secondary)
	{
		Btn_KBM_Secondary->OnRebindComplete.RemoveAll(this);
		Btn_KBM_Secondary->OnCaptureStateChanged.RemoveAll(this);
	}
	if (Btn_Gamepad_Primary)
	{
		Btn_Gamepad_Primary->OnRebindComplete.RemoveAll(this);
		Btn_Gamepad_Primary->OnCaptureStateChanged.RemoveAll(this);
	}
	if (Btn_Gamepad_Secondary)
	{
		Btn_Gamepad_Secondary->OnRebindComplete.RemoveAll(this);
		Btn_Gamepad_Secondary->OnCaptureStateChanged.RemoveAll(this);
	}

	Super::NativeDestruct();
}

// ============================================================================
// PUBLIC API
// ============================================================================

void UMCore_KeyBindingRow::InitFromAction(APlayerController* OwningPlayer, UInputAction* Action,
	bool bShowSecondary)
{
	this->PlayerRef = OwningPlayer;
	BoundAction = Action;

	if (Txt_Label && OwningPlayer && Action)
	{
		Txt_Label->SetText(
			UMCore_InputDisplayLibrary::GetActionDisplayName(OwningPlayer, Action));
	}

	if (Btn_KBM_Primary)
	{
		Btn_KBM_Primary->InitForSlot(OwningPlayer, Action, EPlayerMappableKeySlot::First, false);
	}
	if (Btn_KBM_Secondary)
	{
		Btn_KBM_Secondary->InitForSlot(OwningPlayer, Action, EPlayerMappableKeySlot::Second, false);
		Btn_KBM_Secondary->SetVisibility(
			bShowSecondary ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
	if (Btn_Gamepad_Primary)
	{
		Btn_Gamepad_Primary->InitForSlot(OwningPlayer, Action, EPlayerMappableKeySlot::First, true);
	}
	if (Btn_Gamepad_Secondary)
	{
		Btn_Gamepad_Secondary->InitForSlot(OwningPlayer, Action, EPlayerMappableKeySlot::Second, true);
		Btn_Gamepad_Secondary->SetVisibility(
			bShowSecondary ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	UE_LOG(LogMaevixUI, Verbose, TEXT("KeyBindingRow::InitFromAction: %s initialized for action [%s]"),
		*GetNameSafe(this), *GetNameSafe(BoundAction));
}

void UMCore_KeyBindingRow::RefreshDisplay()
{
	if (Btn_KBM_Primary) { Btn_KBM_Primary->RefreshKeyDisplay(); }
	if (Btn_KBM_Secondary) { Btn_KBM_Secondary->RefreshKeyDisplay(); }
	if (Btn_Gamepad_Primary) { Btn_Gamepad_Primary->RefreshKeyDisplay(); }
	if (Btn_Gamepad_Secondary) { Btn_Gamepad_Secondary->RefreshKeyDisplay(); }
}

// ============================================================================
// HANDLERS
// ============================================================================

void UMCore_KeyBindingRow::HandleRebindComplete(bool bSuccess, FText ErrorMessage)
{
	if (!bSuccess && !ErrorMessage.IsEmpty())
	{
		UE_LOG(LogMaevixUI, Warning, TEXT("KeyBindingRow::HandleRebindComplete: %s rebind rejected: %s"),
			*GetNameSafe(this), *ErrorMessage.ToString());
	}
	else if (bSuccess)
	{
		UE_LOG(LogMaevixUI, Log, TEXT("KeyBindingRow::HandleRebindComplete: %s rebind succeeded"),
			*GetNameSafe(this));
	}

	OnRowRebindResult.Broadcast(bSuccess, ErrorMessage);
	RefreshDisplay();
	OnRowRebindCompleted.Broadcast();
}

void UMCore_KeyBindingRow::HandleCaptureStateChanged(UMCore_KeyBindingButton* Button, bool bCapturing)
{
	OnRowCaptureStateChanged.Broadcast(Button, bCapturing);
}
