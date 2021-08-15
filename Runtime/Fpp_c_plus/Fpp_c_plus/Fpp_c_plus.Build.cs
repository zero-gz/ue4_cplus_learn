// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Fpp_c_plus : ModuleRules
{
	public Fpp_c_plus(ReadOnlyTargetRules Target) : base(Target)
	{
        OptimizeCode = CodeOptimization.Never;
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "RenderCore", "RHI" });
	}
}
