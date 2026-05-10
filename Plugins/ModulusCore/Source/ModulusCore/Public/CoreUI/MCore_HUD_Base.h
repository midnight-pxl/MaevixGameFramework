// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MCore_HUD_Base.generated.h"

class UMCore_UISubsystem;

/**
 * Base HUD class bridging AHUD with the CommonUI-based PrimaryGameLayout and UISubsystem.
 * Set your GameMode's HUDClass to this or a derived Blueprint.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class MODULUSCORE_API AMCore_HUD_Base : public AHUD
{
	GENERATED_BODY()

public:
	AMCore_HUD_Base(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category = "UI|HUD")
	bool HasValidPrimaryGameLayout() const;

	/** Returns nullptr if the owning PlayerController has no LocalPlayer. */
	UFUNCTION(BlueprintPure, Category = "UI|HUD")
	UMCore_UISubsystem* GetUISubsystem() const;
};
