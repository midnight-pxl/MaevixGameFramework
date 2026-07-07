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
class UMCore_ToastBase;
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
class UWorld;

/**
 * Developer settings for Maevix Game Framework (Project Settings > Game > Maevix Core).
 * Configures themes, UI layout class overrides, settings collections,
 * and editor-only debug options.
 */
UCLASS(Config=MaevixCore, DefaultConfig, meta=(DisplayName="Maevix Core"))
class MAEVIXCORE_API UMCore_CoreSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UMCore_CoreSettings();

	// ============================================================================
	// DEVELOPER SETTINGS INTERFACE
	// ============================================================================

	virtual FName GetCategoryName() const override { return TEXT("Game"); }
	virtual FName GetSectionName() const override { return TEXT("Maevix Core"); }
#if WITH_EDITOR
	virtual FText GetSectionText() const override;
	virtual FText GetSectionDescription() const override;
#endif

	static const UMCore_CoreSettings* Get();

	// ============================================================================
	// UI LAYOUT
	// ============================================================================

	/**
	 * PrimaryGameLayout Widget Class.
	 * Root UI widget containing the 4-layer CommonUI stacks.
	 *
	 * Leave empty to use default UMCore_PrimaryGameLayout.
	 * Set to a Blueprint subclass for custom layout structure.
	 */
	UPROPERTY(Config, EditAnywhere, Category="MaevixCore|UI", meta=(DisplayName="Primary Game Layout Class"))
	TSoftClassPtr<UMCore_PrimaryGameLayout> PrimaryGameLayoutClass;

	/** Z-Order for PrimaryGameLayout in viewport. Higher values render on top. */
	UPROPERTY(Config, EditAnywhere, Category="MaevixCore|UI", meta=(DisplayName="Layout Z-Order", ClampMin="-100", ClampMax="100"))
	int32 PrimaryGameLayoutZOrder{0};

	// ============================================================================
	// MENU HUB
	// ============================================================================

	/**
	 * GameMenuHub Widget Class.
	 * The tabbed menu container for plugin screen(s).
	 *
	 * Leave empty to use default UMCore_GameMenuHub.
	 * Set to a Blueprint subclass for custom hub appearance.
	 */
	UPROPERTY(Config, EditAnywhere, Category="MaevixCore|Menu Hub", meta=(DisplayName="Menu Hub Class"))
	TSoftClassPtr<UMCore_GameMenuHub> MenuHubClass;

	/**
	 * Menu tab(s) registered on UISubsystem initialization.
	 * 
	 * Configure all default tabs here, including tabs from/for other Maevix plugins.
	 * Additional tabs can be added at runtime via RegisterMenuScreen().
	 */
	UPROPERTY(Config, EditAnywhere, Category="MaevixCore|Menu Hub", meta=(DisplayName="Default Menu Tabs"))
	TArray<FMCore_MenuTab> DefaultMenuTabs;

	// ============================================================================
	// THEME CONFIGURATION
	// ============================================================================

	/**
	 * UI themes available for this project.
	 * 
	 * Each entry has a display name, description, and theme DataAsset.
	 * Order determines display order in theme selection UI.
	 */
	UPROPERTY(Config, EditAnywhere, Category="MaevixCore|Theme", meta=(DisplayName="Available Themes"))
	TArray<FMCore_ThemeEntry> AvailableThemes;

	/** Index into AvailableThemes for the default theme. Loads automatically on UI init. */
	UPROPERTY(Config, EditAnywhere, Category="MaevixCore|Theme", meta=(DisplayName="Default Theme", ClampMin="0"))
	int32 DefaultThemeIndex{0};

	// ============================================================================
	// SETTINGS COLLECTIONS
	// ============================================================================

	/**
	 * Settings collections for this project.
	 * All collections aggregate into a unified settings inventory.
	 * Categories with same GameplayTag merge across collections.
	 * 
	 * Display order follows collection's order in Settings array; cross-collection
	 * ordering follows position of each collection in this array.
	 */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="MaevixCore|Settings",
		meta=(DisplayName="Settings Collections"))
	TArray<TSoftObjectPtr<UMCore_DA_SettingsCollection>> SettingsCollections;
	
	/** Widget class used to render Slider-type settings. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="MaevixCore|Settings")
	TSubclassOf<UMCore_SettingsWidget_Slider> SettingsSliderWidgetClass;

	/** Widget class used to render Switcher-type settings (Toggle and Dropdown). */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="MaevixCore|Settings")
	TSubclassOf<UMCore_SettingsWidget_Switcher> SettingsSwitcherWidgetClass;

	/** Widget class for inline key binding panel content in the Settings Panel. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="MaevixCore|Settings")
	TSubclassOf<UMCore_KeyBindingPanel_Base> KeyBindingPanelClass;

	/** Show secondary binding columns in the key binding panel. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="MaevixCore|Settings")
	bool bShowSecondaryBindings{false};

	/**
	 * Input mapping contexts to display as tabs in the key binding panel.
	 * Each entry becomes a tab with the given display name.
	 * Order determines tab order. If empty, the panel falls back to a flat list.
	 */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="MaevixCore|Settings",
		meta=(DisplayName="Key Binding Contexts"))
	TArray<FMCore_KeyBindingContext> KeyBindingContexts;

	/**
	 * Default dialog class used for destructive action confirmations (Reset All, etc).
	 * used wherever confirmation is required and not locally set for custom functionality
	 */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="MaevixCore|Settings")
	TSubclassOf<UMCore_ConfirmationDialog> DefaultConfirmationDialogClass;

	/** Widget class for the revert countdown overlay when a setting with bRequiresConfirmation is changed. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="MaevixCore|Settings")
	TSubclassOf<UMCore_SettingsRevertCountdown> SettingsRevertCountdownClass;
	
	/**
	 * Delay for showing revert countdown after a confirmation-required setting change.
	 * Resets on each new change, preventing modal spam during setting cycling.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "MaevixCore|Settings", meta = (ClampMin = "0.0", ClampMax = "3.0", Units = "s"))
	float ConfirmationDebounceDelay{0.75f};

	/** How long the revert countdown modal displays before auto-reverting. */
	UPROPERTY(Config, EditAnywhere, Category = "MaevixCore|Settings", meta = (ClampMin = "5.0", ClampMax = "30.0", Units = "s"))
	float ConfirmationRevertDelay{10.0f};

	// ============================================================================
	// AUDIO
	// ============================================================================

	/**
	 * SoundMix routing volume slider commits via SetSoundMixClassOverride.
	 * Defaults to MCore_VolumeMix shipped with the plugin.
	 * 
	 * To customize, author your own SoundMix with class adjusters for each
	 * SoundClass referenced by your audio settings DAs.
	 * 
	 * SoundClass hierarchy expects one Master parent.
	 * Master slider multiplies through parent chain at playback resolution.
	 */
	UPROPERTY(config, EditAnywhere, Category = "MaevixCore|Audio")
	TSoftObjectPtr<USoundMix> VolumeMix;

	// ============================================================================
	// NETWORK SAFETY
	// ============================================================================

	/* Bounds for UMCore_GlobalEventSubsystem::ValidateEventRequest. ClampMin enforces
	 * safety floor; ClampMax prevents overly permissive overrides to prevent DoS.
	 */

	/** Max FMCore_EventParameter count per global event. Floor: 1, Ceiling: 32. */
	UPROPERTY(Config, EditDefaultsOnly, Category="MaevixCore|Networking|Safety|Event Validation",
		meta=(ClampMin="1", ClampMax="32"))
	int32 MaxEventParams{8};

	/** Max ContextID string length in chars. Floor: 16, Ceiling: 256. */
	UPROPERTY(Config, EditDefaultsOnly, Category="MaevixCore|Networking|Safety|Event Validation",
		meta=(ClampMin="16", ClampMax="256"))
	int32 MaxEventContextIDLength{64};

	/** Max FInstancedStruct sizeof for typed payloads. Floor: 256 B, Ceiling: 8 KB. */
	UPROPERTY(Config, EditDefaultsOnly, Category="MaevixCore|Networking|Safety|Event Validation",
		meta=(ClampMin="256", ClampMax="8192"))
	int32 MaxEventStructSizeBytes{2048};

	/** Max FBufferArchive serialized payload size for typed payloads. Floor: 512 B, Ceiling: 16 KB. */
	UPROPERTY(Config, EditDefaultsOnly, Category="MaevixCore|Networking|Safety|Event Validation",
		meta=(ClampMin="512", ClampMax="16384"))
	int32 MaxEventSerializedPayloadBytes{4096};

	// ============================================================================
	// EVENT DIAGNOSTICS
	// ============================================================================

	/**
	 * Suppresses one-shot warning that fires first time when true.
	 * 
	 * UMCore_GlobalEventSubsystem::BroadcastGlobalEvent runs in Standalone mode
	 * without a UMCore_GlobalEventReplicator on the GameState.
	 * Warning exists to flag degraded multiplayer code during PIE testing.
	 * 
	 * Suppress if project intentionally uses Global scope for local-only delivery.
	 */
	UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category = "MaevixCore|Events|Diagnostics")
	bool bSuppressStandaloneNoReplicatorWarning{false};

	// ============================================================================
	// LOADING SCREEN
	// ============================================================================

	/**
	 * Widget class shown by MoviePlayer during PreLoadMap transition(s).
	 * Leave empty to disable loading screen entirely.
	 */
	UPROPERTY(Config, EditAnywhere, Category="MaevixCore|Loading|Screen",
		meta=(DisplayName="Loading Screen Widget Class"))
	TSoftClassPtr<UMCore_LoadingScreenWidget> LoadingScreenWidgetClass;

	/** Libraries of loading screen entries. All entries aggregate into one selection pool. */
	UPROPERTY(Config, EditAnywhere, Category="MaevixCore|Loading|Screen",
		meta=(DisplayName="Loading Screen Libraries"))
	TArray<TSoftObjectPtr<UMCore_PDA_LoadingScreens>> LoadingScreenLibraries;

	UPROPERTY(Config, EditAnywhere, Category="MaevixCore|Loading|Screen",
		meta=(DisplayName="Selection Mode"))
	EMCore_LoadingScreenSelectionMode LoadingScreenSelectionMode = EMCore_LoadingScreenSelectionMode::TagBased;

	/**
	 * Controls how loading screen exits after level loading finishes.
	 * 
	 * InputRequired (default) gates dismissal on player input.
	 * Manual disables both and is intended for async post-load work or
	 * server-synchronized ready barriers.
	 */
	UPROPERTY(Config, EditAnywhere, Category="MaevixCore|Loading|Screen",
		meta=(DisplayName="Dismissal Mode"))
	EMCore_LoadingDismissalMode LoadingDismissalMode = EMCore_LoadingDismissalMode::InputRequired;

	/**
	 * What input action icon is displayed in the loading screen's dismiss prompt.
	 * If null && Dismissal Mode is InputRequired, widget hides the icon
	 * and dismisses on any input.
	 */
	UPROPERTY(Config, EditAnywhere, Category="MaevixCore|Loading|Screen",
		meta=(DisplayName="Dismissal Input Action",
		      EditCondition="LoadingDismissalMode==EMCore_LoadingDismissalMode::InputRequired",
		      EditConditionHides))
	TSoftObjectPtr<UInputAction> LoadingDismissalAction;

	/**
	 * Floor for loading screen visibility. Screen stays visible at least
	 * this long, even when map finishes loading or input received.
	 */
	UPROPERTY(Config, EditAnywhere, Category="MaevixCore|Loading|Screen",
		meta=(ClampMin="0.0", ClampMax="10.0", Units="s",
		      DisplayName="Minimum Display Time"))
	float MinimumLoadingScreenTime{0.5f};

	UPROPERTY(Config, EditAnywhere, Category="MaevixCore|Loading|Screen",
		meta=(DisplayName="Loading Screen Enabled By Default"))
	bool bLoadingScreenEnabledByDefault{true};

	// ============================================================================
	// GAME FLOW
	// ============================================================================

	/**
	 * Level New-Game flow travels to (see UMCore_GameFlowLibrary::StartNewGame).
	 * Stored as soft pointer; level is referenced by path and only loaded on
	 * travel.
	 * 
	 * Leave unset to disable New-Game travel.
	 */
	UPROPERTY(Config, EditAnywhere, Category="MaevixCore|Game Flow",
		meta=(DisplayName="New Game Target Level"))
	TSoftObjectPtr<UWorld> NewGameTargetLevel;

	// ============================================================================
	// TOAST NOTIFICATIONS
	// ============================================================================

	/** Default toast widget class when a request supplies no WidgetOverride. */
	UPROPERTY(Config, EditAnywhere, Category="MaevixCore|UI|Toast",
		meta=(DisplayName="Default Toast Widget Class"))
	TSoftClassPtr<UMCore_ToastBase> DefaultToastWidgetClass;

	/** 
	 * Anchor used when request's AnchorTag is empty/unresolved.
	 * If left empty, service falls back to MCore.UI.Toast.Anchor.BottomRight. */
	UPROPERTY(Config, EditAnywhere, Category="MaevixCore|UI|Toast",
		meta=(DisplayName="Default Toast Anchor", Categories="MCore.UI.Toast.Anchor"))
	FGameplayTag DefaultToastAnchorTag;

	/**
	 * Max visible toasts per anchor; overflow queues, never drops.
	 * Default == 1: one toast at a time per anchor, the queue shows the next as slot frees.
	 */
	UPROPERTY(Config, EditAnywhere, Category="MaevixCore|UI|Toast",
		meta=(ClampMin="1", ClampMax="16"))
	int32 DefaultMaxVisibleToasts{1};

	/**
	 * Escape hatch to raise specific anchor's cap above the default, ships empty.
	 * Unlisted anchors use DefaultMaxVisibleToasts.
	 */
	UPROPERTY(Config, EditAnywhere, Category="MaevixCore|UI|Toast",
		meta=(DisplayName="Max Visible Per Anchor", Categories="MCore.UI.Toast.Anchor"))
	TMap<FGameplayTag, int32> ToastMaxVisiblePerAnchor;

	/**
	 * Timer for WBP intro and outro animations.
	 * 
	 * When a toast sets bAwaitIntroAnimation or bAwaitOutroAnimation
	 * but never calls Notify*Complete, Subsystem forces completion
	 * after this long to free anchor slot.
	 */
	UPROPERTY(Config, EditAnywhere, Category="MaevixCore|UI|Toast",
		meta=(ClampMin="1.0", ClampMax="30.0", Units="s"))
	float AwaitAnimationTimeoutSeconds{5.0f};

	// ============================================================================
	// DEBUG (EDITOR ONLY)
	// ============================================================================

