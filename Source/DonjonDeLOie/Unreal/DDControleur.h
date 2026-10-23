// Donjon de l'Oie — le joueur : la souris reste visible et libre (le jeu se joue à la souris), le clavier sert aux
// raccourcis (Échap, Espace, flèches, F1, Tab…). Les entrées sont lues par la scène (ADDScene) à chaque image.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "DDControleur.generated.h"

UCLASS()
class ADDControleur : public APlayerController
{
    GENERATED_BODY()

public:
    ADDControleur();

protected:
    virtual void BeginPlay() override;
};
