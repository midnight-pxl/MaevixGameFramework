// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/MCore_UISubsystem.h"

#include "CoreData/Logging/LogModulusUI.h"
#include "CoreData/DevSettings/MCore_CoreSettings.h"
#include "CoreData/Settings/MCore_PlayerSettingsSubsystem.h"
#include "CoreData/Tags/MCore_UILayerTags.h"
#include "CoreData/Tags/MCore_ThemeTags.h"
#include "CoreData/Types/Settings/MCore_PlayerSettingsSave.h"
#include "CoreUI/Widgets/MCore_GameMenuHub.h"
#include "CoreUI/Widgets/MCore_PrimaryGameLayout.h"
#include "CoreData/Assets/UI/Themes/MCore_PDA_UITheme_Base.h"
#include "CoreEvents/MCore_LocalEventSubsystem.h"
#include "CoreData/Types/Events/MCore_EventData.h"
#include "CoreData/Tags/MCore_SettingsTags.h"
#include "GameplayTagContainer.h"
#include "Engine/LocalPlayer.h"

// ============================================================================
// INITIALIZATION
// ============================================================================

void UMCore_UISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// TODO: Remove after subsystem init order diagnostic (2026-05-08)
	UE_LOG(LogModulusUI, Log,
		TEXT("UISubsystem::Initialize -- entered"));

	if (IsRunningDedicatedServer())
	{
		UE_LOG(LogModulusUI, Log, TEXT("UISubsystem::Initialize -- skipping on dedicated server"));
		return;
	}

	LoadWidgetClasses();
	CreatePrimaryGameLayout();
	
	/* Load theming with fallback chain: saved → DefaultThemeIndex → first valid */
	const UMCore_CoreSettings* DevSettings = UMCore_CoreSettings::Get();
	if (UMCore_PDA_UITheme_Base* SavedTheme = LoadSavedActiveTheme())
	{
		SetActiveTheme(SavedTheme);
		UE_LOG(LogModulusUI, Verbose,
			TEXT("UISubsystem::Initialize -- loaded saved theme '%s'"),
			*SavedTheme->GetPathName());
	}
	else if (DevSettings && DevSettings->IsValidThemeIndex(DevSettings->DefaultThemeIndex))
	{
		SetActiveThemeByIndex(DevSettings->DefaultThemeIndex);
		UE_LOG(LogModulusUI, Verbose, TEXT("UISubsystem::Initialize -- loaded default theme from index %d"),
			DevSettings->DefaultThemeIndex);
	}
	else if (DevSettings)
	{
		/* Last-resort floor: pick the first valid AvailableThemes entry. */
		for (int32 i = 0; i < DevSettings->AvailableThemes.Num(); ++i)
		{
			if (!DevSettings->AvailableThemes[i].ThemeAsset.IsNull())
			{
				UE_LOG(LogModulusUI, Warning,
					TEXT("UISubsystem::Initialize -- DefaultThemeIndex misconfigured; falling back to first valid theme at index %d"),
					i);
				SetActiveThemeByIndex(i);
				break;
			}
		}
	}

	/* Register default menu tabs from CoreSettings */
	if (DevSettings)
	{
		for (const FMCore_MenuTab& Tab : DevSettings->DefaultMenuTabs)
		{
			if (Tab.TabID.IsValid() && Tab.ScreenWidgetClass)
			{
				RegisterMenuScreen(Tab.TabID, Tab.ScreenWidgetClass, Tab.Priority, Tab.TabIcon);
			}
			else
			{
				UE_LOG(LogModulusUI, Warning,
					TEXT("UISubsystem::Initialize -- skipping DefaultMenuTabs entry with invalid TabID or null ScreenWidgetClass"));
			}
		}
	}

	/* Subscribe to local events for text size changes */
	if (UMCore_LocalEventSubsystem* LocalEvents = GetLocalPlayer()->GetSubsystem<UMCore_LocalEventSubsystem>())
	{
		// TODO: Switch to tag-filtered subscription (Event System Phase 2) to avoid per-event dispatch overhead
		LocalEventHandle = LocalEvents->OnLocalEventBroadcast.AddUObject(this, &ThisClass::HandleLocalEvent);
	}

	UE_LOG(LogModulusUI, Log, TEXT("UISubsystem::Initialize -- initialized for LocalPlayer"));
}

