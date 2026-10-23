// Donjon de l'Oie — les six héros et les habitants de Valcreux, en voxels sculptés (style Battlegrade).
// Repère commun à toutes les pièces : 1 voxel = 5 cm, origine au sol entre les pieds, le héros regarde vers +z, sa
// droite est +x. Chaque classe est construite dans son propre fichier (Heros/HerosModeles.<Classe>.cpp).
#pragma once

#include "../Regles/Donnees.h"
#include "../Voxel/VoxelModel.h"

namespace DonjonDeLOie::Modeles
{
    using Voxel::VoxelModel;
    using Voxel::VoxelPart;
    using Voxel::VoxelRig;

    /// Ce qui distingue un habitant d'un autre. Les champs du visage sont ceux que montre l'avis de recherche.
    struct Allure
    {
        int Peau = 0, Cheveux = 0, Coiffure = 0, Barbe = 0, Couvre = 0, CouleurCouvre = 0, Oeil = 0;   // le visage
        bool Cicatrice = false;
        int Tunique = 0, Pantalon = 0, Objet = 0;                                                     // le reste (souvent caché)
        bool Tablier = false;

        static constexpr int NPeaux = 4, NCheveux = 6, NCoiffures = 5, NBarbes = 4, NCouvres = 6, NCouleurs = 6;

        Ptr<Allure> Copie() const { return Nouveau<Allure>(*this); }
        bool MemeVisage(const Allure& o) const;
        static Ptr<Allure> Hasard(AleaNet& r);
        /// Un sosie : le même visage à un ou deux détails près (le piège pour l'œil trop pressé).
        Ptr<Allure> Sosie(AleaNet& r) const;
    };

    namespace HerosModeles
    {
        Ptr<VoxelRig> Construire(Regles::ClasseHeros c, Color32 couleur);
        /// Un habitant de Valcreux, d'après son allure.
        Ptr<VoxelRig> Habitant(const Allure& al);
        /// L'allure d'un évadé : tirée de sa graine, et fidèle à son surnom (le Borgne, la Rousse, le Balafré…).
        Ptr<Allure> AllureEvade(int graine, const string& nom, bool feminin);
        /// L'icône du portrait d'un évadé (calculée une fois, puis gardée parmi les icônes dynamiques).
        string IconePortrait(int graine, const string& nom, bool feminin);
        /// La tête vue de face, en 11 × 11 pixels : l'avis de recherche. Ligne 0 en bas.
        Liste<Color32> Portrait(const VoxelRig& rig);
    }
}
