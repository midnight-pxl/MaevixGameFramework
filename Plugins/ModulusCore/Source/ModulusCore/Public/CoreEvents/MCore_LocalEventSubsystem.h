// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "MCore_LocalEventSubsystem.generated.h"

class UMCore_EventListenerComponent;
struct FMCore_EventData;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnLocalEventBroadcast, const FMCore_EventData& /*EventData*/);

/**
 * Local event subsystem for client-only, non-networked GameplayTag events.
 * Per-LocalPlayer (split-screen safe); use UMCore_GlobalEventSubsystem for networked broadcasts.
 */
UCLASS()
class MODULUSCORE_API UMCore_LocalEventSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	/** Register listener component. Called automatically by UMCore_EventListenerComponent::BeginPlay() */
	void RegisterLocalListener(UMCore_EventListenerComponent* ListenerComponent);

	/** Unregister listener component. Called automatically by UMCore_EventListenerComponent::EndPlay() */
	void UnregisterLocalListener(UMCore_EventListenerComponent* ListenerComponent);

	/**
	 * Broadcast event to all registered local listeners.
	 *
	 * Use UMCore_EventFunctionLibrary::BroadcastLocalEvent() instead of calling this directly.
	 */
	void BroadcastLocalEvent(const FMCore_EventData& EventData);

	/** Native delegate fired on every local event broadcast. Subsystems can bind here instead of using EventListenerComp. */
	FOnLocalEventBroadcast OnLocalEventBroadcast;

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
private:
	/** Registered local listener components */
	UPROPERTY()
	TArray<TWeakObjectPtr<UMCore_EventListenerComponent>> LocalListeners;
};
