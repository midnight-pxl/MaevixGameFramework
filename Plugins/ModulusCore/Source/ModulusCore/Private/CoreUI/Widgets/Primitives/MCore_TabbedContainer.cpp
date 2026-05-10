// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/Widgets/Primitives/MCore_TabbedContainer.h"

#include "CoreData/Logging/LogModulusUI.h"
#include "CoreUI/Widgets/Primitives/MCore_ActionButton.h"

#include "CommonButtonBase.h"
#include "CommonTabListWidgetBase.h"
#include "CommonAnimatedSwitcher.h"

UMCore_TabbedContainer::UMCore_TabbedContainer()
{
}

void UMCore_TabbedContainer::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UE_LOG(LogModulusUI, Verbose, TEXT("TabbedContainer::NativeOnInitialized: this=%p"), this);

	TabList->OnTabSelected.AddDynamic(this, &ThisClass::HandleTabSelected);

	if (Btn_TabPrev) { Btn_TabPrev->SetDisplayMode(EMCore_ButtonDisplayMode::IconOnly); }
	if (Btn_TabNext) { Btn_TabNext->SetDisplayMode(EMCore_ButtonDisplayMode::IconOnly); }
}

void UMCore_TabbedContainer::NativeDestruct()
{
	UE_LOG(LogModulusUI, Verbose, TEXT("TabbedContainer::NativeDestruct: this=%p, TabCount=%d"), this, GetTabCount());

	if (TabList)
	{
		TabList->OnTabSelected.RemoveAll(this);
	}

	Super::NativeDestruct();
}

bool UMCore_TabbedContainer::AddTab(FName TabID, UWidget* PageWidget)
{
	if (TabID.IsNone())
	{
		UE_LOG(LogModulusUI, Warning, TEXT("TabbedContainer::AddTab: invalid TabID (NAME_None)"));
		return false;
	}

	if (!PageWidget)
	{
		UE_LOG(LogModulusUI, Warning, TEXT("TabbedContainer::AddTab: PageWidget is null for tab '%s'"),
			*TabID.ToString());
		return false;
	}

	if (PageWidgets.Contains(TabID))
	{
		UE_LOG(LogModulusUI, Warning, TEXT("TabbedContainer::AddTab: tab '%s' already exists, skipping duplicate"),
			*TabID.ToString());
		return false;
	}

	TSubclassOf<UCommonButtonBase> ButtonClass = TabButtonClass;
	if (!ButtonClass)
	{
		ButtonClass = UCommonButtonBase::StaticClass();
		UE_LOG(LogModulusUI, Warning,
			TEXT("TabbedContainer::AddTab: TabButtonClass not set, using framework default"));
	}

	PageWidgets.Add(TabID, PageWidget);
	TabOrder.Add(TabID);
	PageSwitcher->AddChild(PageWidget);
	TabList->RegisterTab(TabID, ButtonClass, PageWidget, PageWidgets.Num() - 1);
	
	if (UCommonButtonBase* SpawnedTabButton = TabList->GetTabButtonBaseByID(TabID))
	{
		if (UMCore_ButtonBase* MCoreTabButton = Cast<UMCore_ButtonBase>(SpawnedTabButton))
		{
			MCoreTabButton->SetStyleMode(EMCore_ButtonStyleMode::Tab);
		}
		else
		{
			UE_LOG(LogModulusUI, Warning,
				TEXT("TabbedContainer::AddTab: TabButtonClass not a UMCore_ButtonBase subclass; tab '%s' will use default text styling."),
				*TabID.ToString());
		}
	}

	if (UCommonButtonBase* TabButton = TabList->GetTabButtonBaseByID(TabID))
	{
		OnTabAdded.Broadcast(TabID, TabButton);
	}

	UE_LOG(LogModulusUI, Verbose, TEXT("TabbedContainer::AddTab: added tab '%s' (total: %d)"),
		*TabID.ToString(), PageWidgets.Num());

	return true;
}

bool UMCore_TabbedContainer::RemoveTab(FName TabID)
{
	if (!PageWidgets.Contains(TabID))
	{
		UE_LOG(LogModulusUI, Warning, TEXT("TabbedContainer::RemoveTab: tab '%s' not found"),
			*TabID.ToString());
		return false;
	}

	/* Store remaining tabs preserving insertion order via iteration */
	TArray<TPair<FName, TObjectPtr<UWidget>>> RemainingTabs;
	for (auto& Pair : PageWidgets)
	{
		if (Pair.Key != TabID)
		{
			RemainingTabs.Add(Pair);
		}
	}

	/* Clear and rebuild to maintain correct indices */
	TabList->RemoveAllTabs();
	PageSwitcher->ClearChildren();
	PageWidgets.Empty();
	TabOrder.Reset();
	SelectedTabID = NAME_None;

	TSubclassOf<UCommonButtonBase> ButtonClass = TabButtonClass;
	if (!ButtonClass)
	{
		ButtonClass = UCommonButtonBase::StaticClass();
	}

	for (int32 Index = 0; Index < RemainingTabs.Num(); ++Index)
	{
		const auto& Pair = RemainingTabs[Index];
		PageSwitcher->AddChild(Pair.Value);
		TabList->RegisterTab(Pair.Key, ButtonClass, Pair.Value, Index);
		PageWidgets.Add(Pair.Key, Pair.Value);
		TabOrder.Add(Pair.Key);
	}

	if (RemainingTabs.Num() > 0)
	{
		TabList->SelectTabByID(RemainingTabs[0].Key);
	}

	UE_LOG(LogModulusUI, Log, TEXT("TabbedContainer::RemoveTab: removed tab '%s' (remaining: %d)"),
		*TabID.ToString(), PageWidgets.Num());

	return true;
}

