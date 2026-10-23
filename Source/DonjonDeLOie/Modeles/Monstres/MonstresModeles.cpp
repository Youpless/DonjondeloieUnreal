// Donjon de l'Oie — les douze monstres (onze, plus le Dragonnoie) : point d'entrée et outils de sculpture communs.
// Chaque monstre est construit dans un fichier partiel (MonstresModeles.<Nom>.cpp) ; ici, le choix par identifiant
// et les primitives « taillées » (boîtes chanfreinées, tronçons octogonaux, chaînes de traits, membranes).
#include "MonstresInterne.h"

namespace DonjonDeLOie::Modeles::MonstresModeles
{
    using namespace DonjonDeLOie::Voxel;

    Ptr<VoxelRig> Construire(const string& id)
    {
        if (id == "rat") return Rat();
        if (id == "gobelin") return Gobelin();
        if (id == "loup") return Loup();
        if (id == "araignee") return Araignee();
        if (id == "gelee") return Gelee();
        if (id == "squelette") return Squelette();
        if (id == "orc") return Orc();
        if (id == "spectre") return Spectre();
        if (id == "troll") return Troll();
        if (id == "wyverne") return Wyverne();
        if (id == "dragon") return Dragon();
        if (id == "dragonnoie") return Dragonnoie();
        // Repli : un simple bloc rouge.
        auto m = Nouveau<VoxelModel>();
        BoiteC(*m, -5, 0, -5, 4, 9, 4, Pal::Rouge, 1);
        auto r = Nouveau<VoxelRig>();
        r->Name = "inconnu";
        r->VoxelSize = 0.05f;
        r->Origin = Vector3::zero;
        r->Add("corps", m, Vector3(0, 5, 0));
        return r;
    }

    // ------------------------------------------------------------------ couleurs

    // C, Lerp et V : en ligne dans MonstresInterne.h (C est constexpr, pour les couleurs fixes du dragon et du Dragonnoie).

    // ------------------------------------------------------------------ rigs

    Ptr<VoxelRig> NouveauRig(const string& nom, float taille)
    {
        auto r = Nouveau<VoxelRig>();
        r->Name = nom;
        r->VoxelSize = taille;
        return r;
    }

    /// Pose l'origine au sol, au centre de l'emprise en z (x = 0 : les corps sont centrés sur l'axe).
    Ptr<VoxelRig> Fin(const Ptr<VoxelRig>& r)
    {
        float zmin = FLT_MAX, zmax = -FLT_MAX;
        for (auto& p : r->Parts)
        {
            Vector3Int a, b;
            if (!p->Model->Bounds(a, b)) continue;
            zmin = std::min<float>(zmin, a.z); zmax = std::max<float>(zmax, b.z + 1);
        }
        r->Origin = Vector3(0, 0, zmin < zmax ? static_cast<float>(ArrondiNet(static_cast<double>((zmin + zmax) * 0.5f))) : 0);
        return r;
    }

    // ------------------------------------------------------------------ volumes taillés

    /// Boîte pleine aux arêtes coupées sur « ch » voxels (biseau).
    void BoiteC(VoxelModel& m, int x0, int y0, int z0, int x1, int y1, int z1, Color32 c, int ch)
    {
        if (x0 > x1) std::swap(x0, x1);
        if (y0 > y1) std::swap(y0, y1);
        if (z0 > z1) std::swap(z0, z1);
        for (int x = x0; x <= x1; x++)
            for (int y = y0; y <= y1; y++)
                for (int z = z0; z <= z1; z++)
                {
                    int dx = std::min(x - x0, x1 - x), dy = std::min(y - y0, y1 - y), dz = std::min(z - z0, z1 - z);
                    if (dx + dy < ch || dx + dz < ch || dy + dz < ch) continue;
                    m.Set(x, y, z, c);
                }
    }

