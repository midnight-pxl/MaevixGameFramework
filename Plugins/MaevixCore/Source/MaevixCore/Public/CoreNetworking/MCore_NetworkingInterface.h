// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MCore_NetworkingInterface.generated.h"

UINTERFACE(BlueprintType, Blueprintable)
class UMCore_NetworkingInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Networking interface providing authority validation and network lifecycle hooks.
 * Implemented by UMCore_NetworkingComponent for actor-based networking.
 */
class MAEVIXCORE_API IMCore_NetworkingInterface
{
	GENERATED_BODY()

public:

	virtual bool HasNetworkAuthority() const = 0;

	virtual void OnNetworkInitialized() {}

	virtual void OnNetworkShutdown() {}

	/** Publishes this component's authority verdict. The base fires this once at initialization;
	    read authority at point of use if you need it live (see UMCore_NetworkingComponent). */
	virtual void OnAuthorityChanged(bool bHasAuthority) {}
};
