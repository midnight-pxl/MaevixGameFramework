// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MCore_ThemeableInterface.generated.h"

class UMCore_PDA_UITheme_Base;

UINTERFACE(MinimalAPI, Blueprintable)
class UMCore_ThemeableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for widgets that respond to theme changes.
 * Implement OnThemeChanged to apply theme properties, and call RefreshTheme
 * in NativeOnInitialized.
 */
class MAEVIXCORE_API IMCore_ThemeableInterface
{
	GENERATED_BODY()

public:
	/**
	 * Called when the active theme changes.
	 *
	 * Apply theme properties to your widget's visual elements here.
	 * Called automatically by RefreshTheme() and when the global theme
	 * changes in Developer Settings.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "MaevixCore|Theme")
	void OnThemeChanged(const UMCore_PDA_UITheme_Base* NewTheme);
	virtual void OnThemeChanged_Implementation(const UMCore_PDA_UITheme_Base* NewTheme) {}

	/**
	 * Request theme refresh from Developer Settings.
	 * Call this in NativeOnInitialized() to apply the initial theme.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "MaevixCore|Theme")
	void RefreshTheme();
	virtual void RefreshTheme_Implementation() {}

};
