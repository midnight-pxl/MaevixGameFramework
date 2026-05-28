// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/Widgets/Primitives/MCore_ActionButton.h"

#include "CoreData/Libraries/MCore_InputDisplayLibrary.h"
#include "CoreData/Logging/LogMaevixUI.h"

#include "CommonInputSubsystem.h"
#include "CommonUITypes.h"
#include "Components/Image.h"
#include "Engine/LocalPlayer.h"

// ============================================================================
// LIFECYCLE
// ============================================================================

void UMCore_ActionButton::NativeOnInitialized()
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

	if (bShowIconOnly)
	{
		SetDisplayMode(EMCore_ButtonDisplayMode::IconOnly);
	}

	RefreshInputIcon();
}

void UMCore_ActionButton::NativeDestruct()
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

// ============================================================================
// PUBLIC API
// ============================================================================

void UMCore_ActionButton::SetInputAction(const FDataTableRowHandle& InAction)
{
	InputAction = InAction;
	RefreshInputIcon();
}

void UMCore_ActionButton::RefreshInputIcon()
{
	if (InputAction.IsNull()) { return; }

	const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!LocalPlayer) { return; }

	UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(LocalPlayer);
	if (!InputSubsystem) { return; }

	/* Look up the DT row to extract the FKey for the current input method */
	const UDataTable* DataTable = InputAction.DataTable.Get();
	if (!DataTable) { return; }

	const FCommonInputActionDataBase* Row =
		DataTable->FindRow<FCommonInputActionDataBase>(InputAction.RowName, TEXT("ActionButton::RefreshInputIcon"));
	if (!Row) { return; }

	/* GetCurrentInputTypeInfo handles input type + gamepad override resolution internally */
	const FKey ResolvedKey = Row->GetCurrentInputTypeInfo(InputSubsystem).GetKey();
	if (!ResolvedKey.IsValid()) { return; }

	FSlateBrush IconBrush;
	const bool bResolved = UMCore_InputDisplayLibrary::GetIconBrushForKey(
		LocalPlayer, ResolvedKey, IconBrush);

	if (bResolved && Img_BtnIcon)
	{
		Img_BtnIcon->SetBrush(IconBrush);
	}
	else if (ButtonIcon)
	{
		/* Fall back to the static ButtonIcon property if brush resolution fails */
		SetButtonIcon(ButtonIcon);
	}
}

// ============================================================================
// HANDLERS
// ============================================================================

void UMCore_ActionButton::HandleInputMethodChanged(ECommonInputType NewInputType)
{
	RefreshInputIcon();
}