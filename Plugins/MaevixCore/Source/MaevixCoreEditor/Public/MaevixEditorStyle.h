// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

/**
 * MaevixEditorStyle.h
 *
 * Slate style set providing consistent styling for all Maevix
 * editor UI elements including colors, brushes, and text styles.
 */

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

class FMaevixEditorStyle
{
public:
	static void Initialize();
	static void Shutdown();
	static void ReloadTextures();

	static const ISlateStyle& Get() { return *StyleInstance; }
	static FName GetStyleSetName();

	static const FName MaevixIconName;
	static const FName SettingsIconName;
	static const FName EcosystemIconName;
	static const FName DocsIconName;

	static FLinearColor GetAccentColor();
	static FLinearColor GetSecondaryColor();
	static FLinearColor GetBackgroundColor();
	static FLinearColor GetHeaderBackgroundColor();
	static FLinearColor GetSuccessColor();
	static FLinearColor GetWarningColor();
	static FLinearColor GetErrorColor();

private:
	static TSharedRef<FSlateStyleSet> Create();
	static TSharedPtr<FSlateStyleSet> StyleInstance;
};
