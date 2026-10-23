// Donjon de l'Oie — la scène : l'acteur qui fait tourner le jeu dans Unreal.
//
// Le jeu (Pilote/DDJeu et tout ce qu'il construit) tourne sur le petit moteur du dossier Moteur/, qui reproduit ce que
// le jeu attendait d'Unity (objets, composants, coroutines, interface uGUI). À chaque image, la scène :
//  1. donne au moteur la souris, le clavier et la taille de la fenêtre ;
//  2. fait avancer le jeu d'une image (Boucle::Image : Update, coroutines, LateUpdate) ;
//  3. reproduit dans Unreal ce que le jeu montre : les maillages (composants de maillage procédural), les lampes, la
//     caméra, la brume et le post-traitement, les sons, et l'interface (widget Slate).
// À la fin de la partie (arrêt du jeu dans l'éditeur), tout est détruit et les variables statiques du jeu remises à
// zéro, pour que la partie suivante reparte de rien.
//
// Les définitions sont réparties en cinq fichiers : DDScene.cpp (démarrage, image, entrées, caméra),
// DDScene.Maillages.cpp, DDScene.Lumieres.cpp, DDScene.Sons.cpp, DDScene.Interface.cpp.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Moteur/Moteur.h"
#include "Rendering/SlateResourceHandle.h"
#include "SDDInterface.h"

#include "DDMateriauxUnreal.h"

#include "DDScene.generated.h"

class UAudioComponent;
class UCameraComponent;
class UDirectionalLightComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UPointLightComponent;
class UProceduralMeshComponent;
class USoundWaveProcedural;
class UTexture2D;
struct FSlateBrush;

/// Ce qu'un composant de maillage montre déjà (pour ne recopier que ce qui change).
struct FDDEtatMaillage
{
    int32 Maillage = -1;
    int32 VersionMaillage = -1;
    int32 Materiau = -1;
    int32 VersionMateriau = -1;
    int32 VersionBloc = -1;
    bool AvecBloc = false;
    Moteur::Matrix4x4 Matrice;
    bool MatriceConnue = false;
    bool Visible = false;
    bool Ombre = false;
    bool SectionVide = true;
    int32 Priorite = 0;
    int32 Vu = 0;
    DDMateriauxUnreal::EGenre Genre = DDMateriauxUnreal::EGenre::Voxel;
};

/// Ce que joue déjà une source de son continue (la musique).
struct FDDSonContinu
{
    int32 Version = -1;
    int32 Clip = -1;
    float Volume = -1.0f;
    int32 OctetsClip = 0;
    bool Boucle = false;
};

/// Une texture de l'interface, prête pour Slate.
struct FDDTextureInterface
{
    int32 Version = -1;
    int32 Vue = 0;
    TSharedPtr<FSlateBrush> Pinceau;
    FSlateResourceHandle Poignee;
};

UCLASS()
class ADDScene : public AActor
{
    GENERATED_BODY()

public:
    ADDScene();

    virtual void Tick(float DeltaSeconds) override;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Raison) override;

