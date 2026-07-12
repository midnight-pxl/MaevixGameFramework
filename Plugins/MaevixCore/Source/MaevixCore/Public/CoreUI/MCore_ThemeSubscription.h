// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"

class UUserWidget;

// Internal helper shared by the widget bases: owns the theme-delegate bind lifecycle so the resolve +
// guard + add/remove is not copied per base. Not a UObject; holds only the bound flag. Each base keeps its
// own HandleThemeChanged UFUNCTION (a dynamic delegate must target a UObject) and passes its name in.
struct FMCore_ThemeSubscription
{
	// Bind Widget's HandlerName UFUNCTION to the player's UISubsystem OnThemeChanged. Idempotent.
	void Bind(UUserWidget* Widget, FName HandlerName);
	// Remove the binding if active. Safe when the player/subsystem is already gone.
	void Unbind(UUserWidget* Widget, FName HandlerName);

private:
	bool bBound = false;
};
