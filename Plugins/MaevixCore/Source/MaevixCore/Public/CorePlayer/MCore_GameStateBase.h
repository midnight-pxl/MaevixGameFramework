// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MCore_GameStateBase.generated.h"

class UMCore_GlobalEventReplicator;

/**
 * Optional GameState base with built-in UMCore_GlobalEventReplicator for network event support.
 * Set as your GameMode's GameStateClass for automatic global event networking.
 *
 * Alternative: add UMCore_GlobalEventReplicator to your own GameState instead.
 */
UCLASS()
class MAEVIXCORE_API AMCore_GameStateBase : public AGameStateBase
{
	GENERATED_BODY()
public:
	AMCore_GameStateBase();

	UFUNCTION(BlueprintPure, Category = "MaevixCore|Events")
	UMCore_GlobalEventReplicator* GetCoreGlobalEventReplicator() const { return GlobalEventReplicator; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MaevixCore|Events")
	TObjectPtr<UMCore_GlobalEventReplicator> GlobalEventReplicator;
};
