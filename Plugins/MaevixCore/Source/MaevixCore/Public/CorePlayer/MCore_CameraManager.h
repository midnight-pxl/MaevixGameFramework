// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "MCore_CameraManager.generated.h"

/**
 * Basic First Person Camera Manager. 
 * Limits min/max look pitch.
 */
UCLASS()
class MAEVIXCORE_API AMCore_CameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	
public:
	
	/** Constructor */
	AMCore_CameraManager();
};
