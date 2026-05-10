// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreData/Logging/LogModulusNetworking.h"
#include "Modules/ModuleManager.h"

class FModulusCoreModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
