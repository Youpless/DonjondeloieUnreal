// Donjon de l'Oie — la fenêtre du jeu : caractères tapés et molette.
#include "DDViewportClient.h"

#include "InputCoreTypes.h"
#include "InputKeyEventArgs.h"

FString UDDViewportClient::Caracteres;
float UDDViewportClient::Crans = 0.0f;

bool UDDViewportClient::InputKey(const FInputKeyEventArgs& EventArgs)
{
    if (EventArgs.Event == IE_Pressed)
    {
        if (EventArgs.Key == EKeys::MouseScrollUp) Crans += 1.0f;
        else if (EventArgs.Key == EKeys::MouseScrollDown) Crans -= 1.0f;
    }
    return Super::InputKey(EventArgs);
}

bool UDDViewportClient::InputChar(FViewport* InViewport, int32 ControllerId, TCHAR Character)
{
    // les caractères de commande (retour arrière, entrée, tabulation, échap) passent par les touches
    if (Character >= 32 && Character != 127) Caracteres.AppendChar(Character);
    return Super::InputChar(InViewport, ControllerId, Character);
}

FString UDDViewportClient::PrendreCaracteres()
{
    FString s = MoveTemp(Caracteres);
    Caracteres.Reset();
    return s;
}

float UDDViewportClient::PrendreMolette()
{
    float c = Crans;
    Crans = 0.0f;
    return c;
}
