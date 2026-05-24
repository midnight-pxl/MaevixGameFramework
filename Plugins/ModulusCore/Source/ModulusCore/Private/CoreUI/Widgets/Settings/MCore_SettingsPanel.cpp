// Copyright Midnight Pixel Studio LLC. All Rights Reserved.

#include "CoreUI/Widgets/Settings/MCore_SettingsPanel.h"

#include "CoreData/Assets/UI/Themes/MCore_PDA_UITheme_Base.h"
#include "CoreData/DevSettings/MCore_CoreSettings.h"
#include "CoreData/Libraries/MCore_GameSettingsLibrary.h"
#include "CoreData/Libraries/MCore_ThemeLibrary.h"
#include "CoreData/Types/Settings/MCore_DA_SettingDefinition.h"
#include "CoreData/Types/Settings/MCore_DA_SettingsCollection.h"
#include "CoreData/Types/Settings/MCore_SettingsTypes.h"
#include "CoreUI/MCore_UISubsystem.h"
#include "CoreUI/Widgets/Primitives/MCore_ActionButton.h"
#include "CoreUI/Widgets/Primitives/MCore_ButtonBase.h"
#include "CoreUI/Widgets/Primitives/MCore_TabbedContainer.h"
#include "CoreUI/Widgets/Settings/MCore_SettingsWidget_Base.h"
#include "CoreUI/Widgets/Settings/MCore_SettingsWidget_Slider.h"
#include "CoreUI/Widgets/Settings/MCore_SettingsWidget_Switcher.h"
#include "CoreUI/Widgets/KeyBindings/MCore_KeyBindingPanel_Base.h"
#include "CoreUI/Widgets/Primitives/MCore_ConfirmationDialog.h"
#include "CoreUI/Widgets/Settings/MCore_SettingsRevertCountdown.h"
#include "CoreData/Logging/LogModulusSettings.h"
#include "CoreData/Tags/MCore_UILayerTags.h"

#include "CommonTextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/SizeBox.h"

// ============================================================================
// FILE-LOCAL HELPERS
// ============================================================================

namespace
{
	FGameplayTag GetParentTag(const FGameplayTag& Tag)
	{
		const FString TagStr = Tag.ToString();
		int32 LastDot;
		if (TagStr.FindLastChar(TEXT('.'), LastDot))
		{
			return FGameplayTag::RequestGameplayTag(FName(*TagStr.Left(LastDot)), false);
		}
		return FGameplayTag();
	}

	void SetTabButtonLabel(UMCore_TabbedContainer* Container, FName TabID, const FGameplayTag& Tag)
	{
		if (UMCore_ButtonBase* TabButton = Cast<UMCore_ButtonBase>(Container->GetTabButton(TabID)))
		{
			TabButton->SetButtonText(UMCore_CoreSettings::Get()->GetCategoryDisplayName(Tag));
		}
	}
}

// ============================================================================
// LIFECYCLE
// ============================================================================

void UMCore_SettingsPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	TabbedContainer_Main->OnTabSelected.AddDynamic(this, &ThisClass::HandleMainTabSelected);

	Btn_ResetAll->OnButtonClicked.AddDynamic(this, &ThisClass::HandleResetAllClicked);
	Btn_ResetCategory->OnButtonClicked.AddDynamic(this, &ThisClass::HandleResetCategoryClicked);
	Btn_Back->OnButtonClicked.AddDynamic(this, &ThisClass::HandleBackClicked);

	UMCore_GameSettingsLibrary::OnSettingsConfirmationRequired.AddUObject(
		this, &ThisClass::HandleConfirmationRequired);
}

