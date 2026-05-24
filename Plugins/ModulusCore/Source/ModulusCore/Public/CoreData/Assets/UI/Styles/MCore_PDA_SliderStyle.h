// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MCore_PDA_SliderStyle.generated.h"

/**
 * Slider styling for UI themes.
 * Referenced by UMCore_PDA_UITheme_Base.
 */
UCLASS(BlueprintType, Blueprintable)
class MODULUSCORE_API UMCore_PDA_SliderStyle : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ModulusCore|Theme|Slider")
	TObjectPtr<UObject> BarImage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ModulusCore|Theme|Slider")
	TObjectPtr<UObject> ThumbImage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ModulusCore|Theme|Slider")
	float BarThickness{ 4.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ModulusCore|Theme|Slider|Colors")
	FSlateColor BarNormalTint = FLinearColor{ 0.3f, 0.3f, 0.3f, 1.0f };

	/** Tint for the filled portion of the bar */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ModulusCore|Theme|Slider|Colors")
	FSlateColor BarFilledTint = FLinearColor{ 0.8f, 0.8f, 0.8f, 1.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ModulusCore|Theme|Slider|Colors")
	FSlateColor ThumbNormalTint = FLinearColor{ 0.5f, 0.5f, 0.5f, 1.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ModulusCore|Theme|Slider|Colors")
	FSlateColor ThumbHoveredTint = FLinearColor{ 0.8f, 0.8f, 0.8f, 1.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ModulusCore|Theme|Slider|Colors")
	FSlateColor ThumbDraggedTint = FLinearColor{ 1.0f, 1.0f, 1.0f, 1.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ModulusCore|Theme|Slider")
	FVector2D ThumbSize{ 16.0, 16.0 };

};
