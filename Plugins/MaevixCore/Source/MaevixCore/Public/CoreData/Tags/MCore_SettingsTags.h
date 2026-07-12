// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "NativeGameplayTags.h"

namespace MCore_SettingsTags
{
    // ============================================================================
    // SETTING CATEGORIES
    // ============================================================================
	
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Category_Video);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Category_Audio);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Category_Controls);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Category_KeyBinding);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Category_Accessibility);

    // ============================================================================
    // DEFAULT SUB-CATEGORIES
    // ============================================================================

    /*
     * Depth-4 tags (Category.X.General) for settings panel tab layout.
     * Single depth-4 child named General = no sub-tab bar render.
     * Multiple depth-4 children under the same parent = sub-tabs render.
     */
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Category_Video_Display);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Category_Video_GraphicQuality);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Category_Audio_General);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Category_Controls_GeneralSettings);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Category_Controls_KeyboardMouse);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Category_Controls_Controller);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Category_KeyBinding_General);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Category_Accessibility_General);
	
    // ============================================================================
    // DISPLAY SETTINGS
    // ============================================================================

    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Display_Brightness);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Display_Resolution);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Display_DynamicResolution);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Display_ResolutionScale);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Display_WindowMode);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Display_FrameRateLimit);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Display_VSync);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Display_HDR);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Display_HDRNits);

    // ============================================================================
    // GRAPHICS SETTINGS
    // ============================================================================

    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Graphics_QualityPreset);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Graphics_GlobalIllumination);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Graphics_TextureQuality);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Graphics_FoliageQuality);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Graphics_PostProcessing);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Graphics_ShadowQuality);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Graphics_ReflectionQuality);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Graphics_ShadingQuality);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Graphics_AntiAliasing);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Graphics_ViewDistance);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Graphics_EffectsQuality);

    // ============================================================================
    // AUDIO SETTINGS
    // ============================================================================

    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Audio_MasterVolume);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Audio_Quality);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Audio_MusicVolume);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Audio_SFXVolume);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Audio_VoiceVolume);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Audio_AmbientVolume);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Audio_UIVolume);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Audio_MuteAudio);

    // ============================================================================
    // CONTROLS SETTINGS
    // ============================================================================
	
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_KeyBinding_Placeholder);

    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Controls_MouseSensitivity);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Controls_MouseSmoothing);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Controls_GamepadSensitivity);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Controls_SensitivityX);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Controls_SensitivityY);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Controls_InvertLookX);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Controls_InvertLookY);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Controls_Vibration);

    // ============================================================================
    // ACCESSIBILITY SETTINGS
    // ============================================================================

    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Accessibility_Subtitles);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Accessibility_SubtitleSize);
	MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Accessibility_UITextSize);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Accessibility_UIScale);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Accessibility_ColorblindMode);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Accessibility_ColorblindStrength);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Accessibility_TooltipDelay);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Accessibility_ActiveTheme);

    // ============================================================================
    // SETTING EVENTS
    // ============================================================================

    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Event_ConfirmationRequired);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Event_GamepadIconSetChanged);
    MAEVIXCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Event_ExternalValueChange);

    // ============================================================================
    // UTILITY FUNCTIONS
    // ============================================================================

    MAEVIXCORE_API FGameplayTag GetCategoryTag(const FString& CategoryName);
    MAEVIXCORE_API FGameplayTag GenerateCustomUISettingTag(const FString& Category, const FString& Setting);
}
