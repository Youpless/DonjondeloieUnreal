// Donjon de l'Oie — le joueur.
#include "DDControleur.h"

ADDControleur::ADDControleur()
{
    bShowMouseCursor = true;
    bEnableClickEvents = false;
    bEnableMouseOverEvents = false;
    DefaultMouseCursor = EMouseCursor::Default;
}

void ADDControleur::BeginPlay()
{
    Super::BeginPlay();
    FInputModeGameAndUI Mode;
    Mode.SetHideCursorDuringCapture(false);
    Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(Mode);
}
