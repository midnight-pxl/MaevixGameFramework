// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "NativeGameplayTags.h"

namespace MCore_InteractionTags
{
	/* Root of the interaction classifier namespace. Registered so meta = (Categories = "MCore.Interaction")
	   resolves in the tag picker; games add their own child tags (e.g. MCore.Interaction.Door) natively or via ini. */
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Interaction);
}
