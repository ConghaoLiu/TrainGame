// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class Space3D : ModuleRules
{
	public Space3D(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
    
    PublicIncludePaths.AddRange( new string[] {
				Path.Combine(ModuleDirectory, "Space3D"),
        Path.Combine(ModuleDirectory, "glm")
			});
      
    PublicDefinitions.Add("SPACE3D_BUILD_DYNAMIC=1");

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
      // Add the import library
      PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "Space3DDyn.lib"));
      
			// Delay-load the DLL, so we can load it from the right place first
			PublicDelayLoadDLLs.Add("Space3DDyn.dll");

			// Ensure that the DLL is staged next to the executable
			RuntimeDependencies.Add("$(TargetOutputDir)/Space3DDyn.dll", Path.Combine(PluginDirectory, "Source/ThirdParty/Space3D/lib/Space3DDyn.dll"));
    }
	}
}
