// Donjon de l'Oie — les outils partagés par les fichiers des héros (Modeles/Heros/) : l'atelier, les outils de
// sculpture, le gabarit commun (tête, jambes, bras) et les six classes. En C#, ce sont les membres privés de la classe
// partielle HerosModeles ; ils sont définis dans HerosModeles.cpp (les classes, chacune dans son fichier).
// Interne à la famille : rien en dehors de Modeles/Heros/ ne l'inclut.
#pragma once

#include "../../Voxel/Pal.h"
#include "../HerosModeles.h"

namespace DonjonDeLOie::Modeles::HerosModeles
{
    // ------------------------------------------------------------------ atelier : les pièces d'un héros

    /// Les pièces d'un héros en cours de construction, avec leurs pivots (valeurs du gabarit commun par défaut).
    struct Atelier
    {
        const Ptr<VoxelModel> Corps = Nouveau<VoxelModel>(), Tete = Nouveau<VoxelModel>();
        const Ptr<VoxelModel> BrasG = Nouveau<VoxelModel>(), BrasD = Nouveau<VoxelModel>();
        const Ptr<VoxelModel> JambeG = Nouveau<VoxelModel>(), JambeD = Nouveau<VoxelModel>();
        const Ptr<VoxelModel> Arme = Nouveau<VoxelModel>(), Bouclier = Nouveau<VoxelModel>(), Cape = Nouveau<VoxelModel>();
        const Ptr<VoxelModel> Lueur = Nouveau<VoxelModel>();
        string NomLueur = "lueur_arme";
        Vector3 PCorps = Vector3(0, 8, 0.5f), PTete = Vector3(0, 16, 0.5f);
        Vector3 PBrasD = Vector3(6.5f, 14.5f, 0.5f), PBrasG = Vector3(-6.5f, 14.5f, 0.5f);
        Vector3 PJambeD = Vector3(3.0f, 8, 0.5f), PJambeG = Vector3(-3.0f, 8, 0.5f);
        Vector3 PArme = Vector3(6.5f, 5.5f, 0.5f), PBouclier = Vector3(-6.5f, 5.5f, 0.5f);
        Vector3 PCape = Vector3(0, 15.5f, -3.5f);
    };

    Ptr<VoxelRig> Assembler(const string& nom, const Atelier& a);

    // ------------------------------------------------------------------ outils de sculpture

    inline constexpr int AretesV = 1, AretesHaut = 2, AretesBas = 4, AretesAvant = 8;

    /// Boîte taillée : la boîte pleine, puis les arêtes choisies retirées (chanfrein d'un voxel).
    void Taille(VoxelModel& m, int x0, int y0, int z0, int x1, int y1, int z1, Color32 c, int aretes);
    /// Repeint les voxels existants d'une boîte.
    void Peint(VoxelModel& m, int x0, int y0, int z0, int x1, int y1, int z1, Color32 c);
    /// Retire les voxels d'une boîte.
    void Vide(VoxelModel& m, int x0, int y0, int z0, int x1, int y1, int z1);
    /// Copie tournée d'un modèle autour d'un pivot (échantillonnage inverse : pas de trous).
    Ptr<VoxelModel> Pivoter(const VoxelModel& src, Vector3 pivot, Quaternion q);
    /// Copie miroir d'un modèle (x ↔ −1 − x) : la pièce droite devient la gauche.
    void Miroir(const VoxelModel& src, VoxelModel& dst);
    /// Symétrise un modèle construit sur sa moitié droite (x ≥ 0).
    void Symetrise(VoxelModel& m);

    // ------------------------------------------------------------------ gabarit commun : tête, jambes, bras

    /// Tête nue taillée : x −4..3, y 16..22, z −3..3 ; visage sur le plan z = 3, yeux en y = 19.
    void TeteNue(VoxelModel& t, Color32 peau, Color32 sourcils);
    /// Jambe droite (x 1..4).
    void Jambe(VoxelModel& j, Color32 chausse, Color32 botte, int hautBotte, Color32 semelle);
    /// Bras droit (x 5..7).
    void Bras(VoxelModel& b, Color32 manche, Color32 main);
    /// Capuche taillée, ouverte devant sur le visage (à appeler avant de dessiner la tête).
    void Capuche(VoxelModel& t, Color32 c, Color32 cS, int pointe);
    Color32 Sh(Color32 c, float f);
    /// Visage commun : yeux (2 voxels sombres), nez, sourcils. z = plan du visage.
    void Visage(VoxelModel& t, int yYeux, int z, Color32 sourcils, bool nez);

    // ------------------------------------------------------------------ les six classes (HerosModeles.<Classe>.cpp)

    Ptr<VoxelRig> Guerrier(Color32 coul);
    Ptr<VoxelRig> Mage(Color32 coul);
    Ptr<VoxelRig> Voleur(Color32 coul);
    Ptr<VoxelRig> Clerc(Color32 coul);
    Ptr<VoxelRig> Rodeur(Color32 coul);
    Ptr<VoxelRig> Barbare(Color32 coul);
}
