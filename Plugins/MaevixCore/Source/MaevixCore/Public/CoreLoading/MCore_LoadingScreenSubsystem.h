// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CoreData/Types/Loading/MCore_LoadingScreenTypes.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MCore_LoadingScreenSubsystem.generated.h"

class UMCore_LoadingScreenWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadingScreenReadyToDismiss);

/**
 * Game-instance subsystem that drives UE's MoviePlayer for level-transition loading screens.
 * Binds PreLoadMap / PostLoadMapWithWorld, selects an FMCore_LoadingScreenEntry per the
 * configured strategy, and hands the resulting widget to MoviePlayer. Skipped on dedicated
 * servers. See EMCore_LoadingDismissalMode for dismissal semantics; access the live widget
 * during the input-wait phase via GetActiveLoadingScreenWidget().
 *
 * Known engine limitation (UE-207172): with bAllowEngineTick=true, FApp::GetDeltaTime()
 * returns the entire load duration on the first tick after the loading screen ends.
 * Downstream consumers that integrate DeltaTime on the first post-load tick (Sequencers,
 * time-based VFX) should discard or clamp it.
 */
UCLASS(Config=MaevixCore)
class MAEVIXCORE_API UMCore_LoadingScreenSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ============================================================================
	// PUBLIC API
	// ============================================================================

	/**
	 * Provides a context hint that influences entry selection on the next transition.
	 * Cleared automatically after the next PostLoadMapWithWorld. Has effect only in Tag-Based mode.
	 */
	UFUNCTION(BlueprintCallable, Category="MaevixCore|Loading")
	void SetNextLoadingContext(const FGameplayTagContainer& ContextTags);

	/**
	 * One-shot override that bypasses the selection strategy entirely.
	 * Highest priority; consumed on the next transition.
	 */
	UFUNCTION(BlueprintCallable, Category="MaevixCore|Loading")
	void OverrideNextLoadingScreen(const FMCore_LoadingScreenEntry& Entry);

	/** Globally enables or disables the loading screen. Initial value comes from CoreSettings. */
	UFUNCTION(BlueprintCallable, Category="MaevixCore|Loading")
	void SetLoadingScreenEnabled(bool bEnabled);

	/**
	 * Manually dismisses the active loading screen. Only effective when
	 * SetNextTransitionManualDismiss(true) was called before the transition.
	 */
	UFUNCTION(BlueprintCallable, Category="MaevixCore|Loading")
	void DismissLoadingScreen();

	/**
	 * Configures whether the next transition waits for an explicit DismissLoadingScreen() call
	 * instead of auto-completing when the level finishes loading. Cleared after the next transition.
	 * Use for async post-load work that must finish before the loading screen disappears.
	 */
	UFUNCTION(BlueprintCallable, Category="MaevixCore|Loading")
	void SetNextTransitionManualDismiss(bool bManualDismiss);

	/**
	 * Selects the loading screen entry for the next transition. Subclasses override this
	 * (Selection Mode = Custom) to implement project-specific logic. Default behavior
	 * implements the four built-in selection modes.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="MaevixCore|Loading")
	FMCore_LoadingScreenEntry SelectLoadingScreenEntry(FName NextMapName);
	virtual FMCore_LoadingScreenEntry SelectLoadingScreenEntry_Implementation(FName NextMapName);

	/**
	 * Returns the loading screen widget currently displayed, or nullptr if no
	 * loading screen is active. Use during the input-wait phase (after
	 * OnLoadingScreenReadyToDismiss fires) to update prompt copy or icon from
	 * a custom coordination layer.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="MaevixCore|Loading")
	UMCore_LoadingScreenWidget* GetActiveLoadingScreenWidget() const;

	/**
	 * Broadcast when the level finishes loading and the configured dismissal mode
	 * is InputRequired. Subscribers can update widget state (prompt text, icon)
	 * or start coordination logic before the user dismisses the screen.
	 */
	UPROPERTY(BlueprintAssignable, Category="MaevixCore|Loading")
	FOnLoadingScreenReadyToDismiss OnLoadingScreenReadyToDismiss;

protected:
	// ============================================================================
	// LIFECYCLE
	// ============================================================================

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	// ============================================================================
	// INTERNAL HANDLERS
	// ============================================================================

	void HandlePreLoadMap(const FString& MapName);
	void HandlePostLoadMap(UWorld* LoadedWorld);

	/* Builds the aggregated entry pool by sync-loading every configured library. */
	TArray<FMCore_LoadingScreenEntry> ResolveEntryPool() const;

	// ============================================================================
	// STATE
	// ============================================================================

	bool bLoadingScreenEnabled{true};

	FGameplayTagContainer PendingContextTags;

	TOptional<FMCore_LoadingScreenEntry> PendingOverrideEntry;

	bool bNextTransitionManualDismiss{false};

	/* Cached after HandlePreLoadMap so HandlePostLoadMap and DismissLoadingScreen
	   can branch without re-reading settings. bNextTransitionManualDismiss
	   overrides the setting and is folded into this value. */
	EMCore_LoadingDismissalMode CurrentDismissalMode{EMCore_LoadingDismissalMode::AutoOnLoadComplete};

	/* True between PostLoadMapWithWorld and DismissLoadingScreen in InputRequired mode.
	   Widget native input handlers gate on the widget's mirror of this flag. */
	bool bAwaitingDismissalInput{false};

	/* Weak ref so the subsystem can drive widget lifecycle (HandleLoadingComplete)
	   and so external code can fetch the live widget to mutate its prompt UX. */
	TWeakObjectPtr<UMCore_LoadingScreenWidget> ActiveWidget;

	/* Cursor for Sequential selection mode. In-memory only; resets on game restart. */
	int32 LastSelectedIndex{INDEX_NONE};

	FDelegateHandle PreLoadMapHandle;
	FDelegateHandle PostLoadMapHandle;
};