void UMCore_UISubsystem::Deinitialize()
{
	UE_LOG(LogModulusUI, Log, TEXT("UISubsystem::Deinitialize -- cleaning up"));

	if (LocalEventHandle.IsValid())
	{
		if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
		{
			if (UMCore_LocalEventSubsystem* LocalEvents = LocalPlayer->GetSubsystem<UMCore_LocalEventSubsystem>())
			{
				LocalEvents->OnLocalEventBroadcast.Remove(LocalEventHandle);
			}
		}
		LocalEventHandle.Reset();
	}

	/* Clear delegate handle if still bound */
	if (PlayerControllerReadyHandle.IsValid())
	{
		if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
		{
			LocalPlayer->OnPlayerControllerChanged().Remove(PlayerControllerReadyHandle);
		}
		PlayerControllerReadyHandle.Reset();
	}
	
	/* Clear layer stack map and tracked widgets */
	LayerStackMap.Empty();
	TrackedWidgets.Empty();

	/* Clean up PrimaryGameLayout */
	if (IsValid(PrimaryGameLayout))
	{
		PrimaryGameLayout->RemoveFromParent();
		PrimaryGameLayout = nullptr;
	}
	
	RegisteredMenuScreens.Empty();
	CachedActiveTheme = nullptr;
	ActiveThemeIndex = INDEX_NONE;
	
	Super::Deinitialize();
}

// ============================================================================
// CONFIGURATION
// ============================================================================

void UMCore_UISubsystem::LoadWidgetClasses()
{
	const UMCore_CoreSettings* DevSettings = UMCore_CoreSettings::Get();
	
	/* Load PrimaryGameLayout from DevSettings or use default */
	if (DevSettings && !DevSettings->PrimaryGameLayoutClass.IsNull())
	{
		PrimaryGameLayoutClass = DevSettings->PrimaryGameLayoutClass.LoadSynchronous();
	}
	
	if (!PrimaryGameLayoutClass)
	{
		PrimaryGameLayoutClass = UMCore_PrimaryGameLayout::StaticClass();
		UE_LOG(LogModulusUI, Log, TEXT("UISubsystem::LoadWidgetClasses -- using default PrimaryGameLayoutClass"));
	}
	
	/* Load MenuHubClass */
	if (DevSettings && !DevSettings->MenuHubClass.IsNull())
	{
		MenuHubClass = DevSettings->MenuHubClass.LoadSynchronous();
	}

	if (!MenuHubClass)
	{
		UE_LOG(LogModulusUI, Error,
			TEXT("UISubsystem::LoadWidgetClasses -- MenuHubClass is nullptr, check class exists"));
		MenuHubClass = UMCore_GameMenuHub::StaticClass();
	}
    
	/* Load ZOrder */
	if (DevSettings)
	{
		PrimaryGameLayoutZOrder = DevSettings->PrimaryGameLayoutZOrder;
	}
	
	UE_LOG(LogModulusUI, Verbose, 
		TEXT("UISubsystem::LoadWidgetClasses -- widget classes loaded, MenuHub: %s"),
		MenuHubClass ? TEXT("OK") : TEXT("FAIL"));
}

// ============================================================================
// WIDGET LIFECYCLE
// ============================================================================

