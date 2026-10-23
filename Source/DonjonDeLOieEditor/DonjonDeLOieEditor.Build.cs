// Donjon de l'Oie — le module d'édition (chargé seulement dans l'éditeur) : il crée les matériaux du jeu.
using UnrealBuildTool;

public class DonjonDeLOieEditor : ModuleRules
{
	public DonjonDeLOieEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.NoPCHs;
		bUseUnity = false;
		CppStandard = CppStandardVersion.Cpp20;

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "UnrealEd", "MaterialEditor", "AssetRegistry", "DonjonDeLOie"
		});
	}
}
