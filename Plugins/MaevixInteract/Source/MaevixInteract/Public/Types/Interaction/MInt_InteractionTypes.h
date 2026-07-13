// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/HitResult.h"
#include "CoreData/Types/Interaction/MCore_InteractionTypes.h"
#include "Types/Execution/MInt_ExecutionTypes.h"
#include "MInt_InteractionTypes.generated.h"

class AActor;

/**
 * A resolved interaction focus: the target actor, the object interface calls dispatch to, its current
 * payload and execution timing, and the sweep hit that found it. A null Target means unfocused.
 */
USTRUCT(BlueprintType)
struct MAEVIXINTERACT_API FMInt_ResolvedInteraction
{
	GENERATED_BODY()

	/** Focused actor. Null means nothing is focused. */
	UPROPERTY(BlueprintReadOnly, Category="MaevixInteract|Interaction")
	TObjectPtr<AActor> Target = nullptr;

	/** Object the IMCore_Interactable calls dispatch to (an interactable component, or the actor itself). */
	UPROPERTY(BlueprintReadOnly, Category="MaevixInteract|Interaction")
	TObjectPtr<UObject> Provider = nullptr;

	/** Prompt text and type tag for the focused interactable in its current state. */
	UPROPERTY(BlueprintReadOnly, Category="MaevixInteract|Interaction")
	FMCore_InteractionData InteractionData;

	/** How the focused interactable is driven (tap, hold, hold-and-repeat). */
	UPROPERTY(BlueprintReadOnly, Category="MaevixInteract|Interaction")
	FMInt_ExecutionConfig ExecutionConfig;

	/** The focus sweep hit; holds the hit component and location. Core's FMCore_InteractionContext stays lean and gains no hit field. */
	UPROPERTY(BlueprintReadOnly, Category="MaevixInteract|Interaction")
	FHitResult Hit;
};
