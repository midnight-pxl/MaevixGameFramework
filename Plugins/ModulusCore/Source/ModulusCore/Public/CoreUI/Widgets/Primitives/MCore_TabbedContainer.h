// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "MCore_TabbedContainer.generated.h"

class UCommonButtonBase;
class UCommonAnimatedSwitcher;
class UCommonTabListWidgetBase;
class UMCore_ActionButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTabbedContainerTabAdded, FName, TabID, UCommonButtonBase*, TabButton);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTabbedContainerTabSelected, FName, TabID);

/**
 * Reusable tabbed container wrapping CommonUI's tab list and animated switcher.
 * Caller owns page widget creation and lifecycle.
 *
 * Requires BindWidget: TabList (UCommonTabListWidgetBase), PageSwitcher (UCommonAnimatedSwitcher).
 * Set TabButtonClass to your tab button Blueprint, then call AddTab() to populate.
 */
UCLASS(Abstract, Blueprintable, BlueprintType, meta=(DisableNativeTick))
class MODULUSCORE_API UMCore_TabbedContainer : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UMCore_TabbedContainer();

	// ============================================================================
	// CORE API
	// ============================================================================

	/** Add a tab with a pre-created page widget. Tabs display in insertion order. Returns true on success. */
	UFUNCTION(BlueprintCallable, Category = "ModulusCore|UI|TabbedContainer")
	bool AddTab(FName TabID, UWidget* PageWidget);

	/** Remove a tab and its page. Does NOT destroy the page widget. Returns true if found. */
	UFUNCTION(BlueprintCallable, Category = "ModulusCore|UI|TabbedContainer")
	bool RemoveTab(FName TabID);

	/**
	 * Remove all tabs and pages. Does NOT destroy page widgets.
	 */
	UFUNCTION(BlueprintCallable, Category = "ModulusCore|UI|TabbedContainer")
	void ClearAllTabs();

	/** Select a tab by ID, switching to its page. Returns true if found. */
	UFUNCTION(BlueprintCallable, Category = "ModulusCore|UI|TabbedContainer")
	bool SelectTab(FName TabID);

	/** Select the next tab in insertion order, wrapping from last to first. */
	UFUNCTION(BlueprintCallable, Category = "ModulusCore|UI|TabbedContainer")
	void SelectNextTab();

	/** Select the previous tab in insertion order, wrapping from first to last. */
	UFUNCTION(BlueprintCallable, Category = "ModulusCore|UI|TabbedContainer")
	void SelectPreviousTab();

	// ============================================================================
	// QUERIES
	// ============================================================================

	UFUNCTION(BlueprintPure, Category = "ModulusCore|UI|TabbedContainer")
	FName GetSelectedTabID() const { return SelectedTabID; }

	UFUNCTION(BlueprintPure, Category = "ModulusCore|UI|TabbedContainer")
	int32 GetTabCount() const { return PageWidgets.Num(); }

	UFUNCTION(BlueprintPure, Category = "ModulusCore|UI|TabbedContainer")
	bool HasTab(FName TabID) const { return PageWidgets.Contains(TabID); }

	/** Returns the page widget for a tab, or nullptr if not found. */
	UFUNCTION(BlueprintPure, Category = "ModulusCore|UI|TabbedContainer")
	UWidget* GetPageWidget(FName TabID) const;

	/** Returns the tab button for a tab, or nullptr if not found. */
	UFUNCTION(BlueprintPure, Category = "ModulusCore|UI|TabbedContainer")
	UCommonButtonBase* GetTabButton(FName TabID) const;

	UFUNCTION(BlueprintPure, Category = "ModulusCore|UI|TabbedContainer")
	UCommonAnimatedSwitcher* GetPageSwitcher() const { return PageSwitcher; }

	/** Returns tab IDs in insertion order. */
	const TArray<FName>& GetTabOrder() const { return TabOrder; }

	// ============================================================================
	// FLANKING ACTION ICONS
	// ============================================================================

	/** Configure the optional tab-cycle icon buttons with input actions for platform glyph display. */
	UFUNCTION(BlueprintCallable, Category = "ModulusCore|UI|TabbedContainer")
	void SetTabCycleActions(const FDataTableRowHandle& PrevAction, const FDataTableRowHandle& NextAction);

	/**
	 * Sets the TabList's visibility. Use Hidden (not Collapsed) to reserve layout space
	 * when the tab list should be invisible but the page area should not shift.
	 */
	UFUNCTION(BlueprintCallable, Category = "ModulusCore|UI|TabbedContainer")
	void SetTabListVisibility(ESlateVisibility InVisibility);

	// ============================================================================
	// TAB STATE
	// ============================================================================

	UFUNCTION(BlueprintCallable, Category = "ModulusCore|UI|TabbedContainer")
	bool SetTabEnabled(FName TabID, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "ModulusCore|UI|TabbedContainer")
	bool SetTabHidden(FName TabID, bool bIsHidden);

	UFUNCTION(BlueprintPure, Category = "ModulusCore|UI|TabbedContainer")
	bool IsTabEnabled(FName TabID) const;

	UFUNCTION(BlueprintPure, Category = "ModulusCore|UI|TabbedContainer")
	bool IsTabHidden(FName TabID) const;

	// ============================================================================
	// EVENTS
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "ModulusCore|UI|TabbedContainer|Events")
	FOnTabbedContainerTabAdded OnTabAdded;

	UPROPERTY(BlueprintAssignable, Category = "ModulusCore|UI|TabbedContainer|Events")
	FOnTabbedContainerTabSelected OnTabSelected;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTabListWidgetBase> TabList;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonAnimatedSwitcher> PageSwitcher;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ModulusCore|UI|TabbedContainer")
	TSubclassOf<UCommonButtonBase> TabButtonClass;

	/** Optional icon button showing the "previous tab" input glyph. Display-only. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UMCore_ActionButton> Btn_TabPrev;

	/** Optional icon button showing the "next tab" input glyph. Display-only. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UMCore_ActionButton> Btn_TabNext;

private:
	UFUNCTION()
	void HandleTabSelected(FName TabNameID);

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UWidget>> PageWidgets;

	TArray<FName> TabOrder;

	FName SelectedTabID;
};
