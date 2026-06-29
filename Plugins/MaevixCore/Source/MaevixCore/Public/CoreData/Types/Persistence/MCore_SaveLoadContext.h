// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "MCore_SaveLoadContext.generated.h"

/** Which half of a save cycle a hook is being invoked for. */
UENUM(BlueprintType)
enum class EMCore_SaveLoadPhase : uint8
{
	Saving,
	Loading
};

/**
 * Read-only context the persistence orchestrator (the future MaevixMemory) hands to a provider's save hooks.
 * Produced by the orchestrator and consumed by IMCore_Savable hooks; intentionally read-only because Core owns
 * no save-format policy and a provider must not mutate the cycle it is participating in.
 */
USTRUCT(BlueprintType)
struct MAEVIXCORE_API FMCore_SaveLoadContext
{
	GENERATED_BODY()

	/** Whether this invocation is part of a save or a load. */
	UPROPERTY(BlueprintReadOnly, Category = "MaevixCore|Persistence")
	EMCore_SaveLoadPhase Phase = EMCore_SaveLoadPhase::Saving;

	/**
	 * Producer-supplied save-format version, stamped by the orchestrator. Core does not interpret it;
	 * Core owns no save-format policy. OnPostLoad implementers may compare it to perform migration.
	 * 0 means unset / unversioned.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "MaevixCore|Persistence")
	int32 SaveVersion = 0;

	/** Target save slot for this cycle. */
	UPROPERTY(BlueprintReadOnly, Category = "MaevixCore|Persistence")
	FString SlotName;

	/** Platform user index that owns the slot. */
	UPROPERTY(BlueprintReadOnly, Category = "MaevixCore|Persistence")
	int32 UserIndex = 0;

	/** True when this instance has authority over the saved game state (server or standalone). */
	UPROPERTY(BlueprintReadOnly, Category = "MaevixCore|Persistence")
	bool bIsServerAuthority = true;
};
