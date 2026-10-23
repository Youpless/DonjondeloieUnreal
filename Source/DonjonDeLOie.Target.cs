// Donjon de l'Oie — le jeu seul (partie empaquetée).
using UnrealBuildTool;
using System.Collections.Generic;

public class DonjonDeLOieTarget : TargetRules
{
	public DonjonDeLOieTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("DonjonDeLOie");
	}
}
