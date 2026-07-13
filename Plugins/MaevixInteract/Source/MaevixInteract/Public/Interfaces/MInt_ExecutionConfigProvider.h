// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Types/Execution/MInt_ExecutionTypes.h"
#include "MInt_ExecutionConfigProvider.generated.h"

UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UMInt_ExecutionConfigProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * Optional execution-timing contract. An interactable implements this to declare how it is driven -
 * tap, hold, or hold-and-repeat. An interactable that does not implement it is treated as a Tap.
 */
class MAEVIXINTERACT_API IMInt_ExecutionConfigProvider
{
	GENERATED_BODY()

public:
	/** Execution timing for this interactable. Default returns a Tap config. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MaevixInteract|Execution")
	FMInt_ExecutionConfig GetExecutionConfig() const;
	virtual FMInt_ExecutionConfig GetExecutionConfig_Implementation() const;
};
