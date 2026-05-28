// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreEvents/MCore_GlobalEventSubsystem.h"

#include "CoreEvents/MCore_GlobalEventReplicator.h"
#include "CoreData/DevSettings/MCore_CoreSettings.h"
#include "CoreData/Logging/LogMaevixEvent.h"
#include "CoreData/Types/Events/MCore_EventData.h"
#include "CoreEvents/MCore_EventListenerComponent.h"
#include "Serialization/BufferArchive.h"

void UMCore_GlobalEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogMaevixEvent, Log, TEXT("GlobalEventSubsystem::Initialize: initializing"));
}

void UMCore_GlobalEventSubsystem::Deinitialize()
{
	GlobalListeners.Empty();
	EventReplicator.Reset();
	
	Super::Deinitialize();
}

void UMCore_GlobalEventSubsystem::BroadcastGlobalEvent(const FMCore_EventData& EventData)
{
	if (!EventData.IsValid())
	{
		UE_LOG(LogMaevixEvent, Warning,
			TEXT("GlobalEventSubsystem::BroadcastGlobalEvent: attempted to broadcast invalid event"));
		return;
	}
	
	UE_LOG(LogMaevixEvent, Verbose,
		TEXT("GlobalEventSubsystem::BroadcastGlobalEvent: broadcasting: %s"),
		*EventData.EventTag.ToString());
	
	/* Route through event replicator if available */
	if (UMCore_GlobalEventReplicator* Replicator = EventReplicator.Get())
	{
		Replicator->RequestBroadcast(EventData);
	}
	else
	{
		/* No replicator: not configured or standalone */
		if (HasGlobalEventAuthority())
		{
			/* Deliver locally */
			DeliverToLocalListeners(EventData);

			if (IsNetworkedGame())
			{
				UE_LOG(LogMaevixEvent, Warning,
					TEXT("GlobalEventSubsystem::BroadcastGlobalEvent -- delivered locally only, no replicator found; "
						 "add GlobalEventReplicator to GameState for network support."));
				return;
			}
			else if (!bHasLoggedStandaloneNoReplicatorWarning)
			{
				/* Standalone-with-authority path: fire a one-shot diagnostic so devs notice
				 * silent multiplayer breakage during PIE testing. Suppressible via
				 * UMCore_CoreSettings::bSuppressStandaloneNoReplicatorWarning. */
				bHasLoggedStandaloneNoReplicatorWarning = true;
				const UMCore_CoreSettings* Settings = UMCore_CoreSettings::Get();
				if (!Settings || !Settings->bSuppressStandaloneNoReplicatorWarning)
				{
					UE_LOG(LogMaevixEvent, Warning,
						TEXT("GlobalEventSubsystem::BroadcastGlobalEvent -- no UMCore_GlobalEventReplicator on GameState; "
							 "Global scope is degrading to local-only delivery on this GameInstance. This is correct for "
							 "Standalone but will silently mask broken multiplayer code in PIE. Suppress via "
							 "UMCore_CoreSettings::bSuppressStandaloneNoReplicatorWarning if intentional."));
				}
			}
		}
		else
		{
			/* Client w/o replicator: cannot request broadcast */
			UE_LOG(LogMaevixEvent, Warning,
				TEXT("GlobalEventSubsystem::BroadcastGlobalEvent -- not authority and no replicator found; "
					 "add GlobalEventReplicator to GameState for network support."))
			return;
		}
	}

	UE_LOG(LogMaevixEvent, Log,
		TEXT("GlobalEventSubsystem::BroadcastGlobalEvent: broadcast complete: %s"),
		*EventData.EventTag.ToString());
}

void UMCore_GlobalEventSubsystem::RegisterEventReplicator(UMCore_GlobalEventReplicator* Replicator)
{
	if (IsValid(Replicator))
	{
		EventReplicator = Replicator;
		UE_LOG(LogMaevixEvent, Log, TEXT("GlobalEventSubsystem::RegisterEventReplicator: registered: %s"),
			Replicator->GetOwner() ? *Replicator->GetOwner()->GetName() : TEXT("Unknown"));
	}
}

