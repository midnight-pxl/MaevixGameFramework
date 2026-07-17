// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "MInt_CollisionChannels.h"

#include "Engine/CollisionProfile.h"
#include "Engine/EngineTypes.h"

bool MInt_IsValidObjectChannel(ECollisionChannel Channel)
{
	return Channel >= ECC_GameTraceChannel1
		&& Channel <= ECC_GameTraceChannel18
		&& UCollisionProfile::Get()->ConvertToObjectType(Channel) != ObjectTypeQuery_MAX;
}

bool MInt_IsValidTraceChannel(ECollisionChannel Channel)
{
	return Channel >= ECC_GameTraceChannel1
		&& Channel <= ECC_GameTraceChannel18
		&& UCollisionProfile::Get()->ConvertToTraceType(Channel) != TraceTypeQuery_MAX;
}

FString MInt_ChannelDisplayName(ECollisionChannel Channel)
{
	return StaticEnum<ECollisionChannel>()->GetDisplayNameTextByValue(static_cast<int64>(Channel)).ToString();
}
