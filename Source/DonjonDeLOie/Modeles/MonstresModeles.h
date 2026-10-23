// Donjon de l'Oie — les douze monstres (onze, plus le Dragonnoie). Chacun est construit dans son fichier
// (Monstres/MonstresModeles.<Nom>.cpp), avec les outils de sculpture communs (Monstres/MonstresInterne.h).
#pragma once

#include "../Voxel/VoxelModel.h"

namespace DonjonDeLOie::Modeles
{
    namespace MonstresModeles
    {
        /// Un monstre par son identifiant (rat, gobelin, loup, araignee, gelee, squelette, orc, spectre, troll, wyverne,
        /// dragon, dragonnoie) ; un bloc rouge pour un identifiant inconnu.
        Ptr<Voxel::VoxelRig> Construire(const string& id);
    }
}