void UMCore_GlobalEventSubsystem::UnregisterEventReplicator(UMCore_GlobalEventReplicator* Replicator)
{
	if (EventReplicator.Get() == Replicator)
	{
		EventReplicator.Reset();
		UE_LOG(LogMaevixEvent, Log, TEXT("GlobalEventSubsystem::UnregisterEventReplicator: unregistered"));
	}
}

void UMCore_GlobalEventSubsystem::RegisterGlobalListener(UMCore_EventListenerComponent* ListenerComponent)
{
	if (IsValid(ListenerComponent))
	{
		GlobalListeners.AddUnique(ListenerComponent);
		UE_LOG(LogMaevixEvent, Verbose, TEXT("GlobalEventSubsystem::RegisterGlobalListener: registered: %s"),
			*ListenerComponent->GetName());
	}
}

void UMCore_GlobalEventSubsystem::UnregisterGlobalListener(UMCore_EventListenerComponent* ListenerComponent)
{
	int32 RemoveCount = GlobalListeners.RemoveAll([ListenerComponent](const TWeakObjectPtr<UMCore_EventListenerComponent>& RemovePtr)
	{
		return !RemovePtr.IsValid() || RemovePtr.Get() == ListenerComponent;
	});

	if (RemoveCount > 0)
	{
		UE_LOG(LogMaevixEvent, Verbose, TEXT("GlobalEventSubsystem::UnregisterGlobalListener: unregistered: %s"),
			ListenerComponent ? *ListenerComponent->GetName() : TEXT("Unknown"));
	}
}

void UMCore_GlobalEventSubsystem::DeliverToLocalListeners(const FMCore_EventData& EventData)
{
	if (GlobalListeners.IsEmpty()) { return; }

	UE_LOG(LogMaevixEvent, Verbose, TEXT("GlobalEventSubsystem::DeliverToLocalListeners -- delivering '%s' to %d listeners"),
		*EventData.EventTag.ToString(), GlobalListeners.Num());

	/* Reverse traverse to remove stale entries */
	for (int32 i = GlobalListeners.Num() - 1; i >= 0; --i)
	{
		TWeakObjectPtr<UMCore_EventListenerComponent>& CurListener = GlobalListeners[i];

		if (CurListener.IsValid())
		{
			UMCore_EventListenerComponent* ListenerComp = CurListener.Get();
			if (ListenerComp && CurListener->ShouldReceiveEvent(EventData, EMCore_EventScope::Global))
			{
				CurListener->DeliverEvent(EventData, EMCore_EventScope::Global);
			}
		}
		else
		{
			/* Invalid pointer, clean up. RemoveAtSwap is O(1); reverse iteration is safe
			 * with swap-from-end because the swapped-in element has already been processed. */
			GlobalListeners.RemoveAtSwap(i, 1, EAllowShrinking::No);
		}
	}
}

bool UMCore_GlobalEventSubsystem::HasGlobalEventAuthority() const
{
	const UWorld* World = GetWorld();
	if (!World) { return false; }

	const ENetMode NetMode = World->GetNetMode();
	
	switch (NetMode) {
	case NM_Standalone:
	case NM_DedicatedServer:
		return true;
        
	case NM_ListenServer:
		return World->GetAuthGameMode() != nullptr;

	case NM_Client:
		return false;
        
	default:
		return false;
	}
}

