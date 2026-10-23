// Donjon de l'Oie — la scène : l'interface. Après chaque image du jeu, le moteur fait la mise en page d'uGUI et rend une
// liste de dessin (Moteur/Dessin.h) ; elle est traduite pour le widget Slate (SDDInterface), et les textures du jeu
// (icônes en pixel art, panneaux, dégradés) deviennent des textures d'Unreal.
#include "DDScene.h"

#include "Engine/GameViewportClient.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/SlateRenderer.h"
#include "Styling/SlateBrush.h"
#include "TextureResource.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

#include "DDConversions.h"

namespace
{
    /// Une texture inutilisée pendant autant d'images est libérée (les portraits des évadés d'une partie finie).
    constexpr int32 ImagesAvantOubli = 600;
}

void ADDScene::InstallerInterface()
{
    UWorld* Monde = GetWorld();
    UGameViewportClient* Fenetre = Monde != nullptr ? Monde->GetGameViewport() : nullptr;
    if (Fenetre == nullptr) return;
    Widget = SNew(SDDInterface);
    Fenetre->AddViewportWidgetContent(Widget.ToSharedRef(), 10);
}

void ADDScene::RetirerInterface()
{
    if (Widget.IsValid())
    {
        UWorld* Monde = GetWorld();
        if (UGameViewportClient* Fenetre = Monde != nullptr ? Monde->GetGameViewport() : nullptr) Fenetre->RemoveViewportWidgetContent(Widget.ToSharedRef());
        Widget.Reset();
    }
    EtatsTextures.Empty();
    TexturesInterface.Empty();
}

FSlateResourceHandle ADDScene::TexturePour(const Moteur::Texture2D* Texture)
{
    const int32 Id = Texture != nullptr ? Texture->GetInstanceID() : 0;
    const int32 Version = Texture != nullptr ? Texture->Version() : 0;
    FDDTextureInterface& Etat = EtatsTextures.FindOrAdd(Id);
    Etat.Vue = ImageUnreal;
    if (Etat.Pinceau.IsValid() && Etat.Version == Version) return Etat.Poignee;

    const int32 W = Texture != nullptr ? FMath::Max(1, Texture->width) : 4;
    const int32 H = Texture != nullptr ? FMath::Max(1, Texture->height) : 4;
    const FName Nom = MakeUniqueObjectName(GetTransientPackage(), UTexture2D::StaticClass(), FName(TEXT("DD_Interface")));
    UTexture2D* T = UTexture2D::CreateTransient(W, H, PF_B8G8R8A8, Nom);
    if (T == nullptr) return FSlateResourceHandle();
    T->Filter = Texture != nullptr && Texture->filterMode == Moteur::FilterMode::Point ? TF_Nearest : TF_Bilinear;
    T->SRGB = true;
    T->AddressX = TA_Clamp;
    T->AddressY = TA_Clamp;
    T->LODGroup = TEXTUREGROUP_UI;
    T->NeverStream = true;
    // les lignes dans l'ordre d'Unity (la première en bas) : les coordonnées de texture du dessin restent celles d'Unity
    FTexture2DMipMap& Niveau = T->GetPlatformData()->Mips[0];
    uint8* Octets = static_cast<uint8*>(Niveau.BulkData.Lock(LOCK_READ_WRITE));
    const bool AvecPixels = Texture != nullptr && Texture->Pixels().Count() == W * H;
    for (int32 i = 0; i < W * H; i++)
    {
        const Moteur::Color32 c = AvecPixels ? Texture->Pixels()[i] : Moteur::Color32(255, 255, 255, 255);
        Octets[i * 4 + 0] = c.b;
        Octets[i * 4 + 1] = c.g;
        Octets[i * 4 + 2] = c.r;
        Octets[i * 4 + 3] = c.a;
    }
    Niveau.BulkData.Unlock();
    T->UpdateResource();
    TexturesInterface.Add(Id, T);

    Etat.Pinceau = MakeShared<FSlateBrush>();
    Etat.Pinceau->SetResourceObject(T);
    Etat.Pinceau->ImageSize = FVector2D(static_cast<double>(W), static_cast<double>(H));
    Etat.Pinceau->DrawAs = ESlateBrushDrawType::Image;
    Etat.Pinceau->Tiling = ESlateBrushTileType::NoTile;
    FSlateRenderer* Rendu = FSlateApplication::IsInitialized() ? FSlateApplication::Get().GetRenderer() : nullptr;
    Etat.Poignee = Rendu != nullptr ? Rendu->GetResourceHandle(*Etat.Pinceau) : FSlateResourceHandle();
    Etat.Version = Version;
    return Etat.Poignee;
}

void ADDScene::ReproduireInterface()
{
    if (!Widget.IsValid()) return;
    Moteur::Liste<Moteur::Dessin::Commande> Commandes;
    Moteur::Dessin::Interface(Moteur::Screen::width, Moteur::Screen::height, Mesureur, Commandes);

    // le dessin compte depuis le bas de l'écran, Slate depuis le haut
    const float Hauteur = static_cast<float>(Moteur::Screen::height);
    TSharedPtr<FDDDessinInterface> Image = MakeShared<FDDDessinInterface>();
    Image->Commandes.Reserve(Commandes.Count());
    for (int32 i = 0; i < Commandes.Count(); i++)
    {
        const Moteur::Dessin::Commande& C = Commandes[i];
        FDDDessinInterface::FCommande& D = Image->Commandes.AddDefaulted_GetRef();
        if (C.sorte == Moteur::Dessin::Commande::Sorte::Texte)
        {
            D.EstTexte = true;
            D.Textes.Reserve(C.morceaux.Count());
            for (int32 k = 0; k < C.morceaux.Count(); k++)
            {
                const Moteur::Dessin::Morceau& M = C.morceaux[k];
                FDDDessinInterface::FTexte& T = D.Textes.AddDefaulted_GetRef();
                T.Texte = DDUnreal::Texte(M.texte);
                T.Position = FVector2f(M.position.x, Hauteur - M.position.y);
                T.TaillePx = M.taillePx;
                T.Gras = M.gras;
                T.Italique = M.italique;
                T.Couleur = DDUnreal::CouleurInterface(M.couleur);
            }
            continue;
        }
        D.Texture = TexturePour(C.texture.get());
        D.Sommets.Reserve(C.sommets.Count());
        for (int32 k = 0; k < C.sommets.Count(); k++)
        {
            const Moteur::Dessin::Sommet& S = C.sommets[k];
            FDDDessinInterface::FSommet& V = D.Sommets.AddDefaulted_GetRef();
            V.Position = FVector2f(S.position.x, Hauteur - S.position.y);
            V.UV = FVector2f(S.uv.x, S.uv.y);
            V.Couleur = DDUnreal::Octets(S.couleur);
        }
    }
    Widget->Montrer(Image);

    // les textures oubliées par le jeu
    for (auto It = EtatsTextures.CreateIterator(); It; ++It)
    {
        if (It.Key() == 0 || ImageUnreal - It.Value().Vue < ImagesAvantOubli) continue;
        TexturesInterface.Remove(It.Key());
        It.RemoveCurrent();
    }
}
