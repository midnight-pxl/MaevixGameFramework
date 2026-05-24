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
UCLASS(Config=ModulusCore)
class MODULUSCORE_API UMCore_GlobalEventSubsystem : public UGameInstanceSubsystem
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
	
	/**
	 * Register the network replicator component.
	 * Called by GlobalEventReplicator::BeginPlay().
	 */
	void RegisterEventReplicator(UMCore_GlobalEventReplicator* Replicator);

	/**
	 * Unregister the network replicator component.
	 * Called by GlobalEventReplicator::EndPlay().
	 */
	void UnregisterEventReplicator(UMCore_GlobalEventReplicator* Replicator);

	/** 
	 * Register listener for global events.
	 * Called by UMCore_EventListenerComponent::BeginPlay()
	 */
	void RegisterGlobalListener(UMCore_EventListenerComponent* ListenerComponent);

	/**
	 * Unregister listener from global events.
	 * Called by UMCore_EventListenerComponent::EndPlay()
	 */
	void UnregisterGlobalListener(UMCore_EventListenerComponent* ListenerComponent);
	
	/**
	 * Deliver event to all registered local listeners.
	 * Called by GlobalEventReplicator after network transport.
	 */
	void DeliverToLocalListeners(const FMCore_EventData& EventData);

	/** Returns true if this instance has authority to broadcast global events (server or standalone). */
	UFUNCTION(BlueprintCallable, Category = "ModulusCore|Events")
	bool HasGlobalEventAuthority() const;
	
	/** Validates an inbound event request. Override to add custom validation rules. */
	bool ValidateEventRequest(const FMCore_EventData& EventData) const;

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	/** Registered global listener components */
	UPROPERTY()
	TArray<TWeakObjectPtr<UMCore_EventListenerComponent>> GlobalListeners;
	
	/* Cached reference to the network replicator on GameState */
	TWeakObjectPtr<UMCore_GlobalEventReplicator> EventReplicator;

	bool IsNetworkedGame() const;
};