bool UMCore_GlobalEventSubsystem::ValidateEventRequest(const FMCore_EventData& EventData) const
{
	if (!EventData.IsValid())
	{
		UE_LOG(LogMaevixEvent, Warning, TEXT("GlobalEventSubsystem::ValidateEventRequest: invalid event data"));
		return false;
	}
	
	/* Network safety limits sourced from UMCore_CoreSettings (Project Settings > Maevix Core > Network Safety).
	 * ClampMin/ClampMax in the UPROPERTY meta enforce floor + ceiling; defaults match historical hardcoded values. */
	const UMCore_CoreSettings* CoreSettings = UMCore_CoreSettings::Get();
	const int32 MaxParams = CoreSettings ? CoreSettings->MaxEventParams : 8;
	const int32 MaxContextIDLength = CoreSettings ? CoreSettings->MaxEventContextIDLength : 64;
	const int32 MaxStructSizeBytes = CoreSettings ? CoreSettings->MaxEventStructSizeBytes : 2048;
	const int32 MaxSerializedPayloadBytes = CoreSettings ? CoreSettings->MaxEventSerializedPayloadBytes : 4096;

	if (EventData.EventParams.Num() > MaxParams)
	{
		UE_LOG(LogMaevixEvent, Warning,
			TEXT("GlobalEventSubsystem::ValidateEventRequest: rejected '%s': %d params exceeds cap of %d. "
				 "Tune MaxEventParams in Project Settings > Maevix Core > Network Safety."),
				 *EventData.EventTag.ToString(), EventData.EventParams.Num(), MaxParams);
		return false;
	}

	if (EventData.ContextID.Len() > MaxContextIDLength)
	{
		UE_LOG(LogMaevixEvent, Warning,
			TEXT("GlobalEventSubsystem::ValidateEventRequest: rejected '%s': ContextID length of %d exceeds cap of %d. "
				 "Tune MaxEventContextIDLength in Project Settings > Maevix Core > Network Safety."),
				 *EventData.EventTag.ToString(), EventData.ContextID.Len(), MaxContextIDLength);
		return false;
	}
	
	/* Typed payload validation; global events cross the network */
	if (EventData.TypedPayload.IsValid())
	{
		const UScriptStruct* PayloadStruct = EventData.TypedPayload.GetScriptStruct();
		if (PayloadStruct)
		{
			/* Fast reject; GetStructureSize() is the C++ sizeof, zero runtime cost */
			if (PayloadStruct->GetStructureSize() > MaxStructSizeBytes)
			{
				UE_LOG(LogMaevixEvent, Warning,
					TEXT("GlobalEventSubsystem::ValidateEventRequest: rejected '%s': typed payload struct '%s' "
						 "sizeof %d exceeds cap of %d"),
					*EventData.EventTag.ToString(),
					*PayloadStruct->GetName(),
					PayloadStruct->GetStructureSize(),
					MaxStructSizeBytes);
				return false;
			}

			/* Serialized size check; catches dynamic content (TArrays, FStrings, maps)
			 * where sizeof is small but serialized form can be arbitrarily large */
			FBufferArchive SizeCounter;
			const_cast<UScriptStruct*>(PayloadStruct)->SerializeItem(SizeCounter,
				const_cast<uint8*>(EventData.TypedPayload.GetMemory()), nullptr);
			if (SizeCounter.Num() > MaxSerializedPayloadBytes)
			{
				UE_LOG(LogMaevixEvent, Warning,
					TEXT("GlobalEventSubsystem::ValidateEventRequest: rejected '%s': typed payload struct '%s' "
						 "serialized size %d exceeds cap of %d"),
					*EventData.EventTag.ToString(),
					*PayloadStruct->GetName(),
					SizeCounter.Num(),
					MaxSerializedPayloadBytes);
				return false;
			}
		}
	}

#if !UE_BUILD_SHIPPING
	if (EventData.TypedPayload.IsValid() && EventData.EventParams.Num() > 0)
	{
		UE_LOG(LogMaevixEvent, Verbose,
			TEXT("GlobalEventSubsystem::ValidateEventRequest: event '%s' has both string params (%d) and typed payload '%s'. "
				 "This is valid but may indicate redundant data during migration."),
			*EventData.EventTag.ToString(),
			EventData.EventParams.Num(),
			*EventData.TypedPayload.GetScriptStruct()->GetName());
	}
#endif

	return true;
}

bool UMCore_GlobalEventSubsystem::IsNetworkedGame() const
{
	const UWorld* World = GetWorld();
	if (!World) { return false; }
	
	return World->GetNetMode() != NM_Standalone;
}