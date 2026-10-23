// Donjon de l'Oie — passage du moteur du jeu (repère et conventions d'Unity) à Unreal.
//
// Repères : le jeu compte comme Unity (x à droite, y en haut, z devant, en mètres) ; Unreal compte X devant, Y à
// droite, Z en haut, en centimètres. Les deux repères tournent dans le même sens (main gauche), et le passage de l'un à
// l'autre est une simple permutation des axes (z, x, y) : une rotation, sans miroir. Une rotation (quaternion) se
// convertit donc en permutant ses trois composantes de la même façon.
//
// Couleurs : comme dans Unity (projet en espace linéaire), les couleurs du jeu sont en sRGB ; les lampes, les
// matériaux et la brume les veulent linéaires ; l'interface (Slate) les veut en sRGB.
#pragma once

#include "CoreMinimal.h"

#include "Moteur/Moteur.h"

#include <string>

namespace DDUnreal
{
    inline FVector Position(const Moteur::Vector3& v) { return FVector(v.z * 100.0, v.x * 100.0, v.y * 100.0); }
    inline FVector Direction(const Moteur::Vector3& v) { return FVector(v.z, v.x, v.y); }
    inline FVector Echelle(const Moteur::Vector3& s) { return FVector(s.z, s.x, s.y); }
    inline FQuat Rotation(const Moteur::Quaternion& q) { return FQuat(q.z, q.x, q.y, q.w); }

    /// sRGB → linéaire, la formule exacte (celle de Mathf.GammaToLinearSpace d'Unity), aussi au-delà de 1 (couleurs HDR).
    inline float Lineaire(float c)
    {
        if (c <= 0.04045f) return c / 12.92f;
        return FMath::Pow((c + 0.055f) / 1.055f, 2.4f);
    }

    inline FLinearColor CouleurLineaire(const Moteur::Color& c) { return FLinearColor(Lineaire(c.r), Lineaire(c.g), Lineaire(c.b), c.a); }

    inline uint8 Octet(float v) { return static_cast<uint8>(FMath::Clamp(FMath::RoundToInt(v * 255.0f), 0, 255)); }

    /// Les octets sRGB d'une couleur du jeu (sommets de l'interface).
    inline FColor Octets(const Moteur::Color& c) { return FColor(Octet(c.r), Octet(c.g), Octet(c.b), Octet(c.a)); }

    /// Une couleur de l'interface pour Slate (qui reconvertit en sRGB) : la même couleur à l'écran que dans Unity.
    inline FLinearColor CouleurInterface(const Moteur::Color& c) { return FLinearColor(Octets(c)); }

    inline FString Texte(const std::string& s) { return FString(UTF8_TO_TCHAR(s.c_str())); }

    inline std::string Utf8(const FString& s) { return std::string(TCHAR_TO_UTF8(*s)); }

    /// Les cartes comptent en mètres sur le plan horizontal (x et z d'Unity) ; un acteur est à X = z × 100, Y = x × 100.
    inline float XCarte(const FVector& P) { return static_cast<float>(P.Y / 100.0); }
    inline float ZCarte(const FVector& P) { return static_cast<float>(P.X / 100.0); }
    inline FVector PositionUnreal(float X, float Z, double Hauteur = 0.0) { return FVector(Z * 100.0, X * 100.0, Hauteur); }
}
