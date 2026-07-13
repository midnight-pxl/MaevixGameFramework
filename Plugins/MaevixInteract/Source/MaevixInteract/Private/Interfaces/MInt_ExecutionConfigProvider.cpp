// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "Interfaces/MInt_ExecutionConfigProvider.h"

FMInt_ExecutionConfig IMInt_ExecutionConfigProvider::GetExecutionConfig_Implementation() const
{
	// Tap config by default; an interactable overrides this to drive Hold or Repeat timing.
	return FMInt_ExecutionConfig();
}
