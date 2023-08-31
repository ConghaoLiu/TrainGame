using UnrealBuildTool;

public class Space3DUnreal : ModuleRules
{
	public Space3DUnreal(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		/* PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			); */
				
		
		/* PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			); */
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
        "AudioMixer"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"AudioExtensions",
				"CoreUObject",
				"Engine",
				"Projects",
				"Slate",
				"SlateCore",
				"Space3D"
				//"UnrealEd"
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				//"AssetTools",
			}
			);
      
    PrivateIncludePathModuleNames.AddRange(
      new string[]
      {
        //"AssetTools",
      }
      );
	}
}
