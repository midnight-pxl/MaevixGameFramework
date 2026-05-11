// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreLoading/MCore_LoadingScreenSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "CoreData/Assets/Loading/MCore_PDA_LoadingScreens.h"
#include "CoreData/DevSettings/MCore_CoreSettings.h"
#include "CoreData/Logging/LogModulusLoading.h"
#include "CoreLoading/MCore_LoadingScreenWidget.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "MoviePlayer.h"

// ============================================================================
// LIFECYCLE
// ============================================================================

bool UMCore_LoadingScreenSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (IsRunningDedicatedServer())
	{
		return false;
	}
	return Super::ShouldCreateSubsystem(Outer);
}

void UMCore_LoadingScreenSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UMCore_CoreSettings* Settings = UMCore_CoreSettings::Get();
	bLoadingScreenEnabled = Settings ? Settings->bLoadingScreenEnabledByDefault : true;

	PreLoadMapHandle  = FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UMCore_LoadingScreenSubsystem::HandlePreLoadMap);
	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UMCore_LoadingScreenSubsystem::HandlePostLoadMap);

	UE_LOG(LogModulusLoading, Log, TEXT("LoadingScreenSubsystem::Initialize: enabled=%s"),
		bLoadingScreenEnabled ? TEXT("true") : TEXT("false"));
}

void UMCore_LoadingScreenSubsystem::Deinitialize()
{
	if (PreLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PreLoadMap.Remove(PreLoadMapHandle);
		PreLoadMapHandle.Reset();
	}
	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}

	PendingOverrideEntry.Reset();
	PendingContextTags.Reset();

	Super::Deinitialize();
}

// ============================================================================
// PUBLIC API
// ============================================================================

void UMCore_LoadingScreenSubsystem::SetNextLoadingContext(const FGameplayTagContainer& ContextTags)
{
	PendingContextTags = ContextTags;
}

void UMCore_LoadingScreenSubsystem::OverrideNextLoadingScreen(const FMCore_LoadingScreenEntry& Entry)
{
	PendingOverrideEntry = Entry;
}

void UMCore_LoadingScreenSubsystem::SetLoadingScreenEnabled(bool bEnabled)
{
	bLoadingScreenEnabled = bEnabled;
}

void UMCore_LoadingScreenSubsystem::DismissLoadingScreen()
{
	bAwaitingDismissalInput = false;
	ActiveWidget.Reset();

	if (GetMoviePlayer())
	{
		GetMoviePlayer()->StopMovie();
	}
}

void UMCore_LoadingScreenSubsystem::SetNextTransitionManualDismiss(bool bManualDismiss)
{
	bNextTransitionManualDismiss = bManualDismiss;
}

UMCore_LoadingScreenWidget* UMCore_LoadingScreenSubsystem::GetActiveLoadingScreenWidget() const
{
	return ActiveWidget.Get();
}

// ============================================================================
// INTERNAL HANDLERS
// ============================================================================

void UMCore_LoadingScreenSubsystem::HandlePreLoadMap(const FString& MapName)
{
	if (!bLoadingScreenEnabled)
	{
		return;
	}

	const UMCore_CoreSettings* Settings = UMCore_CoreSettings::Get();
	if (!Settings || Settings->LoadingScreenWidgetClass.IsNull())
	{
		UE_LOG(LogModulusLoading, Verbose,
			TEXT("LoadingScreenSubsystem::HandlePreLoadMap: no widget class configured, skipping (map=%s)"),
			*MapName);
		return;
	}

	FMCore_LoadingScreenEntry SelectedEntry;
	if (PendingOverrideEntry.IsSet())
	{
		SelectedEntry = PendingOverrideEntry.GetValue();
	}
	else
	{
		SelectedEntry = SelectLoadingScreenEntry(FName(*MapName));
	}

	/* Sync-load the background. Game thread is already blocked on the level swap; async would race MoviePlayer setup. */
	if (!SelectedEntry.BackgroundImage.IsNull())
	{
		SelectedEntry.BackgroundImage.LoadSynchronous();
	}

	UClass* WidgetClass = Settings->LoadingScreenWidgetClass.LoadSynchronous();
	if (!WidgetClass)
	{
		UE_LOG(LogModulusLoading, Warning,
			TEXT("LoadingScreenSubsystem::HandlePreLoadMap: widget class failed to load, skipping (map=%s)"),
			*MapName);
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogModulusLoading, Warning,
			TEXT("LoadingScreenSubsystem::HandlePreLoadMap: no GameInstance, skipping (map=%s)"),
			*MapName);
		return;
	}

	UMCore_LoadingScreenWidget* Widget = CreateWidget<UMCore_LoadingScreenWidget>(GameInstance, WidgetClass);
	if (!Widget)
	{
		UE_LOG(LogModulusLoading, Warning,
			TEXT("LoadingScreenSubsystem::HandlePreLoadMap: CreateWidget returned null, skipping (map=%s)"),
			*MapName);
		return;
	}

	Widget->Initialize(SelectedEntry);
	ActiveWidget = Widget;

	/* bNextTransitionManualDismiss is a per-transition override that wins over
	   the configured default. Fold it into CurrentDismissalMode so downstream
	   handlers branch on a single source of truth. */
	CurrentDismissalMode = bNextTransitionManualDismiss
		? EMCore_LoadingDismissalMode::Manual
		: Settings->LoadingDismissalMode;

	const bool bManualOrInputDriven =
		CurrentDismissalMode == EMCore_LoadingDismissalMode::InputRequired ||
		CurrentDismissalMode == EMCore_LoadingDismissalMode::Manual;

	if (CurrentDismissalMode == EMCore_LoadingDismissalMode::InputRequired &&
		Settings->LoadingDismissalAction.IsNull())
	{
		UE_LOG(LogModulusLoading, Warning,
			TEXT("LoadingScreenSubsystem::HandlePreLoadMap: InputRequired mode active but LoadingDismissalAction is unset, falling back to any-input dismiss (map=%s)"),
			*MapName);
	}

	FLoadingScreenAttributes Attributes;
	Attributes.WidgetLoadingScreen              = Widget->TakeWidget();
	Attributes.bAllowEngineTick                 = true;
	Attributes.bAutoCompleteWhenLoadingCompletes = !bManualOrInputDriven;
	Attributes.bWaitForManualStop               = bManualOrInputDriven;
	Attributes.MinimumLoadingScreenDisplayTime  = Settings->MinimumLoadingScreenTime;

	GetMoviePlayer()->SetupLoadingScreen(Attributes);

	/* Clear one-shot state so the next transition starts fresh. */
	PendingOverrideEntry.Reset();
	bNextTransitionManualDismiss = false;

	UE_LOG(LogModulusLoading, Verbose,
		TEXT("LoadingScreenSubsystem::HandlePreLoadMap: displaying for map %s"), *MapName);
}

