// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

/**
 * Theme-system event tags. Lives outside MCore_SettingsTags because a theme
 * change is a UI-system event, not a settings-internal event. Future
 * cross-plugin triggers (debug commands, programmatic accessibility
 * overrides, asset-load notifications) belong here, not in the settings
 * namespace.
 */

#pragma once

#include "NativeGameplayTags.h"

namespace MCore_ThemeTags
{
    /* Broadcast on UMCore_LocalEventSubsystem when the active UI theme changes.
     * Payload (FMCore_EventData::EventParams):
     *   ThemePath = soft object path of the new UMCore_PDA_UITheme_Base. */
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Theme_Changed);

    /* Stable identity tags for the shipped UI themes. Plugins match their own
     * per-theme styles against UMCore_PDA_UITheme_Base::ThemeId by comparing these,
     * so they never hard-reference a Core theme DataAsset. */
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Theme_Id_Minimal);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Theme_Id_ModFi);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Theme_Id_Sample);
}
