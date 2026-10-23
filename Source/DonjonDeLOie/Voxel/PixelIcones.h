// Donjon de l'Oie — petites icônes en pixel art (11 × 11), dessinées en texte.
// Servent à l'interface (textures) et aux motifs gravés sur les dalles des cases (voxels).
// Légende : '.' vide, 'k' contour sombre, 'w' blanc, 'r' rouge, 'R' rouge sombre, 'y' or, 'Y' or sombre, 'g' gris clair,
// 'G' gris, 'b' bleu, 'B' bleu sombre, 'v' vert, 'V' vert sombre, 'p' violet, 'P' violet sombre, 'o' orange, 'n' brun,
// 'N' brun sombre, 's' peau.
#pragma once

#include "Pal.h"

namespace DonjonDeLOie::Voxel
{
    struct PixelIcones
    {
        static constexpr int Taille = 11;
        /// Les dessins : 11 lignes de 11 caractères, la première ligne en haut.
        static const Dico<string, Liste<string>>& Dessins();
        /// Couleur d'un caractère de dessin (alpha 0 : transparent).
        static Color32 Couleur(char c);
        /// Icônes calculées en jeu (le portrait d'un évadé, par exemple) : Taille × Taille pixels, ligne 0 en bas.
        static Dico<string, Liste<Color32>>& Dynamiques();
        /// Pixel (x, y) avec y = 0 en bas ; transparent : alpha 0.
        static Color32 Pixel(const string& id, int x, int y);
    };
}
