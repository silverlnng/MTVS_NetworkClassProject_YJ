// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NetClassProject_YJ : ModuleRules
{
	public NetClassProject_YJ(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