    /// Une section octogonale (rectangle de demi-côtés a × b, coins coupés de ch) dans un plan.
    /// axe 0 : plan (y, z) à x = k ; axe 1 : plan (x, z) à y = k ; axe 2 : plan (x, y) à z = k. (u, v) : centre dans le plan.
    void Section(VoxelModel& m, int axe, int k, float cu, float cv, float a, float b, float ch, Color32 c)
    {
        int u0 = Mathf::FloorToInt(cu - a), u1 = Mathf::CeilToInt(cu + a);
        int v0 = Mathf::FloorToInt(cv - b), v1 = Mathf::CeilToInt(cv + b);
        for (int u = u0; u <= u1; u++)
            for (int v = v0; v <= v1; v++)
            {
                float du = std::abs(u + 0.5f - cu), dv = std::abs(v + 0.5f - cv);
                if (du > a || dv > b || du + dv > a + b - ch) continue;
                if (axe == 2) m.Set(u, v, k, c);
                else if (axe == 1) m.Set(u, k, v, c);
                else m.Set(k, u, v, c);
            }
    }

    /// Tronçon allongé selon z (de z0 à z1) : centre (cx, cy), demi-largeur w, demi-hauteur h, interpolés.
    void TronconZ(VoxelModel& m, int z0, int z1, float cx, float cy0, float cy1, float w0, float w1, float h0, float h1, float ch, Color32 c)
    {
        for (int z = z0; z <= z1; z++)
        {
            float t = z1 == z0 ? 0 : (z - z0) / static_cast<float>(z1 - z0);
            Section(m, 2, z, cx, Lerp(cy0, cy1, t), Lerp(w0, w1, t), Lerp(h0, h1, t), ch, c);
        }
    }

    /// Tronçon vertical (de y0 à y1) : centre (cx, cz), demi-largeur w (x), demi-profondeur d (z), interpolés.
    void TronconY(VoxelModel& m, int y0, int y1, float cx, float cz0, float cz1, float w0, float w1, float d0, float d1, float ch, Color32 c)
    {
        for (int y = y0; y <= y1; y++)
        {
            float t = y1 == y0 ? 0 : (y - y0) / static_cast<float>(y1 - y0);
            Section(m, 1, y, cx, Lerp(cz0, cz1, t), Lerp(w0, w1, t), Lerp(d0, d1, t), ch, c);
        }
    }

    /// Corps allongé selon z, profil par clés : k = { z, cy, demi-largeur, demi-hauteur,  z, cy, … } (z croissants).
    /// Le centre en x est cx ; les sections sont octogonales (coins coupés selon « ch » × la plus petite demi-taille).
    void ProfilZ(VoxelModel& m, float cx, const Liste<float>& k, float ch, Color32 c)
    {
        int n = k.Length() / 4;
        for (int i = 1; i < n; i++)
        {
            int z0 = static_cast<int>(k[i * 4 - 4]), z1 = static_cast<int>(k[i * 4]);
            for (int z = z0; z <= z1; z++)
            {
                float t = z1 == z0 ? 0 : (z - z0) / static_cast<float>(z1 - z0);
                float cy = Lerp(k[i * 4 - 3], k[i * 4 + 1], t), w = Lerp(k[i * 4 - 2], k[i * 4 + 2], t), h = Lerp(k[i * 4 - 1], k[i * 4 + 3], t);
                Section(m, 2, z, cx, cy, w, h, ch * std::min(w, h), c);
            }
        }
    }

    /// Corps vertical, profil par clés : k = { y, cz, demi-largeur (x), demi-profondeur (z), … } (y croissants).
    void ProfilY(VoxelModel& m, float cx, const Liste<float>& k, float ch, Color32 c)
    {
        int n = k.Length() / 4;
        for (int i = 1; i < n; i++)
        {
            int y0 = static_cast<int>(k[i * 4 - 4]), y1 = static_cast<int>(k[i * 4]);
            for (int y = y0; y <= y1; y++)
            {
                float t = y1 == y0 ? 0 : (y - y0) / static_cast<float>(y1 - y0);
                float cz = Lerp(k[i * 4 - 3], k[i * 4 + 1], t), w = Lerp(k[i * 4 - 2], k[i * 4 + 2], t), d = Lerp(k[i * 4 - 1], k[i * 4 + 3], t);
                Section(m, 1, y, cx, cz, w, d, ch * std::min(w, d), c);
            }
        }
    }

