// Donjon de l'Oie — éléments des cryptes, de l'antre et du marais (voxel de 0,05 m) : tombe, croix de pierre, os,
// cierge, tas d'or, feu follet.
#include "NatureInterne.h"

#include <cmath>

namespace DonjonDeLOie::Modeles::DecorsModeles::Nature
{
    using namespace DonjonDeLOie::Voxel;

    namespace
    {
        constexpr Color32 NaTombe = Pal::C("8a9098"), NaTombeClaire = Pal::C("a4aab0"), NaTombeSombre = Pal::C("666c76");
        constexpr Color32 NaHerbeMorte = Pal::C("8a8458"), NaHerbeMorteSombre = Pal::C("6a6444");
        constexpr Color32 NaOs = Pal::C("e6dcc3");
        [[maybe_unused]] constexpr Color32 NaOsSombre = Pal::C("b8ab8c");     // (ne sert nulle part, en C# non plus)

        /// Pierre taillée : chaque voxel en bord de bloc éclaircit (arête) ; lichen par plaques.
        void NaPierreTaillee(VoxelModel& m, Color32 lichen, float pLichen)
        {
            m.Paint([&](int x, int y, int z, Color32 c)
            {
                int vides = 0;
                if (!m.Has(x + 1, y, z)) vides++;
                if (!m.Has(x - 1, y, z)) vides++;
                if (!m.Has(x, y, z + 1)) vides++;
                if (!m.Has(x, y, z - 1)) vides++;
                if (!m.Has(x, y + 1, z)) vides++;
                if (Pal::Hash01(x / 2, y / 2, z / 2) < pLichen && y < 12) return Pal::Mix(c, lichen, 0.7f);
                if (vides >= 2) return Pal::Shade(c, 1.1f);
                return c;
            });
        }

        /// Touffes d'herbe morte et terre retournée au pied.
        void NaPiedTombe(VoxelModel& m, AleaNet& r, int rx, int rz)
        {
            for (int x = -rx; x <= rx; x++)
                for (int z = -rz; z <= rz + 6; z++)
                {
                    if (m.Has(x, 0, z)) continue;
                    float h = Pal::Hash01(x, 5, z);
                    if (h < 0.18f)
                    {
                        m.Set(x, 0, z, NaHerbeMorte);
                        if (h < 0.06f) m.Set(x, 1, z, NaHerbeMorteSombre);
                    }
                }
        }
    }

    // ------------------------------------------------------------------ tombe (stèle, 0,5 – 0,8 m)

