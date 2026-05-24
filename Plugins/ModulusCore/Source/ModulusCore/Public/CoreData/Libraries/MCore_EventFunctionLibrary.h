// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CoreData/Types/Events/MCore_EventData.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MCore_EventFunctionLibrary.generated.h"

struct FGameplayTag;
struct FMCore_EventData;
enum class EMCore_EventScope : uint8;

/**
 * Event broadcasting and parameter helpers for GameplayTag-based events.
 * Stateless static functions routing to Local or Global event subsystems based on scope.
 *
 * Local scope delivers to this client only (split-screen safe).
 * Global scope is server-authoritative and multicasts to all clients.
 */
UCLASS()
class MODULUSCORE_API UMCore_EventFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
// ============================================================================
// BROADCAST
// ============================================================================
	
	/**
	 * Broadcast signal-only event (most common). Listeners query subsystems for current state.
	 * No parameters attached; use BroadcastEventWithContext when you need an identifier.
	 */
	UFUNCTION(BlueprintCallable, Category = "ModulusCore|Events",
			  meta = (DefaultToSelf = "WorldContext"))
	static void BroadcastSimpleEvent(const UObject* WorldContext,
		FGameplayTag EventTag,
		EMCore_EventScope EventScope = EMCore_EventScope::Local);

	/**
	 * Broadcast event with a single context identifier.
	 * Use when you need to include minimal context (quest ID, item name, player name).
	 */
	UFUNCTION(BlueprintCallable, Category = "ModulusCore|Events",
			  meta = (DefaultToSelf = "WorldContext"))
	static void BroadcastEventWithContext(const UObject* WorldContext,
		FGameplayTag EventTag,
		const FString& ContextID,
		EMCore_EventScope EventScope = EMCore_EventScope::Local);

    /**
	 * Broadcast event with multiple key-value parameters.
	 * Only use when you need to pass multiple pieces of data not available elsewhere.
	 */
    UFUNCTION(BlueprintCallable, Category = "ModulusCore|Events",
              meta = (DefaultToSelf = "WorldContext"))
	static void BroadcastEvent(const UObject* WorldContext,
		FGameplayTag EventTag,
		const TMap<FString, FString>& EventParams,
		EMCore_EventScope EventScope = EMCore_EventScope::Local);
	
// ============================================================================
// PARAMETER ACCESSORS
// ============================================================================
	
	/** Get context ID from event data (for events broadcast with BroadcastEventWithContext) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ModulusCore|Events")
	static FString GetEventContextID(const FMCore_EventData& EventData);
	
	/** Get raw string parameter (returns DefaultValue if key not found) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ModulusCore|Events")
	static FString GetEventParameter(const FMCore_EventData& EventData,
		const FString& Key,
		const FString& DefaultValue = TEXT(""));
	
	/** Get parameter as bool (parses "true"/"false"/"1"/"0"/"on"/"off"/"yes"/"no") */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ModulusCore|Events")
	static bool GetBoolParameter(const FMCore_EventData& EventData,
		const FString& Key,
		bool DefaultValue = false);

	/** Get parameter as int32 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ModulusCore|Events")
	static int32 GetIntParameter(const FMCore_EventData& EventData,
		const FString& Key,
		int32 DefaultValue = 0);

	/** Get parameter as float (parses string to float) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ModulusCore|Events")
	static float GetFloatParameter(const FMCore_EventData& EventData,
		const FString& Key,
		float DefaultValue = 0.0f);

// ============================================================================
// TYPED PAYLOAD
// ============================================================================

	/**
	 * Broadcast an event with a typed struct payload.
	 * The struct must be a USTRUCT(BlueprintType). Receivers extract via
	 * GetTypedPayload<T>() in C++ or GetTypedPayload node in Blueprint.
	 */
	UFUNCTION(BlueprintCallable, Category = "ModulusCore|Events",
			  meta = (DefaultToSelf = "WorldContext"))
	static void BroadcastTypedEvent(const UObject* WorldContext,
		FGameplayTag EventTag,
		const FInstancedStruct& TypedPayload,
		EMCore_EventScope EventScope = EMCore_EventScope::Local);

	/**
	 * C++ convenience: broadcast a typed event from a concrete struct instance.
	 * Usage: BroadcastTypedEvent(this, Tag, FMyPayload{ItemID, Quantity});
	 */
	template<typename T>
	static void BroadcastTypedEvent(const UObject* WorldContext,
		FGameplayTag EventTag, const T& Payload,
		EMCore_EventScope EventScope = EMCore_EventScope::Local)
	{
		BroadcastTypedEvent(WorldContext, EventTag,
			FInstancedStruct::Make<T>(Payload), EventScope);
	}

	/** Returns true if the event data carries a typed struct payload. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ModulusCore|Events")
	static bool HasTypedPayload(const FMCore_EventData& EventData);

	/** Get the typed payload as FInstancedStruct. Check HasTypedPayload() first. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ModulusCore|Events")
	static FInstancedStruct GetTypedPayload(const FMCore_EventData& EventData);

	/**
	 * Extract typed payload with compile-time type safety.
	 * Returns nullptr if payload is empty or the contained type doesn't match T.
	 */
	template<typename T>
	static const T* GetTypedPayloadAs(const FMCore_EventData& EventData)
	{
		return EventData.TypedPayload.GetPtr<T>();
	}

private:
	/* Routes event data to the appropriate subsystem based on scope */
	static void RouteEventToSubsystem(const UObject* WorldContext,
		const FMCore_EventData& EventData, EMCore_EventScope EventScope);
};
