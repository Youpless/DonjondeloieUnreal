// Donjon de l'Oie — transforme un VoxelModel en maillage : seules les faces visibles sont gardées, avec une occlusion
// ambiante par sommet (les creux s'assombrissent) et un léger grain de couleur par voxel.
#pragma once

#include "VoxelModel.h"

namespace DonjonDeLOie::Voxel
{
    /// Un maillage en construction : sommets, normales, couleurs (occlusion comprise), triangles.
    class MeshData
    {
    public:
        Liste<Vector3> V;
        Liste<Vector3> N;
        Liste<Color32> C;
        Liste<int> T;

        int Triangles() const { return T.Count() / 3; }

        /// Ajoute un autre maillage, tourné autour de y (degrés), mis à l'échelle et déplacé.
        void Append(const MeshData& o, Vector3 decalage, float rotY = 0.0f, float echelle = 1.0f);
        void Append(const MeshData& o, const Matrix4x4& m);
        void Teinter(const std::function<Color32(Color32)>& f);
    };

    struct VoxelMesher
    {
        /// Construit le maillage. pivot : point du repère voxel qui devient l'origine du maillage. taille : mètres par voxel.
        static Ptr<MeshData> Build(const VoxelModel& m, Vector3 pivot, float taille, float grain = 0.035f, bool occlusion = true);

        /// Comme Build, mais une fonction peut dire qu'une face donnant sur (x, y, z) est cachée (voxels d'un voisin,
        /// terrain).
        static void AppendTo(MeshData& md, const VoxelModel& m, Vector3 pivot, float taille, float grain, bool occlusion,
                             const std::function<bool(int, int, int)>& pleinDehors);
    };
}
