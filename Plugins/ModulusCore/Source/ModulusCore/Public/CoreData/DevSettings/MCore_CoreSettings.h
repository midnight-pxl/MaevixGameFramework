// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CoreData/Types/UI/MCore_ThemeTypes.h"
#include "CoreData/Types/UI/MCore_MenuTabTypes.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "CoreData/Types/Settings/MCore_DA_SettingsCollection.h"
#include "CoreData/Types/Input/MCore_KeyBindingTypes.h"
#include "CoreData/Types/Loading/MCore_LoadingScreenTypes.h"
#include "MCore_CoreSettings.generated.h"

class UMCore_PDA_UITheme_Base;
class UMCore_PrimaryGameLayout;
class UMCore_GameMenuHub;
class UMCore_SettingsWidget_Slider;
class UMCore_SettingsWidget_Switcher;
class UMCore_ConfirmationDialog;
class UMCore_KeyBindingPanel_Base;
class UMCore_SettingsRevertCountdown;
class UMCore_LoadingScreenWidget;
class UMCore_PDA_LoadingScreens;
class USoundMix;
class UInputAction;

/**
 * Developer settings for the Modulus Game Framework (Project Settings > Game > Modulus Core).
 * Configures themes, UI layout class overrides, settings collections,
 * and editor-only debug options.
 */
UCLASS(Config=ModulusCore, DefaultConfig, meta=(DisplayName="Modulus Core"))
class MODULUSCORE_API UMCore_CoreSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UMCore_CoreSettings();

	// ============================================================================
	// DEVELOPER SETTINGS INTERFACE
	// ============================================================================

	virtual FName GetCategoryName() const override { return TEXT("Game"); }
	virtual FName GetSectionName() const override { return TEXT("Modulus Core"); }
#if WITH_EDITOR
	virtual FText GetSectionText() const override;
	virtual FText GetSectionDescription() const override;
