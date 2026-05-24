// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CoreData/Assets/UI/Themes/MCore_PDA_ExtensionStyle_Base.h"
#include "MCore_PDA_ExtensionStyle_Color.generated.h"

/**
 * Color-valued extension style.
 *
 * Covers rarity tints, damage type colors, status effect colors, and any other
 * tag-keyed single-color use case where downstream plugins need to surface a
 * theme-aware color without subclassing the theme DA.
 */
UCLASS(Blueprintable, ClassGroup="ModulusCore", DisplayName="Extension Style: Color")
class MODULUSCORE_API UMCore_PDA_ExtensionStyle_Color : public UMCore_PDA_ExtensionStyle_Base
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ModulusCore|Theme|Color")
	FLinearColor Color = FLinearColor::White;
};