void UMCore_LoadingScreenSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
	PendingContextTags.Reset();

	if (CurrentDismissalMode == EMCore_LoadingDismissalMode::InputRequired)
	{
		bAwaitingDismissalInput = true;

		/* Direct call first so the widget's bAwaitingInput / focus / icon refresh
		   are guaranteed regardless of external subscriber count. */
		if (UMCore_LoadingScreenWidget* Widget = ActiveWidget.Get())
		{
			Widget->HandleLoadingComplete();
		}

		/* Broadcast after the widget transition so listeners see consistent state. */
		OnLoadingScreenReadyToDismiss.Broadcast();
	}

	UE_LOG(LogModulusLoading, Verbose,
		TEXT("LoadingScreenSubsystem::HandlePostLoadMap: cleared pending context (world=%s)"),
		LoadedWorld ? *LoadedWorld->GetName() : TEXT("null"));
}

// ============================================================================
// SELECTION
// ============================================================================

TArray<FMCore_LoadingScreenEntry> UMCore_LoadingScreenSubsystem::ResolveEntryPool() const
{
	TArray<FMCore_LoadingScreenEntry> Pool;

	const UMCore_CoreSettings* Settings = UMCore_CoreSettings::Get();
	if (!Settings)
	{
		return Pool;
	}

	for (const TSoftObjectPtr<UMCore_PDA_LoadingScreens>& LibraryPtr : Settings->LoadingScreenLibraries)
	{
		if (LibraryPtr.IsNull())
		{
			continue;
		}

		const UMCore_PDA_LoadingScreens* Library = LibraryPtr.LoadSynchronous();
		if (!Library)
		{
			continue;
		}

		Pool.Append(Library->Entries);
	}

	return Pool;
}

FMCore_LoadingScreenEntry UMCore_LoadingScreenSubsystem::SelectLoadingScreenEntry_Implementation(FName NextMapName)
{
	const TArray<FMCore_LoadingScreenEntry> Pool = ResolveEntryPool();

	if (Pool.Num() == 0)
	{
		UE_LOG(LogModulusLoading, Warning,
			TEXT("LoadingScreenSubsystem::SelectLoadingScreenEntry: entry pool is empty, returning default"));
		return FMCore_LoadingScreenEntry();
	}

	const UMCore_CoreSettings* Settings = UMCore_CoreSettings::Get();
	const EMCore_LoadingScreenSelectionMode Mode = Settings
		? Settings->LoadingScreenSelectionMode
		: EMCore_LoadingScreenSelectionMode::TagBased;

	switch (Mode)
	{
	case EMCore_LoadingScreenSelectionMode::Sequential:
	{
		LastSelectedIndex = (LastSelectedIndex + 1) % Pool.Num();
		return Pool[LastSelectedIndex];
	}

	case EMCore_LoadingScreenSelectionMode::TagBased:
	{
		TArray<FMCore_LoadingScreenEntry> Filtered;
		Filtered.Reserve(Pool.Num());
		for (const FMCore_LoadingScreenEntry& Entry : Pool)
		{
			const bool bTagMatch = !PendingContextTags.IsEmpty() && Entry.ContextTags.HasAny(PendingContextTags);
			const bool bMapMatch = !NextMapName.IsNone() && Entry.AssociatedMaps.Contains(NextMapName);
			if (bTagMatch || bMapMatch)
			{
				Filtered.Add(Entry);
			}
		}

		if (Filtered.Num() > 0)
		{
			return Filtered[FMath::RandRange(0, Filtered.Num() - 1)];
		}

		/* No direct matches: fall back to entries with no constraints (the "any context" pool). */
		TArray<FMCore_LoadingScreenEntry> AnyContext;
		AnyContext.Reserve(Pool.Num());
		for (const FMCore_LoadingScreenEntry& Entry : Pool)
		{
			if (Entry.ContextTags.IsEmpty() && Entry.AssociatedMaps.Num() == 0)
			{
				AnyContext.Add(Entry);
			}
		}

		if (AnyContext.Num() > 0)
		{
			return AnyContext[FMath::RandRange(0, AnyContext.Num() - 1)];
		}

		/* Still no match: fall back to full-pool random. */
		return Pool[FMath::RandRange(0, Pool.Num() - 1)];
	}

	case EMCore_LoadingScreenSelectionMode::Custom:
		/* Subclasses override SelectLoadingScreenEntry. Default falls through to Random. */
	case EMCore_LoadingScreenSelectionMode::Random:
	default:
		return Pool[FMath::RandRange(0, Pool.Num() - 1)];
	}
}
