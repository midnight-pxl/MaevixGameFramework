// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CorePlayer/MCore_GameStateBase.h"

#include "CoreEvents/MCore_GlobalEventReplicator.h"

AMCore_GameStateBase::AMCore_GameStateBase()
{
	GlobalEventReplicator = CreateDefaultSubobject<UMCore_GlobalEventReplicator>(
		TEXT("GlobalEventReplicator"));
}
