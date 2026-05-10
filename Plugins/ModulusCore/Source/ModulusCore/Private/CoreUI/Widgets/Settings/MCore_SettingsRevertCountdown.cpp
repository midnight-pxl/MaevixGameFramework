// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/Widgets/Settings/MCore_SettingsRevertCountdown.h"

#include "CoreUI/Widgets/Primitives/MCore_ButtonBase.h"
#include "CoreData/Logging/LogModulusSettings.h"
#include "CoreData/Libraries/MCore_GameSettingsLibrary.h"
#include "CoreData/DevSettings/MCore_CoreSettings.h"

#include "CommonTextBlock.h"
#include "Engine/World.h"
#include "TimerManager.h"

UMCore_SettingsRevertCountdown::UMCore_SettingsRevertCountdown(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bShouldFocusOnActivation = true;
}

// ============================================================================
// LIFECYCLE
// ============================================================================

void UMCore_SettingsRevertCountdown::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_Confirm)
	{
		Btn_Confirm->OnButtonClicked.AddDynamic(this, &ThisClass::HandleConfirmClicked);
	}

	if (Btn_Revert)
	{
		Btn_Revert->OnButtonClicked.AddDynamic(this, &ThisClass::HandleRevertClicked);
	}
}

void UMCore_SettingsRevertCountdown::NativeOnDeactivated()
{
	/* Always kill timer to prevent ticking after deactivation */
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(CountdownTimerHandle);
	}
	
	if (!bResolved)
	{
		UE_LOG(LogModulusSettings, Warning,
			TEXT("SettingsRevertCountdown::NativeOnDeactivated: deactivated w/o resolution, auto-reverting"));
		
		ApplySettings(false);
		OnCountdownResult.Broadcast(false);
	}
	
	/* Clear external delegates to prevent stale references */
	OnCountdownResult.Clear();
	
	/* Reset state */
	PendingTags.Empty();
	RemainingSeconds = 0.f;
	TotalSeconds = 0.f;
	
	Super::NativeOnDeactivated();
}

void UMCore_SettingsRevertCountdown::NativeDestruct()
{
	/*
	 * Defensive timer clear (NativeOnDeactivated should've handled this)
	 * Just in case guarantee
	 */
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(CountdownTimerHandle);
	}
	
	/* Clear external delegate in case NativeOnDeactivated was skipped somehow */
	OnCountdownResult.Clear();
	
	if (Btn_Confirm) { Btn_Confirm->OnButtonClicked.RemoveAll(this); }

	if (Btn_Revert)	{ Btn_Revert->OnButtonClicked.RemoveAll(this); }

	Super::NativeDestruct();
}

UWidget* UMCore_SettingsRevertCountdown::NativeGetDesiredFocusTarget() const
{
	/* Default focus to Revert for safety (timer auto-reverts anyway) */
	return Btn_Revert;
}

// ============================================================================
// COUNTDOWN
// ============================================================================

void UMCore_SettingsRevertCountdown::StartCountdown(const TArray<FGameplayTag>& AffectedTags)
{
	bResolved = false;

	PendingTags = AffectedTags;

	const UMCore_CoreSettings* CoreSettings = UMCore_CoreSettings::Get();
	TotalSeconds = CoreSettings ? CoreSettings->ConfirmationRevertDelay : 15.0f;
	RemainingSeconds = TotalSeconds;

	if (Txt_Countdown)
	{
		Txt_Countdown->SetText(FText::AsNumber(FMath::CeilToInt(RemainingSeconds)));
	}

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(
			CountdownTimerHandle, this,
			&ThisClass::HandleCountdownTick, 1.0f, true);
	}

	UE_LOG(LogModulusSettings, Log,
		TEXT("SettingsRevertCountdown::StartCountdown: started: %.0fs, %d setting(s), widget=%s"),
		TotalSeconds, PendingTags.Num(), *GetNameSafe(this));
}

void UMCore_SettingsRevertCountdown::HandleCountdownTick()
{
	RemainingSeconds -= 1.0f;

	if (Txt_Countdown)
	{
		Txt_Countdown->SetText(FText::AsNumber(FMath::CeilToInt(RemainingSeconds)));
	}

	if (RemainingSeconds <= 0.f)
	{
		ResolveCountdown(false);
	}
}

// ============================================================================
// RESOLUTION
// ============================================================================

void UMCore_SettingsRevertCountdown::HandleConfirmClicked()
{
	ResolveCountdown(true);
}

void UMCore_SettingsRevertCountdown::HandleRevertClicked()
{
	ResolveCountdown(false);
}

void UMCore_SettingsRevertCountdown::ResolveCountdown(bool bConfirmed)
{
	if (bResolved) { return; }
	bResolved = true;

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(CountdownTimerHandle);
	}

	ApplySettings(bConfirmed);

	UE_LOG(LogModulusSettings, Log,
		TEXT("SettingsRevertCountdown::ResolveCountdown: resolved: %s, widget=%s"),
		bConfirmed ? TEXT("Confirmed") : TEXT("Reverted"),
		*GetNameSafe(this));

	OnCountdownResult.Broadcast(bConfirmed);
	DeactivateWidget();
}

void UMCore_SettingsRevertCountdown::SuppressFallbackForReplacement()
{
	bResolved = true;
}

void UMCore_SettingsRevertCountdown::ApplySettings(bool bConfirmed)
{
	if (bConfirmed)
	{
		UMCore_GameSettingsLibrary::SavePlayerSettings(this);
	}
	else
	{
		UMCore_GameSettingsLibrary::ReloadAndApplyFromDisk(this);
	}
}
