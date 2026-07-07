// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CoreData/Types/Events/MCore_EventData.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MCore_GlobalEventSubsystem.generated.h"

class UMCore_EventListenerComponent;
class UMCore_GlobalEventReplicator;

/**
 * Server-authoritative event subsystem for networked GameplayTag events (one per GameInstance).
 * Requires UMCore_GlobalEventReplicator on GameState for transport; without one, broadcasts
 * stay local only with a warning.
 */
UCLASS(Config=MaevixCore)
class MAEVIXCORE_API UMCore_GlobalEventSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Broadcast a GameplayTag event to all clients via the active
	 * GlobalEventReplicator (server multicasts; clients send a server RPC).
	 * Falls back to local-listeners-only with a warning when no replicator
	 * is configured. Prefer UMCore_EventFunctionLibrary::BroadcastGlobalEvent().
	 */
	void BroadcastGlobalEvent(const FMCore_EventData& EventData);
	
	/** Called by UMCore_GlobalEventReplicator::BeginPlay(). */
	void RegisterEventReplicator(UMCore_GlobalEventReplicator* Replicator);

	/** Called by UMCore_GlobalEventReplicator::EndPlay(). */
	void UnregisterEventReplicator(UMCore_GlobalEventReplicator* Replicator);

	/** Called by UMCore_EventListenerComponent::BeginPlay(). */
	void RegisterGlobalListener(UMCore_EventListenerComponent* ListenerComponent);

	/** Called by UMCore_EventListenerComponent::EndPlay(). */
	void UnregisterGlobalListener(UMCore_EventListenerComponent* ListenerComponent);

	/**
	 * Called by UMCore_GlobalEventReplicator after network transport; fans
	 * out to all registered local listeners.
	 */
	void DeliverToLocalListeners(const FMCore_EventData& EventData);

	/** Returns true if this instance has authority to broadcast global events (server or standalone). */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|Events")
	bool HasGlobalEventAuthority() const;
	
	/**
	 * Server-side validation for client-originated broadcast requests; enforces the
	 * Network Safety caps in MCore_CoreSettings. Returns false to reject the event.
	 */
	bool ValidateEventRequest(const FMCore_EventData& EventData) const;

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	UPROPERTY()
	TArray<TWeakObjectPtr<UMCore_EventListenerComponent>> GlobalListeners;

	// Cached replicator; lives on GameState, weak so teardown doesn't dangle
	TWeakObjectPtr<UMCore_GlobalEventReplicator> EventReplicator;

	/* One-shot guard for the Standalone-no-replicator diagnostic. Resets on
	 * subsystem deinit. Not config, not replicated, not serialized. */
	bool bHasLoggedStandaloneNoReplicatorWarning = false;

	bool IsNetworkedGame() const;
};