void UMCore_UISubsystem::CreatePrimaryGameLayout()
{
	if (IsValid(PrimaryGameLayout)) { return; }
	
	if (!PrimaryGameLayoutClass)
	{
		UE_LOG(LogModulusUI, Error, TEXT("UISubsystem::CreatePrimaryGameLayout -- PrimaryGameLayoutClass not set"));
		return;
	}
	
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer) { return; }
	
	APlayerController* OwningPlayer = LocalPlayer->GetPlayerController(GetWorld());
	if (!OwningPlayer)
	{
		/* OwningPlayer not yet ready, defer creation */
		UE_LOG(LogModulusUI, Verbose,
			TEXT("UISubsystem::CreatePrimaryGameLayout -- deferring layout creation until OwningPlayer ready"));
		
		if (ULocalPlayer* ThisPlayer = GetLocalPlayer())
		{
			PlayerControllerReadyHandle = ThisPlayer->OnPlayerControllerChanged().AddUObject(
				this, &UMCore_UISubsystem::OnPlayerControllerReady);
		}
		return;
	}
	
	/* Create widget */
	PrimaryGameLayout = CreateWidget<UMCore_PrimaryGameLayout>(OwningPlayer, PrimaryGameLayoutClass);
	if (!IsValid(PrimaryGameLayout))
	{
		UE_LOG(LogModulusUI, Error,
			TEXT("UISubsystem::CreatePrimaryGameLayout -- failed to create PrimaryGameLayout widget"));
		return;
	}
	
	PrimaryGameLayout->AddToPlayerScreen(PrimaryGameLayoutZOrder);
	BuildLayerStackMap();
		
	UE_LOG(LogModulusUI, Log, TEXT("UISubsystem::CreatePrimaryGameLayout -- PrimaryGameLayout created (ZOrder: %d)"),
		PrimaryGameLayoutZOrder);
	
	/* Notify subclasses and listeners */
	OnPrimaryGameLayoutCreated(PrimaryGameLayout);
	OnPrimaryGameLayoutReady.Broadcast(PrimaryGameLayout);
}

void UMCore_UISubsystem::OnPlayerControllerReady(APlayerController* OwningPlayer)
{
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		LocalPlayer->OnPlayerControllerChanged().Remove(PlayerControllerReadyHandle);
		PlayerControllerReadyHandle.Reset(); 
	}
	
	if (IsValid(OwningPlayer) && !IsValid(PrimaryGameLayout))
	{
		UE_LOG(LogModulusUI, Log,
			TEXT("UISubsystem::OnPlayerControllerReady -- OwningPlayer initialized, creating PrimaryGameLayout"));
		CreatePrimaryGameLayout();
	}
}

void UMCore_UISubsystem::OnPrimaryGameLayoutCreated_Implementation(UMCore_PrimaryGameLayout* Layout)
{
	/* Default empty; override in Blueprint or C++ subclasses */
}

void UMCore_UISubsystem::BuildLayerStackMap()
{
	LayerStackMap.Empty();
	
	if (!IsValid(PrimaryGameLayout))
	{
		UE_LOG(LogModulusUI, Warning,
			TEXT("UISubsystem::BuildLayerStackMap -- cannot build layer map, no PrimaryGameLayout"));
		return;
	}
	
	using namespace MCore_UILayerTags;
	
	/* Map tags to layer stacks */
	if (PrimaryGameLayout->MCore_GameLayer)
	{
		LayerStackMap.Add(MCore_UI_Layer_Game, PrimaryGameLayout->MCore_GameLayer);
	}
	
	if (PrimaryGameLayout->MCore_GameMenuLayer)
	{
		LayerStackMap.Add(MCore_UI_Layer_GameMenu, PrimaryGameLayout->MCore_GameMenuLayer);
	}
	
	if (PrimaryGameLayout->MCore_MenuLayer)
	{
		LayerStackMap.Add(MCore_UI_Layer_Menu, PrimaryGameLayout->MCore_MenuLayer);
	}
	
	if (PrimaryGameLayout->MCore_ModalLayer)
	{
		LayerStackMap.Add(MCore_UI_Layer_Modal, PrimaryGameLayout->MCore_ModalLayer);
	}
	
	UE_LOG(LogModulusUI, Log,
		TEXT("UISubsystem::BuildLayerStackMap -- LayerStackMap created with %d layers"),
		LayerStackMap.Num());
}

// ============================================================================
// LAYER STACKS
// ============================================================================

UCommonActivatableWidgetStack* UMCore_UISubsystem::GetLayerStack(FGameplayTag LayerTag) const
{
	if (!LayerTag.IsValid())
	{
		UE_LOG(LogModulusUI, Warning, TEXT("UISubsystem::GetLayerStack -- invalid LayerTag"));
		return nullptr;
	}
	
	if (const TObjectPtr<UCommonActivatableWidgetStack> ThisStack = LayerStackMap.FindRef(LayerTag))
	{
		UE_LOG(LogModulusUI, Verbose, TEXT("UISubsystem::GetLayerStack -- Tag='%s', Stack=%s"),
			*LayerTag.ToString(), ThisStack ? *GetNameSafe(ThisStack.Get()) : TEXT("NULL"));
		
		return ThisStack;
	}
	
	UE_LOG(LogModulusUI, Warning, TEXT("UISubsystem::GetLayerStack -- no stack found for tag '%s'"),
		*LayerTag.ToString());
	return nullptr;
}