#endif

	static const UMCore_CoreSettings* Get();

	// ============================================================================
	// UI LAYOUT
	// ============================================================================

	/**
	 * Widget class for the PrimaryGameLayout.
	 * This is the root UI widget containing the 4-layer CommonUI stack.
	 *
	 * Leave empty to use default UMCore_PrimaryGameLayout.
	 * Set to a Blueprint subclass for custom layout structure.
	 */
	UPROPERTY(Config, EditAnywhere, Category="UI", meta=(DisplayName="Primary Game Layout Class"))
	TSoftClassPtr<UMCore_PrimaryGameLayout> PrimaryGameLayoutClass;

	/** Z-Order for PrimaryGameLayout in viewport. Higher values render on top. */
	UPROPERTY(Config, EditAnywhere, Category="UI", meta=(DisplayName="Layout Z-Order", ClampMin="-100", ClampMax="100"))
	int32 PrimaryGameLayoutZOrder{0};

	// ============================================================================
	// MENU HUB
	// ============================================================================

	/**
	 * Widget class for the GameMenuHub.
	 * This is the tabbed menu container for plugin screens.
	 *
	 * Leave empty to use default UMCore_GameMenuHub.
	 * Set to a Blueprint subclass for custom hub appearance.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Menu Hub", meta=(DisplayName="Menu Hub Class"))
	TSoftClassPtr<UMCore_GameMenuHub> MenuHubClass;

	/**
	 * Menu tabs registered on UISubsystem initialization.
	 * Configure all default tabs here, including tabs from other Modulus plugins.
	 * Additional tabs can be added at runtime via RegisterMenuScreen().
	 */
	UPROPERTY(Config, EditAnywhere, Category="Menu Hub", meta=(DisplayName="Default Menu Tabs"))
	TArray<FMCore_MenuTab> DefaultMenuTabs;

	// ============================================================================
	// THEME CONFIGURATION
	// ============================================================================

	/**
	 * Available UI themes for this project.
	 * Each entry has a display name, description, and theme DataAsset.
	 * Order determines display order in theme selection UI.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Theme", meta=(DisplayName="Available Themes"))
	TArray<FMCore_ThemeEntry> AvailableThemes;

	/** Index into AvailableThemes for the default theme. Loads automatically on UI init. */
	UPROPERTY(Config, EditAnywhere, Category="Theme", meta=(DisplayName="Default Theme", ClampMin="0"))
	int32 DefaultThemeIndex{0};

	// ============================================================================
	// SETTINGS COLLECTIONS
	// ============================================================================

	/**
	 * Settings collections for this project. All collections aggregate into a unified
	 * settings inventory. Categories with the same GameplayTag merge across collections.
	 * Display order follows each collection's Settings array; cross-collection ordering
	 * follows the position of each collection in this array.
	 */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="Settings",
		meta=(DisplayName="Settings Collections"))
	TArray<TSoftObjectPtr<UMCore_DA_SettingsCollection>> SettingsCollections;
	
	/** Widget class used to render Slider-type settings. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="Settings")
	TSubclassOf<UMCore_SettingsWidget_Slider> SettingsSliderWidgetClass;

	/** Widget class used to render Switcher-type settings (Toggle and Dropdown). */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="Settings")
	TSubclassOf<UMCore_SettingsWidget_Switcher> SettingsSwitcherWidgetClass;

	/** Widget class for inline key binding panel content in the Settings Panel. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="Settings")
	TSubclassOf<UMCore_KeyBindingPanel_Base> KeyBindingPanelClass;

	/** Show secondary binding columns in the key binding panel. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="Settings")
	bool bShowSecondaryBindings = false;

	/**
	 * Input mapping contexts to display as tabs in the key binding panel.
	 * Each entry becomes a tab with the given display name.
	 * Order determines tab order. If empty, the panel falls back to a flat list.
	 */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="Settings",
		meta=(DisplayName="Key Binding Contexts"))
	TArray<FMCore_KeyBindingContext> KeyBindingContexts;

	/**
	 * Default dialog class used for destructive action confirmations (Reset All, etc).
	 * used wherever confirmation is required and not locally set for custom functionality
	 */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="Settings")
	TSubclassOf<UMCore_ConfirmationDialog> DefaultConfirmationDialogClass;

	/** Widget class for the revert countdown overlay when a setting with bRequiresConfirmation is changed. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="Settings")
	TSubclassOf<UMCore_SettingsRevertCountdown> SettingsRevertCountdownClass;
	
	/**
	 * Delay for showing revert countdown after a confirmation-required setting change.
	 * Resets on each new change, preventing modal spam during rapid cycling.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = "0.0", ClampMax = "3.0", Units = "s"))
	float ConfirmationDebounceDelay{0.75f};

	/** How long the revert countdown modal displays before auto-reverting. */
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = "5.0", ClampMax = "30.0", Units = "s"))
	float ConfirmationRevertDelay = 15.0f;

	// ============================================================================
	// AUDIO
	// ============================================================================

	/** SoundMix routing volume slider commits via SetSoundMixClassOverride.
	 * Defaults to MCore_VolumeMix shipped with the plugin. To customize,
	 * author your own SoundMix with class adjusters for every SoundClass
	 * referenced by your audio settings DAs. The SoundClass hierarchy must
	 * have one Master parent so the Master slider multiplies through the
	 * parent chain at playback resolution. See README "Customizing the
	 * Volume Mix" section. */
	UPROPERTY(config, EditAnywhere, Category = "Audio")
	TSoftObjectPtr<USoundMix> VolumeMix;

	// ============================================================================
	// LOADING SCREEN
	// ============================================================================

	/**
	 * Widget class shown by the MoviePlayer during PreLoadMap transitions.
	 * Leave empty to disable the loading screen entirely.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Loading Screen",
		meta=(DisplayName="Loading Screen Widget Class"))
	TSoftClassPtr<UMCore_LoadingScreenWidget> LoadingScreenWidgetClass;

	/** Libraries of loading screen entries. All entries aggregate into one selection pool. */
	UPROPERTY(Config, EditAnywhere, Category="Loading Screen",
		meta=(DisplayName="Loading Screen Libraries"))
	TArray<TSoftObjectPtr<UMCore_PDA_LoadingScreens>> LoadingScreenLibraries;

	UPROPERTY(Config, EditAnywhere, Category="Loading Screen",
		meta=(DisplayName="Selection Mode"))
	EMCore_LoadingScreenSelectionMode LoadingScreenSelectionMode = EMCore_LoadingScreenSelectionMode::TagBased;

	/**
	 * Controls how the loading screen exits after the level finishes loading.
	 * AutoOnLoadComplete reproduces Chunk 1 behavior. InputRequired (default)
	 * gates dismissal on player input. Manual disables both and is intended
	 * for async post-load work or server-synchronized ready barriers.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Loading Screen",
		meta=(DisplayName="Dismissal Mode"))
	EMCore_LoadingDismissalMode LoadingDismissalMode = EMCore_LoadingDismissalMode::InputRequired;

	/**
	 * Input action whose icon is displayed in the loading screen's dismiss prompt.
	 * If null while Dismissal Mode is InputRequired, the widget hides the icon
	 * and dismisses on any input.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Loading Screen",
		meta=(DisplayName="Dismissal Input Action",
		      EditCondition="LoadingDismissalMode==EMCore_LoadingDismissalMode::InputRequired",
		      EditConditionHides))
	TSoftObjectPtr<UInputAction> LoadingDismissalAction;

	/**
	 * Floor on loading screen visibility. The screen stays visible at least
	 * this long even when the map finishes loading or input arrives sooner.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Loading Screen",
		meta=(ClampMin="0.0", ClampMax="10.0", Units="s",
		      DisplayName="Minimum Display Time"))
	float MinimumLoadingScreenTime{0.5f};

	UPROPERTY(Config, EditAnywhere, Category="Loading Screen",
		meta=(DisplayName="Loading Screen Enabled By Default"))
	bool bLoadingScreenEnabledByDefault{true};

	// ============================================================================
	// DEBUG (EDITOR ONLY)
	// ============================================================================

#if WITH_EDITORONLY_DATA
	/**
	 * Log all event broadcasts and subscriptions to Output Log.
	 * Shows: Event tag, payload data, subscriber count, broadcast scope.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Debug", meta=(DisplayName="Enable Event System Logging"))
	bool bEnableEventSystemLogging{false};

	/**
	 * Show debug overlay for UI layer stack.
	 * Helps debug: Widget not appearing, wrong layer, input not working.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Debug", meta=(DisplayName="Show UI Layer Debug Overlay"))
	bool bShowUILayerDebugOverlay{false};
#endif

	// ============================================================================
	// PIE
	// ============================================================================

	/**
	 * Whether GameUserSettings-driven setting changes that mutate the host process's
	 * window/display state (Resolution, Window Mode, HDR toggle, HDR nits)
	 * should apply when running in PIE.
	 *
	 * Default false because applying these in PIE resizes the editor's host
	 * window and disrupts the editor process. Set to true if you specifically
	 * need PIE to behave like a packaged build for these settings.
	 *
	 * Has no effect outside PIE; packaged builds (Development, Test,
	 * Shipping) always apply these settings normally.
	 *
	 * Mirrors Lyra's bApplyFrameRateSettingsInPIE / bApplyFrontEndPerformanceOptionsInPIE
	 * pattern from LyraPlatformEmulationSettings.
	 */
	UPROPERTY(EditAnywhere, config, Category = "ModulusCore|PIE",
		meta = (DisplayName = "Apply Display Settings in PIE"))
	bool bApplyDisplaySettingsInPIE = false;

	/**
	 * Whether GameUserSettings-driven setting changes that mutate process-global
	 * renderer state (scalability quality groups, resolution scale,
	 * dynamic resolution toggle) should apply when running in PIE.
	 *
	 * Default false because applying these in PIE recreates the shadow
	 * virtual physical page pool, invalidates TSR history, and (for
	 * ResolutionScale) resizes render targets, disrupting the editor
	 * process. Multi-PIE-window setups amplify the disruption because
	 * renderer state is shared across PIE worlds. Set to true if you
	 * specifically need PIE to behave like a packaged build for these
	 * settings.
	 *
	 * Gated keys: OverallScalabilityLevel, the eight individual
	 * ScalabilityQuality fields (Texture, Shadow, AntiAliasing,
	 * PostProcess, ViewDistance, Foliage, Shading, Effects),
	 * GlobalIlluminationQuality, ReflectionQuality, ResolutionScale,
	 * bUseDynamicResolution.
	 *
	 * Has no effect outside PIE. Packaged builds (Development, Test,
	 * Shipping) always apply these settings normally.
	 *
	 * Mirrors Lyra's bApplyFrontEndPerformanceOptionsInPIE pattern from
	 * LyraPlatformEmulationSettings.
	 */
	UPROPERTY(EditAnywhere, config, Category = "ModulusCore|PIE",
		meta = (DisplayName = "Apply Scalability Settings in PIE"))
	bool bApplyScalabilitySettingsInPIE = false;

	// ============================================================================
	// HELPERS
	// ============================================================================

	/** Returns true if event system logging is enabled. Always false in shipping builds. */
	UFUNCTION(BlueprintPure, Category="Modulus|Debug")
	bool IsEventLoggingEnabled() const;

	/** Returns true if the UI debug overlay is enabled. Always false in shipping builds. */
	UFUNCTION(BlueprintPure, Category="Modulus|Debug")
	bool IsUIDebugOverlayEnabled() const;

	/** Returns the theme DataAsset at DefaultThemeIndex (loads synchronously). Returns nullptr if index is invalid. */
	UFUNCTION(BlueprintCallable, Category="Modulus|Theme")
	UMCore_PDA_UITheme_Base* GetDefaultTheme() const;
	
	// ============================================================================
	// SETTINGS AGGREGATION HELPERS
	// ============================================================================

	/** Returns all loaded settings collections. Resolves soft references on first call, caches results. */
	UFUNCTION(BlueprintPure, Category = "Modulus|Settings")
	const TArray<UMCore_DA_SettingsCollection*>& GetAllSettingsCollections() const;

	/** Searches all collections for a setting definition matching the tag. Returns first match. */
	UFUNCTION(BlueprintPure, Category = "Modulus|Settings")
	UMCore_DA_SettingDefinition* FindSettingDefinitionByTag(const FGameplayTag& SettingTag) const;

	/** Returns all settings across all collections for a category, ordered by collection
	 *  array index then by setting array position within each collection. */
	UFUNCTION(BlueprintPure, Category = "Modulus|Settings")
	TArray<UMCore_DA_SettingDefinition*> GetSettingsForCategory(const FGameplayTag& CategoryTag) const;

	/** Returns all unique category tags across all collections in first-seen order
	 *  (collection array index, then setting array position). */
	UFUNCTION(BlueprintPure, Category = "Modulus|Settings")
	TArray<FGameplayTag> GetAllSettingsCategories() const;

	/** Searches all collections for a category display name. Returns tag leaf segment as fallback. */
	FText GetCategoryDisplayName(const FGameplayTag& CategoryTag) const;

	/** Returns true if at least one collection is assigned and loadable. */
	UFUNCTION(BlueprintPure, Category = "Modulus|Settings")
	bool HasValidSettingsCollections() const;

	/** Clears the resolved collection cache. Next GetAllSettingsCollections() call will re-resolve. */
	void InvalidateCollectionCache();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// ============================================================================
	// THEME HELPERS
	// ============================================================================

	/**
	 * Returns the default theme for design-time preview (static, no UISubsystem needed).
	 * Use in NativePreConstruct() for UMG editor preview. At runtime, prefer UISubsystem->GetActiveTheme().
	 */
	UFUNCTION(BlueprintPure, Category="Modulus|Theme")
	static UMCore_PDA_UITheme_Base* GetDesignTimeTheme();

	UFUNCTION(BlueprintPure, Category="Modulus|Theme")
	bool IsValidThemeIndex(int32 Index) const;

};