void UMCore_SettingsPanel::NativeOnActivated()
{
	Super::NativeOnActivated();

	UE_LOG(LogModulusSettings, Verbose,
		TEXT("SettingsPanel::NativeOnActivated: this=%p, IsActivated()=%s, TabbedContainer_Main=%s, bNeedsFullRebuild=%s"),
		this,
		IsActivated() ? TEXT("true") : TEXT("false"),
		TabbedContainer_Main ? TEXT("valid") : TEXT("NULL"),
		bNeedsFullRebuild ? TEXT("true") : TEXT("false"));

	if (!IsActivated())
	{
		UE_LOG(LogModulusSettings, Verbose, TEXT("SettingsPanel::NativeOnActivated: early exit, not activated (BlockTags?)"));
		return;
	}

	/* Register input action bindings for panel navigation */
	{
		FMCore_InputActionBindingHandle Handle;

		if (!TabNextAction.IsNull())
		{
			FInputActionExecutedDelegate Delegate;
			Delegate.BindDynamic(this, &UMCore_SettingsPanel::HandleTabNextInput);
			RegisterBinding(TabNextAction, Delegate, Handle);
		}
		if (!TabPrevAction.IsNull())
		{
			FInputActionExecutedDelegate Delegate;
			Delegate.BindDynamic(this, &UMCore_SettingsPanel::HandleTabPrevInput);
			RegisterBinding(TabPrevAction, Delegate, Handle);
		}
		if (!SubTabNextAction.IsNull())
		{
			FInputActionExecutedDelegate Delegate;
			Delegate.BindDynamic(this, &UMCore_SettingsPanel::HandleSubTabNextInput);
			RegisterBinding(SubTabNextAction, Delegate, Handle);
		}
		if (!SubTabPrevAction.IsNull())
		{
			FInputActionExecutedDelegate Delegate;
			Delegate.BindDynamic(this, &UMCore_SettingsPanel::HandleSubTabPrevInput);
			RegisterBinding(SubTabPrevAction, Delegate, Handle);
		}
		if (!RevertSettingsAction.IsNull())
		{
			FInputActionExecutedDelegate Delegate;
			Delegate.BindDynamic(this, &UMCore_SettingsPanel::HandleResetCategoryInput);
			RegisterBinding(RevertSettingsAction, Delegate, Handle);
		}
		if (!ResetDefaultsAction.IsNull())
		{
			FInputActionExecutedDelegate Delegate;
			Delegate.BindDynamic(this, &UMCore_SettingsPanel::HandleResetAllInput);
			RegisterBinding(ResetDefaultsAction, Delegate, Handle);
		}
	}

	/* Configure optional action bar icon buttons */
	if (Btn_ActionRevert) { Btn_ActionRevert->SetInputAction(RevertSettingsAction); }
	if (Btn_ActionReset) { Btn_ActionReset->SetInputAction(ResetDefaultsAction); }
	if (Btn_ActionBack) { Btn_ActionBack->SetInputAction(BackAction); }

	if (bNeedsFullRebuild)
	{
		UE_LOG(LogModulusSettings, Verbose, TEXT("SettingsPanel::NativeOnActivated: post-destruct rebuild, rebinding child delegates + full BuildPanel"));
		bNeedsFullRebuild = false;

		/* Rebind the 4 child delegates that NativeOnInitialized originally bound.
		 * NativeDestruct unbound all of these; AddDynamic on clean delegates, no double-bind risk */
		TabbedContainer_Main->OnTabSelected.AddDynamic(this, &ThisClass::HandleMainTabSelected);
		Btn_ResetAll->OnButtonClicked.AddDynamic(this, &ThisClass::HandleResetAllClicked);
		Btn_ResetCategory->OnButtonClicked.AddDynamic(this, &ThisClass::HandleResetCategoryClicked);
		Btn_Back->OnButtonClicked.AddDynamic(this, &ThisClass::HandleBackClicked);

		UMCore_GameSettingsLibrary::OnSettingsConfirmationRequired.AddUObject(
			this, &ThisClass::HandleConfirmationRequired);

		/* BuildPanel internally clears stale state:
		 *   ClearAllTabs() resets PageWidgets/TabList/PageSwitcher (fixes stale GetTabCount=5)
		 *   Rebuilds SubContainer->OnTabSelected and Widget->OnSettingFocused delegates via helpers */
		BuildPanel();
	}
	else if (TabbedContainer_Main->GetTabCount() == 0)
	{
		UE_LOG(LogModulusSettings, Verbose, TEXT("SettingsPanel::NativeOnActivated: calling BuildPanel (first activation)"));
		BuildPanel();
	}
	else
	{
		UE_LOG(LogModulusSettings, Verbose,
			TEXT("SettingsPanel::NativeOnActivated: calling RefreshAllWidgets (re-activation, %d tabs exist)"),
			TabbedContainer_Main->GetTabCount());
		RefreshAllWidgets();
	}
}

UWidget* UMCore_SettingsPanel::NativeGetDesiredFocusTarget() const
{
	if (TabbedContainer_Main)
	{
		const FName SelectedTab = TabbedContainer_Main->GetSelectedTabID();
		if (!SelectedTab.IsNone())
		{
			if (UWidget* TabButton = TabbedContainer_Main->GetTabButton(SelectedTab))
			{
				return TabButton;
			}
		}
	}

	return Super::NativeGetDesiredFocusTarget();
}

void UMCore_SettingsPanel::NativeOnDeactivated()
{
	UE_LOG(LogModulusSettings, Verbose, TEXT("SettingsPanel::NativeOnDeactivated: this=%p, TabCount=%d"),
		this,
		TabbedContainer_Main ? TabbedContainer_Main->GetTabCount() : -1);

	/* Dismiss any pending confirmation dialog before this panel leaves the stack */
	if (PendingConfirmationDialog.IsValid())
	{
		PendingConfirmationDialog->OnDialogResult.RemoveAll(this);
		PendingConfirmationDialog->DeactivateWidget();
		PendingConfirmationDialog.Reset();
	}

	/* Dismiss any active revert countdown */
	if (ActiveRevertCountdown.IsValid())
	{
		ActiveRevertCountdown->OnCountdownResult.RemoveAll(this);
		ActiveRevertCountdown->DeactivateWidget();
		ActiveRevertCountdown.Reset();
	}

	/* Cancel any pending confirmation debounce timer. Without this, the timer
	 * fires after deactivation and spawns a Modal-layer countdown on a non-interactable panel. */
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ConfirmationDebounceTimer);
	}

	PendingConfirmationTags = TArray<FGameplayTag>();

	Super::NativeOnDeactivated();
}