UCommonActivatableWidget* UMCore_UISubsystem::PushWidgetToLayer(
	TSubclassOf<UCommonActivatableWidget> WidgetClass,
	FGameplayTag LayerTag)
{
	if (!WidgetClass)
	{
		UE_LOG(LogModulusUI, Warning, TEXT("UISubsystem::PushWidgetToLayer -- invalid WidgetClass"));
		return nullptr;
	}
	
	UCommonActivatableWidgetStack* ThisStack = GetLayerStack(LayerTag);
	if (!ThisStack)
	{
		UE_LOG(LogModulusUI, Warning, TEXT("UISubsystem::PushWidgetToLayer -- no stack for tag '%s'"),
			*LayerTag.ToString());
		return nullptr;
	}
	
	UCommonActivatableWidget* NewWidget = nullptr;

	if (LayerTag == MCore_UILayerTags::MCore_UI_Layer_Modal)
	{
		/* Modal widgets are single-use: explicit creation with PlayerController as Outer,
		   then instance-based push so the stack does not manage the widget's lifetime. */
		const ULocalPlayer* LocalPlayer = GetLocalPlayer();
		APlayerController* PC = LocalPlayer ? LocalPlayer->GetPlayerController(GetWorld()) : nullptr;

		if (!PC)
		{
			UE_LOG(LogModulusUI, Warning, TEXT("UISubsystem::PushWidgetToLayer -- no PlayerController for modal creation"));
			return nullptr;
		}

		NewWidget = CreateWidget<UCommonActivatableWidget>(PC, WidgetClass);
		if (NewWidget)
		{
			ThisStack->AddWidgetInstance(*NewWidget);
		}
	}
	else
	{
		/* Non-modal layers: stack-managed creation via class-based overload */
		NewWidget = ThisStack->AddWidget<UCommonActivatableWidget>(WidgetClass);
	}

	if (NewWidget)
	{
		TrackedWidgets.FindOrAdd(LayerTag).Add(NewWidget);
		OnWidgetPushed.Broadcast(NewWidget, LayerTag);

		UE_LOG(LogModulusUI, Log, TEXT("UISubsystem::PushWidgetToLayer -- pushed '%s' to layer '%s'"),
			*WidgetClass->GetName(), *LayerTag.ToString());
	}
	else
	{
		UE_LOG(LogModulusUI, Warning, TEXT("UISubsystem::PushWidgetToLayer -- failed to add '%s' to layer '%s'"),
			*WidgetClass->GetName(), *LayerTag.ToString());
	}
	
	return NewWidget;
}

UCommonActivatableWidget* UMCore_UISubsystem::OpenScreen(
	TSubclassOf<UCommonActivatableWidget> ScreenClass,
	FGameplayTag LayerTag,
	bool bAllowDuplicates)
{
	if (!ScreenClass || !HasPrimaryGameLayout()) { return nullptr; }

	// TODO: Temporary diagnostic — remove after modal lifecycle audit
	UE_LOG(LogModulusUI, Log, TEXT("OpenScreen -- Class=%s Layer=%s TrackedCount=%d"),
		*GetNameSafe(ScreenClass), *LayerTag.ToString(),
		TrackedWidgets.Contains(LayerTag) ? TrackedWidgets[LayerTag].Num() : 0);

	if (!bAllowDuplicates)
	{
		CompactTrackedWidgets(LayerTag);

		TArray<TWeakObjectPtr<UCommonActivatableWidget>>* Widgets = TrackedWidgets.Find(LayerTag);

		if (Widgets)
		{
			for (int32 i = Widgets->Num() - 1; i >= 0; --i)
			{
				TWeakObjectPtr<UCommonActivatableWidget>& Weak = (*Widgets)[i];

				// TODO: Temporary diagnostic — remove after modal lifecycle audit
				UE_LOG(LogModulusUI, Log, TEXT("OpenScreen -- Found tracked: %s IsValid=%s IsActivated=%s"),
					*GetNameSafe(Weak.Get()),
					Weak.IsValid() ? TEXT("Y") : TEXT("N"),
					(Weak.IsValid() && Weak->IsActivated()) ? TEXT("Y") : TEXT("N"));

				if (Weak->GetClass() == ScreenClass)
				{
					if (Weak->IsActivated())
					{
						return Weak.Get();
					}
					/* Stale entry: tracked but no longer activated */
					UntrackWidget(Weak.Get(), LayerTag);
				}
			}
		}
	}
	
	return PushWidgetToLayer(ScreenClass, LayerTag);
}

