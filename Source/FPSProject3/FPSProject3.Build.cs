// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FPSProject3 : ModuleRules
{
	public FPSProject3(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"FPSProject3",
			"FPSProject3/Variant_Horror",
			"FPSProject3/Variant_Horror/UI",
			"FPSProject3/Variant_Shooter",
			"FPSProject3/Variant_Shooter/AI",
			"FPSProject3/Variant_Shooter/UI",
			"FPSProject3/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
