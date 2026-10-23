// Donjon de l'Oie — décors : la nature (Nature/*.cpp) et les bâtiments (Batiments/*.cpp).
#pragma once

#include "../Regles/Donnees.h"
#include "../Voxel/VoxelModel.h"

namespace DonjonDeLOie::Modeles
{
    namespace DecorsModeles
    {
        /// Un élément de décor (arbre, rocher, maison…) ; un petit bloc rouge pour un identifiant inconnu.
        Ptr<Voxel::VoxelRig> Construire(const string& id, int graine);

        // la nature (Nature/)
        /// Identifiants des décors de nature d'une zone.
        Liste<string> DeZone(Regles::Zone z);
        /// Un élément de nature, ou nullptr si l'id n'en est pas un.
        Ptr<Voxel::VoxelRig> ConstruireNature(const string& id, int graine);

        // les bâtiments (Batiments/)
        /// Hauteur des cachettes (caisses, tonneaux) : un homme debout derrière ne montre que la tête.
        inline constexpr float HauteurCachette = 0.9f;
        /// Les bâtiments et objets « spéciaux » (posés à part, jamais semés au hasard).
        const Liste<string>& Speciaux();
        /// Un bâtiment, ou nullptr si l'id n'en est pas un.
        Ptr<Voxel::VoxelRig> ConstruireBatiment(const string& id, int graine);
        Ptr<Voxel::VoxelRig> Maison(int graine, int demiXMax, bool abords);
        /// Le plateau du marché du mini-jeu de l'évadé (pavés, caniveau, bordure).
        Ptr<Voxel::VoxelRig> PlateauMarche(int largeur, int profondeur, int trottoir);
        /// Le panneau d'affichage du marché, avec l'avis de recherche (le portrait 11 × 11).
        Ptr<Voxel::VoxelRig> PanneauAvis(const Liste<Color32>& portrait);
    }
}