void UMCore_UISubsystem::CloseScreen(UCommonActivatableWidget* Screen)
{
	if (!Screen) { return; }

	bool bWasTracked{false};
	for (auto& Pair : TrackedWidgets)
	{
		const bool bFound = Pair.Value.ContainsByPredicate([Screen](const TWeakObjectPtr<UCommonActivatableWidget>& WeakWidget)
		{
			return WeakWidget.Get() == Screen;
		});

		if (bFound)
		{
			UntrackWidget(Screen, Pair.Key);
			bWasTracked = true;
			break;
		}
	}

	if (!bWasTracked)
	{
		UE_LOG(LogModulusUI, Warning, TEXT("UISubsystem::CloseScreen -- widget '%s' was not tracked"),
			*GetNameSafe(Screen));
	}

	Screen->DeactivateWidget();
}

bool UMCore_UISubsystem::PopLayer(FGameplayTag LayerTag)
{
	UCommonActivatableWidgetStack* Stack = GetLayerStack(LayerTag);
	if (!Stack) { return false; }

	UCommonActivatableWidget* TopWidget = Stack->GetActiveWidget();
	if (!TopWidget) { return false; }

	CloseScreen(TopWidget);
	return true;
}

bool UMCore_UISubsystem::RemoveWidgetFromLayer(UCommonActivatableWidget* Widget, FGameplayTag LayerTag)
{
	if (!Widget) { return false; }

	const TArray<TWeakObjectPtr<UCommonActivatableWidget>>* Widgets = TrackedWidgets.Find(LayerTag);
	if (!Widgets) { return false; }

	const bool bFound = Widgets->ContainsByPredicate([Widget](const TWeakObjectPtr<UCommonActivatableWidget>& WeakWidget)
	{
		return WeakWidget.Get() == Widget;
	});

	if (!bFound) { return false; }

	CloseScreen(Widget);
	return true;
}

UCommonActivatableWidget* UMCore_UISubsystem::GetActiveWidgetInLayer(FGameplayTag LayerTag) const
{
	const UCommonActivatableWidgetStack* Stack = GetLayerStack(LayerTag);
	return Stack ? Stack->GetActiveWidget() : nullptr;
}

int32 UMCore_UISubsystem::GetWidgetCountInLayer(FGameplayTag LayerTag) const
{
	const TArray<TWeakObjectPtr<UCommonActivatableWidget>>* Widgets = TrackedWidgets.Find(LayerTag);
	if (!Widgets) { return 0; }

	int32 Count{0};
	for (const TWeakObjectPtr<UCommonActivatableWidget>& Weak : *Widgets)
	{
		if (Weak.IsValid()) { Count++; }
	}
	return Count;
}

bool UMCore_UISubsystem::IsLayerActive(FGameplayTag LayerTag) const
{
	return GetActiveWidgetInLayer(LayerTag) != nullptr;
}

// ============================================================================
// WIDGET TRACKING
// ============================================================================

void UMCore_UISubsystem::CompactTrackedWidgets(FGameplayTag LayerTag)
{
	TArray<TWeakObjectPtr<UCommonActivatableWidget>>* Widgets = TrackedWidgets.Find(LayerTag);
	if (!Widgets) { return; }

	Widgets->RemoveAllSwap([](const TWeakObjectPtr<UCommonActivatableWidget>& WeakWidget)
	{
		return !WeakWidget.IsValid();
	});
}

void UMCore_UISubsystem::UntrackWidget(UCommonActivatableWidget* Widget, FGameplayTag LayerTag)
{
	TArray<TWeakObjectPtr<UCommonActivatableWidget>>* Widgets = TrackedWidgets.Find(LayerTag);
	if (!Widgets) { return; }

	const int32 Removed = Widgets->RemoveAllSwap([Widget](const TWeakObjectPtr<UCommonActivatableWidget>& WeakWidget)
	{
		return WeakWidget.Get() == Widget;
	});

	if (Removed > 0)
	{
		OnWidgetRemoved.Broadcast(Widget, LayerTag);
	}
}

