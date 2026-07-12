// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreEvents/MCore_EventListenerComponent.h"

#include "CoreData/Libraries/MCore_EventFunctionLibrary.h"
#include "CoreData/Logging/LogMaevixEvent.h"
#include "CoreData/Types/Events/MCore_EventData.h"
#include "CoreEvents/MCore_GlobalEventSubsystem.h"
#include "CoreEvents/MCore_LocalEventSubsystem.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UMCore_EventListenerComponent::UMCore_EventListenerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = false;

	bReceiveLocalEvents = true;
	bReceiveGlobalEvents = true;
}

void UMCore_EventListenerComponent::BeginPlay()
{
	Super::BeginPlay();

	const UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogMaevixEvent, Warning, TEXT("EventListenerComp::BeginPlay: no valid world"));
		return;
	}

	/* Register with local event subsystem (per-LocalPlayer, split-screen safe) */
	if (bReceiveLocalEvents)
	{
		if (ULocalPlayer* LocalPlayer = ResolveOwningLocalPlayer())
		{
			if (UMCore_LocalEventSubsystem* LocalEventSys = LocalPlayer->GetSubsystem<UMCore_LocalEventSubsystem>())
			{
				CachedLocalSubsystem = LocalEventSys;
				LocalEventSys->RegisterLocalListener(this);
			}
			else
			{
				UE_LOG(LogMaevixEvent, Warning, TEXT("EventListenerComp::BeginPlay: failed to get LocalEventSubsystem for %s"), *GetNameSafe(this));
			}
		}
	}

	/* Register with global event subsystem (GameInstance-scoped) */
	if (bReceiveGlobalEvents)
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UMCore_GlobalEventSubsystem* GlobalEventSys = GameInstance->GetSubsystem<UMCore_GlobalEventSubsystem>())
			{
				CachedGlobalSubsystem = GlobalEventSys;
				GlobalEventSys->RegisterGlobalListener(this);
			}
			else
			{
				UE_LOG(LogMaevixEvent, Warning, TEXT("EventListenerComp::BeginPlay: failed to get GlobalEventSubsystem for %s"), *GetNameSafe(this));
			}
		}
	}

	UE_LOG(LogMaevixEvent, Verbose, TEXT("EventListenerComp::BeginPlay: initialized %s (Local: %s, Global: %s)"),
		   *GetNameSafe(this),
		   bReceiveLocalEvents ? TEXT("Yes") : TEXT("No"),
		   bReceiveGlobalEvents ? TEXT("Yes") : TEXT("No"));
}

ULocalPlayer* UMCore_EventListenerComponent::ResolveOwningLocalPlayer() const
{
	if (ULocalPlayer* LocalPlayer = UMCore_EventFunctionLibrary::ResolveLocalPlayerFromContext(this))
	{
		return LocalPlayer;
	}

	/* No owner chain resolved to a specific LocalPlayer. No Player 0 fallback (would
	 * contaminate split-screen); fail loudly so the component is attached to a
	 * player-owned actor or wired via UMCore_LocalEventSubsystem explicitly. */
	UE_LOG(LogMaevixEvent, Warning,
		TEXT("EventListenerComponent::ResolveOwningLocalPlayer -- actor '%s' has no LocalPlayer owner chain. "
			 "Component will not register. Attach to a player-owned actor (Pawn/Controller/Widget) "
			 "or call UMCore_LocalEventSubsystem::RegisterLocalListener directly with an explicit LocalPlayer."),
		*GetNameSafe(GetOwner()));
	return nullptr;
}

void UMCore_EventListenerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	/* Unregister from both subsystems */
	if (UMCore_LocalEventSubsystem* LocalEventSys = CachedLocalSubsystem.Get())
	{
		LocalEventSys->UnregisterLocalListener(this);
	}
	CachedLocalSubsystem.Reset();

	if (UMCore_GlobalEventSubsystem* GlobalEventSys = CachedGlobalSubsystem.Get())
	{
		GlobalEventSys->UnregisterGlobalListener(this);
	}
	CachedGlobalSubsystem.Reset();

	UE_LOG(LogMaevixEvent, Verbose, TEXT("EventListenerComp::EndPlay: cleanup %s"), *GetNameSafe(this));

	Super::EndPlay(EndPlayReason);
}

void UMCore_EventListenerComponent::DeliverEvent(const FMCore_EventData& EventData, EMCore_EventScope SourceScope)
{
	const TCHAR* ScopeName = TEXT("Unknown");
	switch (SourceScope)
	{
	case EMCore_EventScope::Local:    ScopeName = TEXT("Local");    break;
	case EMCore_EventScope::AllLocal: ScopeName = TEXT("AllLocal"); break;
	case EMCore_EventScope::Global:   ScopeName = TEXT("Global");   break;
	default: checkNoEntry();          break;
	}
	UE_LOG(LogMaevixEvent, VeryVerbose, TEXT("EventListenerComp::DeliverEvent -- delivering to %s: %s (Scope: %s)"),
	   *GetNameSafe(this), *EventData.EventTag.ToString(), ScopeName);

	OnEventReceived(EventData, SourceScope);
}

bool UMCore_EventListenerComponent::ShouldReceiveEvent(const FMCore_EventData& EventData, EMCore_EventScope SourceScope) const
{
	switch (SourceScope)
	{
	case EMCore_EventScope::Local:
		if (!bReceiveLocalEvents) { return false; }
		break;
	case EMCore_EventScope::AllLocal:
		if (!bReceiveAllLocalEvents) { return false; }
		break;
	case EMCore_EventScope::Global:
		if (!bReceiveGlobalEvents) { return false; }
		break;
	default:
		checkNoEntry();
		return false;
	}

	/* Empty subscription list means receive all events. */
	if (!SubscribedEvents.IsEmpty())
	{
		return EventData.EventTag.MatchesAny(SubscribedEvents);
	}

	return true;
}