void UMCore_SettingsPanel::NativeDestruct()
{
	bNeedsFullRebuild = true;

	UE_LOG(LogModulusSettings, Verbose,
		TEXT("SettingsPanel::NativeDestruct: this=%p, TabbedContainer_Main=%s, TabCount=%d"),
		this,
		TabbedContainer_Main ? TEXT("valid") : TEXT("NULL"),
		TabbedContainer_Main ? TabbedContainer_Main->GetTabCount() : -1);

	if (TabbedContainer_Main)
	{
		TabbedContainer_Main->OnTabSelected.RemoveAll(this);
	}

	for (UMCore_TabbedContainer* SubContainer : SubTabContainers)
	{
		if (IsValid(SubContainer))
		{
			SubContainer->OnTabSelected.RemoveAll(this);
		}
	}
	SubTabContainers.Empty();
	MainTabToSubContainer.Empty();

	for (UMCore_SettingsWidget_Base* Widget : AllSettingWidgets)
	{
		if (IsValid(Widget))
		{
			Widget->OnSettingFocused.RemoveAll(this);
		}
	}
	AllSettingWidgets.Empty();
	SpawnedSectionHeaders.Reset();

	if (PendingConfirmationDialog.IsValid())
	{
		PendingConfirmationDialog->OnDialogResult.RemoveAll(this);
		PendingConfirmationDialog.Reset();
	}

	UMCore_GameSettingsLibrary::OnSettingsConfirmationRequired.RemoveAll(this);

	if (ActiveRevertCountdown.IsValid())
	{
		ActiveRevertCountdown->OnCountdownResult.RemoveAll(this);
		ActiveRevertCountdown.Reset();
	}

	/* Cancel any pending confirmation debounce timer. Defends against the narrow
	 * Destruct-to-GC window where the timer could fire on a mid-teardown widget. */
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ConfirmationDebounceTimer);
	}

	PendingConfirmationTags = TArray<FGameplayTag>();

	if (Btn_ResetAll) { Btn_ResetAll->OnButtonClicked.RemoveAll(this); }
	if (Btn_ResetCategory) { Btn_ResetCategory->OnButtonClicked.RemoveAll(this); }
	if (Btn_Back) { Btn_Back->OnButtonClicked.RemoveAll(this); }

	Super::NativeDestruct();
}

// ============================================================================
// BLUEPRINT EXTENSION POINTS
// ============================================================================

void UMCore_SettingsPanel::OnCategoryPageCreated_Implementation(
	const FGameplayTag& CategoryTag, UScrollBox* PageScrollBox)
{
	/* Default: no-op. Blueprint subclasses can override to inject custom widgets. */
}

void UMCore_SettingsPanel::OnPanelBuildComplete_Implementation()
{
	/* Default: no-op. Blueprint subclasses can override for post-build setup. */
}

// ============================================================================
// THEME
// ============================================================================

void UMCore_SettingsPanel::ApplyTheme_Implementation(UMCore_PDA_UITheme_Base* NewTheme)
{
	Super::ApplyTheme_Implementation(NewTheme);
	if (!NewTheme) { return; }

	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	for (UCommonTextBlock* Header : SpawnedSectionHeaders)
	{
		if (Header)
		{
			UMCore_ThemeLibrary::ApplyTextStyleFromTheme(
				LocalPlayer, Header, NewTheme->HeadingTextStyle);
		}
	}
}

// ============================================================================
// PANEL BUILD
// ============================================================================

