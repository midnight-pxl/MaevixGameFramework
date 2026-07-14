// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameplayTagContainer.h"
#include "CoreData/Interfaces/MCore_Interactable.h"
#include "Interfaces/MInt_ExecutionConfigProvider.h"
#include "MInt_InteractableComponent.generated.h"

/**
 * Detection volume and authored-data holder that makes its owning actor interactable. Implements
 * IMCore_Interactable and IMInt_ExecutionConfigProvider, forwarding to the owner actor when the owner
 * implements the same interface, and otherwise answering from its own authored properties.
 */
UCLASS(ClassGroup=(MaevixInteract), BlueprintType, meta=(BlueprintSpawnableComponent, DisplayName="Maevix Interactable"))
class MAEVIXINTERACT_API UMInt_InteractableComponent
	: public USphereComponent, public IMCore_Interactable, public IMInt_ExecutionConfigProvider
{
	GENERATED_BODY()

public:
	UMInt_InteractableComponent();

	/** Prompt text used when the owner does not answer GetInteractionData itself. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MaevixInteract|Interactable")
	FText PromptText;

	/** Type tag classifying this interaction for consumers that branch on kind (pickup, door, dialogue). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MaevixInteract|Interactable", meta=(Categories="MCore.Interaction"))
	FGameplayTag InteractionTypeTag;

	/** Execution timing used when the owner does not implement IMInt_ExecutionConfigProvider. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MaevixInteract|Interactable")
	FMInt_ExecutionConfig ExecutionConfig;

	/** Detection sphere radius, in centimeters. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MaevixInteract|Interactable",
		meta=(ClampMin="0.0", UIMin="0.0", Units="cm"))
	float DetectionRadius{150.0f};

	// IMCore_Interactable
	virtual bool CanInteract_Implementation(const FMCore_InteractionContext& Context) const override;
	virtual void OnInteract_Implementation(const FMCore_InteractionContext& Context) override;
	virtual FMCore_InteractionData GetInteractionData_Implementation(const FMCore_InteractionContext& Context) const override;
	virtual int32 GetInteractionStateVersion_Implementation() const override;

	// IMInt_ExecutionConfigProvider
	virtual FMInt_ExecutionConfig GetExecutionConfig_Implementation() const override;

protected:
	virtual void OnRegister() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
