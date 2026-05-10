// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "MCore_ThemeTypes.generated.h"

class UMCore_PDA_UITheme_Base;

/** Maps a display name and description to a soft-referenced theme DataAsset. */
USTRUCT(BlueprintType)
struct MODULUSCORE_API FMCore_ThemeEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FText DisplayName{};

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FText Description{};

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSoftObjectPtr<UMCore_PDA_UITheme_Base> ThemeAsset{ nullptr };

	bool IsValid() const { return !ThemeAsset.IsNull(); }
};
