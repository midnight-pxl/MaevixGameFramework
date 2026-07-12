// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MCore_InteractionTypes.generated.h"

class AActor;
class AController;

/**
 * Identifies who is attempting an interaction. Passed to every IMCore_Interactable call so implementers
 * never need a world-context object.
 */
USTRUCT(BlueprintType)
struct MAEVIXCORE_API FMCore_InteractionContext
{
	GENERATED_BODY()

	/**
	 * Actor performing interaction, typically a pawn but not required to be one. Null on a
	 * default-constructed context.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaevixCore|Interaction")
	TObjectPtr<AActor> Interactor = nullptr;

	/**
	 * Controller driving Interactor. Authority checks and per-player logic key off this rather than
	 * the pawn.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaevixCore|Interaction")
	TObjectPtr<AController> InteractorController = nullptr;
};

/** Universal interaction payload exposed by any interactable, independent of how the interaction is executed. */
USTRUCT(BlueprintType)
struct MAEVIXCORE_API FMCore_InteractionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaevixCore|Interaction")
	FText PromptText;

	/**
	 * Classifies the interaction for consumers that branch on kind (pickup, door, dialogue). Core itself
	 * never reads it.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaevixCore|Interaction",
		meta = (Categories = "MCore.Interaction"))
	FGameplayTag InteractionTypeTag;
};
