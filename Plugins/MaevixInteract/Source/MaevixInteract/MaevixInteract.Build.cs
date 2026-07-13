// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

using UnrealBuildTool;

public class MaevixInteract : ModuleRules
{
	public MaevixInteract(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);

		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags",
				"DeveloperSettings",
				"MaevixCore"
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
			}
			);

		bUseUnity = true;
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;
	}
}
