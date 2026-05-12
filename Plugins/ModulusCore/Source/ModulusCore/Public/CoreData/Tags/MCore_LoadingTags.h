// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

/**
 * Loading-screen context tags. Drive Tag-Based selection in
 * UMCore_LoadingScreenSubsystem when LoadingScreenSelectionMode=TagBased.
 *
 * Game code calls SetNextLoadingContext({...}) before OpenLevel/ServerTravel
 * to scope which library entries are eligible. An entry matches when any of
 * its ContextTags overlap the pending context.
 */

#pragma once

#include "NativeGameplayTags.h"

namespace MCore_LoadingTags
{
    /* Initial boot to main menu transition. */
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Loading_Context_Startup);

    /* Gameplay-to-gameplay level transitions. */
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Loading_Context_InGame);

    /* Returning from gameplay to the main menu map. */
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Loading_Context_MainMenu);
}
