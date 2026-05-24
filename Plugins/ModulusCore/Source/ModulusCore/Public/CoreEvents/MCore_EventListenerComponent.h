// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "MCore_EventListenerComponent.generated.h"

class UMCore_LocalEventSubsystem;
class UMCore_GlobalEventSubsystem;
struct FMCore_EventData;

/**
 * Drop-in component for actors that receive Local and Global GameplayTag events.
 * Handles subsystem registration + tag filtering. Set SubscribedEvents to filter
 * (empty = all events); implement OnEventReceived in Blueprint to handle.
 */
UCLASS(ClassGroup=(ModulusCore), BlueprintType, meta=(BlueprintSpawnableComponent))
class MODULUSCORE_API UMCore_EventListenerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMCore_EventListenerComponent();

	/** Tags to filter events (e.g., MCore.Events.Player.*, MCore.Events.Quest.Completed). Leave empty to receive all events */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ModulusCore|Events|Listening", meta = (Categories = "MCore.Events"))
	FGameplayTagContainer SubscribedEvents;

	/** Receive events broadcast locally (this client only) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ModulusCore|Events|Listening")
	bool bReceiveLocalEvents{true};

	/** Receive events broadcast globally (networked from server) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ModulusCore|Events|Listening")
	bool bReceiveGlobalEvents{true};

	/**
	 * Called when a subscribed event is received.
	 *
	 * Implement in Blueprint to handle event notifications.
	 * Use EventData.EventTag to determine event type, then query EventData.Parameters or subsystems for details.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "ModulusCore|Events|Handling")
	void OnEventReceived(const FMCore_EventData& EventData, bool bWasGlobalEvent);
	
	/** Called by subsystems to deliver events. Do not call directly. */
	void DeliverEvent(const FMCore_EventData& EventData, bool bWasGlobalEvent);

	/** Check if this component should receive a specific event based on tag filters */
	bool ShouldReceiveEvent(const FMCore_EventData& EventData, bool bIsGlobalEvent) const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/* Resolves the LocalPlayer from the owning actor by trying, in order:
	 *   (1) PlayerController owner -> GetLocalPlayer();
	 *   (2) Pawn owner -> Controller -> GetLocalPlayer();
	 *   (3) Instigator chain -> Controller -> GetLocalPlayer();
	 *   (4) Owner-chain walk -> first AController encountered -> GetLocalPlayer();
	 *   (5) Fallback: GEngine->GetFirstGamePlayer().
	 * Step 5 fires only for actors with no player ownership (e.g. world-spawned
	 * reward actors). On dedicated server, returns nullptr (no LocalPlayer exists). */
	ULocalPlayer* ResolveOwningLocalPlayer() const;

	/** Cached reference to local event subsystem */
	UPROPERTY()
	TWeakObjectPtr<UMCore_LocalEventSubsystem> CachedLocalSubsystem;

	/** Cached reference to global event subsystem */
	UPROPERTY()
	TWeakObjectPtr<UMCore_GlobalEventSubsystem> CachedGlobalSubsystem;
};
