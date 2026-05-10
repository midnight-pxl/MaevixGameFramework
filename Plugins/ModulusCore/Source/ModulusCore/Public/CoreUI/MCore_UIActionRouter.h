// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Input/CommonUIActionRouterBase.h"
#include "MCore_UIActionRouter.generated.h"

/**
 * Concrete CommonUI action router required for widget activation and input routing.
 *
 * CommonUI's UCommonUIActionRouterBase is abstract. Without a concrete subclass,
 * the engine creates no action router subsystem, which silently breaks:
 * - Widget activation (NativeOnActivated never fires)
 * - Keyboard/gamepad input routing
 * - Action bar population
 *
 * This class provides the required concrete implementation.
 * The base class handles all routing logic - no overrides needed.
 */
UCLASS()
class MODULUSCORE_API UMCore_UIActionRouter : public UCommonUIActionRouterBase
{
	GENERATED_BODY()
};
