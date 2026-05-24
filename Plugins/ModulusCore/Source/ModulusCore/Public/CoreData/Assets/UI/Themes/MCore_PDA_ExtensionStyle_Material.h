// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CoreData/Assets/UI/Themes/MCore_PDA_ExtensionStyle_Base.h"
#include "MCore_PDA_ExtensionStyle_Material.generated.h"

class UMaterialInterface;

/**
 * Material-valued extension style.
 *
 * Covers shader-driven effects, animated backgrounds, custom button materials,
 * and any other tag-keyed material reference where downstream plugins need to
 * surface a theme-aware material without subclassing the theme DA.
 */
UCLASS(Blueprintable, ClassGroup="ModulusCore", DisplayName="Extension Style: Material")
class MODULUSCORE_API UMCore_PDA_ExtensionStyle_Material : public UMCore_PDA_ExtensionStyle_Base
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ModulusCore|Theme|Material")
	TSoftObjectPtr<UMaterialInterface> Material;
};
