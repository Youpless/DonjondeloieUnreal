// Donjon de l'Oie — petite végétation : buisson et souche (0,1 m) ; fleurs, herbes hautes, champignons, fougère,
// roseaux (0,05 m).
#include "NatureInterne.h"

#include <cmath>

namespace DonjonDeLOie::Modeles::DecorsModeles::Nature
{
    using namespace DonjonDeLOie::Voxel;

    // ------------------------------------------------------------------ buisson (0,8 – 1,2 m)

    Ptr<VoxelRig> Buisson(AleaNet& r)
    {
        auto rig = NaRig("buisson", 0.1f);
        auto pied = Nouveau<VoxelModel>(); auto feu = Nouveau<VoxelModel>();
        float H = NaF(r, 8, 10.5f), L = H * NaF(r, 0.72f, 0.92f);
        float t = NaF(r, 0, 1);
        Color32 vBase = NaTeinte(Pal::Mix(Pal::C("527e36"), Pal::C("386638"), t), r, 0.05f);
        Color32 vClair = Pal::Mix(vBase, Pal::C("94b058"), 0.35f);
        Color32 vSombre = Pal::Shade(vBase, 0.6f);
        Color32 vTouffe = Pal::Mix(vBase, Pal::C("2f5a34"), 0.5f);
        int n = 3; float a0 = NaF(r, 0, 6.28f);
        float rotCentre = NaF(r, 0, 1.5f);
        NaMasse(*feu, 0, H * 0.55f, 0, L * 0.5f, H * 0.45f, L * 0.5f, 2, rotCentre, vBase);
        for (int i = 0; i < n; i++)
        {
            float a = a0 + static_cast<float>(i) * 6.2832f / static_cast<float>(n) + NaF(r, -0.4f, 0.4f), d = L * NaF(r, 0.35f, 0.5f);
            float cy = H * NaF(r, 0.3f, 0.45f), rx = L * NaF(r, 0.34f, 0.44f), ry = H * NaF(r, 0.28f, 0.36f), rz = L * NaF(r, 0.34f, 0.44f), rot = NaF(r, 0, 1.5f);
            NaMasse(*feu, Mathf::Cos(a) * d, cy, Mathf::Sin(a) * d, rx, ry, rz, 2, rot, vBase);
        }
        NaSol(*feu);
        NaPeindreFeuillage(*feu, vClair, vSombre, vTouffe, r.Next(100));
        // baies (rouges, ou fleurs blanches au village) piquées sur la surface
        bool baies = NaP(r, 0.6f);
        Color32 cBaie = baies ? Pal::C("b8392f") : Pal::C("ece4d0");
        for (auto& kv : feu->Clone()->Tous())
        {
            Vector3Int p = kv.Key;
            if (p.y > 2 && !feu->Has(p.x, p.y + 1, p.z) && Pal::Hash01(p.x, p.y, p.z + 3) < 0.035f) feu->Set(p.x, p.y + 1, p.z, cBaie);
        }
        // tiges visibles au pied
        for (int i = 0; i < 3; i++)
        {
            float a = NaF(r, 0, 6.28f);
            pied->Line(0, 0, 0, Mathf::Cos(a) * 2, 3, Mathf::Sin(a) * 2, 0.6f, NaEcorceSombre);
        }

        rig->Add("corps", pied, Vector3::zero);
        rig->Add("feuillage", feu, Vector3(0, 1, 0), "corps");
        return rig;
    }

    // ------------------------------------------------------------------ souche (0,1 m)

