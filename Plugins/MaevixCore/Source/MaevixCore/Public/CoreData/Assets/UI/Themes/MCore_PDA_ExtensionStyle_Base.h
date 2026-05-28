// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "MCore_PDA_ExtensionStyle_Base.generated.h"

/**
 * Abstract base for plugin-injectable extension styles.
 *
 * Downstream plugins subclass this to author specific style types (color, material,
 * sound, etc.) and reference them from a theme's ExtensionStyles map via the
 * ExtensionTag key. Editor-time IsDataValid ensures ExtensionTag is set; the
 * parent theme's IsDataValid ensures the map key matches the assigned style's tag.
 *
 * Designed to keep cross-plugin theme extension type-safe: the theme DA's
 * ExtensionStyles map is constrained to subclasses of this base, so the editor
 * asset picker filters automatically.
 */
UCLASS(Abstract, Blueprintable, ClassGroup="MaevixCore")
class MAEVIXCORE_API UMCore_PDA_ExtensionStyle_Base : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Tag under which this style is registered in the parent theme's ExtensionStyles map. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MaevixCore|Theme|Extension",
		meta=(Categories="Maevix.Theme.Extension"))
	FGameplayTag ExtensionTag;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
