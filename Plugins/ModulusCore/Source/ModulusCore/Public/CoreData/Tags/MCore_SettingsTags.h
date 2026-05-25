// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "NativeGameplayTags.h"

namespace MCore_SettingsTags
{
    // ========================================================================
    // SETTING CATEGORIES
    // ========================================================================
	
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Category_Video);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Category_Audio);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Category_Controls);
	MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Category_KeyBinding);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Category_Accessibility);

    // ========================================================================
    // DEFAULT SUB-CATEGORIES
    // ========================================================================

    /*
     * Depth-4 tags (Category.X.General) for settings panel tab layout.
     * Single depth-4 child named General = no sub-tab bar render.
     * Multiple depth-4 children under the same parent = sub-tabs render.
     */
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Category_Video_Display);
	MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Category_Video_GraphicQuality);
	MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Category_Audio_General);
	MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Category_Controls_GeneralSettings);
	MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Category_Controls_KeyboardMouse);
	MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Category_Controls_Controller);
	MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Category_KeyBinding_General);
	MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Category_Accessibility_General);
	
    // ========================================================================
    // DISPLAY SETTINGS
    // ========================================================================

    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Display_Brightness);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Display_Resolution);
	MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Display_DynamicResolution);
	MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Display_ResolutionScale);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Display_WindowMode);
	MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Display_FrameRateLimit);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Display_VSync);
	MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Display_HDR);
	MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Display_HDRNits);

    // ========================================================================
    // GRAPHICS SETTINGS
    // ========================================================================

    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Graphics_QualityPreset);
	MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Graphics_GlobalIllumination);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Graphics_TextureQuality);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Graphics_FoliageQuality);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Graphics_PostProcessing);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Graphics_ShadowQuality);
	MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Graphics_ReflectionQuality);
	MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Graphics_ShadingQuality);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Graphics_AntiAliasing);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Graphics_ViewDistance);
	MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Graphics_EffectsQuality);

    // ========================================================================
    // AUDIO SETTINGS
    // ========================================================================

    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Audio_MasterVolume);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Audio_Quality);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Audio_MusicVolume);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Audio_SFXVolume);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Audio_VoiceVolume);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Audio_AmbientVolume);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Audio_UIVolume);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Audio_MuteAudio);

    // ========================================================================
    // CONTROLS SETTINGS
    // ========================================================================
	
	MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_KeyBinding_Placeholder);

    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Controls_MouseSensitivity);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Controls_MouseSmoothing);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Controls_GamepadSensitivity);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Controls_SensitivityX);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Controls_SensitivityY);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Controls_InvertLookX);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Controls_InvertLookY);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Controls_Vibration);

    // ========================================================================
    // ACCESSIBILITY SETTINGS
    // ========================================================================

    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Accessibility_Subtitles);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Accessibility_SubtitleSize);
	MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Accessibility_UITextSize);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Accessibility_UIScale);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Accessibility_ColorblindMode);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Accessibility_ColorblindStrength);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Accessibility_TooltipDelay);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Accessibility_ActiveTheme);

    // ========================================================================
    // SETTING EVENTS
    // ========================================================================

    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Event_ConfirmationRequired);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Event_GamepadIconSetChanged);
    MODULUSCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MCore_Settings_Event_ExternalValueChange);

    // ========================================================================
    // UTILITY FUNCTIONS
    // ========================================================================

    MODULUSCORE_API FGameplayTag GetCategoryTag(const FString& CategoryName);
    MODULUSCORE_API FGameplayTag GenerateCustomUISettingTag(const FString& Category, const FString& Setting);
}
