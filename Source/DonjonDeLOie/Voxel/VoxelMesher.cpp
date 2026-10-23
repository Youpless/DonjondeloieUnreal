// Donjon de l'Oie — le maillage des voxels.
#include "VoxelMesher.h"

#include "Pal.h"

namespace DonjonDeLOie::Voxel
{
    void MeshData::Append(const MeshData& o, Vector3 decalage, float rotY, float echelle)
    {
        Quaternion q = Quaternion::AngleAxis(rotY, Vector3::up);
        int b = V.Count();
        for (int i = 0; i < o.V.Count(); i++)
        {
            V.Add(q * (o.V[i] * echelle) + decalage);
            N.Add(q * o.N[i]);
            C.Add(o.C[i]);
        }
        for (int i = 0; i < o.T.Count(); i++) T.Add(o.T[i] + b);
    }

    void MeshData::Append(const MeshData& o, const Matrix4x4& m)
    {
        int b = V.Count();
        for (int i = 0; i < o.V.Count(); i++)
        {
            V.Add(m.MultiplyPoint3x4(o.V[i]));
            N.Add(m.MultiplyVector(o.N[i]).normalized());
            C.Add(o.C[i]);
        }
        for (int i = 0; i < o.T.Count(); i++) T.Add(o.T[i] + b);
    }

    void MeshData::Teinter(const std::function<Color32(Color32)>& f)
    {
        for (int i = 0; i < C.Count(); i++) C[i] = f(C[i]);
    }

    namespace
    {
        const float Ao[4] = {0.52f, 0.68f, 0.84f, 1.0f};

        // Pour chaque face : la normale.
        const int Normales[6][3] = {{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}};
        // Axes tangents (u, v) tels que u × v = n, pour que les triangles regardent vers l'extérieur.
        const int AxesU[6][3] = {{0, 1, 0}, {0, 0, 1}, {0, 0, 1}, {1, 0, 0}, {1, 0, 0}, {0, 1, 0}};
        const int AxesV[6][3] = {{0, 0, 1}, {0, 1, 0}, {1, 0, 0}, {0, 0, 1}, {0, 1, 0}, {1, 0, 0}};

        bool Plein(const VoxelModel& m, const std::function<bool(int, int, int)>& dehors, int x, int y, int z)
        {
            return m.Has(x, y, z) || (dehors && dehors(x, y, z));
        }

        void Face(MeshData& md, const VoxelModel& m, const std::function<bool(int, int, int)>& dehors, int x, int y, int z, int f, Color32 col,
                  Vector3 pivot, float taille, bool occlusion)
        {
            const int* n = Normales[f];
            const int* u = AxesU[f];
            const int* v = AxesV[f];
            // Coin de base de la face : sur le côté positif de la normale.
            float bx = static_cast<float>(x + (n[0] > 0 ? 1 : 0)), by = static_cast<float>(y + (n[1] > 0 ? 1 : 0)), bz = static_cast<float>(z + (n[2] > 0 ? 1 : 0));
            int b = md.V.Count();
            int ao[4];
            // coins : (0,0) (1,0) (1,1) (0,1) en (u, v)
            for (int k = 0; k < 4; k++)
            {
                int cu = (k == 1 || k == 2) ? 1 : 0, cv = (k >= 2) ? 1 : 0;
                Vector3 p(bx + static_cast<float>(u[0] * cu + v[0] * cv), by + static_cast<float>(u[1] * cu + v[1] * cv), bz + static_cast<float>(u[2] * cu + v[2] * cv));
                md.V.Add((p - pivot) * taille);
                md.N.Add(Vector3(static_cast<float>(n[0]), static_cast<float>(n[1]), static_cast<float>(n[2])));
                int a = 3;
                if (occlusion)
                {
                    int su = cu == 1 ? 1 : -1, sv = cv == 1 ? 1 : -1;
                    int ox = x + n[0], oy = y + n[1], oz = z + n[2];
                    bool s1 = Plein(m, dehors, ox + u[0] * su, oy + u[1] * su, oz + u[2] * su);
                    bool s2 = Plein(m, dehors, ox + v[0] * sv, oy + v[1] * sv, oz + v[2] * sv);
                    bool cc = Plein(m, dehors, ox + u[0] * su + v[0] * sv, oy + u[1] * su + v[1] * sv, oz + u[2] * su + v[2] * sv);
                    a = (s1 && s2) ? 0 : 3 - ((s1 ? 1 : 0) + (s2 ? 1 : 0) + (cc ? 1 : 0));
                }
                ao[k] = a;
                md.C.Add(Pal::Shade(col, Ao[a]));
            }
            // Diagonale choisie pour que l'occlusion se dégrade sans artefact.
            if (ao[0] + ao[2] >= ao[1] + ao[3])
            {
                md.T.Add(b);
                md.T.Add(b + 1);
                md.T.Add(b + 2);
                md.T.Add(b);
                md.T.Add(b + 2);
                md.T.Add(b + 3);
            }
            else
            {
                md.T.Add(b + 1);
                md.T.Add(b + 2);
                md.T.Add(b + 3);
                md.T.Add(b + 1);
                md.T.Add(b + 3);
                md.T.Add(b);
            }
        }
    }

    Ptr<MeshData> VoxelMesher::Build(const VoxelModel& m, Vector3 pivot, float taille, float grain, bool occlusion)
    {
        auto md = Nouveau<MeshData>();
        AppendTo(*md, m, pivot, taille, grain, occlusion, nullptr);
        return md;
    }

    void VoxelMesher::AppendTo(MeshData& md, const VoxelModel& m, Vector3 pivot, float taille, float grain, bool occlusion,
                               const std::function<bool(int, int, int)>& pleinDehors)
    {
        for (auto& kv : m.Tous())
        {
            int x = kv.Key.x, y = kv.Key.y, z = kv.Key.z;
            Color32 col = kv.Value;
            if (grain > 0) col = Pal::Var(col, x, y, z, grain);
            for (int f = 0; f < 6; f++)
            {
                const int* n = Normales[f];
                int nx = x + n[0], ny = y + n[1], nz = z + n[2];
                if (m.Has(nx, ny, nz)) continue;
                if (pleinDehors && pleinDehors(nx, ny, nz)) continue;
                Face(md, m, pleinDehors, x, y, z, f, col, pivot, taille, occlusion);
            }
        }
    }
}
