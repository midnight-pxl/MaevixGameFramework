// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MCore_GameFlowLibrary.generated.h"

/**
 * Blueprint-callable helpers for high-level game-flow transitions.
 * Thin client-side wrappers over engine travel that also prime the loading-screen
 * context, so the menu can start a New Game without hard-referencing demo content.
 */
UCLASS()
class MAEVIXCORE_API UMCore_GameFlowLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Travels to UMCore_CoreSettings::NewGameTargetLevel, priming the loading-screen
	 * context to MCore.Loading.Context.InGame first. No-ops with a warning when no
	 * target level is configured.
	 *
	 * Network: client-local. Performs no authority check and issues no RPC; this is the
	 * single-player / menu-to-game entry point only. A networked New Game (ServerTravel)
	 * is out of scope.
	 */
	UFUNCTION(BlueprintCallable, Category="MaevixCore|Game Flow",
		meta=(WorldContext="WorldContextObject"))
	static void StartNewGame(const UObject* WorldContextObject);
};
