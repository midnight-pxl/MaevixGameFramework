// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

/**
 * SMaevixHubTab.h
 *
 * Central hub panel for the Maevix Game Framework editor integration.
 * Displays installed plugins, debug settings, theme info, and documentation links.
 */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SCheckBox;

class SMaevixHubTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMaevixHubTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	static const FName TabId;

private:

	// ============================================================================
	// SETTINGS
	// ============================================================================

	FReply OnOpenProjectSettingsClicked();
	void OnEventLoggingCheckStateChanged(ECheckBoxState NewState);
	void OnUIDebugOverlayCheckStateChanged(ECheckBoxState NewState);
	ECheckBoxState GetEventLoggingCheckState() const;
	ECheckBoxState GetUIDebugOverlayCheckState() const;

	// ============================================================================
	// UI THEME
	// ============================================================================

	FReply OnEditThemeAssetClicked();
	FText GetActiveThemeName() const;

	// ============================================================================
	// ECOSYSTEM
	// ============================================================================

	void RefreshInstalledPlugins();

	// ============================================================================
	// DOCUMENTATION
	// ============================================================================

	FReply OnQuickStartClicked();
	FReply OnAPIReferenceClicked();

	// ============================================================================
	// HELPERS
	// ============================================================================

	void UpdatePluginListUI();
	void OpenURL(const FString& URL);

	TArray<FString> InstalledPluginNames;
	TArray<bool> InstalledPluginEnabled;

	TSharedPtr<SVerticalBox> PluginListBox;
	TSharedPtr<SCheckBox> EventLoggingCheckbox;
	TSharedPtr<SCheckBox> UIDebugOverlayCheckbox;
};
