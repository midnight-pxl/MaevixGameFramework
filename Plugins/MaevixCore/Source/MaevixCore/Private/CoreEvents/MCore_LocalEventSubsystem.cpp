// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreEvents/MCore_LocalEventSubsystem.h"

#include "CoreData/Logging/LogMaevixEvent.h"
#include "CoreData/DevSettings/MCore_CoreSettings.h"
#include "CoreEvents/MCore_EventListenerComponent.h"
#include "CoreData/Types/Events/MCore_EventData.h"

/* Conditional logging macro. Only logs when Event System Logging is enabled in Project Settings. */
#define MCORE_EVENT_LOG(Format, ...) \
	do { \
		if (const UMCore_CoreSettings* Settings = UMCore_CoreSettings::Get()) \
		{ \
			if (Settings->IsEventLoggingEnabled()) \
			{ \
				UE_LOG(LogMaevixEvent, Log, Format, ##__VA_ARGS__); \
			} \
		} \
	} while(0)

void UMCore_LocalEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogMaevixEvent, Log, TEXT("LocalEventSubsystem::Initialize: initialized"));
}

void UMCore_LocalEventSubsystem::Deinitialize()
{
	UE_LOG(LogMaevixEvent, Log, TEXT("LocalEventSubsystem::Deinitialize -- cleaning up, %d listener(s)"), LocalListeners.Num());
	LocalListeners.Reset();

	Super::Deinitialize();
}

void UMCore_LocalEventSubsystem::RegisterLocalListener(UMCore_EventListenerComponent* ListenerComponent)
{
	if (IsValid(ListenerComponent))
	{
		LocalListeners.AddUnique(ListenerComponent);
		MCORE_EVENT_LOG(TEXT("LocalEventSubsystem::RegisterLocalListener: registered: %s"),
			*ListenerComponent->GetName());
	}
}

void UMCore_LocalEventSubsystem::UnregisterLocalListener(UMCore_EventListenerComponent* ListenerComponent)
{
	int32 RemovedCount = LocalListeners.RemoveAll([ListenerComponent](
		const TWeakObjectPtr<UMCore_EventListenerComponent>& WeakListener)
	{
		return !WeakListener.IsValid() || WeakListener.Get() == ListenerComponent;
	});
	
	if (RemovedCount > 0)
	{
		MCORE_EVENT_LOG(TEXT("LocalEventSubsystem::UnregisterLocalListener: unregistered: %s"),
			ListenerComponent ? *ListenerComponent->GetName() : TEXT("Unknown"));
	}
}

void UMCore_LocalEventSubsystem::BroadcastLocalEvent(const FMCore_EventData& EventData,
	EMCore_EventScope SourceScope)
{
	if (!EventData.IsValid()) { return; }

	/* Native delegate fires regardless of scope; the per-component listeners receive SourceScope. */
	OnLocalEventBroadcast.Broadcast(EventData);

	MCORE_EVENT_LOG(TEXT("LocalEventSubsystem::BroadcastLocalEvent -- broadcasting: %s (Scope: %s)"),
		*EventData.EventTag.ToString(),
		SourceScope == EMCore_EventScope::AllLocal ? TEXT("AllLocal") : TEXT("Local"));

	for (int32 i = LocalListeners.Num() - 1; i >= 0; --i)
	{
		TWeakObjectPtr<UMCore_EventListenerComponent>& WeakListener = LocalListeners[i];

		if (WeakListener.IsValid())
		{
			UMCore_EventListenerComponent* CurrentListener = WeakListener.Get();

			if (CurrentListener->ShouldReceiveEvent(EventData, SourceScope))
			{
				CurrentListener->DeliverEvent(EventData, SourceScope);
			}
		}
		else
		{
			/* Reverse iteration + RemoveAtSwap: swapped-in element has already been processed. */
			LocalListeners.RemoveAtSwap(i, 1, EAllowShrinking::No);
		}
	}
}
