// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "Modules/ModuleManager.h"

class FMaevixInteractModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