void UMCore_UISubsystem::NotifyWidgetDestroyed(UCommonActivatableWidget* Widget)
{
	if (!Widget) { return; }

	for (auto& Pair : TrackedWidgets)
	{
		const int32 Removed = Pair.Value.RemoveAllSwap([Widget](const TWeakObjectPtr<UCommonActivatableWidget>& WeakWidget)
		{
			return WeakWidget.Get() == Widget;
		});

		if (Removed > 0)
		{
			OnWidgetRemoved.Broadcast(Widget, Pair.Key);
		}
	}
}

// ============================================================================
// MENU HUB
// ============================================================================

UMCore_GameMenuHub* UMCore_UISubsystem::OpenMenuHub()
{
	if (!MenuHubClass)
	{
		UE_LOG(LogModulusUI, Warning, TEXT("UISubsystem::OpenMenuHub -- MenuHubClass not loaded"));
		return nullptr;
	}

	UCommonActivatableWidget* Screen = OpenScreen(MenuHubClass, MCore_UILayerTags::MCore_UI_Layer_GameMenu);
	UMCore_GameMenuHub* Hub = Cast<UMCore_GameMenuHub>(Screen);

	if (Hub)
	{
		Hub->RebuildTabBar();
	}

	return Hub;
}

void UMCore_UISubsystem::CloseMenuHub()
{
	if (UMCore_GameMenuHub* Hub = FindTrackedMenuHub())
	{
		CloseScreen(Hub);
	}
}

UMCore_GameMenuHub* UMCore_UISubsystem::FindTrackedMenuHub() const
{
	const TArray<TWeakObjectPtr<UCommonActivatableWidget>>* Widgets =
		TrackedWidgets.Find(MCore_UILayerTags::MCore_UI_Layer_GameMenu);
	if (!Widgets) { return nullptr; }

	for (const TWeakObjectPtr<UCommonActivatableWidget>& Weak : *Widgets)
	{
		if (UMCore_GameMenuHub* Hub = Cast<UMCore_GameMenuHub>(Weak.Get()))
		{
			return Hub;
		}
	}
	return nullptr;
}

/**
 * @param TabID              Unique tag identifying this tab. Duplicates are rejected with a warning.
 * @param ScreenWidgetClass  Widget class to instantiate as the tab's page content. Must be non-null.
 * @param Priority           Sort order (lower values appear first). Default: 100.
 */
void UMCore_UISubsystem::RegisterMenuScreen(FGameplayTag TabID,
	TSubclassOf<UCommonActivatableWidget> ScreenWidgetClass,
	int32 Priority, UTexture2D* TabIcon)
{
	if (!ScreenWidgetClass)
	{
		UE_LOG(LogModulusUI, Warning, 
			TEXT("UISubsystem::RegisterMenuScreen -- invalid ScreenClass provided"));
		return;
	}

	if (!TabID.IsValid())
	{
		UE_LOG(LogModulusUI, Error, 
			TEXT("UISubsystem::RegisterMenuScreen -- invalid TabID"));
		return;
	}
	
	/* Check for duplicate registration */
	if (RegisteredMenuScreens.ContainsByPredicate([TabID](const FMCore_MenuTab& Tab)
	 {
		 return Tab.TabID == TabID;
	 }))
	{
		UE_LOG(LogModulusUI, Warning, 
			TEXT("UISubsystem::RegisterMenuScreen -- tab '%s' already registered, skipping duplicate"),
			*TabID.ToString());
		return;
	}

	/* Create registration entry */
	FMCore_MenuTab NewTab;
	NewTab.TabID = TabID;
	NewTab.ScreenWidgetClass = ScreenWidgetClass;
	NewTab.Priority = Priority;
	NewTab.TabIcon = TabIcon;
    
	/* Insert sorted by priority (lower = earlier in tab list) */
	int32 InsertIndex{0};
	for (; InsertIndex < RegisteredMenuScreens.Num(); ++InsertIndex)
	{
		if (RegisteredMenuScreens[InsertIndex].Priority > Priority)
		{
			break;
		}
	}
	RegisteredMenuScreens.Insert(NewTab, InsertIndex);
    
	UE_LOG(LogModulusUI, Log,
		TEXT("UISubsystem::RegisterMenuScreen -- %s registered at priority %d (Total: %d)"),
		*NewTab.GetDisplayName().ToString(), Priority, RegisteredMenuScreens.Num());

	/* If MenuHub is active, rebuild tab bar */
	if (UMCore_GameMenuHub* Hub = FindTrackedMenuHub())
	{
		Hub->RebuildTabBar();
	}
}

