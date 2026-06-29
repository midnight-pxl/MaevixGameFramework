// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "GameplayTagContainer.h"
#include "CoreUI/Widgets/MCore_PrimaryGameLayout.h"
#include "CoreData/Types/UI/MCore_MenuTabTypes.h"
#include "CoreData/Types/UI/MCore_ThemeTypes.h"
#include "MCore_UISubsystem.generated.h"

class UCommonActivatableWidgetStack;
class UCommonActivatableWidget;
class UMCore_PDA_UITheme_Base;
class UMCore_GameMenuHub;
class UTexture2D;
struct FGameplayTag;
struct FMCore_EventData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThemeChanged, UMCore_PDA_UITheme_Base*, NewTheme);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPrimaryGameLayoutReady, UMCore_PrimaryGameLayout*, Layout);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWidgetLayerChanged, UCommonActivatableWidget*, Widget, FGameplayTag, LayerTag);


/**
 * Central access point for MaevixCore UI systems. One instance per LocalPlayer (split-screen safe).
 *
 * Manages PrimaryGameLayout lifecycle, 4-layer widget stack access via gameplay tags,
 * theme distribution, and menu hub orchestration.
 */
UCLASS()
class MAEVIXCORE_API UMCore_UISubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem Interface

	UFUNCTION(BlueprintPure, Category = "MaevixCore|UI|Layout")
	bool HasPrimaryGameLayout() const
	{
		return IsValid(PrimaryGameLayout) && PrimaryGameLayout->AreAllLayersBound();
	}

	/** The PrimaryGameLayout if created, else nullptr. Lets sibling subsystems (e.g. the
	 *  toast service) attach to it, including on the already-ready late-init path. */
	UFUNCTION(BlueprintPure, Category = "MaevixCore|UI|Layout")
	UMCore_PrimaryGameLayout* GetPrimaryGameLayout() const { return PrimaryGameLayout; }

	UPROPERTY(BlueprintAssignable, Category = "MaevixCore|UI|Events")
	FOnPrimaryGameLayoutReady OnPrimaryGameLayoutReady;
	
// ============================================================================
// SCREEN MANAGEMENT
// ============================================================================

	/** Opens a screen on the specified layer. Returns existing instance if already active (dedup by default). */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|UI", meta = (DeterminesOutputType = "ScreenClass"))
	UCommonActivatableWidget* OpenScreen(TSubclassOf<UCommonActivatableWidget> ScreenClass,
		UPARAM(meta = (Categories = "MCore.UI.Layer")) FGameplayTag LayerTag,
		bool bAllowDuplicates = false);

	/** Closes a screen: untracks it and deactivates it. */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|UI")
	void CloseScreen(UCommonActivatableWidget* Screen);

	/** Pops the top widget from the specified layer stack. Returns true if a widget was closed. */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|UI")
	bool PopLayer(UPARAM(meta = (Categories = "MCore.UI.Layer")) FGameplayTag LayerTag);

	/** Removes a specific widget from a specific layer. Returns true if found and closed. */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|UI")
	bool RemoveWidgetFromLayer(UCommonActivatableWidget* Widget,
		UPARAM(meta = (Categories = "MCore.UI.Layer")) FGameplayTag LayerTag);

	/** Returns the top active widget on the specified layer, or nullptr. */
	UFUNCTION(BlueprintPure, Category = "MaevixCore|UI")
	UCommonActivatableWidget* GetActiveWidgetInLayer(UPARAM(meta = (Categories = "MCore.UI.Layer")) FGameplayTag LayerTag) const;

	/** Returns the number of tracked widgets on the specified layer. */
	UFUNCTION(BlueprintPure, Category = "MaevixCore|UI")
	int32 GetWidgetCountInLayer(UPARAM(meta = (Categories = "MCore.UI.Layer")) FGameplayTag LayerTag) const;

	/** Returns true if the layer's stack has any tracked, valid widget. */
	UFUNCTION(BlueprintPure, Category = "MaevixCore|UI")
	bool IsLayerActive(UPARAM(meta = (Categories = "MCore.UI.Layer")) FGameplayTag LayerTag) const;

	UPROPERTY(BlueprintAssignable, Category = "MaevixCore|UI|Events")
	FOnWidgetLayerChanged OnWidgetPushed;

	UPROPERTY(BlueprintAssignable, Category = "MaevixCore|UI|Events")
	FOnWidgetLayerChanged OnWidgetRemoved;

	void NotifyWidgetDestroyed(UCommonActivatableWidget* Widget);

// ============================================================================
// MENU HUB
// ============================================================================

	/** Opens the GameMenuHub on the GameMenu layer. Returns existing if already active. */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|Menu Hub")
	UMCore_GameMenuHub* OpenMenuHub();

	/** Closes the GameMenuHub if it's currently on the GameMenu layer. */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|Menu Hub")
	void CloseMenuHub();

	/** Register a menu screen tab in the MenuHub. Duplicate TabIDs are rejected. */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|Menu Hub")
	void RegisterMenuScreen(
		FGameplayTag TabID,
		TSubclassOf<UCommonActivatableWidget> ScreenWidgetClass,
		int32 Priority = 100,
		UTexture2D* TabIcon = nullptr);

	/** Unregister a menu screen tab. Returns true if found and removed. */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|Menu Hub")
	bool UnregisterMenuScreen(FGameplayTag TabID);

	UFUNCTION(BlueprintPure, Category = "MaevixCore|Menu Hub")
	const TArray<FMCore_MenuTab>& GetRegisteredMenuScreens() const { return RegisteredMenuScreens; }

	/** Force rebuild of MenuHub tab bar. Auto-called when screens registered/unregistered while hub is active. */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|Menu Hub")
	void RebuildMenuHubTabBar();
	
