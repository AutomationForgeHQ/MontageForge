using UnrealBuildTool;

public class MontageForgeEditor : ModuleRules
{
	public MontageForgeEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags",
				"MontageForge",     // the notify class recipes refer to
				"EditorSubsystem",  // the builder is an editor subsystem
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"UnrealEd",         // UAnimMontageFactory, asset creation
				"AssetTools",
				"AssetRegistry",
				"Slate",
				"SlateCore",
			}
			);
	}
}
