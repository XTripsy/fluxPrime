// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class fluxPrime : ModuleRules
{
	public fluxPrime(ReadOnlyTargetRules Target) : base(Target)
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
			
		
		// Keep this list limited to modules whose types appear in Public/ headers:
		// GameplayTagContainer.h, NavigationSystem.h, NavMesh/RecastNavMesh.h,
		// AnimToTextureDataAsset.h, StructUtils/InstancedStruct.h, Niagara headers,
		// and FUniqueNetIdRepl (defined in Engine, implemented by CoreOnline).
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", 
				"GameplayTags",
				"NavigationSystem",
				"Navmesh",
				"AnimToTexture",
				"StructUtils", 
				"Niagara",
				"CoreOnline"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine"
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
