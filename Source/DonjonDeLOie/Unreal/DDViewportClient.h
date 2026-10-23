// Donjon de l'Oie — la fenêtre du jeu (UGameViewportClient) : elle note les caractères tapés (pour les champs de
// saisie du salon) et les crans de la molette, que la scène (ADDScene) transmet au jeu à chaque image.
// Choisie dans Config/DefaultEngine.ini (GameViewportClientClassName).
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"

#include "DDViewportClient.generated.h"

UCLASS()
class UDDViewportClient : public UGameViewportClient
{
    GENERATED_BODY()

public:
    virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
    virtual bool InputChar(FViewport* InViewport, int32 ControllerId, TCHAR Character) override;

    /// Ce qui a été tapé depuis la dernière lecture (vidé par la lecture).
    static FString PrendreCaracteres();
    /// Crans de molette depuis la dernière lecture (positif : vers l'avant), vidés par la lecture.
    static float PrendreMolette();

private:
    static FString Caracteres;
    static float Crans;
};
