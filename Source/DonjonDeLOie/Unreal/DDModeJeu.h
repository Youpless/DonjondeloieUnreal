// Donjon de l'Oie — le mode de jeu : pas de personnage à piloter ; au lancement de la partie, la scène du jeu
// (ADDScene) est créée : c'est elle qui fait tourner le jeu image par image et le montre.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "DDModeJeu.generated.h"

UCLASS()
class ADDModeJeu : public AGameModeBase
{
    GENERATED_BODY()

public:
    ADDModeJeu();

    virtual void StartPlay() override;
};
