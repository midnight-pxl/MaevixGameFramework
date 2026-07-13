// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "MInt_ExecutionTypes.generated.h"

/** How an interaction is driven from the input binding. All three share one physical key and differ only in timing. */
UENUM(BlueprintType)
enum class EMInt_ExecutionType : uint8
{
	/** Fires once on press. */
	Tap,
	/** Fires once after the key is held for the configured duration. */
	Hold,
	/** Fires repeatedly at the configured interval while the key is held. */
	Repeat
};

/** Optional per-interactable execution timing. An interactable that does not provide one is driven as a Tap. */
USTRUCT(BlueprintType)
struct MAEVIXINTERACT_API FMInt_ExecutionConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaevixInteract|Execution")
	EMInt_ExecutionType ExecutionType = EMInt_ExecutionType::Tap;

	/**
	 * Ignored when ExecutionType is Tap. For Hold, the time the key must be held to complete.
	 * For Repeat, the interval between fires while the key is held.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaevixInteract|Execution",
		meta = (ClampMin = "0.0", UIMin = "0.0"))
	float BaseHoldSeconds = 1.0f;
};
