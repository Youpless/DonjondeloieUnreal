// Donjon de l'Oie — l'interface du jeu à l'écran (widget Slate) et la mesure des textes.
#include "SDDInterface.h"

#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Layout/Geometry.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderer.h"
#include "Styling/CoreStyle.h"

#include "DDConversions.h"

namespace
{
    FSlateVertex Sommet(const FSlateRenderTransform& Transformation, FVector2f Local, FVector2f UV, FColor Couleur)
    {
        FSlateVertex S;
        FMemory::Memzero(&S, sizeof(S));
        S.Position = Transformation.TransformPoint(Local);
        S.TexCoords[0] = UV.X;
        S.TexCoords[1] = UV.Y;
        S.TexCoords[2] = 1.0f;
        S.TexCoords[3] = 1.0f;
        S.MaterialTexCoords = UV;
        S.Color = Couleur;
        S.SecondaryColor = FColor(0, 0, 0, 0);
        return S;
    }
}

void SDDInterface::Construct(const FArguments& InArgs)
{
    SetVisibility(EVisibility::HitTestInvisible);
    SetCanTick(false);
}

void SDDInterface::Montrer(TSharedPtr<const FDDDessinInterface> Image)
{
    Dessin = MoveTemp(Image);
    Invalidate(EInvalidateWidgetReason::Paint);
}

FVector2D SDDInterface::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
    return FVector2D(1920.0, 1080.0);
}

int32 SDDInterface::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
                            int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    if (!Dessin.IsValid()) return LayerId;
    // le dessin est en pixels de la fenêtre ; l'espace local du widget est en unités de Slate (pixels / échelle DPI)
    const float Echelle = AllottedGeometry.GetAccumulatedLayoutTransform().GetScale();
    const float Inverse = Echelle > 0.0f ? 1.0f / Echelle : 1.0f;
    const FSlateRenderTransform& Transformation = AllottedGeometry.GetAccumulatedRenderTransform();
    int32 Couche = LayerId;
    for (const FDDDessinInterface::FCommande& C : Dessin->Commandes)
    {
        Couche++;
        if (C.EstTexte)
        {
            for (const FDDDessinInterface::FTexte& T : C.Textes)
            {
                const FSlateFontInfo Police = FDDMesureurSlate::Police(T.TaillePx * Inverse, T.Gras, T.Italique);
                const FVector2f Coin = T.Position * Inverse;
                const FVector2f Taille(16384.0f, T.TaillePx * 2.0f * Inverse);
                const FPaintGeometry Geometrie = AllottedGeometry.ToPaintGeometry(Taille, FSlateLayoutTransform(Coin));
                FSlateDrawElement::MakeText(OutDrawElements, Couche, Geometrie, T.Texte, Police, ESlateDrawEffect::None, T.Couleur);
            }
            continue;
        }
        const int32 N = C.Sommets.Num() / 4;
        if (N == 0) continue;
        Sommets.Reset(N * 4);
        Indices.Reset(N * 6);
        for (int32 q = 0; q < N; q++)
        {
            const SlateIndex Base = static_cast<SlateIndex>(Sommets.Num());
            for (int32 k = 0; k < 4; k++)
            {
                const FDDDessinInterface::FSommet& S = C.Sommets[q * 4 + k];
                Sommets.Add(Sommet(Transformation, S.Position * Inverse, S.UV, S.Couleur));
            }
            Indices.Add(Base);
            Indices.Add(Base + 1);
            Indices.Add(Base + 2);
            Indices.Add(Base);
            Indices.Add(Base + 2);
            Indices.Add(Base + 3);
        }
        FSlateDrawElement::MakeCustomVerts(OutDrawElements, Couche, C.Texture, Sommets, Indices, nullptr, 0, 0);
    }
    return Couche;
}

// ------------------------------------------------------------------ mesure des textes

FSlateFontInfo FDDMesureurSlate::Police(float taillePx, bool gras, bool italique)
{
    const TCHAR* Graisse = gras ? (italique ? TEXT("BoldItalic") : TEXT("Bold")) : (italique ? TEXT("Italic") : TEXT("Regular"));
    return FCoreStyle::GetDefaultFontStyle(FName(Graisse), FMath::Max(1.0f, taillePx * 0.75f));
}

float FDDMesureurSlate::Largeur(const std::string& texte, float taillePx, bool gras, bool italique)
{
    if (texte.empty()) return 0.0f;
    FSlateRenderer* Rendu = FSlateApplication::IsInitialized() ? FSlateApplication::Get().GetRenderer() : nullptr;
    if (Rendu == nullptr) return Moteur::Dessin::MesureurApproximatif().Largeur(texte, taillePx, gras, italique);
    const TSharedRef<FSlateFontMeasure> Mesure = Rendu->GetFontMeasureService();
    const FString Texte = DDUnreal::Texte(texte);
    const FVector2D Taille = Mesure->Measure(Texte, Police(taillePx, gras, italique), 1.0f);
    return static_cast<float>(Taille.X);
}

float FDDMesureurSlate::HauteurLigne(float taillePx)
{
    FSlateRenderer* Rendu = FSlateApplication::IsInitialized() ? FSlateApplication::Get().GetRenderer() : nullptr;
    if (Rendu == nullptr) return taillePx * 1.15f;
    const TSharedRef<FSlateFontMeasure> Mesure = Rendu->GetFontMeasureService();
    return static_cast<float>(Mesure->GetMaxCharacterHeight(Police(taillePx, false, false), 1.0f));
}
