// Donjon de l'Oie — l'éditeur avec le jeu et ses outils.
using UnrealBuildTool;
using System.Collections.Generic;

public class DonjonDeLOieEditorTarget : TargetRules
{
	public DonjonDeLOieEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange(new string[] { "DonjonDeLOie", "DonjonDeLOieEditor" });
	}
}