    Ptr<VoxelRig> Tombe(AleaNet& r)
    {
        auto rig = NaRig("tombe", 0.05f);
        auto m = Nouveau<VoxelModel>();
        Color32 pierre = NaTeinte(NaTombe, r, 0.06f); Color32 sombre = Pal::Shade(pierre, 0.75f);
        int H = NaI(r, 10, 15), L = NaI(r, 4, 6), E = 1;
        int forme = r.Next(3);
        // socle
        m->Box(-L - 1, 0, -E - 2, L, 1, E + 1, sombre);
        // stèle
        m->Box(-L, 2, -E, L - 1, H, E, pierre);
        if (forme == 0)          // sommet arrondi en escalier
        {
            for (int k = 0; k < 3; k++) m->Box(-L + 1 + k, H + 1 + k, -E, L - 2 - k, H + 1 + k, E, pierre);
        }
        else if (forme == 1)     // pignon pointu
        {
            for (int k = 0; k <= L; k++) m->Box(-L + k, H + 1 + k / 1, -E, L - 1 - k, H + 1 + k / 1, E, pierre);
            m->Carve([&](int x, int y, int z) { return y > H + L - 1; });
        }
        else                     // croix gravée sur stèle carrée, chanfreins
        {
            m->Remove(-L, H, -E); m->Remove(-L, H, E); m->Remove(L - 1, H, -E); m->Remove(L - 1, H, E);
            m->Box(-L - 1, H + 1, -E - 1, L, H + 1, E + 1, sombre);
        }
        // gravure : croix en creux sur la face avant
        int cy = H - 4;
        for (int y = cy - 3; y <= cy + 2; y++) m->Set(0, y, E, Pal::Shade(pierre, 0.55f));
        m->Set(-1, cy, E, Pal::Shade(pierre, 0.55f)); m->Set(1, cy, E, Pal::Shade(pierre, 0.55f));
        // fissure, coin ébréché
        if (NaP(r, 0.6f))
        {
            int x = NaI(r, -L + 1, L - 2);
            for (int y = 3; y < H - 2; y += 1) m->Set(x + (y / 4) % 2, y, -E, Pal::Shade(pierre, 0.6f));
        }
        if (NaP(r, 0.5f)) m->Box(L - 2, H - 1, -E, L - 1, H + 3, E, Pal::Shade(pierre, 0.7f));
        if (NaP(r, 0.5f))
        {
            m->Remove(L - 1, H, -E); m->Remove(L - 1, H, E); m->Remove(L - 1, H - 1, E);
        }
        NaPierreTaillee(*m, Pal::C("6e7a5a"), 0.18f);
        // tertre devant
        if (NaP(r, 0.7f)) m->Box(-L + 1, 0, E + 2, L - 2, 0, E + 9, Pal::C("6a5a48"));
        NaPiedTombe(*m, r, L + 3, E + 3);
        // penchée (en voxels : on décale les rangées)
        int sens = NaP(r, 0.5f) ? 1 : -1; float pente = NaF(r, 0.0f, 0.18f);
        auto penchee = Nouveau<VoxelModel>();
        for (auto& kv : m->Tous())
        {
            Vector3Int p = kv.Key; int dz = p.y > 1 ? static_cast<int>(std::nearbyint(static_cast<double>(static_cast<float>(p.y) * pente))) * sens : 0;
            penchee->Set(p.x, p.y, p.z + dz, kv.Value);
        }

        rig->Add("corps", penchee, Vector3::zero);
        return rig;
    }

    // ------------------------------------------------------------------ croix de pierre (1 – 1,4 m)

    Ptr<VoxelRig> CroixPierre(AleaNet& r)
    {
        auto rig = NaRig("croix_pierre", 0.05f);
        auto m = Nouveau<VoxelModel>();
        Color32 pierre = NaTeinte(NaTombeClaire, r, 0.06f); Color32 sombre = Pal::Shade(pierre, 0.72f);
        int H = NaI(r, 20, 27), b = NaI(r, 5, 7), ybras = H - NaI(r, 7, 9);
        // socle en deux marches
        m->Box(-5, 0, -5, 4, 1, 4, sombre);
        m->Box(-4, 2, -4, 3, 3, 3, Pal::Shade(pierre, 0.85f));
        // fût et bras
        m->Box(-2, 4, -1, 1, H, 1, pierre);
        m->Box(-2 - b, ybras, -1, 1 + b, ybras + 3, 1, pierre);
        // croix celtique : anneau autour du croisement
        if (NaP(r, 0.55f))
        {
            for (int k = 0; k < 40; k++)
            {
                float a = static_cast<float>(k) * 6.2832f / 40; int x = static_cast<int>(std::nearbyint(static_cast<double>(Mathf::Cos(a) * 5.2f - 0.5f))),
                                                                    y = static_cast<int>(std::nearbyint(static_cast<double>(static_cast<float>(ybras) + 1.5f + Mathf::Sin(a) * 5.2f)));
                m->Set(x, y, 0, pierre); m->Set(x, y, -1, pierre);
            }
        }
        // extrémités évasées
        m->Box(-3 - b, ybras - 1, -1, -2 - b, ybras + 4, 1, pierre); m->Box(1 + b, ybras - 1, -1, 2 + b, ybras + 4, 1, pierre);
        m->Box(-3, H, -1, 2, H + 1, 1, pierre);
        // usure : un bras ébréché, lichen
        if (NaP(r, 0.5f))
            m->Carve([&](int x, int y, int z)
            {
                AleaNet rUsure(x * 7 + y);
                return x > 1 + b - NaI(rUsure, 0, 2) && y > ybras + 2;
            });
        NaPierreTaillee(*m, Pal::C("7a8a5c"), 0.22f);
        NaPiedTombe(*m, r, 7, 7);
        // penchée
        float pente = NaF(r, 0.0f, 0.12f); int sens = NaP(r, 0.5f) ? 1 : -1;
        auto penchee = Nouveau<VoxelModel>();
        for (auto& kv : m->Tous())
        {
            Vector3Int p = kv.Key; int dx = p.y > 3 ? static_cast<int>(std::nearbyint(static_cast<double>(static_cast<float>(p.y) * pente))) * sens : 0;
            penchee->Set(p.x + dx, p.y, p.z, kv.Value);
        }

        rig->Add("corps", penchee, Vector3::zero);
        return rig;
    }

