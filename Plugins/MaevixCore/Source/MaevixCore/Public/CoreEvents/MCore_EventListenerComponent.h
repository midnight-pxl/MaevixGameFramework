// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CoreData/Types/Events/MCore_EventData.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "MCore_EventListenerComponent.generated.h"

class UMCore_LocalEventSubsystem;
class UMCore_GlobalEventSubsystem;

/**
 * Drop-in component for actors that receive Local and Global GameplayTag events.
 * Handles subsystem registration + tag filtering. Set SubscribedEvents to filter
 * (empty = all events); implement OnEventReceived in Blueprint to handle.
 */
UCLASS(ClassGroup=(MaevixCore), BlueprintType, meta=(BlueprintSpawnableComponent))
class MAEVIXCORE_API UMCore_EventListenerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMCore_EventListenerComponent();

	/**
	 * Tags to filter events (e.g., MCore.Events.Player.*, MCore.Events.Quest.Completed).
	 * Leave empty to receive all events.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaevixCore|Events|Listening", meta = (Categories = "MCore.Events"))
	FGameplayTagContainer SubscribedEvents;

	/** Receive events broadcast with EMCore_EventScope::Local (this LocalPlayer only). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaevixCore|Events|Listening")
	bool bReceiveLocalEvents{true};

	/**
	 * Receive events broadcast with EMCore_EventScope::AllLocal (every LocalPlayer on this GameInstance).
	 * Defaults true to match the inclusive defaults of bReceiveLocalEvents/bReceiveGlobalEvents. Disable
	 * for per-LocalPlayer concerns that should not respond to couch-co-op shared broadcasts (e.g.
	 * per-player input rebinding UI).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaevixCore|Events|Listening")
	bool bReceiveAllLocalEvents{true};

	/** Receive events broadcast with EMCore_EventScope::Global (networked, server-authoritative). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaevixCore|Events|Listening")
	bool bReceiveGlobalEvents{true};

	/**
	 * Called when a subscribed event is delivered to this component.
	 *
	 * SourceScope identifies how the event was broadcast:
	 *   - Local:    targeted at this component's owning LocalPlayer only
	 *   - AllLocal: targeted at every LocalPlayer on this GameInstance (couch co-op / split-screen)
	 *   - Global:   targeted at every connected client via server multicast
	 * BP authors should branch on SourceScope when a tag may be broadcast in multiple scopes and
	 * the listener's response differs per scope (e.g. UI vs gameplay state).
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "MaevixCore|Events|Handling")
	void OnEventReceived(const FMCore_EventData& EventData, EMCore_EventScope SourceScope);

	/**
	 * Internal: called by the LocalEventSubsystem / GlobalEventSubsystem that delivered this event.
	 * Invokes the BP-side OnEventReceived hook with the source scope. Do not call directly.
	 */
	void DeliverEvent(const FMCore_EventData& EventData, EMCore_EventScope SourceScope);

	/**
	 * Returns true if this listener's tag filter and scope opt-ins both accept the event.
	 * Called by both LocalEventSubsystem (with Local or AllLocal) and GlobalEventSubsystem (with Global)
	 * before invoking DeliverEvent.
	 */
	bool ShouldReceiveEvent(const FMCore_EventData& EventData, EMCore_EventScope SourceScope) const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/* Resolves the LocalPlayer owning this component's actor by delegating to
	 * UMCore_EventFunctionLibrary::ResolveLocalPlayerFromContext. Returns nullptr and
	 * logs a Warning when no owner chain resolves; components on unowned actors (and
	 * any actor in a dedicated server build) will not register. Split-screen safety:
	 * no Player 0 fallback. */
	ULocalPlayer* ResolveOwningLocalPlayer() const;

	// Cached reference to local event subsystem
	UPROPERTY()
	TWeakObjectPtr<UMCore_LocalEventSubsystem> CachedLocalSubsystem;

	// Cached reference to global event subsystem
	UPROPERTY()
	TWeakObjectPtr<UMCore_GlobalEventSubsystem> CachedGlobalSubsystem;
};