    /// Suite de traits épais passant par des points (x, y, z, x, y, z…), rayon passant de r0 à r1.
    void Chaine(VoxelModel& m, const Liste<float>& p, float r0, float r1, Color32 c)
    {
        int n = p.Length() / 3;
        float total = 0;
        for (int i = 1; i < n; i++) total += (V(p[i * 3], p[i * 3 + 1], p[i * 3 + 2]) - V(p[i * 3 - 3], p[i * 3 - 2], p[i * 3 - 1])).magnitude();
        float acc = 0;
        for (int i = 1; i < n; i++)
        {
            Vector3 a = V(p[i * 3 - 3], p[i * 3 - 2], p[i * 3 - 1]); Vector3 b = V(p[i * 3], p[i * 3 + 1], p[i * 3 + 2]);
            float l = (b - a).magnitude();
            float ra = Lerp(r0, r1, acc / total), rb = Lerp(r0, r1, (acc + l) / total);
            m.Line(a.x, a.y, a.z, b.x, b.y, b.z, ra, rb, c);
            acc += l;
        }
    }

    /// Membrane : triangle plein d'environ un voxel et demi d'épaisseur (ailes, voiles).
    void Triangle(VoxelModel& m, Vector3 a, Vector3 b, Vector3 c, Color32 col)
    {
        Vector3 n = Vector3::Cross(b - a, c - a).normalized() * 0.55f;
        float l = std::max((b - a).magnitude(), std::max((c - a).magnitude(), (c - b).magnitude()));
        int k = Mathf::CeilToInt(l * 2.2f) + 1;
        for (int i = 0; i <= k; i++)
            for (int j = 0; j <= k - i; j++)
            {
                Vector3 p = a + (b - a) * (i / static_cast<float>(k)) + (c - a) * (j / static_cast<float>(k));
                Vector3 q = p + n;
                m.Set(Mathf::FloorToInt(p.x), Mathf::FloorToInt(p.y), Mathf::FloorToInt(p.z), col);
                m.Set(Mathf::FloorToInt(q.x), Mathf::FloorToInt(q.y), Mathf::FloorToInt(q.z), col);
            }
    }

    /// Disque face à +z (bouclier), de z0 à z1.
    void DisqueZ(VoxelModel& m, float cx, float cy, int z0, int z1, float r, Color32 c)
    {
        for (int z = z0; z <= z1; z++)
            for (int x = Mathf::FloorToInt(cx - r); x <= Mathf::CeilToInt(cx + r); x++)
                for (int y = Mathf::FloorToInt(cy - r); y <= Mathf::CeilToInt(cy + r); y++)
                {
                    float dx = x + 0.5f - cx, dy = y + 0.5f - cy;
                    if (dx * dx + dy * dy <= r * r) m.Set(x, y, z, c);
                }
    }

    // ------------------------------------------------------------------ retouches

    /// z du voxel le plus en avant de la colonne (x, y), ou int.MinValue (INT_MIN).
    int Avant(const VoxelModel& m, int x, int y, int zmin, int zmax)
    {
        for (int z = zmax; z >= zmin; z--)
            if (m.Has(x, y, z)) return z;
        return INT_MIN;
    }

    /// y du voxel le plus haut de la colonne (x, z), ou int.MinValue (INT_MIN).
    int Haut(const VoxelModel& m, int x, int z, int ymin, int ymax)
    {
        for (int y = ymax; y >= ymin; y--)
            if (m.Has(x, y, z)) return y;
        return INT_MIN;
    }

    /// Retire de « a » tous les voxels présents dans « b » (évite les faces confondues entre pièces).
    void Soustraire(VoxelModel& a, const VoxelModel& b)
    {
        for (auto& kv : b.Tous()) a.Remove(kv.Key.x, kv.Key.y, kv.Key.z);
    }

    /// Pose un voxel lumineux dans « lueur » et le retire de la pièce porteuse (œil affleurant).
    void Oeil(VoxelModel& porteur, VoxelModel& lueur, int x, int y, int z, Color32 c)
    {
        porteur.Remove(x, y, z); lueur.Set(x, y, z, c);
    }

    /// Repeint selon un bruit déterministe : au-dessus du seuil, couleur « b ».
    void Taches(VoxelModel& m, Color32 depuis, Color32 b, float seuil, int graine)
    {
        m.Paint([&](int x, int y, int z, Color32 c) { return (c.r == depuis.r && c.g == depuis.g && c.b == depuis.b && Pal::Hash01(x + graine, y * 3 + graine, z) > seuil) ? b : c; });
    }

    // Meme : en ligne dans MonstresInterne.h.
}