    // ------------------------------------------------------------------ os (0,05 m)

    namespace
    {
        void NaOsLong(VoxelModel& m, Vector3 a, Vector3 b, float rayon, Color32 c, Color32 bout)
        {
            m.Line(a.x, a.y, a.z, b.x, b.y, b.z, rayon, c);
            Vector3 d = (b - a).normalized(); Vector3 n(-d.z, 0, d.x);
            // épiphyses doubles à chaque bout
            m.Sphere(a.x + n.x * rayon, a.y, a.z + n.z * rayon, rayon * 1.6f, bout);
            m.Sphere(a.x - n.x * rayon, a.y, a.z - n.z * rayon, rayon * 1.6f, bout);
            m.Sphere(b.x + n.x * rayon, b.y, b.z + n.z * rayon, rayon * 1.6f, bout);
            m.Sphere(b.x - n.x * rayon, b.y, b.z - n.z * rayon, rayon * 1.6f, bout);
        }
    }

    Ptr<VoxelRig> Os(AleaNet& r)
    {
        auto rig = NaRig("os", 0.05f);
        auto m = Nouveau<VoxelModel>();
        Color32 os = NaTeinte(NaOs, r, 0.05f); Color32 osS = Pal::Shade(os, 0.82f);
        int n = NaI(r, 2, 4);
        for (int i = 0; i < n; i++)
        {
            float a = NaF(r, 0, 3.14f), L = NaF(r, 7, 12), cx = NaF(r, -4, 4), cz = NaF(r, -4, 4);
            Vector3 d = Vector3(Mathf::Cos(a), 0, Mathf::Sin(a)) * (L * 0.5f);
            Vector3 c(cx, 1.0f + static_cast<float>(i) * 0.6f, cz);
            NaOsLong(*m, c - d, c + d, 0.9f, i % 2 == 0 ? os : osS, os);
        }
        // un crâne, une fois sur deux, ou une cage thoracique
        int choix = r.Next(3);
        if (choix == 0)
        {
            float a = NaF(r, 0, 6.28f); float x = Mathf::Cos(a) * 4, z = Mathf::Sin(a) * 4;
            m->Ellipsoid(x, 3.2f, z, 3.4f, 3.0f, 3.6f, os);
            m->Box(static_cast<int>(x) - 2, 0, static_cast<int>(z) + 1, static_cast<int>(x) + 1, 1, static_cast<int>(z) + 3, osS);
            // orbites et nez, vers +z
            Color32 noir = Pal::C("2a2420");
            int zf = static_cast<int>(std::floor(static_cast<double>(z + 3.2f)));
            m->Set(static_cast<int>(x) - 2, 3, zf, noir); m->Set(static_cast<int>(x) + 1, 3, zf, noir); m->Set(static_cast<int>(x) - 2, 4, zf, noir); m->Set(static_cast<int>(x) + 1, 4, zf, noir);
            m->Set(static_cast<int>(x) - 1, 3, zf, osS); m->Set(static_cast<int>(x), 2, zf, noir);
            m->Carve([&](int xx, int yy, int zz) { return yy < 0; });
        }
        else if (choix == 1)
        {
            // côtes : arcs posés
            float cx = NaF(r, -2, 2);
            for (int k = 0; k < 4; k++)
                for (int s = -1; s <= 1; s += 2)
                    m->Line(cx, 1, static_cast<float>(k) * 2.2f - 3, cx + static_cast<float>(s) * 4.2f, 3.5f - static_cast<float>(k) * 0.3f, static_cast<float>(k) * 2.2f - 2.5f, 0.55f, os);
            m->Line(cx, 1, -4, cx, 1, 5, 0.8f, osS);
        }
        NaSol(*m);
        rig->Add("corps", m, Vector3::zero);
        return rig;
    }

