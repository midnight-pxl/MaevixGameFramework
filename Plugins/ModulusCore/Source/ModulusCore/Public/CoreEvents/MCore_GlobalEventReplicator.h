// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MCore_GlobalEventReplicator.generated.h"

struct FMCore_EventData;
class UMCore_GlobalEventSubsystem;

/**
 * Replicated component handling global event network transport via Server RPCs and Multicast.
 * Attach to GameState (or use AMCore_GameStateBase) to enable cross-network event broadcasting.
 *
 * Pure network transport; all business logic lives in UMCore_GlobalEventSubsystem.
 *
 * Late-join semantics: this replicator does NOT persist event history. Clients that join the
 * session after a global event has fired will not receive that event. Consumers requiring
 * durable state should propagate state via replicated properties on the GameState or a
 * dedicated replicated subobject, then derive UI events from property change notifications --
 * do not rely on Global-scope event broadcasts as a state-restoration mechanism.
 */
UCLASS(ClassGroup=(ModulusCore), meta=(BlueprintSpawnableComponent, DisplayName="Global Event Replicator"))
class MODULUSCORE_API UMCore_GlobalEventReplicator : public UActorComponent
{
	GENERATED_BODY()

public:
	UMCore_GlobalEventReplicator();

	/**
	 * Request a global event broadcast.
	 * Routes to server RPC or direct multicast based on authority.
	 * Called by GlobalEventSubsystem - do not call directly.
	 */
	void RequestBroadcast(const FMCore_EventData& EventData);
	
protected:
	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End UActorComponent Interface
	
	/**
	 * Client -> Server: Request event broadcast.
	 *
	 * Network:
	 *   Clients      - Send event request to server
	 *   Server       - Validates via GlobalEventSubsystem, then multicasts
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRequestBroadcast(const FMCore_EventData& EventData);

	/**
	 * Server -> All Clients: Deliver validated event.
	 *
	 * Network:
	 *   Clients      - Receive and deliver to local listeners
	 *   Server       - Skips delivery (already delivered in RequestBroadcast)
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastToClients(const FMCore_EventData& EventData);

private:
	/* Find and cache the GlobalEventSubsystem */
	UMCore_GlobalEventSubsystem* GetEventSubsystem() const;
	
	/* Cached reference to avoid repeated lookups */
	mutable TWeakObjectPtr<UMCore_GlobalEventSubsystem> CachedSubsystem;
};
