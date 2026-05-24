// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MCore_PDA_ScrollbarStyle.generated.h"

/**
 * Scrollbar styling for UI themes.
 * Referenced by UMCore_PDA_UITheme_Base.
 */
UCLASS(BlueprintType, Blueprintable)
class MODULUSCORE_API UMCore_PDA_ScrollbarStyle : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ModulusCore|Theme|Scrollbar")
	TObjectPtr<UObject> TrackImage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ModulusCore|Theme|Scrollbar")
	TObjectPtr<UObject> ThumbImage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ModulusCore|Theme|Scrollbar")
	float TrackWidth{ 12.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ModulusCore|Theme|Scrollbar|Colors")
	FSlateColor TrackTint = FLinearColor{ 0.1f, 0.1f, 0.1f, 0.5f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ModulusCore|Theme|Scrollbar|Colors")
	FSlateColor ThumbNormalTint = FLinearColor{ 0.4f, 0.4f, 0.4f, 1.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ModulusCore|Theme|Scrollbar|Colors")
	FSlateColor ThumbHoveredTint = FLinearColor{ 0.6f, 0.6f, 0.6f, 1.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ModulusCore|Theme|Scrollbar|Colors")
	FSlateColor ThumbDraggedTint = FLinearColor{ 0.8f, 0.8f, 0.8f, 1.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ModulusCore|Theme|Scrollbar")
	float MinThumbLength{ 32.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ModulusCore|Theme|Scrollbar")
	FMargin Padding{ 2.0f };

};
