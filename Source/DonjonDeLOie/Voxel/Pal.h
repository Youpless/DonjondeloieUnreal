// Donjon de l'Oie — couleurs : lecture hexadécimale, mélanges, nuances, et la palette commune du jeu.
// Les couleurs de la palette sont calculées à la compilation (constexpr) : elles sont prêtes avant toute autre donnée.
#pragma once

#include "../Moteur/Maths.h"
#include "../Noyau/Jeu.h"

namespace DonjonDeLOie::Voxel
{
    namespace DetailPal
    {
        constexpr int Chiffre(char c)
        {
            return (c >= '0' && c <= '9') ? c - '0' : (c >= 'a' && c <= 'f') ? c - 'a' + 10 : (c >= 'A' && c <= 'F') ? c - 'A' + 10 : 0;
        }
        constexpr uint8_t Octet(const char* h, int i) { return static_cast<uint8_t>(Chiffre(h[i]) * 16 + Chiffre(h[i + 1])); }
        constexpr Color32 Hex(const char* hex)
        {
            int d = hex[0] == '#' ? 1 : 0;
            return Color32(Octet(hex, d), Octet(hex, d + 2), Octet(hex, d + 4), 255);
        }
    }

    struct Pal
    {
        /// "#a1b2c3" ou "a1b2c3" (à la compilation pour un texte écrit dans le code).
        static constexpr Color32 C(const char* hex) { return DetailPal::Hex(hex); }
        static Color32 C(const string& hex) { return hex.size() >= 6 ? DetailPal::Hex(hex.c_str()) : Color32(0, 0, 0, 255); }
        static Color32 Mix(Color32 a, Color32 b, float t);
        /// f < 1 assombrit, f > 1 éclaircit.
        static Color32 Shade(Color32 c, float f);
        /// Petite variation déterministe d'une couleur selon une position (texture de pierre, d'écorce…).
        static Color32 Var(Color32 c, int x, int y, int z, float ampleur) { return Shade(c, 1.0f + (Hash01(x, y, z) - 0.5f) * 2.0f * ampleur); }
        static float Hash01(int x, int y, int z);

        // Une palette commune, chaude et un peu désaturée, dans l'esprit des cartes de Battlegrade.
        static constexpr Color32 Peau = DetailPal::Hex("e0ac86"), PeauOmbre = DetailPal::Hex("b9805f"), PeauClaire = DetailPal::Hex("f0c7a2");
        static constexpr Color32 Cheveux = DetailPal::Hex("5a3a22"), Barbe = DetailPal::Hex("8a5a32");
        static constexpr Color32 Fer = DetailPal::Hex("9aa3ab"), FerSombre = DetailPal::Hex("5f676f"), Acier = DetailPal::Hex("c4ccd3");
        static constexpr Color32 Or = DetailPal::Hex("e8b64a"), OrSombre = DetailPal::Hex("a77a23");
        static constexpr Color32 Cuir = DetailPal::Hex("7a4e2d"), CuirSombre = DetailPal::Hex("543320");
        static constexpr Color32 Bois = DetailPal::Hex("8b5e3c"), BoisSombre = DetailPal::Hex("5e3d26");
        static constexpr Color32 Pierre = DetailPal::Hex("8d8a84"), PierreSombre = DetailPal::Hex("5f5d59"), PierreClaire = DetailPal::Hex("b3afa6");
        static constexpr Color32 Herbe = DetailPal::Hex("6f9a43"), HerbeSombre = DetailPal::Hex("4c7330"), HerbeClaire = DetailPal::Hex("8fb555");
        static constexpr Color32 Terre = DetailPal::Hex("8a6a48"), Sable = DetailPal::Hex("d6c08a");
        static constexpr Color32 Eau = DetailPal::Hex("3f7fa6"), EauProfonde = DetailPal::Hex("2a587a");
        static constexpr Color32 Os = DetailPal::Hex("e6dcc3"), Noir = DetailPal::Hex("1c1a1e"), Blanc = DetailPal::Hex("f4f1ea");
        static constexpr Color32 Rouge = DetailPal::Hex("b8392f"), Bleu = DetailPal::Hex("3d64b0"), Vert = DetailPal::Hex("3f8a46"), Violet = DetailPal::Hex("6e4aa0");

        /// Couleurs des six joueurs (blasons, capes, socles) : rouge, bleu, vert, jaune, violet, orange.
        static constexpr Color32 Joueurs[6] = {DetailPal::Hex("d9483b"), DetailPal::Hex("3f78d1"), DetailPal::Hex("49a84f"), DetailPal::Hex("e8b335"), DetailPal::Hex("9a5bd1"), DetailPal::Hex("e07b2e")};
        static constexpr int NombreJoueurs = 6;
        static constexpr const char* NomsCouleurs[6] = {"Rouge", "Bleu", "Vert", "Jaune", "Violet", "Orange"};
    };
}
