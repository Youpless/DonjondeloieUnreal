// Donjon de l'Oie — point d'entrée de tous les modèles voxel du jeu. Chaque modèle est un VoxelRig : des pièces
// modelées dans un même repère, avec pivots, que la présentation anime.
#pragma once

#include "DecorsModeles.h"
#include "HerosModeles.h"
#include "IconesCases.h"
#include "MonstresModeles.h"

namespace DonjonDeLOie::Modeles
{
    namespace Catalogue
    {
        /// Héros d'une classe ; « couleur » est celle du joueur (cape, tabard, plumet…).
        inline Ptr<VoxelRig> Heros(Regles::ClasseHeros c, Color32 couleur) { return HerosModeles::Construire(c, couleur); }
        /// Monstre par son identifiant (voir Donnees::Monstres).
        inline Ptr<VoxelRig> Monstre(const string& id) { return MonstresModeles::Construire(id); }
        /// Élément de décor (arbre, rocher, maison…). « graine » fait varier la forme d'un exemplaire à l'autre.
        inline Ptr<VoxelRig> Decor(const string& id, int graine) { return DecorsModeles::Construire(id, graine); }
        /// Petite figurine posée au bord d'une case pour en montrer le type.
        inline Ptr<VoxelRig> IconeCase(Regles::TypeCase t) { return IconesCases::Construire(t); }
        /// Identifiants des décors disponibles pour une zone.
        inline Liste<string> DecorsDeZone(Regles::Zone z) { return DecorsModeles::DeZone(z); }
    }
}