void UMCore_SettingsPanel::BuildPanel()
{
	UE_LOG(LogModulusSettings, VeryVerbose, TEXT("SettingsPanel::BuildPanel: rebuilding from %d existing tabs"),
		TabbedContainer_Main->GetTabCount());
	
	const UMCore_CoreSettings* CoreSettings = UMCore_CoreSettings::Get();
	if (!CoreSettings || !CoreSettings->HasValidSettingsCollections())
	{
		UE_LOG(LogModulusSettings, Error,
			TEXT("SettingsPanel::BuildPanel: CoreSettings is null or no SettingsCollections configured, "
			     "ensure CoreSettings has at least one valid SettingsCollection assigned"));
		return;
	}

	TabbedContainer_Main->ClearAllTabs();
	TabIDToLeafTag.Reset();
	AllSettingWidgets.Reset();
	SpawnedSectionHeaders.Reset();
	SubTabContainers.Reset();
	MainTabToSubContainer.Reset();

	const TArray<FGameplayTag> AllCategories = CoreSettings->GetAllSettingsCategories();

	/* Group depth-4 tags under their depth-3 parent, preserving encounter order */
	TArray<FGameplayTag> MainTabOrder;
	TMap<FGameplayTag, TArray<FGameplayTag>> ParentToChildren;

	for (const FGameplayTag& LeafTag : AllCategories)
	{
		const FGameplayTag ParentTag = GetParentTag(LeafTag);
		if (!ParentTag.IsValid())
		{
			continue;
		}

		if (!ParentToChildren.Contains(ParentTag))
		{
			MainTabOrder.Add(ParentTag);
		}
		ParentToChildren.FindOrAdd(ParentTag).Add(LeafTag);
	}

	static const FGameplayTag KeyBindingTag =
		FGameplayTag::RequestGameplayTag(FName("MCore.Settings.Category.KeyBinding"), false);

	FName FirstTabID{NAME_None};

	for (const FGameplayTag& ParentTag : MainTabOrder)
	{
		const FName TabID = FName(*ParentTag.ToString());
		UWidget* PageWidget = nullptr;

		if (ParentTag.MatchesTagExact(KeyBindingTag))
		{
			if (CoreSettings->KeyBindingPanelClass)
			{
				PageWidget = CreateWidget<UMCore_KeyBindingPanel_Base>(this, CoreSettings->KeyBindingPanelClass);
			}

			if (!PageWidget)
			{
				UE_LOG(LogModulusSettings, Warning,
				TEXT("SettingsPanel::BuildPanel: KeyBindingPanelClass not set in CoreSettings"));
				PageWidget = NewObject<USizeBox>(this);
			}
		}
		else
		{
			const TArray<FGameplayTag>& Children = ParentToChildren[ParentTag];

			PageWidget = BuildTabbedPage(ParentTag, Children);

			if (PageWidget)
			{
				if (UMCore_TabbedContainer* AsTabbed = Cast<UMCore_TabbedContainer>(PageWidget))
				{
					MainTabToSubContainer.Add(TabID, AsTabbed);
				}

				/* Single-child main tabs (Audio, Accessibility) still go through the tabbed-container
				 * path so the page layout is shape-stable across all main tabs. The TabList is hidden
				 * but space-reserved so settings content lands at the same Y on every tab. */
				if (Children.Num() == 1)
				{
					TabIDToLeafTag.Add(TabID, Children[0]);
				}
			}
		}

		UE_LOG(LogModulusSettings, VeryVerbose,
			TEXT("SettingsPanel::BuildPanel: adding tab %s at index %d"),
			*TabID.ToString(), MainTabOrder.IndexOfByKey(ParentTag));
		if (PageWidget && TabbedContainer_Main->AddTab(TabID, PageWidget))
		{
			SetTabButtonLabel(TabbedContainer_Main, TabID, ParentTag);

			if (FirstTabID.IsNone())
			{
				FirstTabID = TabID;
			}
		}
	}

	if (!FirstTabID.IsNone())
	{
		TabbedContainer_Main->SelectTab(FirstTabID);
	}

	/* Configure flanking tab-cycle icons on all tab bars */
	TabbedContainer_Main->SetTabCycleActions(TabPrevAction, TabNextAction);

	for (auto& Pair : MainTabToSubContainer)
	{
		if (IsValid(Pair.Value))
		{
			Pair.Value->SetTabCycleActions(SubTabPrevAction, SubTabNextAction);
		}
	}

	OnPanelBuildComplete();
}

UMCore_TabbedContainer* UMCore_SettingsPanel::BuildTabbedPage(
	const FGameplayTag& ParentTag,
	const TArray<FGameplayTag>& ChildTags)
{
	UMCore_TabbedContainer* SubContainer =
		CreateWidget<UMCore_TabbedContainer>(this, SubTabContainerClass);
	if (!SubContainer)
	{
		UE_LOG(LogModulusSettings, Error,
			TEXT("SettingsPanel::BuildTabbedPage: failed to create SubTabContainer, SubTabContainerClass may be null or invalid"));
		return nullptr;
	}

	SubTabContainers.Add(SubContainer);

	FName FirstSubTabID{NAME_None};

	for (const FGameplayTag& ChildTag : ChildTags)
	{
		const FName SubTabID = FName(*ChildTag.ToString());
		UScrollBox* SubPage = NewObject<UScrollBox>(this);
		PopulatePage(SubPage, ChildTag);

		if (SubContainer->AddTab(SubTabID, SubPage))
		{
			SetTabButtonLabel(SubContainer, SubTabID, ChildTag);
			TabIDToLeafTag.Add(SubTabID, ChildTag);

			OnCategoryPageCreated(ChildTag, SubPage);

			if (FirstSubTabID.IsNone())
			{
				FirstSubTabID = SubTabID;
			}
		}
	}

	SubContainer->OnTabSelected.AddDynamic(this, &ThisClass::HandleSubTabSelected);

	if (!FirstSubTabID.IsNone())
	{
		SubContainer->SelectTab(FirstSubTabID);
	}

	/* Hide the TabList (but reserve its layout space) when there's only one child.
	 * This keeps the page area shape-consistent across all main tabs, so settings
	 * content lands at the same Y position regardless of sub-tab presence. */
	if (ChildTags.Num() <= 1)
	{
		SubContainer->SetTabListVisibility(ESlateVisibility::Hidden);
	}

	return SubContainer;
}