    Ptr<VoxelRig> Souche(AleaNet& r)
    {
        auto rig = NaRig("souche", 0.1f);
        auto m = Nouveau<VoxelModel>();
        int H = NaI(r, 5, 8);
        Color32 ecorce = NaTeinte(NaEcorce, r, 0.06f);
        NaTronc(*m, r, H, 3.4f, 3.0f, 0, 0, 0.2f, 8, ecorce, NaEcorceSombre);
        int nRacines = NaI(r, 4, 5); float lRacines = NaF(r, 2, 3.0f);
        NaRacines(*m, r, nRacines, 2.0f, lRacines, ecorce);
        // coupe en biais, cernes au sommet, cœur un peu creusé
        float cx = NaF(r, -0.3f, 0.3f), cz = NaF(r, -0.3f, 0.3f);
        m->Carve([&](int x, int y, int z) { return static_cast<float>(y) > static_cast<float>(H - 2) + cx * static_cast<float>(x) + cz * static_cast<float>(z); });
        Color32 bois = Pal::C("c8a070"), cerne = Pal::C("9a7448");
        m->Paint([&](int x, int y, int z, Color32 c)
        {
            if (m->Has(x, y + 1, z)) return c;
            float d = Mathf::Sqrt((static_cast<float>(x) + 0.5f) * (static_cast<float>(x) + 0.5f) + (static_cast<float>(z) + 0.5f) * (static_cast<float>(z) + 0.5f));
            if (d > 3.0f) return c;
            return (static_cast<int>(d)) % 2 == 0 ? bois : cerne;
        });
        // un champignon de souche ou de la mousse
        if (NaP(r, 0.6f))
        {
            float a = NaF(r, 0, 6.28f); int x = static_cast<int>(Mathf::Cos(a) * 3.4f), z = static_cast<int>(Mathf::Sin(a) * 3.4f);
            m->Box(x - 1, H / 2, z - 1, x + 1, H / 2, z + 1, Pal::C("d8b07a"));
        }
        else m->Paint([&](int x, int y, int z, Color32 c) { return !m->Has(x, y + 1, z) && y < 3 && Pal::Hash01(x, y, z) < 0.7f ? Pal::C("5a7a34") : c; });
        NaSol(*m);
        rig->Add("corps", m, Vector3::zero);
        return rig;
    }

    // ------------------------------------------------------------------ fleurs (0,05 m)

    Ptr<VoxelRig> Fleurs(AleaNet& r)
    {
        auto rig = NaRig("fleurs", 0.05f);
        auto m = Nouveau<VoxelModel>();
        const Liste<Color32> couleurs = {Pal::C("e8c33a"), Pal::C("e8e2d4"), Pal::C("c75a8a"), Pal::C("8a6ac8"), Pal::C("d9573b"), Pal::C("6a8ad8")};
        Color32 c1 = couleurs[r.Next(couleurs.Length())]; Color32 c2 = couleurs[r.Next(couleurs.Length())];
        Color32 tige = NaTeinte(Pal::C("5b8a36"), r, 0.08f); Color32 feuille = Pal::Shade(tige, 1.12f);
        // touffe de feuilles au sol
        for (int i = 0; i < 6; i++)
        {
            float a = NaF(r, 0, 6.28f), L = NaF(r, 2, 4);
            float hFeuille = NaF(r, 1, 2);
            m->Line(0, 0, 0, Mathf::Cos(a) * L, hFeuille, Mathf::Sin(a) * L, 0.7f, feuille);
        }
        int n = NaI(r, 5, 9);
        for (int i = 0; i < n; i++)
        {
            float a = NaF(r, 0, 6.28f), d = NaF(r, 0, 4.5f), h = NaF(r, 5, 10);
            int x = static_cast<int>(std::nearbyint(static_cast<double>(Mathf::Cos(a) * d))), z = static_cast<int>(std::nearbyint(static_cast<double>(Mathf::Sin(a) * d))), y = static_cast<int>(h);
            m->Line(0, 0, 0, static_cast<float>(x) + 0.5f, static_cast<float>(y), static_cast<float>(z) + 0.5f, 0.5f, tige);
            Color32 c = NaP(r, 0.65f) ? c1 : c2;
            // corolle en croix, cœur jaune
            m->Set(x, y, z, Pal::C("f0c040"));
            m->Set(x + 1, y, z, c); m->Set(x - 1, y, z, c); m->Set(x, y, z + 1, c); m->Set(x, y, z - 1, c);
            if (NaP(r, 0.5f)) m->Set(x, y + 1, z, Pal::Shade(c, 1.1f));
        }
        NaSol(*m);
        rig->Add("corps", m, Vector3::zero);
        return rig;
    }

