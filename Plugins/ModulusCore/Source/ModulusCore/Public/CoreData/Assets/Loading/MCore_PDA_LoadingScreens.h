// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CoreData/Types/Loading/MCore_LoadingScreenTypes.h"
#include "Engine/DataAsset.h"
#include "MCore_PDA_LoadingScreens.generated.h"

/**
 * Library of loading screen entries (backgrounds, tips, selection hints).
 *
 * List one or more in Project Settings > Modulus Core > Loading Screen > Libraries.
 * Entries from all libraries are aggregated into a single selection pool at transition time.
 * PrimaryAssetType is "MCore_LoadingScreens" so AssetManager always cooks referenced textures.
 */
UCLASS(BlueprintType)
class MODULUSCORE_API UMCore_PDA_LoadingScreens : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loading Screens")
	TArray<FMCore_LoadingScreenEntry> Entries;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
