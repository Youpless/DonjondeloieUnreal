// Donjon de l'Oie — l'interface du jeu à l'écran : un widget Slate, posé sur toute la fenêtre du jeu, qui peint la liste
// de dessin préparée par le moteur du jeu (Moteur/Dessin.h : des quadrilatères texturés et des lignes de texte, toute
// la mise en page d'uGUI étant déjà faite). Il ne reçoit aucun clic : la souris est lue par la scène (ADDScene) et
// donnée au système d'événements du jeu, comme dans Unity.
//
// Et la mesure des textes pour le moteur du jeu (largeur d'un morceau, hauteur d'une ligne) avec la police d'Unreal,
// pour que les retours à la ligne et les tailles automatiques tombent juste à l'écran.
#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
#include "Rendering/RenderingCommon.h"
#include "Rendering/SlateResourceHandle.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SLeafWidget.h"

#include "Moteur/Dessin.h"

#include <string>

/// Une image de l'interface, en pixels de la fenêtre (origine en haut à gauche).
struct FDDDessinInterface
{
    struct FSommet
    {
        FVector2f Position;
        FVector2f UV;
        FColor Couleur;
    };

    struct FTexte
    {
        FString Texte;
        FVector2f Position;           // coin haut-gauche de la boîte de ligne
        float TaillePx = 14.0f;
        bool Gras = false;
        bool Italique = false;
        FLinearColor Couleur;
    };

    struct FCommande
    {
        bool EstTexte = false;
        FSlateResourceHandle Texture;
        TArray<FSommet> Sommets;      // des quadrilatères : 4 sommets chacun (bas-gauche, haut-gauche, haut-droit, bas-droit)
        TArray<FTexte> Textes;
    };

    TArray<FCommande> Commandes;
};

class SDDInterface : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(SDDInterface) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    /// L'image à peindre désormais (préparée par la scène après chaque image du jeu).
    void Montrer(TSharedPtr<const FDDDessinInterface> Image);

    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
                          int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

    virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;

private:
    TSharedPtr<const FDDDessinInterface> Dessin;
    mutable TArray<FSlateVertex> Sommets;
    mutable TArray<SlateIndex> Indices;
};

/// La mesure des textes du jeu avec la police d'Unreal (Roboto, celle de Slate).
class FDDMesureurSlate : public Moteur::Dessin::MesureurTexte
{
public:
    virtual float Largeur(const std::string& texte, float taillePx, bool gras, bool italique) override;
    virtual float HauteurLigne(float taillePx) override;

    /// La police pour une taille donnée en pixels (corps du texte, comme fontSize dans Unity). Slate compte en points à
    /// 96 points par pouce : 1 pixel = 0,75 point.
    static FSlateFontInfo Police(float taillePx, bool gras, bool italique);
};
