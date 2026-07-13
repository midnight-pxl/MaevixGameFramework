// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/EngineTypes.h"
#include "MInt_DeveloperSettings.generated.h"

/** Project settings for MaevixInteract (Project Settings > Game > Maevix Interact). */
UCLASS(Config=MaevixInteract, DefaultConfig, meta=(DisplayName="Maevix Interact"))
class MAEVIXINTERACT_API UMInt_DeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return TEXT("Game"); }
	virtual FName GetSectionName() const override { return TEXT("Maevix Interact"); }
#if WITH_EDITOR
	virtual FText GetSectionText() const override;
	virtual FText GetSectionDescription() const override;
#endif

	static const UMInt_DeveloperSettings* Get();

	/**
	 * Object channel the Interactor's volume (player or AI) lives on. Interactable volumes respond Overlap
	 * to it. Defaults to WorldStatic, which is not a valid interaction channel; the interactable component
	 * errors at registration until a custom Object channel is selected.
	 */
	UPROPERTY(Config, EditAnywhere, Category="MaevixInteract|Detection", meta=(DisplayName="Interactor Object Channel"))
	TEnumAsByte<ECollisionChannel> InteractorObjectChannel = ECC_WorldStatic;

	/**
	 * Object channel interactable detection volumes live on. The Interactor's volume responds Overlap to it.
	 * Must differ from Interactor Object Channel. Defaults to WorldStatic, which is not a valid interaction
	 * channel; the interactable component errors at registration until a custom Object channel is selected.
	 */
	UPROPERTY(Config, EditAnywhere, Category="MaevixInteract|Detection", meta=(DisplayName="Interactable Object Channel"))
	TEnumAsByte<ECollisionChannel> InteractableObjectChannel = ECC_WorldStatic;
};
