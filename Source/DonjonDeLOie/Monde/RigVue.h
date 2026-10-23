// Donjon de l'Oie — un VoxelRig posé dans la scène : une hiérarchie d'objets, une pièce par objet, pivots respectés.
#pragma once

#include "../Moteur/Moteur.h"
#include "../Voxel/VoxelModel.h"

namespace DonjonDeLOie::Monde
{
    class RigVue : public MonoBehaviour
    {
        DD_TYPE_OBJET(RigVue, MonoBehaviour)
    public:
        Ptr<Voxel::VoxelRig> Rig;
        Dico<string, Ref<Transform>> Pieces;
        Dico<string, Quaternion> Repos;
        Dico<string, Vector3> PositionsRepos;
        Liste<Ref<Renderer>> Rendus;
        float Hauteur = 0.0f;          // hauteur du modèle en mètres (pour poser les bulles au-dessus)

        /// Construit la hiérarchie. cle : préfixe du cache de maillages (vide : pas de cache).
        static Ref<RigVue> Creer(const Ptr<Voxel::VoxelRig>& rig, const Ref<Transform>& parent, const string& cle, bool ombres = true, float intensiteLueur = 2.4f);

        Ref<Transform> Piece(const string& nom) const;

        void Teinte(Color c);
        void Emission(Color c);

        /// Éclair de couleur (coup reçu, soin, niveau) qui s'éteint en « duree » secondes.
        void Eclair(Color c, float duree);

        void Visible(bool v);

    private:
        Ptr<MaterialPropertyBlock> bloc;
        Color teinte = Color::white, emission = Color::black;
        Ptr<Coroutine> eclairCo;

        void Appliquer();
        Tache EclairCo(Color c, float duree);
    };
}