    // ------------------------------------------------------------------ herbes hautes (0,05 m)

    Ptr<VoxelRig> HerbesHautes(AleaNet& r)
    {
        auto rig = NaRig("herbes_hautes", 0.05f);
        auto m = Nouveau<VoxelModel>();
        // vertes au village, paille morte (la graine en décide un tiers du temps, pour les cryptes)
        bool morte = NaP(r, 0.33f);
        Color32 b1 = morte ? Pal::C("a8996a") : NaTeinte(Pal::C("7ea446"), r, 0.06f);
        Color32 b2 = morte ? Pal::C("837650") : Pal::Shade(b1, 0.78f);
        Color32 epi = morte ? Pal::C("c4b48a") : Pal::C("c8c070");
        int n = NaI(r, 14, 22);
        for (int i = 0; i < n; i++)
        {
            float a = NaF(r, 0, 6.28f), d = NaF(r, 0, 4.0f), h = NaF(r, 8, 16);
            float x0 = Mathf::Cos(a) * d, z0 = Mathf::Sin(a) * d;
            float pa = NaF(r, 0, 6.28f), pl = NaF(r, 1, 4);
            float xm = x0 + Mathf::Cos(pa) * pl * 0.4f, zm = z0 + Mathf::Sin(pa) * pl * 0.4f;
            float x1 = x0 + Mathf::Cos(pa) * pl, z1 = z0 + Mathf::Sin(pa) * pl;
            Color32 c = NaP(r, 0.5f) ? b1 : b2;
            m->Line(x0, 0, z0, xm, h * 0.6f, zm, 0.5f, c);
            m->Line(xm, h * 0.6f, zm, x1, h, z1, 0.45f, Pal::Shade(c, 1.08f));
            if (NaP(r, 0.35f))
            {
                m->Set(static_cast<int>(std::floor(static_cast<double>(x1))), static_cast<int>(h), static_cast<int>(std::floor(static_cast<double>(z1))), epi);
                m->Set(static_cast<int>(std::floor(static_cast<double>(x1))), static_cast<int>(h) + 1, static_cast<int>(std::floor(static_cast<double>(z1))), epi);
            }
        }
        NaSol(*m);
        rig->Add("corps", m, Vector3::zero);
        return rig;
    }

    // ------------------------------------------------------------------ champignons du bois (0,05 m)

    namespace
    {
        void NaChapeau(VoxelModel& m, float x, float y, float z, float rayon, float h, Color32 c, Color32 dessous, Color32 point, int graine)
        {
            NaMasse(m, x, y, z, rayon, h, rayon, 1, 0.4f, c);
            m.Carve([&](int xx, int yy, int zz)
            {
                return yy < static_cast<int>(y) && (static_cast<float>(xx) - x) * (static_cast<float>(xx) - x) + (static_cast<float>(zz) - z) * (static_cast<float>(zz) - z) < (rayon + 2) * (rayon + 2) &&
                       yy >= static_cast<int>(y - h);
            });
            m.Paint([&](int xx, int yy, int zz, Color32 cc) { return NaEgal(cc, c) && !m.Has(xx, yy - 1, zz) ? dessous : cc; });
            m.Paint([&](int xx, int yy, int zz, Color32 cc) { return NaEgal(cc, c) && !m.Has(xx, yy + 1, zz) && Pal::Hash01(xx, graine, zz) < 0.12f ? point : cc; });
        }
    }

