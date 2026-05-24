// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "InputMappingContext.h"
#include "MCore_KeyBindingTypes.generated.h"

/**
 * Associates an InputMappingContext with a display name for the key binding panel.
 * Each entry becomes a tab in the tabbed key binding layout.
 */
USTRUCT(BlueprintType)
struct MODULUSCORE_API FMCore_KeyBindingContext
{
	GENERATED_BODY()

	/** Tab display name shown in the key binding panel. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "ModulusCore|UI|KeyBinding")
	FText DisplayName;

	/** The InputMappingContext whose remappable actions populate this tab. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "ModulusCore|UI|KeyBinding")
	TSoftObjectPtr<UInputMappingContext> MappingContext;
};
