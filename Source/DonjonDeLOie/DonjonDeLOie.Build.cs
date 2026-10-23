// Donjon de l'Oie — le module du jeu. Le code du jeu (dossiers Noyau, Moteur, Regles, Voxel, Modeles, Monde,
// Interface, Pilote, Cartes) est du C++20 standard, sans rien d'Unreal ; seul le dossier Unreal/ parle au moteur.
using UnrealBuildTool;

public class DonjonDeLOie : ModuleRules
{
	public DonjonDeLOie(ReadOnlyTargetRules Target) : base(Target)
	{
		// Pas d'en-tête précompilé : chaque fichier se compile seul (le code du jeu n'inclut rien d'Unreal).
		PCHUsage = PCHUsageMode.NoPCHs;
		// Pas de regroupement des fichiers (« unity build ») : deux fichiers du jeu peuvent avoir des fonctions locales de même nom.
		bUseUnity = false;
		CppStandard = CppStandardVersion.Cpp20;

		// Les en-têtes s'incluent depuis la racine du module (« Moteur/Moteur.h », « Unreal/DDScene.h »…),
		// aussi pour le module d'édition.
		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "ProceduralMeshComponent", "Slate", "SlateCore"
		});
	}
}
