// Donjon de l'Oie — le mode de jeu.
#include "DDModeJeu.h"

#include "DDControleur.h"
#include "DDScene.h"
#include "Engine/World.h"

ADDModeJeu::ADDModeJeu()
{
    DefaultPawnClass = nullptr;              // la caméra est celle du jeu (CameraRig), portée par la scène
    PlayerControllerClass = ADDControleur::StaticClass();
}

void ADDModeJeu::StartPlay()
{
    Super::StartPlay();
    UWorld* Monde = GetWorld();
    if (Monde == nullptr) return;
    FActorSpawnParameters Parametres;
    Parametres.Name = TEXT("DonjonDeLOie");
    Parametres.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    Monde->SpawnActor<ADDScene>(ADDScene::StaticClass(), FTransform::Identity, Parametres);
}