    Ptr<VoxelRig> Champignon(AleaNet& r)
    {
        auto rig = NaRig("champignon", 0.05f);
        auto m = Nouveau<VoxelModel>();
        int sorte = r.Next(3);
        Color32 ch, pt;
        if (sorte == 0)                // amanite
        {
            ch = Pal::C("b8392f");
            pt = Pal::C("efe6d2");
        }
        else if (sorte == 1)           // cèpe
        {
            ch = Pal::C("9a6a3c");
            pt = Pal::C("b88a54");
        }
        else                           // champignon des murmures
        {
            ch = Pal::C("6a5aa0");
            pt = Pal::C("a898d8");
        }
        ch = NaTeinte(ch, r, 0.06f);
        Color32 pied = Pal::C("e6dcc3"); Color32 lames = Pal::C("cbbd9c");
        int n = NaI(r, 1, 3);
        for (int i = 0; i < n; i++)
        {
            float a = NaF(r, 0, 6.28f), d = i == 0 ? 0 : NaF(r, 3, 5);
            float x = Mathf::Cos(a) * d, z = Mathf::Sin(a) * d;
            float h = i == 0 ? NaF(r, 6, 9) : NaF(r, 3, 6), rp = i == 0 ? 1.4f : 1.0f;
            m->Cylinder(x, z, 0, static_cast<int>(h), rp, pied);
            m->Cylinder(x, z, 0, 1, rp + 0.7f, pied);
            float rChapeau = h * NaF(r, 0.55f, 0.75f); int graineChapeau = r.Next(100);
            NaChapeau(*m, x, h + 0.5f, z, rChapeau, h * 0.42f, ch, lames, pt, graineChapeau);
        }
        // mousse au pied
        for (int i = 0; i < 6; i++)
        {
            float a = NaF(r, 0, 6.28f), d = NaF(r, 1, 5);
            m->Set(static_cast<int>(Mathf::Cos(a) * d), 0, static_cast<int>(Mathf::Sin(a) * d), Pal::C("4f7a34"));
        }
        NaSol(*m);
        rig->Add("corps", m, Vector3::zero);
        return rig;
    }

    // ------------------------------------------------------------------ champignons du marais (0,05 m, plus grands, livides)

    Ptr<VoxelRig> ChampignonMarais(AleaNet& r)
    {
        auto rig = NaRig("champignon_marais", 0.05f);
        auto m = Nouveau<VoxelModel>(); auto lueur = Nouveau<VoxelModel>();
        Color32 ch = NaTeinte(Pal::C("7e8a4a"), r, 0.08f);
        Color32 pt = Pal::C("b4be6a"); Color32 pied = Pal::C("b8b090"); Color32 lames = Pal::C("8a8a5a");
        int n = NaI(r, 2, 4);
        for (int i = 0; i < n; i++)
        {
            float a = NaF(r, 0, 6.28f), d = i == 0 ? 0 : NaF(r, 3.5f, 6);
            float x = Mathf::Cos(a) * d, z = Mathf::Sin(a) * d;
            float h = i == 0 ? NaF(r, 10, 15) : NaF(r, 4, 8);
            float pa = NaF(r, 0, 6.28f), pp = NaF(r, 0.5f, 2.0f);
            float xh = x + Mathf::Cos(pa) * pp, zh = z + Mathf::Sin(pa) * pp;
            m->Line(x, 0, z, xh, h, zh, 1.4f, 1.0f, pied);
            // chapeau plat en ombrelle
            float rChapeau = h * NaF(r, 0.42f, 0.6f); int graineChapeau = r.Next(100);
            NaChapeau(*m, xh, h + 0.5f, zh, rChapeau, h * 0.22f, ch, lames, pt, graineChapeau);
            // de petites pustules phosphorescentes sur le grand
            if (i == 0)
                for (auto& kv : m->Clone()->Tous())
                {
                    Vector3Int p = kv.Key;
                    if (static_cast<float>(p.y) > h && !m->Has(p.x, p.y + 1, p.z) && Pal::Hash01(p.x, p.y, p.z + 11) < 0.06f) lueur->Set(p.x, p.y + 1, p.z, Pal::C("b8f070"));
                }
        }
        NaSol(*m);
        rig->Add("corps", m, Vector3::zero);
        if (lueur->Count() > 0) rig->Add("lueurSpores", lueur, Vector3::zero, "corps");
        return rig;
    }

    // ------------------------------------------------------------------ fougère (0,05 m)

