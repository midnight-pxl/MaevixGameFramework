// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CoreData/Types/Events/MCore_EventData.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "MCore_LocalEventSubsystem.generated.h"

class UMCore_EventListenerComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnLocalEventBroadcast, const FMCore_EventData& /*EventData*/);

/**
 * Local event subsystem for client-only, non-networked GameplayTag events.
 * Per-LocalPlayer (split-screen safe); use UMCore_GlobalEventSubsystem for networked broadcasts.
 */
UCLASS()
class MAEVIXCORE_API UMCore_LocalEventSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	/** Called by UMCore_EventListenerComponent::BeginPlay(). */
	void RegisterLocalListener(UMCore_EventListenerComponent* ListenerComponent);

	/** Called by UMCore_EventListenerComponent::EndPlay(). */
	void UnregisterLocalListener(UMCore_EventListenerComponent* ListenerComponent);

	/**
	 * Broadcast event to all registered local listeners on this LocalPlayer's subsystem.
	 *
	 * SourceScope identifies whether the broadcast originated from a Local-scope call or an
	 * AllLocal iteration; delivered to listeners via OnEventReceived so they can differentiate
	 * without inspecting EventData. Defaults to Local for source compatibility with direct
	 * callers (e.g. UMCore_UISubsystem). Use UMCore_EventFunctionLibrary::BroadcastSimpleEvent()
	 * or sibling variants instead of calling this directly.
	 */
	void BroadcastLocalEvent(const FMCore_EventData& EventData,
		EMCore_EventScope SourceScope = EMCore_EventScope::Local);

	/**
	 * Native delegate fired on every local event broadcast. Subsystems can bind here
	 * instead of attaching a UMCore_EventListenerComponent.
	 */
	FOnLocalEventBroadcast OnLocalEventBroadcast;

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
private:
	UPROPERTY()
	TArray<TWeakObjectPtr<UMCore_EventListenerComponent>> LocalListeners;
};
