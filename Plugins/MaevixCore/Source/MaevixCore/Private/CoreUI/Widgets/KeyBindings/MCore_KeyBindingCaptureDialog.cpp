// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/Widgets/KeyBindings/MCore_KeyBindingCaptureDialog.h"

#include "CoreUI/Widgets/Primitives/MCore_ButtonBase.h"
#include "CoreData/Assets/UI/Themes/MCore_PDA_UITheme_Base.h"
#include "CoreData/Libraries/MCore_ThemeLibrary.h"
#include "CoreData/Logging/LogMaevixUI.h"

#include "CommonTextBlock.h"
#include "Engine/World.h"
#include "TimerManager.h"

UMCore_KeyBindingCaptureDialog::UMCore_KeyBindingCaptureDialog(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bShouldFocusOnActivation = true;
}

// ============================================================================
// PUBLIC API
// ============================================================================

void UMCore_KeyBindingCaptureDialog::ConfigureForCapture(const FText& ActionName,
	const FText& SlotContext, const FText& PromptText)
{
	if (Txt_ActionName) { Txt_ActionName->SetText(ActionName); }
	if (Txt_SlotContext) { Txt_SlotContext->SetText(SlotContext); }

	CachedPromptText = PromptText;

	if (Txt_PromptOrError) { Txt_PromptOrError->SetText(PromptText); }
}

void UMCore_KeyBindingCaptureDialog::ShowCaptureError(const FText& ErrorMessage)
{
	if (Txt_PromptOrError)
	{
		Txt_PromptOrError->SetText(ErrorMessage);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ErrorCooldownTimerHandle);
		World->GetTimerManager().SetTimer(
			ErrorCooldownTimerHandle, this, &ThisClass::HandleErrorCooldownExpired,
			ErrorDisplayDuration, false);
	}

	UE_LOG(LogMaevixUI, Log,
		TEXT("KeyBindingCaptureDialog::ShowCaptureError: displaying error for %.1fs [%s]"),
		ErrorDisplayDuration, *GetNameSafe(this));
}

// ============================================================================
// LIFECYCLE
// ============================================================================

void UMCore_KeyBindingCaptureDialog::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_Cancel)
	{
		Btn_Cancel->OnButtonClicked.AddDynamic(this, &ThisClass::HandleCancelClicked);
	}
}

void UMCore_KeyBindingCaptureDialog::NativeOnActivated()
{
	Super::NativeOnActivated();

	if (!IsActivated()) { return; }

	if (!BackInputAction.IsNull())
	{
		FInputActionExecutedDelegate CancelDelegate;
		CancelDelegate.BindDynamic(this, &UMCore_KeyBindingCaptureDialog::HandleBackInput);
		RegisterBinding(BackInputAction, CancelDelegate, BackBindingHandle);
	}

	if (Txt_PromptOrError && !CachedPromptText.IsEmpty())
	{
		Txt_PromptOrError->SetText(CachedPromptText);
		Txt_PromptOrError->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	UE_LOG(LogMaevixUI, Log,
		TEXT("KeyBindingCaptureDialog::NativeOnActivated: dialog activated [%s]"),
		*GetNameSafe(this));
}

void UMCore_KeyBindingCaptureDialog::NativeOnDeactivated()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ErrorCooldownTimerHandle);
	}

	Super::NativeOnDeactivated();
}

void UMCore_KeyBindingCaptureDialog::NativeDestruct()
{
	if (ErrorCooldownTimerHandle.IsValid())
	{
		if (const UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(ErrorCooldownTimerHandle);
		}
	}

	if (Btn_Cancel)
	{
		Btn_Cancel->OnButtonClicked.RemoveAll(this);
	}

	Super::NativeDestruct();
}

UWidget* UMCore_KeyBindingCaptureDialog::NativeGetDesiredFocusTarget() const
{
	return Btn_Cancel;
}

// ============================================================================
// THEME
// ============================================================================

void UMCore_KeyBindingCaptureDialog::ApplyTheme_Implementation(UMCore_PDA_UITheme_Base* NewTheme)
{
	CachedTheme = NewTheme;

	if (!NewTheme) { return; }

	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();

	UMCore_ThemeLibrary::ApplyTextStyleFromTheme(
		LocalPlayer, Txt_ActionName, NewTheme->HeadingTextStyle);

	UMCore_ThemeLibrary::ApplyTextStyleFromTheme(
		LocalPlayer, Txt_SlotContext, NewTheme->LabelTextStyle);

	UMCore_ThemeLibrary::ApplyTextStyleFromTheme(
		LocalPlayer, Txt_PromptOrError, NewTheme->DescriptionTextStyle);

	K2_OnThemeApplied(NewTheme);
}

// ============================================================================
// HANDLERS
// ============================================================================

void UMCore_KeyBindingCaptureDialog::HandleCancelClicked()
{
	DismissDialog();
}

void UMCore_KeyBindingCaptureDialog::HandleBackInput(FName ActionName)
{
	DismissDialog();
}

void UMCore_KeyBindingCaptureDialog::HandleErrorCooldownExpired()
{
	if (Txt_PromptOrError)
	{
		Txt_PromptOrError->SetText(CachedPromptText);
	}

	OnReadyForCapture.Broadcast();
}

void UMCore_KeyBindingCaptureDialog::DismissDialog()
{
	OnDialogDismissed.Broadcast();
	DeactivateWidget();
}