private:
    // ================================================================== DDScene.cpp : démarrage, image, entrées, caméra

    void Demarrer();
    void Arreter();
    void LireEntrees();
    void ReproduireCamera();
    /// Le contrôleur du joueur regarde par la caméra de la scène.
    void PrendreLaVue();

    UPROPERTY()
    TObjectPtr<USceneComponent> Racine;

    UPROPERTY()
    TObjectPtr<UCameraComponent> Camera;

    bool Lance = false;
    double Temps = 0.0;
    int32 ImageUnreal = 0;
    int32 ImagesParSeconde = -1;

    // ================================================================== DDScene.Maillages.cpp : maillages et matériaux

    void ChargerMateriaux();
    void ReproduireMaillages();
    void CopierMaillage(UProceduralMeshComponent* Composant, const Moteur::Mesh& Maillage, FDDEtatMaillage& Etat);
    UMaterialInterface* MateriauPour(int32 IdRendu, const Moteur::Renderer& Rendu, const Moteur::Material& Materiau, DDMateriauxUnreal::EGenre GenreRendu);
    void ReglerMateriau(UMaterialInstanceDynamic* Instance, const Moteur::Material& Materiau, const Moteur::MaterialPropertyBlock* Bloc, DDMateriauxUnreal::EGenre GenreRendu);
    void ReglerAmbiance(UMaterialInstanceDynamic* Instance);
    void LireAmbiance();
    static DDMateriauxUnreal::EGenre Genre(const Moteur::Material& Materiau);

    UPROPERTY()
    TArray<TObjectPtr<UMaterialInterface>> Maitres;          // un par genre (EGenre)

    UPROPERTY()
    TMap<int32, TObjectPtr<UProceduralMeshComponent>> Maillages;    // par identifiant du MeshRenderer

    UPROPERTY()
    TMap<int32, TObjectPtr<UMaterialInstanceDynamic>> MateriauxPartages;   // par identifiant du Material

    UPROPERTY()
    TMap<int32, TObjectPtr<UMaterialInstanceDynamic>> MateriauxPropres;    // par identifiant du MeshRenderer (MaterialPropertyBlock)

    TMap<int32, FDDEtatMaillage> Etats;
    TMap<int32, int32> VersionsMateriauxPartages;
    TMap<int32, FIntVector> VersionsMateriauxPropres;        // (identifiant du matériau, sa version, version du bloc)
    FLinearColor AmbianceCiel = FLinearColor::Black;
    FLinearColor AmbianceEquateur = FLinearColor::Black;
    FLinearColor AmbianceSol = FLinearColor::Black;
    bool AmbianceChangee = true;
    int32 FacesInversees = -1;

    // ================================================================== DDScene.Lumieres.cpp : soleil, lampes, post-traitement

    void ReproduireLumieres();
    void ReproduirePostTraitement();

    UPROPERTY()
    TObjectPtr<UDirectionalLightComponent> Soleil;           // la part ombrée de la lumière du soleil

    UPROPERTY()
    TObjectPtr<UDirectionalLightComponent> SoleilDiffus;     // la part qui traverse les ombres (1 − shadowStrength)

    UPROPERTY()
    TMap<int32, TObjectPtr<UPointLightComponent>> Lampes;    // par identifiant de la Light

    TMap<int32, int32> LampesVues;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> Brume;

    // ================================================================== DDScene.Sons.cpp : sons et musique

    void ReproduireSons();
    const TArray<uint8>& EchantillonsDe(const Moteur::AudioClip& Clip);
    void JouerPonctuel(const Moteur::AudioClip& Clip, float Volume, float Hauteur);
    void ArreterSons();

    TMap<int32, TArray<uint8>> Echantillons;                 // par identifiant de l'AudioClip, en 16 bits

    UPROPERTY()
    TMap<int32, TObjectPtr<UAudioComponent>> Continus;       // par identifiant de l'AudioSource

    UPROPERTY()
    TMap<int32, TObjectPtr<USoundWaveProcedural>> OndesContinues;

    TMap<int32, FDDSonContinu> EtatsContinus;

    UPROPERTY()
    TArray<TObjectPtr<UAudioComponent>> Ponctuels;

    TArray<double> FinsPonctuels;

    // ================================================================== DDScene.Interface.cpp : l'interface (Slate)

    void InstallerInterface();
    void RetirerInterface();
    void ReproduireInterface();
    FSlateResourceHandle TexturePour(const Moteur::Texture2D* Texture);

    TSharedPtr<SDDInterface> Widget;
    FDDMesureurSlate Mesureur;

    UPROPERTY()
    TMap<int32, TObjectPtr<UTexture2D>> TexturesInterface;   // par identifiant de la Texture2D (0 : blanc uni)

    TMap<int32, FDDTextureInterface> EtatsTextures;
};