bool UMCore_UISubsystem::UnregisterMenuScreen(FGameplayTag TabID)
{
	if (!TabID.IsValid())
	{
		UE_LOG(LogModulusUI, Warning, 
			TEXT("UISubsystem::UnregisterMenuScreen -- invalid TabID"));
		return false;
	}

	const int32 RemovedCount = RegisteredMenuScreens.RemoveAll([TabID](const FMCore_MenuTab& Tab)
	{
		return Tab.TabID == TabID;
	});

	if (RemovedCount > 0)
	{
		UE_LOG(LogModulusUI, Log,
			TEXT("UISubsystem::UnregisterMenuScreen -- '%s' unregistered (Remaining: %d)"),
			*TabID.ToString(), RegisteredMenuScreens.Num());

		if (UMCore_GameMenuHub* Hub = FindTrackedMenuHub())
		{
			Hub->RebuildTabBar();
		}
		return true;
	}

	UE_LOG(LogModulusUI, Warning,
		TEXT("UISubsystem::UnregisterMenuScreen -- tab '%s' not found"),
		*TabID.ToString());
	return false;
}

void UMCore_UISubsystem::RebuildMenuHubTabBar()
{
	if (UMCore_GameMenuHub* Hub = FindTrackedMenuHub())
	{
		Hub->RebuildTabBar();
	}
}

// ============================================================================
// THEME
// ============================================================================

const TArray<FMCore_ThemeEntry>& UMCore_UISubsystem::GetAvailableThemes() const
{
	const UMCore_CoreSettings* Settings = UMCore_CoreSettings::Get();
	if (Settings)
	{
		return Settings->AvailableThemes;
	}

	static TArray<FMCore_ThemeEntry> EmptyArray;
	return EmptyArray;
}

bool UMCore_UISubsystem::SetActiveThemeByIndex(int32 ThemeIndex)
{
	const UMCore_CoreSettings* Settings = UMCore_CoreSettings::Get();
	if (!Settings || !Settings->IsValidThemeIndex(ThemeIndex))
	{
		UE_LOG(LogModulusUI, Warning, TEXT("UISubsystem::SetActiveThemeByIndex -- invalid theme index %d"), ThemeIndex);
		return false;
	}

	if (ThemeIndex == ActiveThemeIndex) { return true; }

	const FMCore_ThemeEntry& ThemeEntry = Settings->AvailableThemes[ThemeIndex];
	if (ThemeEntry.ThemeAsset.IsNull())
	{
		UE_LOG(LogModulusUI, Warning, TEXT("UISubsystem::SetActiveThemeByIndex -- theme at index %d has no asset"), ThemeIndex);
		return false;
	}

	UMCore_PDA_UITheme_Base* LoadedTheme = ThemeEntry.ThemeAsset.LoadSynchronous();
	if (!LoadedTheme)
	{
		UE_LOG(LogModulusUI, Warning, TEXT("UISubsystem::SetActiveThemeByIndex -- failed to load theme at index %d"), ThemeIndex);
		return false;
	}

	SetActiveTheme(LoadedTheme);
	ActiveThemeIndex = ThemeIndex;

	UE_LOG(LogModulusUI, Log, TEXT("UISubsystem::SetActiveThemeByIndex -- theme changed to '%s' (index %d)"),
		*ThemeEntry.DisplayName.ToString(), ThemeIndex);

	return true;
}

void UMCore_UISubsystem::SetActiveTheme(UMCore_PDA_UITheme_Base* NewTheme)
{
	if (!NewTheme || NewTheme == CachedActiveTheme) { return; }

	CachedActiveTheme = NewTheme;
	ActiveThemeIndex = INDEX_NONE;

	OnThemeChanged.Broadcast(CachedActiveTheme);
	BroadcastThemeChangedTagEvent(CachedActiveTheme);
	PersistActiveTheme(CachedActiveTheme);

	UE_LOG(LogModulusUI, Log, TEXT("UISubsystem::SetActiveTheme -- theme changed to '%s'"),
		*NewTheme->GetPathName());
}

