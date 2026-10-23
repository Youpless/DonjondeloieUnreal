// Donjon de l'Oie — les matériaux Unreal qui rendent les shaders du jeu (DonjonDeLOie/Voxel, /Lueur, /Fx) et la brume.
// Le module d'édition (DonjonDeLOieEditor) les crée dans /Game/DonjonDeLOie/Materiaux à la première ouverture du
// projet ; le jeu les charge par leur chemin. Les noms de paramètres sont ceux des propriétés des shaders d'Unity.
#pragma once

#include "CoreMinimal.h"

namespace DDMateriauxUnreal
{
    /// Augmente quand le graphe d'un matériau change : le module d'édition les recrée alors.
    inline constexpr int32 Version = 1;

    inline constexpr const TCHAR* Dossier = TEXT("/Game/DonjonDeLOie/Materiaux");

    /// Les genres de rendu du jeu (voir DDScene.Maillages.cpp : Genre()).
    enum class EGenre : uint8
    {
        Voxel,      // DonjonDeLOie/Voxel : éclairé (soleil, lampes, ombres), couleur des sommets, lumière ambiante en trois tons
        Lueur,      // DonjonDeLOie/Lueur : sans éclairage, couleur × intensité (le halo la fait rayonner)
        FxAdd,      // DonjonDeLOie/Fx, additif
        FxAlpha,    // DonjonDeLOie/Fx, transparence
        FxDessus,   // DonjonDeLOie/Fx, additif, toujours visible (ZTest Always)
        Eau,        // DonjonDeLOie/Fx, transparence, sommets qui ondulent (_Vague)
        Nombre
    };

    inline const TCHAR* Nom(EGenre g)
    {
        switch (g)
        {
            case EGenre::Voxel: return TEXT("M_DD_Voxel");
            case EGenre::Lueur: return TEXT("M_DD_Lueur");
            case EGenre::FxAdd: return TEXT("M_DD_FxAdd");
            case EGenre::FxAlpha: return TEXT("M_DD_FxAlpha");
            case EGenre::FxDessus: return TEXT("M_DD_FxDessus");
            case EGenre::Eau: return TEXT("M_DD_Eau");
            default: return TEXT("M_DD_Voxel");
        }
    }

    /// Le post-traitement de brume linéaire (RenderSettings.fog d'Unity) et de couleur de fond.
    inline constexpr const TCHAR* NomBrouillard = TEXT("M_DD_Brouillard");

    /// « /Game/DonjonDeLOie/Materiaux/M_DD_Voxel »
    inline FString Paquet(const TCHAR* NomAsset) { return FString(Dossier) / NomAsset; }

    /// « /Game/DonjonDeLOie/Materiaux/M_DD_Voxel.M_DD_Voxel »
    inline FString Chemin(const TCHAR* NomAsset) { return Paquet(NomAsset) + TEXT(".") + NomAsset; }

    // Les paramètres (mêmes noms que les propriétés des shaders d'Unity, plus ceux de l'ambiance et de la brume).
    inline constexpr const TCHAR* Teinte = TEXT("_BaseColor");
    inline constexpr const TCHAR* Emission = TEXT("_EmissionColor");
    inline constexpr const TCHAR* Intensite = TEXT("_Intensite");
    inline constexpr const TCHAR* Pulsation = TEXT("_Pulsation");
    inline constexpr const TCHAR* Vague = TEXT("_Vague");
    inline constexpr const TCHAR* AmbiantCiel = TEXT("DD_AmbiantCiel");
    inline constexpr const TCHAR* AmbiantEquateur = TEXT("DD_AmbiantEquateur");
    inline constexpr const TCHAR* AmbiantSol = TEXT("DD_AmbiantSol");
    inline constexpr const TCHAR* BrumeCouleur = TEXT("DD_Couleur");
    inline constexpr const TCHAR* BrumeFond = TEXT("DD_Fond");
    inline constexpr const TCHAR* BrumeDebut = TEXT("DD_Debut");
    inline constexpr const TCHAR* BrumeFin = TEXT("DD_Fin");
    inline constexpr const TCHAR* BrumeActive = TEXT("DD_Actif");
}
