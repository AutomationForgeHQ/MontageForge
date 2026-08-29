using UnrealBuildTool;

public class MontageForge : ModuleRules
{
	public MontageForge(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",           // UAnimNotify
				"GameplayTags",     // the tag the notify carries
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"GameplayAbilities",  // SendGameplayEventToActor - the only reason this module exists
			}
			);
	}
}
