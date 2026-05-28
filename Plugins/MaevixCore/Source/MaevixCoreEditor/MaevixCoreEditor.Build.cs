// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

using UnrealBuildTool;

public class MaevixCoreEditor : ModuleRules
{
    public MaevixCoreEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "MaevixCore",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "EditorWidgets",
                "UnrealEd",
                "ToolMenus",
                "ToolWidgets",
                "Slate",
                "SlateCore",
                "EditorSubsystem",
                "UMG",
                "UMGEditor",
                "Blutility",
                "Json",
                "JsonUtilities",
                "DesktopPlatform",
                "Projects",
                "WorkspaceMenuStructure",
                "Settings"
            }
        );
    }
}