void UMCore_SettingsPanel::PopulatePage(
	UScrollBox* ScrollBox, const FGameplayTag& CategoryTag)
{
	const UMCore_CoreSettings* CoreSettings = UMCore_CoreSettings::Get();

	TArray<UMCore_DA_SettingDefinition*> Definitions =
		CoreSettings->GetSettingsForCategory(CategoryTag);

	/* Merge Sections from all loaded collections; first-seen-wins on SectionID.
	 * The bucket-empty check below filters sections nobody references, so we
	 * don't pre-filter by category here. */
	TArray<FMCore_SettingsSection> MergedSections;
	TSet<FName> SeenSectionIDs;
	for (const UMCore_DA_SettingsCollection* Collection : CoreSettings->GetAllSettingsCollections())
	{
		if (!Collection) { continue; }
		for (const FMCore_SettingsSection& Section : Collection->Sections)
		{
			if (Section.SectionID.IsNone()) { continue; }
			bool bDup = false;
			SeenSectionIDs.Add(Section.SectionID, &bDup);
			if (!bDup) { MergedSections.Add(Section); }
		}
	}

	/* Bucket settings by SectionID; preserve in-array order within each bucket. */
	TMap<FName, TArray<UMCore_DA_SettingDefinition*>> Buckets;
	for (UMCore_DA_SettingDefinition* Def : Definitions)
	{
		if (!Def) { continue; }
		Buckets.FindOrAdd(Def->SectionID).Add(Def);
	}

	auto AppendSetting = [&](const UMCore_DA_SettingDefinition* Def)
	{
		if (UMCore_SettingsWidget_Base* Widget = CreateSettingWidget(Def))
		{
			ScrollBox->AddChild(Widget);
			AllSettingWidgets.Add(Widget);
			Widget->OnSettingFocused.AddDynamic(this, &ThisClass::HandleSettingFocused);
		}
	};

	auto AppendHeader = [&](const FText& HeaderText)
	{
		UCommonTextBlock* Header = NewObject<UCommonTextBlock>(this);
		Header->SetText(HeaderText);
		if (CachedTheme.IsValid())
		{
			UMCore_ThemeLibrary::ApplyTextStyleFromTheme(
				GetOwningLocalPlayer(), Header, CachedTheme->HeadingTextStyle);
		}
		if (UScrollBoxSlot* Slot = Cast<UScrollBoxSlot>(ScrollBox->AddChild(Header)))
		{
			Slot->SetPadding(FMargin(16.0f, 0.0f, 0.0f, 4.0f));
		}
		SpawnedSectionHeaders.Add(Header);
	};

	/* 1. Unsectioned bucket (NAME_None) renders first, no header. */
	if (TArray<UMCore_DA_SettingDefinition*>* NoneBucket = Buckets.Find(NAME_None))
	{
		for (const UMCore_DA_SettingDefinition* Def : *NoneBucket) { AppendSetting(Def); }
		Buckets.Remove(NAME_None);
	}

	/* 2. Declared sections, in declaration order. */
	TSet<FName> RenderedSectionIDs;
	for (const FMCore_SettingsSection& Section : MergedSections)
	{
		TArray<UMCore_DA_SettingDefinition*>* Bucket = Buckets.Find(Section.SectionID);
		if (!Bucket || Bucket->Num() == 0) { continue; }

		AppendHeader(Section.SectionDisplayName);
		for (const UMCore_DA_SettingDefinition* Def : *Bucket) { AppendSetting(Def); }
		RenderedSectionIDs.Add(Section.SectionID);
	}

	/* 3. Orphan settings (SectionID set but undeclared); append unheaded.
	 *    Editor-time validation surfaces this as a warning. */
	for (auto& Pair : Buckets)
	{
		if (Pair.Key.IsNone() || RenderedSectionIDs.Contains(Pair.Key)) { continue; }
		UE_LOG(LogModulusSettings, Verbose,
			TEXT("SettingsPanel::PopulatePage: orphan SectionID '%s' for category '%s'"),
			*Pair.Key.ToString(), *CategoryTag.ToString());
		for (const UMCore_DA_SettingDefinition* Def : Pair.Value) { AppendSetting(Def); }
	}
}