    // ------------------------------------------------------------------ cierges (0,05 m)

    Ptr<VoxelRig> Cierge(AleaNet& r)
    {
        auto rig = NaRig("cierge", 0.05f);
        auto m = Nouveau<VoxelModel>(); auto flamme = Nouveau<VoxelModel>();
        Color32 cire = NaTeinte(Pal::C("e8dcbc"), r, 0.04f); Color32 coulure = Pal::Shade(cire, 0.9f);
        // pierre plate ou crâne servant de bougeoir
        Liste<Color32> tonsSocle = NaTonsPierre(1, r);
        NaRoche(*m, r, Vector3(0, 0.5f, 0), Vector3(5.5f, 2.2f, 5), 6, tonsSocle, 0, NaTombeSombre);
        int n = NaI(r, 1, 3);
        for (int i = 0; i < n; i++)
        {
            float a = NaF(r, 0, 6.28f), d = i == 0 ? 0 : NaF(r, 2.5f, 3.5f);
            int x = static_cast<int>(std::nearbyint(static_cast<double>(Mathf::Cos(a) * d))), z = static_cast<int>(std::nearbyint(static_cast<double>(Mathf::Sin(a) * d)));
            int h = i == 0 ? NaI(r, 8, 12) : NaI(r, 4, 7);
            m->Box(x - 1, 2, z - 1, x, 2 + h, z, cire);
            // coulures et pied de cire fondue
            m->Box(x - 2, 2, z - 2, x + 1, 2, z + 1, coulure);
            if (NaP(r, 0.7f)) m->Box(x + 1, 2 + h - 3, z, x + 1, 2 + h, z, coulure);
            m->Set(x, 3 + h, z, Pal::C("2a2420"));     // mèche
            // flamme : deux voxels de cœur jaune, une pointe orange
            flamme->Set(x, 4 + h, z, Pal::C("ffd27a")); flamme->Set(x - 1, 4 + h, z, Pal::C("ffb24a"));
            flamme->Set(x, 5 + h, z, Pal::C("ff9a3a"));
            if (i == 0) flamme->Set(x, 6 + h, z, Pal::C("ff8a30"));
        }
        NaSol(*m);
        rig->Add("corps", m, Vector3::zero);
        rig->Add("lueurFlamme", flamme, Vector3::zero, "corps");
        return rig;
    }

    // ------------------------------------------------------------------ tas d'or de l'antre (0,05 m)

    Ptr<VoxelRig> TasOr(AleaNet& r)
    {
        auto rig = NaRig("tas_or", 0.05f);
        auto m = Nouveau<VoxelModel>();
        float R = NaF(r, 5.5f, 8), H = NaF(r, 4, 6.5f);
        Color32 or1 = Pal::C("e8b64a"), or2 = Pal::C("c9952e"), or3 = Pal::C("f4d070");
        // monticule en terrasses de pièces
        for (int y = 0; y <= static_cast<int>(H); y++)
        {
            float rr = R * (1 - static_cast<float>(y) / (H + 1.0f)) + 0.5f;
            for (int x = -static_cast<int>(R) - 1; static_cast<float>(x) <= R + 1; x++)
                for (int z = -static_cast<int>(R) - 1; static_cast<float>(z) <= R + 1; z++)
                {
                    float d = Mathf::Sqrt((static_cast<float>(x) + 0.5f) * (static_cast<float>(x) + 0.5f) + (static_cast<float>(z) + 0.5f) * (static_cast<float>(z) + 0.5f)) +
                              (Pal::Hash01(x, y, z) - 0.5f) * 1.6f;
                    if (d > rr) continue;
                    float h = Pal::Hash01(x, y + 9, z);
                    m->Set(x, y, z, h < 0.2f ? or3 : (h < 0.55f ? or2 : or1));
                }
        }
        // pièces éparses autour
        for (int i = 0; i < 8; i++)
        {
            float a = NaF(r, 0, 6.28f), d = NaF(r, R + 1, R + 4);
            int x = static_cast<int>(Mathf::Cos(a) * d), z = static_cast<int>(Mathf::Sin(a) * d);
            m->Set(x, 0, z, or1);
            if (NaP(r, 0.4f)) m->Set(x + 1, 0, z, or2);
        }
        // un trésor qui dépasse : coupe, gemme, épée
        int t = r.Next(3);
        if (t == 0)
        {
            int y = static_cast<int>(H); m->Box(-1, y, -1, 0, y + 1, 0, or2); m->Box(-2, y + 2, -2, 1, y + 4, 1, or1); m->Box(-1, y + 3, -1, 0, y + 4, 0, Pal::C("6a3a1a"));
        }
        else if (t == 1)
        {
            m->Line(1, H - 1, 0, 6, H + 7, 2, 0.6f, Pal::C("c4ccd3")); m->Line(0, H - 2, -1, 3, H - 1, 3, 0.6f, or2);
            m->Set(0, static_cast<int>(H) - 3, 0, Pal::C("b8392f"));
        }
        else
        {
            int y = static_cast<int>(H) + 1;
            m->Box(-1, y, -1, 0, y + 1, 0, Pal::C("3d64b0")); m->Box(2, static_cast<int>(H * 0.6f), 3, 3, static_cast<int>(H * 0.6f) + 1, 4, Pal::C("b8392f"));
        }
        NaSol(*m);
        rig->Add("corps", m, Vector3::zero);
        return rig;
    }

