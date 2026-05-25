// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreData/Libraries/MCore_EventFunctionLibrary.h"

#include "CoreData/Logging/LogModulusEvent.h"
#include "CoreData/Types/Events/MCore_EventData.h"
#include "CoreEvents/MCore_GlobalEventSubsystem.h"
#include "CoreEvents/MCore_LocalEventSubsystem.h"

#include "Engine/World.h"
#include "Engine/LocalPlayer.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

namespace
{
	/* Resolves the LocalPlayer from a WorldContext object by trying, in order:
	 *   (1) WorldContext is a PlayerController -> GetLocalPlayer();
	 *   (2) WorldContext is a Pawn -> Controller -> GetLocalPlayer();
	 *   (3) WorldContext is an ActorComponent -> recurse on its Owner;
	 *   (4) WorldContext is a UserWidget -> GetOwningLocalPlayer();
	 *   (5) WorldContext is an Actor -> Instigator -> Controller -> GetLocalPlayer();
	 *   (6) Owner-chain walk -> first APlayerController encountered -> GetLocalPlayer();
	 * Returns nullptr with a warning log when no owner chain resolves -- callers must
	 * pass a player-owned WorldContext or use EMCore_EventScope::AllLocal/Global instead.
	 * Split-screen safety: no Player 0 fallback.
	 *
	 * @see UMCore_EventListenerComponent::ResolveOwningLocalPlayer -- the Actor-based
	 *      symmetric chain used by listener registration in BeginPlay. Both share
	 *      the same no-Player-0-fallback policy (audit §5.6, Phase 1 + Phase 2). */
	ULocalPlayer* ResolveLocalPlayer(const UObject* WorldContext)
	{
		if (!WorldContext) { return nullptr; }

		if (const APlayerController* PlayerController = Cast<APlayerController>(WorldContext))
		{
			return PlayerController->GetLocalPlayer();
		}

		if (const APawn* Pawn = Cast<APawn>(WorldContext))
		{
			if (const APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController()))
			{
				return PlayerController->GetLocalPlayer();
			}
		}

		if (const UActorComponent* Component = Cast<UActorComponent>(WorldContext))
		{
			return ResolveLocalPlayer(Component->GetOwner());
		}

		if (const UUserWidget* Widget = Cast<UUserWidget>(WorldContext))
		{
			return Widget->GetOwningLocalPlayer();
		}

		if (const AActor* Actor = Cast<AActor>(WorldContext))
		{
			if (const APawn* Instigator = Actor->GetInstigator())
			{
				if (const APlayerController* PlayerController = Cast<APlayerController>(Instigator->GetController()))
				{
					return PlayerController->GetLocalPlayer();
				}
			}

			AActor* OwnerActor = Actor->GetOwner();
			while (OwnerActor)
			{
				if (const APlayerController* PlayerController = Cast<APlayerController>(OwnerActor))
				{
					return PlayerController->GetLocalPlayer();
				}
				OwnerActor = OwnerActor->GetOwner();
			}
		}

		/* No owner chain resolved to a specific LocalPlayer. Symmetric to the
		 * UMCore_EventListenerComponent fix (Phase 1) -- in split-screen this fallback
		 * would have contaminated Player 0. Library callers must either pass a
		 * player-owned WorldContext (PlayerController/Pawn/Widget) or use
		 * EMCore_EventScope::AllLocal to deliver to every LocalPlayer on this
		 * GameInstance. */
		UE_LOG(LogModulusEvent, Warning,
			TEXT("EventFunctionLibrary::ResolveLocalPlayer -- WorldContext '%s' has no LocalPlayer owner chain. "
				 "Pass a PlayerController, Pawn, or Widget as WorldContext, or use EMCore_EventScope::AllLocal "
				 "to target every LocalPlayer on this GameInstance, or EMCore_EventScope::Global for networked delivery."),
			*GetNameSafe(WorldContext));
		return nullptr;
	}
}

// ============================================================================
// BROADCAST
// ============================================================================

void UMCore_EventFunctionLibrary::BroadcastSimpleEvent(const UObject* WorldContext,
	FGameplayTag EventTag,
	EMCore_EventScope EventScope)
{
	if (!WorldContext || !EventTag.IsValid())
	{
		UE_LOG(LogModulusEvent, Warning,
			TEXT("EventFunctionLibrary::BroadcastSimpleEvent: invalid parameters (WorldContext: %s, Tag: %s)"),
			WorldContext ? TEXT("Valid") : TEXT("NULL"), *EventTag.ToString());
		return;
	}
	
	FMCore_EventData EventData(EventTag);
	RouteEventToSubsystem(WorldContext, EventData, EventScope);
}