// ============================================================================
// WIDGET FACTORY
// ============================================================================

/**
 * @param Definition  Must be non-null. SettingType drives which widget class is spawned.
 * @return            Initialized widget, or nullptr if the widget class is not configured on CoreSettings.
 */
UMCore_SettingsWidget_Base* UMCore_SettingsPanel::CreateSettingWidget(
	const UMCore_DA_SettingDefinition* Definition)
{
	const UMCore_CoreSettings* CoreSettings = UMCore_CoreSettings::Get();
	UMCore_SettingsWidget_Base* Widget = nullptr;

	if (Definition->WidgetClassOverride)
	{
		Widget = CreateWidget<UMCore_SettingsWidget_Base>(this, Definition->WidgetClassOverride);
	}

	if (!Widget)
	{
		switch (Definition->SettingType)
		{
		case EMCore_SettingType::Toggle:
		case EMCore_SettingType::Dropdown:
			if (CoreSettings->SettingsSwitcherWidgetClass)
			{
				Widget = CreateWidget<UMCore_SettingsWidget_Switcher>(
					this, CoreSettings->SettingsSwitcherWidgetClass);
			}
			break;

		case EMCore_SettingType::Slider:
			if (CoreSettings->SettingsSliderWidgetClass)
			{
				Widget = CreateWidget<UMCore_SettingsWidget_Slider>(
					this, CoreSettings->SettingsSliderWidgetClass);
			}
			break;
		}
	}

	if (Widget)
	{
		Widget->InitFromDefinition(Definition);
	}
	else
	{
		UE_LOG(LogModulusSettings, Warning,
			TEXT("SettingsPanel::CreateSettingWidget: no widget class configured for SettingType %d, setting='%s'"),
			(int32)Definition->SettingType, *Definition->SettingTag.ToString());
	}

	return Widget;
}

// ============================================================================
// TAB CALLBACKS
// ============================================================================

void UMCore_SettingsPanel::HandleMainTabSelected(FName TabID)
{
	/* Resolve ActiveLeafCategory from page type */
	UWidget* Page = TabbedContainer_Main->GetPageWidget(TabID);

	if (UMCore_TabbedContainer* SubContainer = Cast<UMCore_TabbedContainer>(Page))
	{
		const FName SubTabID = SubContainer->GetSelectedTabID();
		if (const FGameplayTag* LeafTag = TabIDToLeafTag.Find(SubTabID))
		{
			ActiveLeafCategory = *LeafTag;
		}
	}
	else if (const FGameplayTag* LeafTag = TabIDToLeafTag.Find(TabID))
	{
		ActiveLeafCategory = *LeafTag;
	}

	FocusFirstWidgetInActivePage();
}

void UMCore_SettingsPanel::HandleSubTabSelected(FName TabID)
{
	if (const FGameplayTag* LeafTag = TabIDToLeafTag.Find(TabID))
	{
		ActiveLeafCategory = *LeafTag;
	}

	FocusFirstWidgetInActivePage();
}

void UMCore_SettingsPanel::FocusFirstWidgetInActivePage()
{
	const FName ActiveMainTab = TabbedContainer_Main->GetSelectedTabID();
	UWidget* Page = TabbedContainer_Main->GetPageWidget(ActiveMainTab);

	UScrollBox* ScrollBox = nullptr;

	if (UMCore_TabbedContainer* SubContainer = Cast<UMCore_TabbedContainer>(Page))
	{
		const FName ActiveSubTab = SubContainer->GetSelectedTabID();
		ScrollBox = Cast<UScrollBox>(SubContainer->GetPageWidget(ActiveSubTab));
	}
	else
	{
		ScrollBox = Cast<UScrollBox>(Page);
	}

	if (ScrollBox && ScrollBox->GetChildrenCount() > 0)
	{
		if (UWidget* FirstChild = ScrollBox->GetChildAt(0))
		{
			FirstChild->SetUserFocus(GetOwningPlayer());
		}
	}
}

// ============================================================================
// ACTION BAR
// ============================================================================

