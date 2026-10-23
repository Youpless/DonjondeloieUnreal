// Donjon de l'Oie — Pics de Braise : la forge des Pics, et la montagne de l'Antre d'Ignathor (cœur de l'île).
#include "BatimentsInterne.h"

namespace DonjonDeLOie::Modeles::DecorsModeles::Batiments
{
    using namespace DonjonDeLOie::Voxel;

    namespace
    {
        constexpr Color32 PcRoche = Pal::C("7a5446"), PcRocheSombre = Pal::C("553a32"), PcCendre = Pal::C("8c8580"), PcCendreClaire = Pal::C("a59d94");
        constexpr Color32 PcBraise = Pal::C("ff8a3a"), PcLave = Pal::C("ff6a24");

        Color32 Roche(int x, int y, int z, int hv, bool dessus);   // (définie après MontagneAntre, comme en C#)
    }

    // ------------------------------------------------------------------ forge
    Ptr<VoxelRig> Forge(int graine)
    {
        ChantierBat c(graine);
        c.Assise = 3;
        c.Longueur = 4;
        c.Mousse = 0.0f;
        auto lu = c.Lueur("lueur_foyer");
        // dalle
        c.Pierre(-15, 0, -13, 14, 0, 12, PcRocheSombre);
        // foyer maçonné à l'arrière gauche, hotte et cheminée
        c.Pierre(-13, 1, -11, -3, 8, -4, PcRoche);
        c.Vide(-11, 6, -9, -5, 8, -4);
        for (int x = -11; x <= -5; x++)
            for (int z = -9; z <= -4; z++)
                if (ChantierBat::H(x, 6, z) < 0.75f) lu->Set(x, 6, z, ChantierBat::H(x, 7, z) < 0.4f ? PcBraise : Pal::C("ffb050"));
                else c.M->Set(x, 6, z, Pal::C("3a2a26"));
        lu->Set(-8, 7, -7, Pal::C("ffd070"));
        lu->Set(-7, 7, -6, Pal::C("ffb050"));
        c.PierreCone(-8, -8, 9, 14, 6.5f, 3.5f, PcRoche);
        c.Vide(-11, 9, -4, -5, 10, -3);
        c.Pierre(-10, 15, -10, -6, 32, -6, PcRoche);
        c.Pierre(-11, 32, -11, -5, 33, -5, PcRocheSombre);
        c.Vide(-9, 30, -9, -7, 33, -7);
        // appentis : poteaux et toit de bardeaux en pente vers l'avant
        Color32 bs = Pal::C("6a4632");
        for (int x : {-14, 0, 13})
        {
            c.Poutre(x, 1, 11, x, 25, 11, bs);
            c.Poutre(x, 1, -12, x, 20, -12, bs);
            c.Trait(x + 0.5f, 20.5f, -11.5f, x + 0.5f, 25.2f, 11.5f, 0.5f, bs);
        }
        for (int x = -16; x <= 15; x++)
            for (int z = -14; z <= 13; z++)
            {
                int y = 21 + (z + 14) * 6 / 27;
                if (x >= -11 && x <= -5 && z >= -10 && z <= -6) continue;
                c.M->Set(x, y, z, Pal::Shade(Pal::C("5e4a3c"), (ChantierBat::Mod(z, 2) == 0 ? 1.0f : 0.86f) * (0.92f + 0.12f * ChantierBat::H(x / 3, z, 1))));
            }
        // mur du fond en planches
        c.Bois(-14, 1, -13, 13, 20, -13, Pal::C("5a4030"), 'x');
        // enclume sur billot
        c.Bois(1, 1, -1, 5, 6, 3, Pal::C("7a5238"), 'y', 3);
        Color32 fer = Pal::C("4a4e56");
        c.M->Box(0, 7, 0, 6, 7, 2, fer);
        c.M->Box(1, 8, -1, 6, 10, 3, fer);
        c.M->Box(-2, 10, 0, 0, 10, 2, fer);
        c.M->Set(-3, 10, 1, fer);
        c.M->Box(2, 11, 0, 5, 11, 2, Pal::Shade(fer, 1.3f));
        c.Trait(4.5f, 12.5f, 1.5f, 7.5f, 12.5f, 5.5f, 0.5f, Pal::Bois);   // marteau posé
        c.M->Box(3, 12, 0, 4, 13, 2, fer);
        lu->Set(1, 11, 1, Pal::C("ffa040"));   // pièce rougeoyante sur l'enclume
        // soufflet de cuir contre le foyer
        c.M->Box(-2, 3, -10, 3, 4, -6, Pal::Cuir);
        c.M->Box(-1, 5, -10, 2, 5, -6, Pal::CuirSombre);
        c.M->Box(-2, 6, -10, 3, 6, -6, Pal::Bois);
        c.Trait(3.5f, 6.5f, -8.0f, 7.5f, 7.5f, -8.0f, 0.5f, Pal::Bois);
        c.Trait(-2.5f, 4.5f, -8.0f, -3.5f, 4.5f, -8.0f, 0.5f, Pal::FerSombre);
        // râtelier d'armes à droite : épées, hache, lance
        c.Poutre(8, 1, -11, 8, 18, -11, bs);
        c.Poutre(12, 1, -11, 12, 18, -11, bs);
        c.Poutre(7, 16, -11, 13, 16, -11, bs);
        c.Poutre(7, 6, -11, 13, 6, -11, bs);
        for (int i = 0; i < 3; i++)
        {
            int x = 9 + i * 1 + (i == 2 ? 0 : 0);
            c.M->Box(x, 4, -10, x, 15, -10, Pal::Acier);
            c.M->Box(x - 1, 15, -10, x + 1, 15, -10, Pal::FerSombre);
            c.M->Box(x, 16, -10, x, 18, -10, Pal::Cuir);
            x++;
        }
        c.Poutre(10, 2, -9, 10, 20, -9, Pal::Bois);
        c.M->Box(9, 20, -9, 11, 22, -9, Pal::Acier);
        c.M->Set(10, 23, -9, Pal::Acier);
        c.Trait(13.5f, 1.5f, -8.0f, 11.5f, 16.5f, -8.0f, 0.5f, Pal::Bois);
        c.M->Box(11, 14, -8, 13, 17, -8, Pal::Acier);
        // bouclier et fers suspendus au fond
        c.M->Ellipsoid(-1.5f, 18.5f, -12.0f, 3.0f, 3.0f, 0.6f, Pal::C("7a3a2a"));
        c.M->Set(-2, 18, -11, Pal::Or);
        c.M->Set(-1, 18, -11, Pal::Or);
        for (int i = 0; i < 4; i++) c.M->Box(-1 + i * 2, 12, -12, -1 + i * 2, 14, -12, Pal::FerSombre);
        // bac de trempe, tas de charbon, lingots
        c.Bois(6, 1, 5, 12, 4, 9, Pal::C("6a4632"), 'z');
        c.M->Box(7, 4, 6, 11, 4, 8, Pal::C("3a5a6a"));
        c.M->Ellipsoid(-11.5f, 1.0f, 6.5f, 3.5f, 2.5f, 3.0f, Pal::C("2a2626"));
        for (int x = -6; x <= -3; x++) c.M->Set(x, 1, 8, Pal::C("c87a4a"));
        c.M->Set(-5, 2, 8, Pal::C("c87a4a"));
        // éboulis de pierre brun-rouge autour
        for (int i = 0; i < 6; i++)
        {
            int x = -16 + c.R.Next(32), z = -15 + c.R.Next(3);
            c.Pierre(x, 0, z, x + 2, 1 + c.R.Next(2), z + 1, PcRoche);
        }
        return c.Rig("forge", 0.1f);
    }

