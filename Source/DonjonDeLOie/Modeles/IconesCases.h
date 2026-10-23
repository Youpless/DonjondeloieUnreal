// Donjon de l'Oie — les petites figurines posées au bord des cases pour en montrer le type.
#pragma once

#include "../Regles/Donnees.h"
#include "../Voxel/VoxelModel.h"

namespace DonjonDeLOie::Modeles
{
    namespace IconesCases
    {
        Ptr<Voxel::VoxelRig> Construire(Regles::TypeCase t);
    }
}