/**
 * @param ContextID  Single identifier string (quest ID, item tag, player name, etc.).
 */
void UMCore_EventFunctionLibrary::BroadcastEventWithContext(const UObject* WorldContext,
	FGameplayTag EventTag,
	const FString& ContextID,
	EMCore_EventScope EventScope)
{
	if (!WorldContext || !EventTag.IsValid())
	{
		UE_LOG(LogModulusEvent, Warning,
			TEXT("EventFunctionLibrary::BroadcastEventWithContext: invalid parameters (WorldContext: %s, Tag: %s)"),
			WorldContext ? TEXT("Valid") : TEXT("NULL"), *EventTag.ToString());
		return;
	}

	FMCore_EventData EventData(EventTag, ContextID);
	RouteEventToSubsystem(WorldContext, EventData, EventScope);
}

/**
 * @param EventParams  String key-value pairs (e.g., {"ItemID": "Sword_001", "Quantity": "5"}).
 */
void UMCore_EventFunctionLibrary::BroadcastEvent(const UObject* WorldContext,
	FGameplayTag EventTag,
	const TMap<FString, FString>& EventParams,
	EMCore_EventScope EventScope)
{
	if (!WorldContext || !EventTag.IsValid())
	{
		UE_LOG(LogModulusEvent, Warning,
			TEXT("EventFunctionLibrary::BroadcastEvent: invalid parameters (WorldContext: %s, Tag: %s)"),
			WorldContext ? TEXT("Valid") : TEXT("NULL"), *EventTag.ToString());
		return;
	}

	FMCore_EventData EventData(EventTag, EventParams);
	RouteEventToSubsystem(WorldContext, EventData, EventScope);
}

// ============================================================================
// PARAMETER ACCESSORS
// ============================================================================

FString UMCore_EventFunctionLibrary::GetEventContextID(const FMCore_EventData& EventData)
{
	return EventData.ContextID;
}

FString UMCore_EventFunctionLibrary::GetEventParameter(const FMCore_EventData& EventData,
	const FString& Key,
	const FString& DefaultValue)
{
	return EventData.GetParameter(Key, DefaultValue);
}

bool UMCore_EventFunctionLibrary::GetBoolParameter(const FMCore_EventData& EventData,
	const FString& Key,
	bool DefaultValue)
{
	const FString InValue = EventData.GetParameter(Key);
	if (InValue.IsEmpty()) { return DefaultValue; }
	
	const FString ParamValue = InValue.ToLower();
	return ParamValue == TEXT("true") || ParamValue == TEXT("1")
	|| ParamValue == TEXT("on") || ParamValue == TEXT("yes");
}

int32 UMCore_EventFunctionLibrary::GetIntParameter(const FMCore_EventData& EventData,
	const FString& Key,
	int32 DefaultValue)
{
	const FString InValue = EventData.GetParameter(Key);
	if (InValue.IsEmpty()) { return DefaultValue; }
	
	return FCString::Atoi(*InValue);
}

float UMCore_EventFunctionLibrary::GetFloatParameter(const FMCore_EventData& EventData,
	const FString& Key,
	float DefaultValue)
{
	FString InValue = EventData.GetParameter(Key);
	if (InValue.IsEmpty()) { return DefaultValue; }
	
	return FCString::Atof(*InValue);
}

// ============================================================================
// TYPED PAYLOAD
// ============================================================================

void UMCore_EventFunctionLibrary::BroadcastTypedEvent(const UObject* WorldContext,
	FGameplayTag EventTag,
	const FInstancedStruct& TypedPayload,
	EMCore_EventScope EventScope)
{
	if (!WorldContext || !EventTag.IsValid())
	{
		UE_LOG(LogModulusEvent, Warning,
			TEXT("EventFunctionLibrary::BroadcastTypedEvent: invalid parameters (WorldContext: %s, Tag: %s)"),
			WorldContext ? TEXT("Valid") : TEXT("NULL"), *EventTag.ToString());
		return;
	}

	FMCore_EventData EventData(EventTag, TypedPayload);
	RouteEventToSubsystem(WorldContext, EventData, EventScope);
}

