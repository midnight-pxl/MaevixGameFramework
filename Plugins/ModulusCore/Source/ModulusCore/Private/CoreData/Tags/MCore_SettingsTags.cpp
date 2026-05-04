// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreData/Tags/MCore_SettingsTags.h"

namespace MCore_SettingsTags
{

    // ========================================================================
    // SETTING CATEGORIES
    // ========================================================================
    
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Category_Video, "MCore.Settings.Category.Video");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Category_Audio, "MCore.Settings.Category.Audio");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Category_Controls, "MCore.Settings.Category.Controls");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Category_KeyBinding, "MCore.Settings.Category.KeyBinding");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Category_Accessibility, "MCore.Settings.Category.Accessibility");

    // ========================================================================
    // DEFAULT SUB-CATEGORIES
    // ========================================================================

    /* Depth-4 tags (Category.X.General) for settings panel tab layout w/o sub-tab render. */
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Category_Video_Display, "MCore.Settings.Category.Video.Display");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Category_Video_GraphicQuality, "MCore.Settings.Category.Video.GraphicQuality");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Category_Audio_General, "MCore.Settings.Category.Audio.General");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Category_Controls_GeneralSettings, "MCore.Settings.Category.Controls.GeneralSettings");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Category_Controls_KeyboardMouse, "MCore.Settings.Category.Controls.KeyboardMouse");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Category_Controls_Controller, "MCore.Settings.Category.Controls.Controller");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Category_KeyBinding_General, "MCore.Settings.Category.KeyBinding.General");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Category_Accessibility_General, "MCore.Settings.Category.Accessibility.General");

    // ========================================================================
    // DISPLAY SETTINGS
    // ========================================================================

    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Display_Brightness, "MCore.Settings.Display.Brightness");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Display_Resolution, "MCore.Settings.Display.Resolution");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Display_DynamicResolution, "MCore.Settings.Display.DynamicResolution");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Display_ResolutionScale, "MCore.Settings.Display.ResolutionScale");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Display_WindowMode, "MCore.Settings.Display.WindowMode");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Display_FrameRateLimit, "MCore.Settings.Display.FrameRateLimit");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Display_VSync, "MCore.Settings.Display.VSync");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Display_HDR, "MCore.Settings.Display.HDR");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Display_HDRNits, "MCore.Settings.Display.HDRNits");

    // ========================================================================
    // GRAPHICS SETTINGS
    // ========================================================================
    
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Graphics_QualityPreset, "MCore.Settings.Graphics.QualityPreset");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Graphics_GlobalIllumination, "MCore.Settings.Graphics.GlobalIllumination");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Graphics_TextureQuality, "MCore.Settings.Graphics.TextureQuality");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Graphics_FoliageQuality, "MCore.Settings.Graphics.FoliageQuality");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Graphics_PostProcessing, "MCore.Settings.Graphics.PostProcessing");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Graphics_ShadowQuality, "MCore.Settings.Graphics.ShadowQuality");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Graphics_ReflectionQuality, "MCore.Settings.Graphics.ReflectionQuality");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Graphics_ShadingQuality, "MCore.Settings.Graphics.ShadingQuality");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Graphics_AntiAliasing, "MCore.Settings.Graphics.AntiAliasing");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Graphics_ViewDistance, "MCore.Settings.Graphics.ViewDistance");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Graphics_EffectsQuality, "MCore.Settings.Graphics.EffectsQuality");

    // ========================================================================
    // AUDIO SETTINGS
    // ========================================================================

    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Audio_MasterVolume, "MCore.Settings.Audio.MasterVolume");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Audio_Quality, "MCore.Settings.Audio.Quality");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Audio_MusicVolume, "MCore.Settings.Audio.MusicVolume");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Audio_SFXVolume, "MCore.Settings.Audio.SFXVolume");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Audio_VoiceVolume, "MCore.Settings.Audio.VoiceVolume");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Audio_AmbientVolume, "MCore.Settings.Audio.AmbientVolume");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Audio_UIVolume, "MCore.Settings.Audio.UIVolume");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Audio_MuteAudio, "MCore.Settings.Audio.MuteAudio");

    // ========================================================================
    // CONTROLS SETTINGS
    // ========================================================================

    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_KeyBinding_Placeholder, "MCore.Settings.KeyBinding.Placeholder"); 
    
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Controls_MouseSensitivity, "MCore.Settings.Controls.MouseSensitivity");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Controls_MouseSmoothing, "MCore.Settings.Controls.MouseSmoothing");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Controls_GamepadSensitivity, "MCore.Settings.Controls.GamepadSensitivity");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Controls_SensitivityX, "MCore.Settings.Controls.SensitivityX");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Controls_SensitivityY, "MCore.Settings.Controls.SensitivityY");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Controls_InvertLookX, "MCore.Settings.Controls.InvertLookX");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Controls_InvertLookY, "MCore.Settings.Controls.InvertLookY");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Controls_Vibration, "MCore.Settings.Controls.Vibration");

    // ========================================================================
    // ACCESSIBILITY SETTINGS
    // ========================================================================

    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Accessibility_Subtitles, "MCore.Settings.Accessibility.Subtitles");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Accessibility_SubtitleSize, "MCore.Settings.Accessibility.SubtitleSize");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Accessibility_UITextSize, "MCore.Settings.Accessibility.UITextSize");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Accessibility_UIScale, "MCore.Settings.Accessibility.UIScale");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Accessibility_ColorblindMode, "MCore.Settings.Accessibility.ColorblindMode");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Accessibility_ColorblindStrength, "MCore.Settings.Accessibility.ColorblindStrength");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Accessibility_TooltipDelay, "MCore.Settings.Accessibility.TooltipDelay");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Accessibility_ActiveTheme, "MCore.Settings.Accessibility.ActiveTheme");

    // ========================================================================
    // SETTING EVENTS
    // ========================================================================

    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Event_ConfirmationRequired, "MCore.Settings.Event.ConfirmationRequired");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Event_GamepadIconSetChanged, "MCore.Settings.Event.GamepadIconSetChanged");
    UE_DEFINE_GAMEPLAY_TAG(MCore_Settings_Event_ExternalValueChange, "MCore.Settings.Event.ExternalValueChange");

    // ========================================================================
    // UTILITY FUNCTIONS
    // ========================================================================

    FGameplayTag GetCategoryTag(const FString& CategoryName)
    {
        if (CategoryName.Equals(TEXT("Video"), ESearchCase::IgnoreCase))
            return MCore_Settings_Category_Video;
        if (CategoryName.Equals(TEXT("Audio"), ESearchCase::IgnoreCase))
            return MCore_Settings_Category_Audio;
        if (CategoryName.Equals(TEXT("Controls"), ESearchCase::IgnoreCase))
            return MCore_Settings_Category_Controls;
        if (CategoryName.Equals(TEXT("KeyBinding"), ESearchCase::IgnoreCase))
            return MCore_Settings_Category_KeyBinding;
        if (CategoryName.Equals(TEXT("Accessibility"), ESearchCase::IgnoreCase))
            return MCore_Settings_Category_Accessibility;

        return FGameplayTag::EmptyTag;
    }

    FGameplayTag GenerateCustomUISettingTag(const FString& Category, const FString& UISetting)
    {
        FString TagString = FString::Printf(TEXT("%s.%s"), *Category, *UISetting);
        return FGameplayTag::RequestGameplayTag(FName(*TagString));
    }
}
