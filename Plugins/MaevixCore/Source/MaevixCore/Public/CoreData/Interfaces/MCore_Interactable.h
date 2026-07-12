// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CoreData/Types/Interaction/MCore_InteractionTypes.h"
#include "MCore_Interactable.generated.h"

UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UMCore_Interactable : public UInterface
{
	GENERATED_BODY()
};

/**
 * World-interaction contract. An actor implements this to answer focus and execution queries without
 * the caller knowing the actor's concrete type. Core declares the seam only: it ships no detection,
 * focus, or execution system and never calls these hooks itself.
 */
class MAEVIXCORE_API IMCore_Interactable
{
	GENERATED_BODY()

public:
	/**
	 * Gate checked when focus resolves and again before execution. Server re-checks before committing.
	 * Default returns true.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MaevixCore|Interaction")
	bool CanInteract(const FMCore_InteractionContext& Context) const;
	virtual bool CanInteract_Implementation(const FMCore_InteractionContext& Context) const;

	/** Commits the interaction. Called on the server only. Default is a no-op. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MaevixCore|Interaction")
	void OnInteract(const FMCore_InteractionContext& Context);
	virtual void OnInteract_Implementation(const FMCore_InteractionContext& Context);

	/**
	 * Prompt text and type tag for this interactable in its current state. May vary per interactor.
	 * Default returns an empty payload.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MaevixCore|Interaction")
	FMCore_InteractionData GetInteractionData(const FMCore_InteractionContext& Context) const;
	virtual FMCore_InteractionData GetInteractionData_Implementation(const FMCore_InteractionContext& Context) const;

	/**
	 * Monotonic counter bumped whenever GetInteractionData or CanInteract would return something different.
	 * Observers poll this and re-read only on change. Never reset or decrement it. Default returns 0, meaning static.
	 *
	 * Network:
	 *   Server  - Bump on any authoritative state change that alters prompt text, type tag, or interactability.
	 *   Client  - Must observe the bump too. Derive it from replicated state, or replicate the counter itself.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MaevixCore|Interaction")
	int32 GetInteractionStateVersion() const;
	virtual int32 GetInteractionStateVersion_Implementation() const;
};