#if WITH_EDITORONLY_DATA
	/**
	 * Log all event broadcasts and subscriptions to Output Log.
	 * Shows: Event tag, payload data, subscriber count, broadcast scope.
	 */
	UPROPERTY(Config, EditAnywhere, Category="MaevixCore|Debug", meta=(DisplayName="Enable Event System Logging"))
	bool bEnableEventSystemLogging{false};

	/**
	 * Show debug overlay for UI layer stack.
	 * Helps debug: Widget not appearing, wrong layer, input not working.
	 */
	UPROPERTY(Config, EditAnywhere, Category="MaevixCore|Debug", meta=(DisplayName="Show UI Layer Debug Overlay"))
	bool bShowUILayerDebugOverlay{false};
#endif

	// ============================================================================
	// PIE
	// ============================================================================

	/**
	 * Whether GameUserSettings changes that mutate host process's
	 * should apply when running in PIE.
	 */
	UPROPERTY(EditAnywhere, config, Category = "MaevixCore|PIE",
		meta = (DisplayName = "Apply Display Settings in PIE"))
	bool bApplyDisplaySettingsInPIE{false};

	/**
	 * Whether GameUserSettings setting changes that mutate
	 * renderer state should apply when running in PIE.
	 */
	UPROPERTY(EditAnywhere, config, Category = "MaevixCore|PIE",
		meta = (DisplayName = "Apply Scalability Settings in PIE"))
	bool bApplyScalabilitySettingsInPIE{false};

	// ============================================================================
	// HELPERS
	// ============================================================================

	/** Returns true if event system logging is enabled. False in shipping builds. */
	UFUNCTION(BlueprintPure, Category="MaevixCore|Debug")
	bool IsEventLoggingEnabled() const;

	/** Returns true if UI debug overlay is enabled. False in shipping builds. */
	UFUNCTION(BlueprintPure, Category="MaevixCore|Debug")
	bool IsUIDebugOverlayEnabled() const;

	/** Returns theme DataAsset at DefaultThemeIndex (loads synchronously). nullptr if index is invalid. */
	UFUNCTION(BlueprintCallable, Category="MaevixCore|Theme")
	UMCore_PDA_UITheme_Base* GetDefaultTheme() const;
	
	// ============================================================================
	// SETTINGS AGGREGATION HELPERS
	// ============================================================================

	/** Returns all loaded settings collections. Resolves soft references on first call, caches results. */
	UFUNCTION(BlueprintPure, Category = "MaevixCore|Settings")
	const TArray<UMCore_DA_SettingsCollection*>& GetAllSettingsCollections() const;

	/** Searches all collections for a setting definition matching passed tag. Returns first match. */
	UFUNCTION(BlueprintPure, Category = "MaevixCore|Settings")
	UMCore_DA_SettingDefinition* FindSettingDefinitionByTag(const FGameplayTag& SettingTag) const;

	/**
	 * Returns all settings across all collections for a category, ordered by collection
	 * array index followed by setting array position within each collection.
	 */
	UFUNCTION(BlueprintPure, Category = "MaevixCore|Settings")
	TArray<UMCore_DA_SettingDefinition*> GetSettingsForCategory(const FGameplayTag& CategoryTag) const;

	/**
	 * Returns all unique category tags across all collections in first-seen order
	 * (collection array index > setting array position)
	 */
	UFUNCTION(BlueprintPure, Category = "MaevixCore|Settings")
	TArray<FGameplayTag> GetAllSettingsCategories() const;

	/** Searches all collections for a category display name. Returns tag leaf segment as fallback. */
	FText GetCategoryDisplayName(const FGameplayTag& CategoryTag) const;

	/** Returns true if at least one collection is assigned and loadable. */
	UFUNCTION(BlueprintPure, Category = "MaevixCore|Settings")
	bool HasValidSettingsCollections() const;

	/** Clears resolved collection cache. Next GetAllSettingsCollections() call will re-resolve. */
	void InvalidateCollectionCache();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// ============================================================================
	// THEME HELPERS
	// ============================================================================

	/**
	 * Returns default theme for design-time preview (static, no UISubsystem needed).
	 * Use in NativePreConstruct() for UMG editor preview.
	 * 
	 * Use UISubsystem->GetActiveTheme() at runtime.
	 */
	UFUNCTION(BlueprintPure, Category="MaevixCore|Theme")
	static UMCore_PDA_UITheme_Base* GetDesignTimeTheme();

	UFUNCTION(BlueprintPure, Category="MaevixCore|Theme")
	bool IsValidThemeIndex(int32 Index) const;

};