UMCore_PDA_UITheme_Base* UMCore_UISubsystem::LoadSavedActiveTheme() const
{
	// TODO: Remove after subsystem init order diagnostic (2026-05-08)
	{
		const ULocalPlayer* LP = GetLocalPlayer();
		const UMCore_PlayerSettingsSubsystem* PSS =
			LP ? LP->GetSubsystem<UMCore_PlayerSettingsSubsystem>() : nullptr;
		UE_LOG(LogModulusUI, Log,
			TEXT("LoadSavedActiveTheme -- entered; LocalPlayer=%s PlayerSettingsSubsystem=%s"),
			LP ? TEXT("valid") : TEXT("null"),
			PSS ? TEXT("valid") : TEXT("null"));
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer) { return nullptr; }

	UMCore_PlayerSettingsSubsystem* SettingsSubsystem = LocalPlayer->GetSubsystem<UMCore_PlayerSettingsSubsystem>();
	if (!SettingsSubsystem) { return nullptr; }

	UMCore_PlayerSettingsSave* Save = SettingsSubsystem->GetPlayerSettings();

	// TODO: Remove after subsystem init order diagnostic (2026-05-08)
	if (Save)
	{
		UE_LOG(LogModulusUI, Log,
			TEXT("LoadSavedActiveTheme -- ActiveThemePath='%s' IsEmpty=%s"),
			*Save->ActiveThemePath.ToString(),
			Save->ActiveThemePath.IsNull() ? TEXT("Y") : TEXT("N"));
	}

	if (!Save || !Save->ActiveThemePath.IsValid()) { return nullptr; }

	return Cast<UMCore_PDA_UITheme_Base>(Save->ActiveThemePath.TryLoad());
}

void UMCore_UISubsystem::PersistActiveTheme(UMCore_PDA_UITheme_Base* NewTheme)
{
	if (!NewTheme) { return; }

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer) { return; }

	UMCore_PlayerSettingsSubsystem* SettingsSubsystem = LocalPlayer->GetSubsystem<UMCore_PlayerSettingsSubsystem>();
	if (!SettingsSubsystem) { return; }

	UMCore_PlayerSettingsSave* Save = SettingsSubsystem->GetPlayerSettings();
	if (!Save) { return; }

	const FSoftObjectPath NewPath(NewTheme);
	if (Save->ActiveThemePath == NewPath) { return; }

	Save->ActiveThemePath = NewPath;
	Save->SaveSettings();
}

void UMCore_UISubsystem::BroadcastThemeChangedTagEvent(UMCore_PDA_UITheme_Base* NewTheme)
{
	if (!NewTheme) { return; }

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer) { return; }

	UMCore_LocalEventSubsystem* LES = LocalPlayer->GetSubsystem<UMCore_LocalEventSubsystem>();
	if (!LES) { return; }

	FMCore_EventData EventData;
	EventData.EventTag = MCore_ThemeTags::MCore_Theme_Changed;
	EventData.EventParams.Emplace(TEXT("ThemePath"), NewTheme->GetPathName());
	LES->BroadcastLocalEvent(EventData);
}

void UMCore_UISubsystem::NotifyTextSizeChanged()
{
	if (CachedActiveTheme)
	{
		OnThemeChanged.Broadcast(CachedActiveTheme);
	}
}

void UMCore_UISubsystem::HandleLocalEvent(const FMCore_EventData& EventData)
{
	// TODO: Replace with tag-filtered subscription on LocalEventSubsystem once Event System Phase 2
	// lands. This handler currently fires for ALL local events and manually compares tags.
	// Tracked settings that require UI re-resolution:
	//   - MCore_Settings_Accessibility_UITextSize (text style array re-index)
	//   - MCore_Settings_Accessibility_ColorblindMode (future: theme color re-resolve)
	//   - MCore_Settings_Accessibility_UIScale (future: ApplicationScale update)
	
	if (EventData.EventTag == MCore_SettingsTags::MCore_Settings_Accessibility_UITextSize)
	{
		NotifyTextSizeChanged();
	}
}
