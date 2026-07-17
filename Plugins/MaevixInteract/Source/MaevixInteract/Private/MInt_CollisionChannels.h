// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

// Module-private collision channel validation helpers shared by the MaevixInteract
// components. Declared here and defined in one .cpp so the unity build cannot merge
// duplicate definitions across the two component .cpp files into one translation unit (C2084).

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

bool MInt_IsValidObjectChannel(ECollisionChannel Channel);
bool MInt_IsValidTraceChannel(ECollisionChannel Channel);
FString MInt_ChannelDisplayName(ECollisionChannel Channel);
