// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/Widgets/Primitives/MCore_ConfirmationDialog.h"

#include "CoreUI/Widgets/Primitives/MCore_ButtonBase.h"
#include "CoreData/Logging/LogModulusUI.h"

#include "CommonTextBlock.h"

#define LOCTEXT_NAMESPACE "ModulusCoreUI"

UMCore_ConfirmationDialog::UMCore_ConfirmationDialog(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bShouldFocusOnActivation = true;
	ConfirmText = NSLOCTEXT("ModulusCoreUI", "ConfirmDefault", "Confirm");
	CancelText = NSLOCTEXT("ModulusCoreUI", "CancelDefault", "Cancel");
}

void UMCore_ConfirmationDialog::SetDialogMessage(FText InMessage)
{
	if (Txt_DialogMessage)
	{
		Txt_DialogMessage->SetText(InMessage);
	}
}

void UMCore_ConfirmationDialog::SetButtonLabels(FText InConfirmText, FText InCancelText)
{
	if (Btn_Confirm)
	{
		Btn_Confirm->SetButtonText(InConfirmText);
	}
	if (Btn_Cancel)
	{
		Btn_Cancel->SetButtonText(InCancelText);
	}
}

void UMCore_ConfirmationDialog::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_Confirm)
	{
		Btn_Confirm->OnButtonClicked.AddDynamic(this, &ThisClass::HandleConfirmClicked);
	}

	if (Btn_Cancel)
	{
		Btn_Cancel->OnButtonClicked.AddDynamic(this, &ThisClass::HandleCancelClicked);
	}
}

void UMCore_ConfirmationDialog::NativeOnActivated()
{
	Super::NativeOnActivated();

	/* If activation was blocked by BlockTags, Super already deactivated */
	if (!IsActivated()) { return; }

	bResolved = false;

	if (Btn_Confirm)
	{
		Btn_Confirm->SetButtonText(ConfirmText);
	}

	if (Btn_Cancel)
	{
		Btn_Cancel->SetButtonText(CancelText);
	}
	
	if (!BackInputAction.IsNull())
	{
		FInputActionExecutedDelegate CancelDelegate;
		CancelDelegate.BindDynamic(this, &UMCore_ConfirmationDialog::HandleCancelInput);
		RegisterBinding(BackInputAction, CancelDelegate, BackBindingHandle);
	}

	UE_LOG(LogModulusUI, Log, TEXT("ConfirmationDialog::NativeOnActivated: dialog activated, widget=%s"), *GetNameSafe(this));
}

void UMCore_ConfirmationDialog::NativeOnDeactivated()
{
	OnDialogResult.Clear();
	bResolved = false;
	Super::NativeOnDeactivated();
}

void UMCore_ConfirmationDialog::NativeDestruct()
{
	if (Btn_Confirm)
	{
		Btn_Confirm->OnButtonClicked.RemoveAll(this);
	}

	if (Btn_Cancel)
	{
		Btn_Cancel->OnButtonClicked.RemoveAll(this);
	}

	Super::NativeDestruct();
}

UWidget* UMCore_ConfirmationDialog::NativeGetDesiredFocusTarget() const
{
	/* Default focus to Cancel for safety (prevent accidental confirms) */
	return Btn_Cancel;
}

void UMCore_ConfirmationDialog::ResolveDialog(bool bConfirmed)
{
	if (bResolved) { return; }
	bResolved = true;

	UE_LOG(LogModulusUI, Log,
		TEXT("ConfirmationDialog::ResolveDialog: resolved: %s, widget=%s"),
		bConfirmed ? TEXT("Confirmed") : TEXT("Cancelled"),
		*GetNameSafe(this));

	OnDialogResult.Broadcast(bConfirmed);
	DeactivateWidget();
}

void UMCore_ConfirmationDialog::HandleConfirmClicked()
{
	ResolveDialog(true);
}

void UMCore_ConfirmationDialog::HandleCancelClicked()
{
	ResolveDialog(false);
}

void UMCore_ConfirmationDialog::HandleCancelInput(FName ActionName)
{
	ResolveDialog(false);
}

#undef LOCTEXT_NAMESPACE