    // ------------------------------------------------------------------ montagne de l'Antre
    // VoxelSize 0,15 : 94 × 94 voxels de base (14 m), ~60 de haut (9 m). Seule la peau est remplie.
    Ptr<VoxelRig> MontagneAntre(int graine)
    {
        auto m = Nouveau<VoxelModel>();
        auto lave = Nouveau<VoxelModel>();
        AleaNet R(static_cast<int>(static_cast<uint32_t>(graine) * 31u + 7u));   // graine * 31 + 7, qui boucle comme en C#
        const int N = 47;
        Liste<int> h((2 * N + 1) * (2 * N + 1));   // int[2N + 1, 2N + 1] : h[i, j] → h[i * (2N + 1) + j]
        // pics : (x, z, hauteur, rayon)
        const float pics[][4] = {{0.0f, -10.0f, 60.0f, 40.0f}, {-22.0f, -6.0f, 42.0f, 28.0f}, {20.0f, -16.0f, 47.0f, 28.0f}, {-8.0f, -30.0f, 38.0f, 24.0f},
                                 {26.0f, 6.0f, 28.0f, 22.0f}, {-26.0f, 14.0f, 22.0f, 20.0f}, {13.0f, 12.0f, 34.0f, 22.0f}, {-12.0f, 10.0f, 32.0f, 20.0f}};
        for (int x = -N; x <= N; x++)
            for (int z = -N; z <= N; z++)
            {
                float best = 0;
                for (const auto& p : pics)
                {
                    float dx = x - p[0], dz = z - p[1];
                    float d = Mathf::Sqrt(dx * dx + dz * dz) / p[3];
                    // profil d'épaule (convexe en haut, évasé en bas), crêtes taillées par le bruit angulaire
                    float ang = Mathf::Atan2(dz, dx);
                    float crete = 1.0f + 0.16f * Mathf::Sin(ang * 4.0f + p[0]) + 0.08f * Mathf::Sin(ang * 7.0f + p[1]);
                    float u = Mathf::Clamp01(d / crete);
                    float v = p[2] * (1.0f - u) * (1.0f - 0.55f * u) / (1.0f - 0.55f * u * u + 0.0001f);
                    if (v > best) best = v;
                }
                float rr = Mathf::Sqrt(static_cast<float>(x * x + z * z)) / N;
                best *= Mathf::Clamp01((1.02f - rr) * 5.0f);
                best += (Pal::Hash01(x / 4, 0, z / 4) - 0.5f) * 4.0f + (Pal::Hash01(x / 9, 2, z / 9) - 0.5f) * 5.0f;
                // replat devant la gueule (côté +z)
                if (z > 20 && Mathf::Abs(x) < 28) best = Mathf::Min(best, Mathf::Lerp(26.0f, 0.0f, (z - 20) / 12.0f) + Mathf::Abs(x) * 0.5f);
                int hv = Mathf::FloorToInt(best / 5.0f) * 5 + (best > 6 ? Mathf::FloorToInt(Pal::Hash01(x / 5, 9, z / 5) * 2.0f) : 0);   // strates de 5 voxels
                if (rr > 0.98f) hv = -1;
                h[(x + N) * (2 * N + 1) + (z + N)] = hv;
            }
        auto H = [&](int x, int z) { return (x < -N || x > N || z < -N || z > N) ? -1 : h[(x + N) * (2 * N + 1) + (z + N)]; };
        // la peau : chaque colonne descend jusqu'au plus bas voisin
        for (int x = -N; x <= N; x++)
            for (int z = -N; z <= N; z++)
            {
                int hv = H(x, z);
                if (hv < 0) continue;
                int bas = hv;
                for (int dx = -1; dx <= 1; dx++)
                    for (int dz = -1; dz <= 1; dz++) bas = Mathf::Min(bas, Mathf::Max(0, H(x + dx, z + dz) + 1));
                bas = Mathf::Max(0, Mathf::Min(bas, hv - 1));
                for (int y = bas; y <= hv; y++) m->Set(x, y, z, Roche(x, y, z, hv, y == hv));
            }
        // la gueule : tunnel en ogive vers −z, ouverture côté +z
        float cy0 = 0.0f;
        int zFond = -4, zBouche = 34;
        auto dansTunnel = [&](int x, int y, int z) -> bool
        {
            if (z < zFond || z > zBouche + 8) return false;
            float t = Mathf::Clamp01((z - zFond) / static_cast<float>(zBouche - zFond));
            float rx = 9.0f + 6.0f * t, ry = 17.0f + 7.0f * t;
            float dx = (x + 0.5f) / rx, dy = (y + 0.5f - cy0) / ry;
            return y >= 0 && dx * dx + dy * dy < 1.0f;
        };
        for (int x = -18; x <= 18; x++)
            for (int z = zFond - 2; z <= zBouche + 8; z++)
            {
                int hv = H(x, z);
                for (int y = 0; y <= Mathf::Min(hv, 30); y++)
                {
                    if (dansTunnel(x, y, z)) m->Remove(x, y, z);
                    else
                    {
                        bool bord = false;
                        for (int k = 0; k < 6 && !bord; k++)
                        {
                            int ax = x + (k == 0 ? 1 : k == 1 ? -1 : 0), ay = y + (k == 2 ? 1 : k == 3 ? -1 : 0), az = z + (k == 4 ? 1 : k == 5 ? -1 : 0);
                            if (ay >= 0 && dansTunnel(ax, ay, az)) bord = true;
                        }
                        if (bord) m->Set(x, y, z, Pal::Var(z < zFond + 6 ? Pal::C("1e1618") : Pal::C("3a2a28"), x, y, z, 0.12f));
                    }
                }
            }
        // sol sombre du tunnel
        for (int x = -16; x <= 16; x++)
            for (int z = zFond; z <= zBouche; z++)
                if (!m->Has(x, 0, z) && dansTunnel(x, 1, z)) m->Set(x, 0, z, Pal::Var(Pal::C("2e2422"), x, 0, z, 0.1f));
        // crocs de pierre (stalactites) au linteau de la gueule, et stalagmites aux coins
        for (int i = -5; i <= 5; i++)
        {
            if (i == 0) continue;
            int x = i * 2 + (i > 0 ? 0 : 1);
            float t = Mathf::Clamp01((zBouche - 3 - zFond) / static_cast<float>(zBouche - zFond));
            float rx = 9.0f + 6.0f * t, ry = 17.0f + 7.0f * t;
            float q = 1.0f - ((x + 0.5f) / rx) * ((x + 0.5f) / rx);
            if (q <= 0) continue;
            int ytop = Mathf::FloorToInt(ry * Mathf::Sqrt(q));
            int lg = 3 + static_cast<int>(Pal::Hash01(i, 5, graine) * 4) + (Mathf::Abs(i) < 3 ? 3 : 0);
            for (int k = 0; k < lg; k++)
            {
                m->Set(x, ytop - k, zBouche - 3, Pal::C("cfc4b0"));
                if (k < lg / 2)
                {
                    m->Set(x + 1, ytop - k, zBouche - 3, Pal::C("bdb29e"));
                    m->Set(x, ytop - k, zBouche - 4, Pal::C("bdb29e"));
                }
            }
        }
        for (int sx : {-1, 1})
            for (int k = 0; k < 8; k++)
            {
                int x = sx * 12 + (sx < 0 ? 0 : -1);
                m->Set(x, k, zBouche - 2, Pal::C("cfc4b0"));
                if (k < 5) m->Set(x - sx, k, zBouche - 2, Pal::C("bdb29e"));
            }
        // lueur rouge tout au fond (les braises du dragon)
        for (int x = -4; x <= 3; x++)
            for (int y = 1; y <= 4; y++)
                if (Pal::Hash01(x, y, 3) < 0.5f) lave->Set(x, y, zFond + 1, y < 3 ? PcLave : Pal::C("c84a20"));
        // filets de lave : on suit la plus grande pente depuis quelques points hauts
        const int sources[][2] = {{1, -9}, {-20, -5}, {19, -15}, {24, 6}, {-11, 9}, {12, 11}, {-7, -28}};
        for (const auto& s : sources)
        {
            int x = s[0], z = s[1];
            for (int pas = 0; pas < 60; pas++)
            {
                int hv = H(x, z);
                if (hv < 1) break;
                lave->Set(x, hv, z, pas % 7 == 0 ? Pal::C("ffb040") : PcLave);
                m->Remove(x, hv, z);
                if (m->Has(x, hv - 1, z)) m->Set(x, hv - 1, z, Pal::C("3a2420"));
                int bx = x, bz = z, bh = hv;
                for (int dx = -1; dx <= 1; dx++)
                    for (int dz = -1; dz <= 1; dz++)
                    {
                        if (dx == 0 && dz == 0) continue;
                        if (Mathf::Abs(x + dx) < 18 && z + dz > 12) continue;   // ne coule pas sur la gueule
                        int hh = H(x + dx, z + dz);
                        if (hh >= 0 && hh < bh)
                        {
                            bh = hh;
                            bx = x + dx;
                            bz = z + dz;
                        }
                    }
                if (bx == x && bz == z) break;
                // chaque marche de strate : la lave descend le long de la paroi
                for (int y = bh + 1; y < hv; y++)
                {
                    lave->Set(bx, y, bz, PcLave);
                    m->Remove(bx, y, bz);
                }
                x = bx;
                z = bz;
            }
        }
        // devant la gueule : tas d'or, coffres, ossements
        Color32 or1 = Pal::C("e2b048"), or2 = Pal::C("b88a2a"), or3 = Pal::C("f2d070");
        const float tas[][4] = {{-7.0f, 38.0f, 6.0f, 4.0f}, {6.0f, 41.0f, 7.0f, 5.0f}, {-1.0f, 30.0f, 8.0f, 5.0f}, {13.0f, 34.0f, 4.0f, 3.0f},
                                {-14.0f, 36.0f, 4.0f, 2.5f}};
        for (const auto& t : tas)
            for (int x = Mathf::FloorToInt(t[0] - t[2]); x <= t[0] + t[2]; x++)
                for (int z = Mathf::FloorToInt(t[1] - t[2]); z <= t[1] + t[2]; z++)
                {
                    float dx = (x + 0.5f - t[0]) / t[2], dz = (z + 0.5f - t[1]) / t[2];
                    float d = dx * dx + dz * dz;
                    if (d > 1) continue;
                    int hh = Mathf::RoundToInt(t[3] * (1.0f - d));
                    for (int y = 0; y <= hh; y++)
                    {
                        float r = Pal::Hash01(x, y, z);
                        m->Set(x, y, z, r < 0.12f ? or3 : r < 0.6f ? or1 : or2);
                    }
                }
        // coffre ouvert et calice, gemmes
        m->Box(9, 0, 28, 13, 2, 30, Pal::C("6a4228"));
        m->Box(9, 3, 27, 13, 5, 27, Pal::C("6a4228"));
        m->Box(10, 3, 28, 12, 3, 29, or3);
        m->Set(9, 1, 31, Pal::FerSombre);
        m->Set(13, 1, 31, Pal::FerSombre);
        m->Set(-6, 5, 38, Pal::C("c83a4a"));
        m->Set(5, 6, 41, Pal::C("3a8ac8"));
        m->Set(-1, 6, 30, Pal::C("4ac86a"));
        m->Box(2, 6, 41, 2, 8, 41, or1);
        m->Box(1, 9, 40, 3, 9, 42, or1);
        // ossements : un grand crâne cornu, des côtes, des os épars
        Color32 os = Pal::C("ddd2b8"), osS = Pal::C("b8ac92");
        m->Box(-20, 0, 30, -15, 4, 35, os);
        m->Box(-19, 1, 36, -16, 3, 39, os);
        m->Set(-19, 3, 34, Pal::Noir);
        m->Set(-16, 3, 34, Pal::Noir);
        m->Set(-19, 3, 35, Pal::Noir);
        m->Set(-16, 3, 35, Pal::Noir);
        m->Line(-19.5f, 4.5f, 31.0f, -23.0f, 9.0f, 27.0f, 1.0f, 0.4f, osS);
        m->Line(-15.5f, 4.5f, 31.0f, -12.0f, 9.0f, 27.0f, 1.0f, 0.4f, osS);
        for (int i = 0; i < 5; i++) m->Line(18.0f + i * 2.0f, 0.5f, 22.0f + i, 21.0f + i * 2.0f, 4.0f + (i % 2), 26.0f + i * 0.5f, 0.55f, i % 2 == 0 ? os : osS);
        m->Line(17.0f, 0.5f, 26.0f, 27.0f, 0.5f, 30.0f, 0.6f, osS);
        for (int i = 0; i < 12; i++)
        {
            // (déclarateurs séparés par des virgules : les tirages restent dans l'ordre du C#)
            float x = -24 + static_cast<float>(R.NextDouble()) * 48, z = 24 + static_cast<float>(R.NextDouble()) * 20;
            float a = static_cast<float>(R.NextDouble()) * 6.28f;
            if (Mathf::Abs(x) < 10 && z < 36) continue;
            m->Line(x, 0.5f, z, x + Mathf::Cos(a) * 3, 0.5f, z + Mathf::Sin(a) * 3, 0.5f, os);
            m->Set(Mathf::FloorToInt(x), 1, Mathf::FloorToInt(z), osS);
        }
        // épées plantées
        const int epees[][2] = {{16, 38}, {-11, 44}};
        for (const auto& e : epees)
        {
            m->Box(e[0], 0, e[1], e[0], 6, e[1], Pal::Acier);
            m->Box(e[0] - 1, 6, e[1], e[0] + 1, 6, e[1], Pal::FerSombre);
            m->Box(e[0], 7, e[1], e[0], 8, e[1], Pal::Cuir);
        }
        auto rig = Nouveau<VoxelRig>();
        rig->Name = "montagne_antre";
        rig->VoxelSize = 0.15f;
        rig->Origin = Vector3::zero;
        rig->Add("corps", m, Vector3::zero);
        rig->Add("lueur_lave", lave, Vector3::zero, "corps");
        return rig;
    }

    namespace
    {
        Color32 Roche(int x, int y, int z, int hv, bool dessus)
        {
            // falaises brun-rouge, replats de cendre ; teinte par gros blocs taillés (6 × 5 × 6)
            int bande = (y + Mathf::FloorToInt(Pal::Hash01(x / 8, 1, z / 8) * 4.0f)) / 5;
            Color32 b = ChantierBat::Mod(bande, 2) == 0 ? PcRoche : Pal::Mix(PcRoche, PcRocheSombre, 0.55f);
            if (y > 42) b = Pal::Mix(b, PcCendre, Mathf::Clamp01((y - 42) / 14.0f));
            if (dessus) b = y > 38 ? PcCendreClaire : Pal::Mix(PcCendre, PcRoche, 0.35f);
            else if (y == hv - 1) b = Pal::Mix(b, PcCendre, 0.3f);   // arête chanfreinée sous le replat
            float t = 0.9f + 0.18f * Pal::Hash01(ChantierBat::Div(x, 6), ChantierBat::Div(y, 5), ChantierBat::Div(z, 6));
            if (!dessus && ChantierBat::Mod(y, 5) == 0) t *= 0.85f;   // joint de strate
            return Pal::Shade(b, t);
        }
    }
}