void UMCore_SettingsPanel::HandleResetAllClicked()
{
	/* Resolve dialog class: per-widget override w/ CoreSettings fallback */
	TSubclassOf<UMCore_ConfirmationDialog> DialogClass = ResetConfirmationDialogClass;
	if (!DialogClass)
	{
		const UMCore_CoreSettings* CoreSettings = UMCore_CoreSettings::Get();
		if (CoreSettings) { DialogClass = CoreSettings->DefaultConfirmationDialogClass; }
		UE_LOG(LogModulusSettings, Log,
			TEXT("SettingsPanel::ResetAllClicked: No local Dialog set, falling back to CoreSettings."));
	}
	
	if (!DialogClass)
	{
		/* No dialog configured, reset directly as safety net */
		UMCore_GameSettingsLibrary::ResetAllSettingsToDefault(GetOwningLocalPlayer());
		RefreshAllWidgets();
		return;
	}
	
	if (UMCore_UISubsystem* UISubsystem = GetOwningLocalPlayer()->GetSubsystem<UMCore_UISubsystem>())
	{
		UCommonActivatableWidget* CAWidget = UISubsystem->OpenScreen(
			DialogClass,
			MCore_UILayerTags::MCore_UI_Layer_Modal);

		if (UMCore_ConfirmationDialog* Dialog = Cast<UMCore_ConfirmationDialog>(CAWidget))
		{
			PendingConfirmationDialog = Dialog;
			Dialog->SetDialogMessage(
				NSLOCTEXT("ModulusCore", "ResetAllMessage", "Reset all setting to defaults?"));
			Dialog->SetButtonLabels(
				NSLOCTEXT("ModulusCore", "ResetConfirm", "Reset"),
				NSLOCTEXT("ModulusCore", "ResetCancel", "Cancel"));
			Dialog->OnDialogResult.AddDynamic(this, &ThisClass::HandleResetAllConfirmResult);
		}
	}
}

void UMCore_SettingsPanel::HandleResetAllConfirmResult(bool bConfirmed)
{
	PendingConfirmationDialog.Reset();

	if (bConfirmed)
	{
		UMCore_GameSettingsLibrary::ResetAllSettingsToDefault(GetOwningLocalPlayer());
		RefreshAllWidgets();
	}
}

void UMCore_SettingsPanel::HandleResetCategoryClicked()
{
	if (!ActiveLeafCategory.IsValid()) { return; }

	/* Resolve dialog class: per-widget override w/ CoreSettings fallback */
	TSubclassOf<UMCore_ConfirmationDialog> DialogClass = ResetConfirmationDialogClass;
	if (!DialogClass)
	{
		const UMCore_CoreSettings* CoreSettings = UMCore_CoreSettings::Get();
		if (CoreSettings) { DialogClass = CoreSettings->DefaultConfirmationDialogClass; }
		UE_LOG(LogModulusSettings, Log,
			TEXT("SettingsPanel::ResetCategoryClicked: No local Dialog set, falling back to CoreSettings."));
	}
	
	if (!DialogClass)
	{
		/* No dialog configured, reset directly as safety net */
		UMCore_GameSettingsLibrary::ResetCategoryToDefault(
			GetOwningLocalPlayer(), ActiveLeafCategory);
		RefreshAllWidgets();
		return;
	}
	
	if (UMCore_UISubsystem* UISubsystem = GetOwningLocalPlayer()->GetSubsystem<UMCore_UISubsystem>())
	{
		UCommonActivatableWidget* CAWidget = UISubsystem->OpenScreen(
			DialogClass,
			MCore_UILayerTags::MCore_UI_Layer_Modal);

		if (UMCore_ConfirmationDialog* Dialog = Cast<UMCore_ConfirmationDialog>(CAWidget))
		{
			PendingConfirmationDialog = Dialog;
			const FText CurrentCategory = UMCore_CoreSettings::Get()->GetCategoryDisplayName(ActiveLeafCategory);
			Dialog->SetDialogMessage(
				FText::Format(NSLOCTEXT("ModulusCore", "ResetCategoryMessage",
					"Reset {0} settings to defaults?"), CurrentCategory));
			Dialog->SetButtonLabels(
				NSLOCTEXT("ModulusCore", "ResetConfirm", "Reset"),
				NSLOCTEXT("ModulusCore", "ResetCancel", "Cancel"));
			Dialog->OnDialogResult.AddDynamic(this, &ThisClass::HandleResetCategoryConfirmResult);
		}
	}
}

void UMCore_SettingsPanel::HandleResetCategoryConfirmResult(bool bConfirmed)
{
	PendingConfirmationDialog.Reset();

	if (bConfirmed && ActiveLeafCategory.IsValid())
	{
		UMCore_GameSettingsLibrary::ResetCategoryToDefault(
			GetOwningLocalPlayer(), ActiveLeafCategory);
		RefreshAllWidgets();
	}
}