void UMCore_TabbedContainer::ClearAllTabs()
{
	UE_LOG(LogModulusUI, Verbose, TEXT("TabbedContainer::ClearAllTabs: this=%p, TabCount=%d"), this, GetTabCount());

	/* Self-heal: if this container went through NativeDestruct and was reused
	 * without NativeOnInitialized firing again, the internal delegate is dead. */
	if (TabList && !TabList->OnTabSelected.IsBound())
	{
		TabList->OnTabSelected.AddDynamic(this, &ThisClass::HandleTabSelected);
	}

	TabList->RemoveAllTabs();
	PageSwitcher->ClearChildren();
	PageWidgets.Empty();
	TabOrder.Reset();
	SelectedTabID = NAME_None;
}

bool UMCore_TabbedContainer::SelectTab(FName TabID)
{
	if (!PageWidgets.Contains(TabID))
	{
		UE_LOG(LogModulusUI, Warning, TEXT("TabbedContainer::SelectTab: tab '%s' not found"),
			*TabID.ToString());
		return false;
	}

	TabList->SelectTabByID(TabID);
	SelectedTabID = TabID;

	return true;
}

void UMCore_TabbedContainer::SelectNextTab()
{
	if (TabOrder.Num() <= 1) { return; }

	const int32 CurrentIndex = TabOrder.IndexOfByKey(SelectedTabID);
	if (CurrentIndex == INDEX_NONE) { return; }

	const int32 NextIndex = (CurrentIndex + 1) % TabOrder.Num();
	SelectTab(TabOrder[NextIndex]);
}

void UMCore_TabbedContainer::SelectPreviousTab()
{
	if (TabOrder.Num() <= 1) { return; }

	const int32 CurrentIndex = TabOrder.IndexOfByKey(SelectedTabID);
	if (CurrentIndex == INDEX_NONE) { return; }

	const int32 PrevIndex = (CurrentIndex - 1 + TabOrder.Num()) % TabOrder.Num();
	SelectTab(TabOrder[PrevIndex]);
}

void UMCore_TabbedContainer::SetTabCycleActions(
	const FDataTableRowHandle& PrevAction, const FDataTableRowHandle& NextAction)
{
	if (Btn_TabPrev) { Btn_TabPrev->SetInputAction(PrevAction); }
	if (Btn_TabNext) { Btn_TabNext->SetInputAction(NextAction); }
}

void UMCore_TabbedContainer::SetTabListVisibility(ESlateVisibility InVisibility)
{
	if (TabList)
	{
		TabList->SetVisibility(InVisibility);
	}
}

void UMCore_TabbedContainer::HandleTabSelected(FName TabNameID)
{
	TObjectPtr<UWidget>* FoundWidget = PageWidgets.Find(TabNameID);
	if (!FoundWidget || !*FoundWidget)
	{
		UE_LOG(LogModulusUI, Warning, TEXT("TabbedContainer::HandleTabSelected: tab '%s' not found in cache"),
			*TabNameID.ToString());
		return;
	}

	if (!PageSwitcher->HasChild(*FoundWidget))
	{
		UE_LOG(LogModulusUI, Error,
			TEXT("TabbedContainer::HandleTabSelected: widget for tab '%s' is not a child of PageSwitcher"),
			*TabNameID.ToString());
		return;
	}

	PageSwitcher->SetActiveWidget(*FoundWidget);
	SelectedTabID = TabNameID;
	OnTabSelected.Broadcast(TabNameID);

	UE_LOG(LogModulusUI, Verbose, TEXT("TabbedContainer::HandleTabSelected: switched to tab '%s'"),
		*TabNameID.ToString());
}

UWidget* UMCore_TabbedContainer::GetPageWidget(FName TabID) const
{
	const TObjectPtr<UWidget>* Found = PageWidgets.Find(TabID);
	return Found ? *Found : nullptr;
}

UCommonButtonBase* UMCore_TabbedContainer::GetTabButton(FName TabID) const
{
	return TabList->GetTabButtonBaseByID(TabID);
}

bool UMCore_TabbedContainer::SetTabEnabled(FName TabID, bool bEnabled)
{
	if (!PageWidgets.Contains(TabID))
	{
		UE_LOG(LogModulusUI, Warning, TEXT("TabbedContainer::SetTabEnabled: tab '%s' not found"),
			*TabID.ToString());
		return false;
	}

	TabList->SetTabEnabled(TabID, bEnabled);
	return true;
}

bool UMCore_TabbedContainer::SetTabHidden(FName TabID, bool bIsHidden)
{
	if (!PageWidgets.Contains(TabID))
	{
		UE_LOG(LogModulusUI, Warning, TEXT("TabbedContainer::SetTabHidden: tab '%s' not found"),
			*TabID.ToString());
		return false;
	}

	TabList->SetTabVisibility(TabID, bIsHidden ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	return true;
}

bool UMCore_TabbedContainer::IsTabEnabled(FName TabID) const
{
	UCommonButtonBase* TabButton = TabList->GetTabButtonBaseByID(TabID);
	if (!TabButton) { return false; }

	return TabButton->GetIsEnabled();
}

bool UMCore_TabbedContainer::IsTabHidden(FName TabID) const
{
	UCommonButtonBase* TabButton = TabList->GetTabButtonBaseByID(TabID);
	if (!TabButton) { return false; }

	return !TabButton->IsVisible();
}