    Ptr<VoxelRig> Fougere(AleaNet& r)
    {
        auto rig = NaRig("fougere", 0.05f);
        auto m = Nouveau<VoxelModel>();
        Color32 v1 = NaTeinte(Pal::C("4f8a3e"), r, 0.07f); Color32 v2 = Pal::Shade(v1, 0.75f); Color32 tige = Pal::C("4a6a2a");
        int n = NaI(r, 6, 9); float a0 = NaF(r, 0, 6.28f);
        for (int i = 0; i < n; i++)
        {
            float a = a0 + static_cast<float>(i) * 6.2832f / static_cast<float>(n) + NaF(r, -0.2f, 0.2f), L = NaF(r, 9, 14), hmax = NaF(r, 6, 10);
            float ca = Mathf::Cos(a), sa = Mathf::Sin(a);
            // fronde : une nervure en arc, des folioles alternées de plus en plus courtes
            Vector3 prec = Vector3::zero;
            for (int k = 0; k <= 10; k++)
            {
                float t = static_cast<float>(k) / 10.0f, dist = t * L, y = hmax * Mathf::Sin(t * 2.6f) * (1 - 0.3f * t) + 0.5f;
                Vector3 p(ca * dist, y, sa * dist);
                if (k > 0) m->Line(prec.x, prec.y, prec.z, p.x, p.y, p.z, 0.5f, tige);
                if (k >= 2 && k < 10)
                {
                    float fl = (1 - t) * 3.5f + 0.6f;
                    float cote = (k % 2 == 0) ? 1.0f : -1.0f;
                    m->Line(p.x, p.y, p.z, p.x - sa * fl * cote + ca * 0.8f, p.y - 0.4f, p.z + ca * fl * cote + sa * 0.8f, 0.55f, k % 3 == 0 ? v2 : v1);
                    m->Line(p.x, p.y, p.z, p.x + sa * fl * cote + ca * 0.8f, p.y - 0.4f, p.z - ca * fl * cote + sa * 0.8f, 0.55f, v1);
                }
                prec = p;
            }
        }
        NaSol(*m);
        rig->Add("corps", m, Vector3::zero);
        return rig;
    }

    // ------------------------------------------------------------------ roseaux (0,05 m, peuvent pousser dans l'eau)

    Ptr<VoxelRig> Roseaux(AleaNet& r)
    {
        auto rig = NaRig("roseaux", 0.05f);
        auto m = Nouveau<VoxelModel>();
        Color32 v1 = NaTeinte(Pal::C("7a8a44"), r, 0.07f); Color32 v2 = Pal::C("9a9a58"); Color32 massette = Pal::C("6a4228");
        int n = NaI(r, 9, 15);
        for (int i = 0; i < n; i++)
        {
            float a = NaF(r, 0, 6.28f), d = NaF(r, 0, 4.5f), h = NaF(r, 14, 26);
            float x0 = Mathf::Cos(a) * d, z0 = Mathf::Sin(a) * d;
            float pa = NaF(r, 0, 6.28f), pl = NaF(r, 0.5f, 3.0f);
            float x1 = x0 + Mathf::Cos(pa) * pl, z1 = z0 + Mathf::Sin(pa) * pl;
            Color32 cTige = NaP(r, 0.6f) ? v1 : v2;
            m->Line(x0, -2, z0, x1, h, z1, 0.55f, cTige);
            if (NaP(r, 0.45f))
            {
                // massette brune au bout
                int mx = static_cast<int>(std::floor(static_cast<double>(x1))), mz = static_cast<int>(std::floor(static_cast<double>(z1))), my = static_cast<int>(h) - 4;
                m->Box(mx, my, mz, mx + 1, my + 3, mz + 1, massette);
                m->Set(mx, my + 4, mz, v2);
            }
            else if (NaP(r, 0.5f))
            {
                // une feuille qui retombe
                float y = h * NaF(r, 0.4f, 0.6f);
                m->Line(x0 + (x1 - x0) * 0.5f, y, z0 + (z1 - z0) * 0.5f, x1 + Mathf::Cos(pa + 1.4f) * 5, y - 3, z1 + Mathf::Sin(pa + 1.4f) * 5, 0.5f, v1);
            }
        }
        m->Carve([&](int x, int y, int z) { return y < -2; });
        rig->Add("corps", m, Vector3::zero);
        return rig;
    }
}
