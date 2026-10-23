// Donjon de l'Oie — formes simples pour les effets et les marqueurs : anneau, colonne de lumière, disque, icône en voxels.
#pragma once

#include "../Moteur/Moteur.h"

namespace DonjonDeLOie::Monde
{
    struct Formes
    {
        /// Anneau à plat (dans le plan xz), en couleur de sommets.
        static Ref<GameObject> Anneau(const Ref<Transform>& parent, float r0, float r1, Color c, const Ref<Material>& mat);

        /// Disque plein (ombre portée sous un pion, socle).
        static Ref<GameObject> Disque(const Ref<Transform>& parent, float r, Color centre, Color bord, const Ref<Material>& mat);

        /// Colonne de lumière qui s'efface vers le haut (additive).
        static Ref<GameObject> Colonne(const Ref<Transform>& parent, float r, float h, Color c);

        /// Une icône en pixel art, extrudée en voxels (taille : mètres par pixel). debout : face vers −z ; sinon à plat.
        static Ref<GameObject> Icone(const Ref<Transform>& parent, const string& id, float taille, bool aPlat);

    private:
        static inline Dico<string, Ref<Mesh>> cache;

        static Ref<GameObject> Objet(const string& nom, const Ref<Transform>& parent, const Ref<Mesh>& m, const Ref<Material>& mat);

        /// Remise des variables statiques à leur valeur de départ (Boucle::Reinitialiser) : inscrite dans le .cpp.
        static const bool remiseAZero;
    };
}
