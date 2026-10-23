// Donjon de l'Oie — décors : aiguillage vers la nature (Nature/) et les bâtiments (Batiments/).
#include "DecorsModeles.h"

#include "../Voxel/Pal.h"

namespace DonjonDeLOie::Modeles
{
    using namespace Voxel;

    Ptr<VoxelRig> DecorsModeles::Construire(const string& id, int graine)
    {
        auto r = ConstruireNature(id, graine);
        if (r == nullptr) r = ConstruireBatiment(id, graine);
        if (r == nullptr)
        {
            auto m = Nouveau<VoxelModel>();
            m->Box(-2, 0, -2, 1, 3, 1, Pal::Rouge);
            r = VoxelRig::Single(id, m, 0.1f, Vector3::zero);
        }
        return r;
    }
}