bool UMCore_EventFunctionLibrary::HasTypedPayload(const FMCore_EventData& EventData)
{
	return EventData.HasTypedPayload();
}

FInstancedStruct UMCore_EventFunctionLibrary::GetTypedPayload(const FMCore_EventData& EventData)
{
	return EventData.TypedPayload;
}

// ============================================================================
// INTERNAL
// ============================================================================

void UMCore_EventFunctionLibrary::RouteEventToSubsystem(const UObject* WorldContext,
	const FMCore_EventData& EventData,
	EMCore_EventScope EventScope)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	if (!World)
	{
		UE_LOG(LogModulusEvent, Warning, TEXT("EventFunctionLibrary::RouteEventToSubsystem -- no valid world context"));
		return;
	}

	const TCHAR* ScopeName = TEXT("Unknown");
	switch (EventScope)
	{
	case EMCore_EventScope::Local:    ScopeName = TEXT("Local");    break;
	case EMCore_EventScope::AllLocal: ScopeName = TEXT("AllLocal"); break;
	case EMCore_EventScope::Global:   ScopeName = TEXT("Global");   break;
	default: checkNoEntry();          break;
	}
	UE_LOG(LogModulusEvent, Verbose, TEXT("EventFunctionLibrary::RouteEventToSubsystem -- routing event: %s (Scope: %s)"),
		   *EventData.EventTag.ToString(), ScopeName);

	switch (EventScope)
	{
	case EMCore_EventScope::Local:
	{
		/* Local: resolve the correct LocalPlayer from the calling context (split-screen safe) */
		ULocalPlayer* LocalPlayer = ResolveLocalPlayer(WorldContext);
		if (!LocalPlayer)
		{
			/* ResolveLocalPlayer already emitted a Warning describing the resolution failure. */
			return;
		}
		if (UMCore_LocalEventSubsystem* LocalSystem = LocalPlayer->GetSubsystem<UMCore_LocalEventSubsystem>())
		{
			LocalSystem->BroadcastLocalEvent(EventData, EMCore_EventScope::Local);
		}
		else
		{
			UE_LOG(LogModulusEvent, Error,
				TEXT("EventFunctionLibrary::RouteEventToSubsystem -- failed to get LocalEventSubsystem"));
		}
		break;
	}
	case EMCore_EventScope::AllLocal:
	{
		UGameInstance* GameInstance = World->GetGameInstance();
		if (!GameInstance)
		{
			UE_LOG(LogModulusEvent, Warning,
				TEXT("EventFunctionLibrary::RouteEventToSubsystem -- AllLocal broadcast of '%s' has no GameInstance."),
				*EventData.EventTag.ToString());
			return;
		}
		const TArray<ULocalPlayer*>& LocalPlayers = GameInstance->GetLocalPlayers();
		if (LocalPlayers.IsEmpty())
		{
			/* Dedicated server or pre-init: no LocalPlayers exist, broadcast is a no-op. */
			UE_LOG(LogModulusEvent, Verbose,
				TEXT("EventFunctionLibrary::RouteEventToSubsystem -- AllLocal broadcast of '%s' with zero LocalPlayers (dedicated server or pre-init)."),
				*EventData.EventTag.ToString());
			return;
		}
		for (ULocalPlayer* LP : LocalPlayers)
		{
			if (UMCore_LocalEventSubsystem* LocalSystem = LP->GetSubsystem<UMCore_LocalEventSubsystem>())
			{
				LocalSystem->BroadcastLocalEvent(EventData, EMCore_EventScope::AllLocal);
			}
		}
		break;
	}
	case EMCore_EventScope::Global:
	{
		UGameInstance* GameInstance = World->GetGameInstance();
		if (!GameInstance)
		{
			UE_LOG(LogModulusEvent, Warning, TEXT("EventFunctionLibrary::RouteEventToSubsystem -- no valid game instance"));
			return;
		}
		if (UMCore_GlobalEventSubsystem* GlobalSystem = GameInstance->GetSubsystem<UMCore_GlobalEventSubsystem>())
		{
			GlobalSystem->BroadcastGlobalEvent(EventData);
		}
		else
		{
			UE_LOG(LogModulusEvent, Error,
				TEXT("EventFunctionLibrary::RouteEventToSubsystem -- failed to get GlobalEventSubsystem"));
		}
		break;
	}
	default:
		checkNoEntry();
		break;
	}
}
