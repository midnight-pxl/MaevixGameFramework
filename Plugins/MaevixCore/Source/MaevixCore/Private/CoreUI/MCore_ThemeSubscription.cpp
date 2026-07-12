// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/MCore_ThemeSubscription.h"

#include "CoreUI/MCore_UISubsystem.h"
#include "CoreData/Logging/LogMaevixUI.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"

void FMCore_ThemeSubscription::Bind(UUserWidget* Widget, FName HandlerName)
{
	if (bBound || !Widget) { return; }

	ULocalPlayer* LocalPlayer = Widget->GetOwningLocalPlayer();
	if (!LocalPlayer) { return; }

	UMCore_UISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UMCore_UISubsystem>();
	if (!UISubsystem) { return; }

	// Expansion of OnThemeChanged.AddDynamic(Widget, &Class::Handler): non-unique Add of a script delegate
	// keyed on (Widget, HandlerName). IsBound() runs FindFunction, so a missing or unregistered handler is
	// caught loudly here rather than silently never applying themes.
	FScriptDelegate Delegate;
	Delegate.BindUFunction(Widget, HandlerName);
	if (!Delegate.IsBound())
	{
		UE_LOG(LogMaevixUI, Error,
			TEXT("Theme handler '%s' failed to bind on '%s'. Signature mismatch."),
			*HandlerName.ToString(), *Widget->GetName());
		return;
	}
	UISubsystem->OnThemeChanged.Add(Delegate);
	bBound = true;
}

void FMCore_ThemeSubscription::Unbind(UUserWidget* Widget, FName HandlerName)
{
	// Flag-clear ordering matches the original UnbindThemeDelegate exactly: cleared in the LocalPlayer-gone
	// branch and again at the end after the removal attempt, never early.
	if (!bBound) { return; }

	ULocalPlayer* LocalPlayer = Widget ? Widget->GetOwningLocalPlayer() : nullptr;
	if (!LocalPlayer)
	{
		bBound = false;
		return;
	}

	if (UMCore_UISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UMCore_UISubsystem>())
	{
		UISubsystem->OnThemeChanged.Remove(Widget, HandlerName);
	}

	bBound = false;
}