    // ------------------------------------------------------------------ feu follet du marais (0,05 m, flotte à ~0,8 m)

    Ptr<VoxelRig> FeuFollet(AleaNet& r)
    {
        auto rig = NaRig("feu_follet", 0.05f);
        auto socle = Nouveau<VoxelModel>(); auto flamme = Nouveau<VoxelModel>();
        // une touffe de vase et de roseaux courts au sol, pour ancrer l'objet
        Color32 vase = Pal::C("4e5534"), herbe = Pal::C("6a7a3a");
        for (int x = -4; x <= 4; x++)
            for (int z = -4; z <= 4; z++)
                if (x * x + z * z <= 14 + static_cast<int>(Pal::Hash01(x, 0, z) * 6)) socle->Set(x, 0, z, Pal::Hash01(x, 1, z) < 0.3f ? herbe : vase);
        for (int i = 0; i < 4; i++)
        {
            float a = NaF(r, 0, 6.28f), d = NaF(r, 1, 3.5f);
            float hBrin = NaF(r, 3, 6);
            socle->Line(Mathf::Cos(a) * d, 0, Mathf::Sin(a) * d, Mathf::Cos(a) * d * 1.2f, hBrin, Mathf::Sin(a) * d * 1.2f, 0.5f, herbe);
        }
        // flamme en goutte : cœur pâle, enveloppe bleu-vert, pointe recourbée
        float h = NaF(r, 14, 18), dx = NaF(r, -1, 1), dz = NaF(r, -1, 1);
        Color32 bord = Pal::C("3fc8a8"), milieu = Pal::C("7ff0d0"), coeur = Pal::C("d8fff0");
        flamme->Ellipsoid(dx, h, dz, 2.6f, 2.8f, 2.6f, bord);
        float px = NaF(r, -1.5f, 1.5f), pz = NaF(r, -1.5f, 1.5f);
        flamme->Line(dx, h + 1, dz, dx + px, h + 6, dz + pz, 1.6f, 0.5f, bord);
        flamme->Ellipsoid(dx, h + 0.5f, dz, 1.6f, 2.2f, 1.6f, milieu);
        flamme->Box(static_cast<int>(std::floor(static_cast<double>(dx))), static_cast<int>(h), static_cast<int>(std::floor(static_cast<double>(dz))), static_cast<int>(std::floor(static_cast<double>(dx))),
                    static_cast<int>(h) + 1, static_cast<int>(std::floor(static_cast<double>(dz))), coeur);
        // deux étincelles qui traînent
        flamme->Set(static_cast<int>(dx + 3), static_cast<int>(h) - 3, static_cast<int>(dz), bord); flamme->Set(static_cast<int>(dx - 2), static_cast<int>(h) - 5, static_cast<int>(dz + 2), milieu);

        rig->Add("corps", socle, Vector3::zero);
        rig->Add("lueurFollet", flamme, Vector3(dx, h, dz), "corps");
        return rig;
    }
}
