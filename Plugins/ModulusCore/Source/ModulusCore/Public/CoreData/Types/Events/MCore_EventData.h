// Copyright 2025, Midnight Pixel Studio LLC.

#pragma once

#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "MCore_EventData.generated.h"

/** Determines how an event is routed: Local (client-only) or Global (server-authoritative). */
UENUM(BlueprintType)
enum class EMCore_EventScope : uint8
{
	Local	UMETA(DisplayName="Local Only", ToolTip = "This client only - UI updates, audio cues, client-side effects, etc."),
	Global	UMETA(DisplayName="Global (All Players)", ToolTip = "All players - Player actions, gameplay state, multiplayer events, etc.")
};

/** Single key-value parameter entry, RPC-safe */
USTRUCT(BlueprintType)
struct MODULUSCORE_API FMCore_EventParameter
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Key;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Value;

	FMCore_EventParameter() = default;

	FMCore_EventParameter(const FString& InKey, const FString& InValue)
		: Key(InKey), Value(InValue){}
};

/**
 * RPC-compatible event payload with GameplayTag identifier and optional parameters.
 *
 * Key Features:
 * - Tag-based identification for decoupled listener matching
 * - Optional ContextID for single-identifier events
 * - TArray-based parameters optimized for <10 entries (95% of use cases)
 */
USTRUCT(BlueprintType)
struct MODULUSCORE_API FMCore_EventData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EventData")
	FGameplayTag EventTag;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EventData")
	FString ContextID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EventData")
	TArray<FMCore_EventParameter> EventParams;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EventData")
	FInstancedStruct TypedPayload;

	FMCore_EventData() = default;

	FMCore_EventData(const FGameplayTag& InEventTag, const FString& InContextID)
		: EventTag(InEventTag), ContextID(InContextID) {}
	
	FMCore_EventData(const FGameplayTag& InEventTag, const TMap<FString, FString>& InEventParams = {})
		: EventTag(InEventTag)
	{
		EventParams.Reserve(InEventParams.Num());
		for (const auto& Pair : InEventParams)
		{
			/* Skip invalid entries */
			if (!Pair.Key.IsEmpty())
			{
				EventParams.Emplace(Pair.Key, Pair.Value);
			}
		}
	}

	FMCore_EventData(const FGameplayTag& InEventTag, const FInstancedStruct& InPayload)
		: EventTag(InEventTag), TypedPayload(InPayload) {}

	bool IsValid() const { return EventTag.IsValid(); }

	/* Parameter lookup by key, returns DefaultValue if not found */
	FString GetParameter(const FString& Key, const FString& DefaultValue = TEXT("")) const
	{
		/* Linear search is optimal for typical 1-8 parameters */
		for (const FMCore_EventParameter& Param : EventParams)
		{
			if (Param.Key == Key)
			{
				return Param.Value;
			}
		}
		return DefaultValue;
	}

	/** Returns true if this event carries a typed struct payload. */
	bool HasTypedPayload() const { return TypedPayload.IsValid(); }

	/**
	 * Extract typed payload with compile-time type safety.
	 * Returns nullptr if payload is empty or type doesn't match.
	 */
	template<typename T>
	const T* GetTypedPayload() const { return TypedPayload.GetPtr<T>(); }
};