// ============================================================================
// THEME
// ============================================================================

	UFUNCTION(BlueprintPure, Category = "MaevixCore|Theme")
	UMCore_PDA_UITheme_Base* GetActiveTheme() const { return CachedActiveTheme; }
	
	UPROPERTY(BlueprintAssignable, Category = "MaevixCore|Theme")
	FOnThemeChanged OnThemeChanged;

	UFUNCTION(BlueprintPure, Category = "MaevixCore|Theme")
	const TArray<FMCore_ThemeEntry>& GetAvailableThemes() const;

	UFUNCTION(BlueprintPure, Category = "MaevixCore|Theme")
	int32 GetActiveThemeIndex() const { return ActiveThemeIndex; }

	UFUNCTION(BlueprintCallable, Category = "MaevixCore|Theme")
	bool SetActiveThemeByIndex(int32 ThemeIndex);

	/**
	 * Apply NewTheme as the active theme. Broadcasts OnThemeChanged and the
	 * MCore.Theme.Changed local event tag, then persists to UMCore_PlayerSettingsSave.
	 * No-op if NewTheme is null or already active.
	 */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|Theme")
	void SetActiveTheme(UMCore_PDA_UITheme_Base* NewTheme);

	/** Re-broadcasts OnThemeChanged so all widgets re-resolve text styles at the new size index. */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|Theme")
	void NotifyTextSizeChanged();

protected:
	/** Widget class for PrimaryGameLayout. Set in project defaults or override in Blueprint. */
	UPROPERTY(EditDefaultsOnly, Category = "MaevixCore|UI")
	TSubclassOf<UMCore_PrimaryGameLayout> PrimaryGameLayoutClass;
	
	/** Widget class for MenuHub (loaded from settings or defaults) */
	UPROPERTY()
	TSubclassOf<UMCore_GameMenuHub> MenuHubClass;

	/** Z-order for layout when added to viewport */
	UPROPERTY(EditDefaultsOnly, Category = "MaevixCore|UI", meta = (ClampMin = "-100", ClampMax = "100"))
	int32 PrimaryGameLayoutZOrder = 0;
	
	/**
	 * Called after PrimaryGameLayout is successfully created and added to viewport.
	 * Override in C++ or Blueprint subclasses for custom initialization.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "MaevixCore|UI")
	void OnPrimaryGameLayoutCreated(UMCore_PrimaryGameLayout* Layout);
	virtual void OnPrimaryGameLayoutCreated_Implementation(UMCore_PrimaryGameLayout* Layout);

private:
	void LoadWidgetClasses();
	void BuildLayerStackMap();
	void CompactTrackedWidgets(FGameplayTag LayerTag);

	UCommonActivatableWidgetStack* GetLayerStack(FGameplayTag LayerTag) const;
	UCommonActivatableWidget* PushWidgetToLayer(TSubclassOf<UCommonActivatableWidget> WidgetClass, FGameplayTag LayerTag);
	void UntrackWidget(UCommonActivatableWidget* Widget, FGameplayTag LayerTag);
	UMCore_GameMenuHub* FindTrackedMenuHub() const;

	/* Creates and adds PrimaryGameLayout to viewport */
	void CreatePrimaryGameLayout();
	/* Deferred layout creation once PlayerController is ready */
	void OnPlayerControllerReady(APlayerController* OwningPlayer);
	
	void HandleLocalEvent(const FMCore_EventData& EventData);

	/* Resolves a saved theme from the player save. Returns nullptr if no save
	 * exists, ActiveThemePath is empty, or the asset failed to load. */
	UMCore_PDA_UITheme_Base* LoadSavedActiveTheme() const;

	/* Writes NewTheme's path to the player save and flushes to disk. */
	void PersistActiveTheme(UMCore_PDA_UITheme_Base* NewTheme);

	/* Broadcasts MCore.Theme.Changed via the local event subsystem, payload
	 * carries ThemePath. Cross-plugin subscribers can react without coupling
	 * to UISubsystem directly. */
	void BroadcastThemeChangedTagEvent(UMCore_PDA_UITheme_Base* NewTheme);

	FDelegateHandle PlayerControllerReadyHandle;
	FDelegateHandle LocalEventHandle;
	
	/** Strong reference; UISubsystem owns the layout lifecycle */
	UPROPERTY(Transient)
	TObjectPtr<UMCore_PrimaryGameLayout> PrimaryGameLayout;
	
	UPROPERTY(Transient)
	TObjectPtr<UMCore_PDA_UITheme_Base> CachedActiveTheme;

	int32 ActiveThemeIndex{INDEX_NONE};
	
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetStack>> LayerStackMap;

	/* Widgets pushed via PushWidgetToLayer, tracked per-layer with weak refs */
	TMap<FGameplayTag, TArray<TWeakObjectPtr<UCommonActivatableWidget>>> TrackedWidgets;

	/** Registered menu screens for this local player */
	UPROPERTY(Transient)
	TArray<FMCore_MenuTab> RegisteredMenuScreens;
};
