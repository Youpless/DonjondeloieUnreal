// Donjon de l'Oie — les formes des modèles voxel.
#include "VoxelModel.h"

#include <cmath>

namespace DonjonDeLOie::Voxel
{
    Liste<VoxelLu> VoxelModel::Tous() const
    {
        Liste<VoxelLu> l;
        l.Vecteur().reserve(static_cast<size_t>(vox.Count()));
        for (auto& kv : vox)
        {
            int x, y, z;
            Decle(kv.Key, x, y, z);
            l.Add(VoxelLu{Vector3Int(x, y, z), kv.Value});
        }
        return l;
    }

    void VoxelModel::Box(int x0, int y0, int z0, int x1, int y1, int z1, Color32 c)
    {
        if (x0 > x1) std::swap(x0, x1);
        if (y0 > y1) std::swap(y0, y1);
        if (z0 > z1) std::swap(z0, z1);
        for (int x = x0; x <= x1; x++)
            for (int y = y0; y <= y1; y++)
                for (int z = z0; z <= z1; z++) Set(x, y, z, c);
    }

    void VoxelModel::Shell(int x0, int y0, int z0, int x1, int y1, int z1, Color32 c)
    {
        if (x0 > x1) std::swap(x0, x1);
        if (y0 > y1) std::swap(y0, y1);
        if (z0 > z1) std::swap(z0, z1);
        for (int x = x0; x <= x1; x++)
            for (int y = y0; y <= y1; y++)
                for (int z = z0; z <= z1; z++)
                    if (x == x0 || x == x1 || y == y0 || y == y1 || z == z0 || z == z1) Set(x, y, z, c);
    }

    void VoxelModel::Ellipsoid(float cx, float cy, float cz, float rx, float ry, float rz, Color32 c)
    {
        int x0 = static_cast<int>(std::floor(static_cast<double>(cx - rx))), x1 = static_cast<int>(std::ceil(static_cast<double>(cx + rx)));
        int y0 = static_cast<int>(std::floor(static_cast<double>(cy - ry))), y1 = static_cast<int>(std::ceil(static_cast<double>(cy + ry)));
        int z0 = static_cast<int>(std::floor(static_cast<double>(cz - rz))), z1 = static_cast<int>(std::ceil(static_cast<double>(cz + rz)));
        for (int x = x0; x <= x1; x++)
            for (int y = y0; y <= y1; y++)
                for (int z = z0; z <= z1; z++)
                {
                    float dx = (static_cast<float>(x) + 0.5f - cx) / rx, dy = (static_cast<float>(y) + 0.5f - cy) / ry, dz = (static_cast<float>(z) + 0.5f - cz) / rz;
                    if (dx * dx + dy * dy + dz * dz <= 1.0f) Set(x, y, z, c);
                }
    }

    void VoxelModel::Cone(float cx, float cz, int y0, int y1, float r0, float r1, Color32 c)
    {
        if (y0 > y1)
        {
            std::swap(y0, y1);
            std::swap(r0, r1);
        }
        float rm = std::max(r0, r1);
        int x0 = static_cast<int>(std::floor(static_cast<double>(cx - rm))), x1 = static_cast<int>(std::ceil(static_cast<double>(cx + rm)));
        int z0 = static_cast<int>(std::floor(static_cast<double>(cz - rm))), z1 = static_cast<int>(std::ceil(static_cast<double>(cz + rm)));
        for (int y = y0; y <= y1; y++)
        {
            float t = y1 == y0 ? 0.0f : static_cast<float>(y - y0) / static_cast<float>(y1 - y0);
            float r = r0 + (r1 - r0) * t;
            for (int x = x0; x <= x1; x++)
                for (int z = z0; z <= z1; z++)
                {
                    float dx = static_cast<float>(x) + 0.5f - cx, dz = static_cast<float>(z) + 0.5f - cz;
                    if (dx * dx + dz * dz <= r * r) Set(x, y, z, c);
                }
        }
    }

    void VoxelModel::Line(float x0, float y0, float z0, float x1, float y1, float z1, float r0, float r1, Color32 c)
    {
        Vector3 a(x0, y0, z0), b(x1, y1, z1);
        Vector3 ab = b - a;
        float l2 = std::max(1e-6f, ab.sqrMagnitude());
        float rm = std::max(r0, r1);
        int xa = static_cast<int>(std::floor(static_cast<double>(std::min(x0, x1) - rm))), xb = static_cast<int>(std::ceil(static_cast<double>(std::max(x0, x1) + rm)));
        int ya = static_cast<int>(std::floor(static_cast<double>(std::min(y0, y1) - rm))), yb = static_cast<int>(std::ceil(static_cast<double>(std::max(y0, y1) + rm)));
        int za = static_cast<int>(std::floor(static_cast<double>(std::min(z0, z1) - rm))), zb = static_cast<int>(std::ceil(static_cast<double>(std::max(z0, z1) + rm)));
        for (int x = xa; x <= xb; x++)
            for (int y = ya; y <= yb; y++)
                for (int z = za; z <= zb; z++)
                {
                    Vector3 p(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f, static_cast<float>(z) + 0.5f);
                    float t = Mathf::Clamp01(Vector3::Dot(p - a, ab) / l2);
                    float r = r0 + (r1 - r0) * t;
                    if ((a + ab * t - p).sqrMagnitude() <= r * r) Set(x, y, z, c);
                }
    }

    void VoxelModel::MirrorX(int axe)
    {
        auto copie = vox.Paires();
        for (auto& kv : copie)
        {
            int x, y, z;
            Decle(kv.Key, x, y, z);
            Set(2 * axe - 1 - x, y, z, kv.Value);
        }
    }

    void VoxelModel::Paint(const std::function<Color32(int, int, int, Color32)>& f)
    {
        auto cles = vox.Keys();
        for (long long k : cles)
        {
            int x, y, z;
            Decle(k, x, y, z);
            vox[k] = f(x, y, z, vox[k]);
        }
    }

    void VoxelModel::Carve(const std::function<bool(int, int, int)>& f)
    {
        auto cles = vox.Keys();
        for (long long k : cles)
        {
            int x, y, z;
            Decle(k, x, y, z);
            if (f(x, y, z)) vox.Remove(k);
        }
    }

    void VoxelModel::Merge(const VoxelModel& o, int dx, int dy, int dz)
    {
        auto copie = o.vox.Paires();      // (o peut être ce modèle-ci)
        for (auto& kv : copie)
        {
            int x, y, z;
            Decle(kv.Key, x, y, z);
            Set(x + dx, y + dy, z + dz, kv.Value);
        }
    }

    Ptr<VoxelModel> VoxelModel::Clone() const
    {
        auto m = Nouveau<VoxelModel>();
        m->Merge(*this, 0, 0, 0);
        return m;
    }

    bool VoxelModel::Bounds(Vector3Int& min, Vector3Int& max) const
    {
        min = Vector3Int(2147483647, 2147483647, 2147483647);
        max = Vector3Int(-2147483647 - 1, -2147483647 - 1, -2147483647 - 1);
        if (vox.Count() == 0)
        {
            min = max = Vector3Int(0, 0, 0);
            return false;
        }
        for (auto& kv : vox)
        {
            int x, y, z;
            Decle(kv.Key, x, y, z);
            if (x < min.x) min.x = x;
            if (y < min.y) min.y = y;
            if (z < min.z) min.z = z;
            if (x > max.x) max.x = x;
            if (y > max.y) max.y = y;
            if (z > max.z) max.z = z;
        }
        return true;
    }
}