void UMCore_SettingsPanel::SpawnRevertCountdown()
{
	const UMCore_CoreSettings* CoreSettings = UMCore_CoreSettings::Get();
	if (!CoreSettings || !CoreSettings->SettingsRevertCountdownClass)
	{
		/* No countdown class configured. Save + clear pending */
		UMCore_GameSettingsLibrary::SavePlayerSettings(this);
		PendingConfirmationTags.Empty();
		return;
	}

	UMCore_UISubsystem* UISubsystem = GetOwningLocalPlayer()
		? GetOwningLocalPlayer()->GetSubsystem<UMCore_UISubsystem>()
		: nullptr;
	if (!UISubsystem)
	{
		UMCore_GameSettingsLibrary::SavePlayerSettings(this);
		PendingConfirmationTags.Empty();
		return;
	}

	UCommonActivatableWidget* Widget = UISubsystem->OpenScreen(
		CoreSettings->SettingsRevertCountdownClass,
		MCore_UILayerTags::MCore_UI_Layer_Modal);

	UMCore_SettingsRevertCountdown* Countdown = Cast<UMCore_SettingsRevertCountdown>(Widget);
	if (!Countdown)
	{
		UMCore_GameSettingsLibrary::SavePlayerSettings(this);
		PendingConfirmationTags.Empty();
		return;
	}

	ActiveRevertCountdown = Countdown;
	Countdown->OnCountdownResult.AddDynamic(this, &ThisClass::HandleCountdownResult);
	Countdown->StartCountdown(PendingConfirmationTags);
}

void UMCore_SettingsPanel::HandleConfirmationRequired(const TArray<FGameplayTag>& AffectedTags)
{
	if (!IsActivated()) { return; }
	
	const UMCore_CoreSettings* CoreSettings = UMCore_CoreSettings::Get();
	if (!CoreSettings) { return; }

	/* Merge incoming tags into pending set */
	for (const FGameplayTag& Tag : AffectedTags) { PendingConfirmationTags.AddUnique(Tag); }

	/* Teardown active countdown if one exists */
	if (ActiveRevertCountdown.IsValid())
	{
		ActiveRevertCountdown->OnCountdownResult.Clear();
		ActiveRevertCountdown->SuppressFallbackForReplacement();
		ActiveRevertCountdown->DeactivateWidget();
		ActiveRevertCountdown.Reset();
	}
	
	UWorld* World = GetWorld();
	if (!World) { return; }
	
	World->GetTimerManager().ClearTimer(ConfirmationDebounceTimer);
	
	const float Delay = CoreSettings->ConfirmationDebounceDelay;
	
	if (Delay > 0.0f)
	{
		World->GetTimerManager().SetTimer(
			ConfirmationDebounceTimer,
			this,
			&ThisClass::SpawnRevertCountdown,
			Delay,
			false);
	}
	else
	{
		SpawnRevertCountdown();
	}
}

void UMCore_SettingsPanel::HandleCountdownResult(bool bConfirmed)
{
	if (ActiveRevertCountdown.IsValid())
	{
		ActiveRevertCountdown->OnCountdownResult.RemoveAll(this);
		ActiveRevertCountdown.Reset();
	}

	PendingConfirmationTags.Empty();
	RefreshAllWidgets();
}

void UMCore_SettingsPanel::HandleBackClicked()
{
	DeactivateWidget();
}

// ============================================================================
// INPUT ACTION HANDLERS
// ============================================================================

void UMCore_SettingsPanel::HandleTabNextInput(FName ActionName)
{
	TabbedContainer_Main->SelectNextTab();
}

void UMCore_SettingsPanel::HandleTabPrevInput(FName ActionName)
{
	TabbedContainer_Main->SelectPreviousTab();
}

void UMCore_SettingsPanel::HandleSubTabNextInput(FName ActionName)
{
	const FName MainTabID = TabbedContainer_Main->GetSelectedTabID();
	if (TObjectPtr<UMCore_TabbedContainer>* Found = MainTabToSubContainer.Find(MainTabID))
	{
		if (IsValid(*Found)) { (*Found)->SelectNextTab(); }
	}
}

void UMCore_SettingsPanel::HandleSubTabPrevInput(FName ActionName)
{
	const FName MainTabID = TabbedContainer_Main->GetSelectedTabID();
	if (TObjectPtr<UMCore_TabbedContainer>* Found = MainTabToSubContainer.Find(MainTabID))
	{
		if (IsValid(*Found)) { (*Found)->SelectPreviousTab(); }
	}
}

void UMCore_SettingsPanel::HandleResetCategoryInput(FName ActionName)
{
	HandleResetCategoryClicked();
}

void UMCore_SettingsPanel::HandleResetAllInput(FName ActionName)
{
	HandleResetAllClicked();
}

// ============================================================================
// SETTING FOCUS
// ============================================================================

void UMCore_SettingsPanel::HandleSettingFocused(FGameplayTag SettingTag, FText Description)
{
	if (Txt_SettingDescription)
	{
		Txt_SettingDescription->SetText(Description);
	}
}



// ============================================================================
// REFRESH
// ============================================================================

void UMCore_SettingsPanel::RefreshAllWidgets()
{
	for (UMCore_SettingsWidget_Base* Widget : AllSettingWidgets)
	{
		if (Widget)
		{
			Widget->RefreshValueFromSettings();
		}
	}
}