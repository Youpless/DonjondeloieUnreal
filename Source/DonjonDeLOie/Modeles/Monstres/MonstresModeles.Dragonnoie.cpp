// Donjon de l'Oie — le Dragonnoie : une oie géante (corps blanc, long cou, bec et pattes orange) aux traits de dragon
// (cornes, crête d'épines rouges, queue écailleuse terminée en fer de lance, griffe au poignet, yeux de braise), ailes à
// demi déployées, en posture de menace. Un collier d'or rappelle ce qu'il vole. Voxel de 7,5 cm (~3,4 m jusqu'aux cornes).
#include "MonstresInterne.h"

namespace DonjonDeLOie::Modeles::MonstresModeles
{
    using namespace DonjonDeLOie::Voxel;

    namespace
    {
        constexpr Color32 OiBlanc = C("f4f1ea"), OiBlancS = C("d8d2c4"), OiBlancC = C("fffdf7"), OiGris = C("9aa0a6"), OiGrisS = C("5d636b");
        constexpr Color32 OiBec = C("f0892a"), OiBecS = C("c0611c"), OiBecC = C("ffb25a"), OiRouge = C("b8392f"), OiRougeS = C("7a221d");
        constexpr Color32 OiCorne = C("2e2622"), OiOr = C("e8b64a"), OiOrS = C("a77a23"), OiOeil = C("ff5a1e"), OiDent = C("f4ead2");

        /// Plumage : rangées de plumes en écailles arrondies (un voxel plus sombre à la base de chaque plume).
        void Plumage(VoxelModel& m, Color32 base_, Color32 ombre, Color32 reflet, int pas)
        {
            m.Paint([&](int x, int y, int z, Color32 c)
            {
                if (!Meme(c, base_)) return c;
                int rang = Mathf::FloorToInt((y + z * 0.35f) / pas);
                int dx = x + (rang % 2 == 0 ? 0 : pas / 2);
                int col = Mathf::FloorToInt(dx / static_cast<float>(pas));
                int ly = Mathf::FloorToInt(y + z * 0.35f) - rang * pas, lx = dx - col * pas;
                if (ly == 0 && lx != pas / 2) return ombre;
                if (ly == pas - 1 && lx == pas / 2 && Pal::Hash01(rang, col, 3) > 0.5f) return reflet;
                return c;
            });
        }
    }

    Ptr<VoxelRig> Dragonnoie()
    {
        auto r = NouveauRig("dragonnoie", 0.075f);

        // ---------------------------------------------------------- corps : ovoïde d'oie, poitrail bombé
        auto corps = Nouveau<VoxelModel>();
        ProfilZ(*corps, 0, {-14, 18, 2.6f, 2.4f, -11, 17, 5, 4.6f, -6, 15.5f, 6.8f, 6.2f, 0, 15, 7.2f, 6.8f,
                            5, 15.5f, 6.8f, 6.8f, 8, 17, 5.6f, 5.8f, 10, 19, 3.8f, 4.2f}, 0.45f, OiBlanc);
        // queue d'oie relevée (plumes en éventail)
        for (int k = -2; k <= 2; k++) corps->Line(k * 0.9f, 19, -13, k * 1.6f, 22.5f, -17, 1.2f, 0.7f, OiBlanc);
        Plumage(*corps, OiBlanc, OiBlancS, OiBlancC, 3);
        // dos de dragon : une ligne d'écailles rouges et des épines du garrot à la queue
        corps->Paint([&](int x, int y, int z, Color32 c)
        {
            return (Meme(c, OiBlanc) || Meme(c, OiBlancS) || Meme(c, OiBlancC)) && std::abs(x + 0.5f) < 1.6f && y >= Haut(*corps, x, z, 0, 40) - 1 && z > -12 && z < 8
                ? ((z & 1) == 0 ? OiRouge : OiRougeS) : c;
        });
        for (int k = 0; k < 7; k++)
        {
            int z = 7 - k * 3; int h = Haut(*corps, 0, z, 0, 40);
            if (h == INT_MIN) continue;
            float haut = 2.6f - std::abs(k - 2) * 0.25f;
            corps->Line(0.5f, h, z, 0.5f, h + haut, z - 1.6f, 0.9f, 0.3f, k % 2 == 0 ? OiRouge : OiRougeS);
        }
        r->Add("corps", corps, V(0, 15, 0));

        // ---------------------------------------------------------- cou et tête (une pièce : le cou entier s'élance au coup de bec)
        auto tete = Nouveau<VoxelModel>();
        Chaine(*tete, {0, 19, 7, 0, 25, 9.5f, 0, 30.5f, 9.5f, 0, 35, 8.5f, 0, 38.5f, 10}, 4.4f, 3.1f, OiBlanc);
        tete->Ellipsoid(0, 41, 11.5f, 3.7f, 3.5f, 4.3f, OiBlanc);
        Plumage(*tete, OiBlanc, OiBlancS, OiBlancC, 2);
        // collier d'or
        for (int y = 26; y <= 27; y++)
            tete->Paint([&](int x, int yy, int z, Color32 c) { return yy == y && ((x * x) + (z - 9.5f) * (z - 9.5f)) > 5.0f ? (Meme(c, OiBlanc) || Meme(c, OiBlancS) || Meme(c, OiBlancC) ? OiOr : c) : c; });
        tete->Paint([&](int x, int y, int z, Color32 c) { return Meme(c, OiOr) && ((x + z) & 3) == 0 ? OiOrS : c; });
        tete->Box(-1, 23, 12, 0, 25, 13, OiOr); tete->Set(-1, 24, 13, OiOrS);        // médaillon
        // crête d'épines rouges sur la nuque
        for (int k = 0; k < 6; k++)
        {
            [[maybe_unused]] float y = 38 - k * 2.6f, z = 8.6f - (k < 3 ? k * 0.2f : 0.8f - (k - 3) * 0.4f);   // (z ne sert pas, comme en C#)
            int arriere = Avant(*tete, 0, static_cast<int>(y), 0, 20);
            if (arriere == INT_MIN) continue;
            int dos = INT_MAX;
            for (int zz = -5; zz <= 20; zz++)
                if (tete->Has(0, static_cast<int>(y), zz))
                {
                    dos = zz;
                    break;
                }
            if (dos == INT_MAX) continue;
            tete->Line(0.5f, y, dos, 0.5f, y + 1.4f, dos - 2.4f + k * 0.15f, 0.8f, 0.3f, k % 2 == 0 ? OiRouge : OiRougeS);
        }
        // bec supérieur : épais à la base, crochu, dents en scie ; bosse orange au front
        for (int z = 14; z <= 22; z++)
        {
            float t = (z - 14) / 8.0f;
            float w = Lerp(2.7f, 1.2f, t), h0 = Lerp(39.6f, 39.2f, t), h1 = Lerp(43.2f, 40.6f, t);
            for (int x = Mathf::FloorToInt(-w); x < Mathf::CeilToInt(w); x++)
                for (int y = Mathf::FloorToInt(h0); y <= Mathf::FloorToInt(h1); y++) tete->Set(x, y, z, z >= 21 ? OiBecS : (y >= Mathf::FloorToInt(h1) && (z & 1) == 0 ? OiBecC : OiBec));
        }
        tete->Set(-1, 38, 22, OiBecS); tete->Set(0, 38, 22, OiBecS); tete->Set(-1, 37, 23, OiCorne); tete->Set(0, 37, 23, OiCorne);   // crochet
        for (int z = 15; z <= 21; z += 2) { tete->Set(-3, 38, z, OiDent); tete->Set(2, 38, z, OiDent); }
        tete->Ellipsoid(0, 44, 14.2f, 2.2f, 2, 2.2f, OiBec);              // bosse d'oie de Guinée
        tete->Set(0, 46, 14, OiBecC); tete->Set(-1, 46, 14, OiBecC);
        tete->Set(-2, 42, 19, OiBecS); tete->Set(1, 42, 19, OiBecS);       // narines
        // cornes de dragon, rabattues vers l'arrière
        for (int s = 0; s < 2; s++)
        {
            float sx = s == 0 ? -1.0f : 1.0f;
            Chaine(*tete, {sx * 2.2f, 43.5f, 10.5f, sx * 3.6f, 47, 8, sx * 4.4f, 48.5f, 4.5f, sx * 4.2f, 48, 2.5f}, 1.4f, 0.4f, OiCorne);
            tete->Box(s == 0 ? -4 : 3, 43, 12, s == 0 ? -4 : 3, 43, 14, OiGrisS);   // sourcil froncé
            tete->Box(s == 0 ? -3 : 2, 44, 14, s == 0 ? -3 : 2, 44, 14, OiGrisS);
        }
        auto lueur = Nouveau<VoxelModel>();
        Oeil(*tete, *lueur, -4, 42, 13, OiOeil); Oeil(*tete, *lueur, 3, 42, 13, OiOeil);
        Oeil(*tete, *lueur, -4, 41, 13, OiOeil); Oeil(*tete, *lueur, 3, 41, 13, OiOeil);
        r->Add("tete", tete, V(0, 19, 7), "corps");
        r->Add("lueur_yeux", lueur, V(0, 41.5f, 13), "tete");

        // bec inférieur (s'ouvre pour cacarder… ou mordre)
        auto mach = Nouveau<VoxelModel>();
        for (int z = 14; z <= 21; z++)
        {
            float t = (z - 14) / 7.0f;
            float w = Lerp(2.2f, 0.9f, t);
            for (int x = Mathf::FloorToInt(-w); x < Mathf::CeilToInt(w); x++)
            {
                mach->Set(x, 38, z, OiBecS);
                if (z < 19) mach->Set(x, 37, z, OiBec);
            }
        }
        for (int z = 16; z <= 20; z += 2) { mach->Set(-2, 39, z, OiDent); mach->Set(1, 39, z, OiDent); }
        mach->Set(-1, 38, 15, OiRouge); mach->Set(0, 38, 16, OiRouge); mach->Set(-1, 38, 17, OiRouge);     // langue
        Soustraire(*tete, *mach);
        r->Add("machoire", mach, V(0, 38.5f, 14), "tete");

        // ---------------------------------------------------------- ailes à demi déployées, relevées en menace
        Liste<Vector3> doigts = {V(23, 42, -6), V(24.5f, 37, -10.5f), V(22.5f, 31.5f, -14.5f), V(18, 25.5f, -16), V(12.5f, 21, -14.5f)};
        Liste<Vector3> flanc = {V(8, 18, -10), V(6.5f, 19, -3)};
        Vector3 poignet = V(16, 35, -2.5f);
        for (int s = 0; s < 2; s++)
        {
            float sx = s == 0 ? -1.0f : 1.0f;
            auto a = Aile(sx, V(6, 20, 3), V(11, 28, 1), poignet, doigts, flanc, 1.4f, OiBlancC, OiBlanc, OiBlancS, OiCorne);
            // rémiges : longues plumes grises au bout de l'aile
            for (int k = 0; k < 4; k++)
            {
                Vector3 b = doigts[k]; Vector3 w = poignet;
                Vector3 d = V(Lerp(w.x, b.x, 0.55f) * sx, Lerp(w.y, b.y, 0.55f), Lerp(w.z, b.z, 0.55f));
                a->Line(d.x, d.y, d.z, b.x * sx, b.y, b.z, 0.9f, 0.5f, k < 2 ? OiGris : OiGrisS);
            }
            a->Paint([&](int x, int y, int z, Color32 c)
            {
                if (!Meme(c, OiBlanc) && !Meme(c, OiBlancS)) return c;
                float px = std::abs(x + 0.5f);
                float loin = (Vector3(px, y, z) - poignet).magnitude();     // les rémiges, au bout de l'aile, sont grises
                if (loin > 8.5f && y > 24 && px > 15) return loin > 10.5f ? OiGrisS : OiGris;
                return (Mathf::FloorToInt(px) + z) % 4 == 0 ? OiBlancS : c;   // stries des plumes
            });
            r->Add(s == 0 ? "aileG" : "aileD", a, V(sx * 6, 20, 3), "corps");
        }

        // ---------------------------------------------------------- pattes orange, pieds palmés à griffes noires
        for (int s = 0; s < 2; s++)
        {
            float sx = s == 0 ? -1.0f : 1.0f;
            auto j = Nouveau<VoxelModel>();
            j->Ellipsoid(sx * 4.2f, 11.5f, -0.5f, 2.4f, 3, 2.8f, OiBlanc);          // cuisse emplumée
            Chaine(*j, {sx * 4.4f, 10, -0.5f, sx * 4.6f, 5, 0.5f, sx * 4.6f, 1.5f, 0.5f}, 1.6f, 1.2f, OiBec);
            j->Paint([&](int x, int y, int z, Color32 c) { return Meme(c, OiBec) && (y % 2 == 0) ? OiBecS : c; });     // écailles de la patte
            int px = Mathf::FloorToInt(sx * 4.6f);
            // palmure : trois doigts et la toile tendue entre eux
            for (int t = -1; t <= 1; t++) j->Line(px + 0.5f, 0.6f, 0.5f, px + 0.5f + t * 2.6f, 0.5f, 5.5f, 0.6f, OiBecS);
            for (int z = 1; z <= 5; z++)
            {
                float l = (z - 0.5f) / 5.0f * 2.6f;
                for (int x = Mathf::FloorToInt(px + 0.5f - l); x <= Mathf::FloorToInt(px + 0.5f + l); x++) j->Set(x, 0, z, OiBec);
            }
            for (int t = -1; t <= 1; t++) j->Set(Mathf::FloorToInt(px + 0.5f + t * 2.8f), 0, 6, OiCorne);
            r->Add(s == 0 ? "jambeG" : "jambeD", j, V(sx * 4.2f, 12, -0.5f), "corps");
        }

        // ---------------------------------------------------------- queue de dragon, écailles rouges, fer de lance
        auto queue = Nouveau<VoxelModel>();
        Chaine(*queue, {0, 14, -11, 0, 10.5f, -17, 1.5f, 7, -23, 3.5f, 5, -28, 2.5f, 5, -33, -0.5f, 7, -36}, 3, 0.9f, OiRouge);
        Ecailles(*queue, OiRouge, OiRougeS, C("d65a40"), 2);
        for (int k = 0; k < 6; k++)
        {
            int z = -14 - k * 4; int x = k < 2 ? 0 : (k < 4 ? 2 : 1);
            int h = Haut(*queue, x, z, 0, 30);
            if (h != INT_MIN) queue->Line(x + 0.5f, h, z, x + 0.5f, h + 1.8f, z - 1.2f, 0.6f, 0.3f, OiCorne);
        }
        for (int z = -41; z <= -36; z++)
        {
            float t = (z + 41) / 5.0f;
            float hh = t < 0.45f ? t * 6 : (1 - t) * 4.5f;
            for (int y = Mathf::FloorToInt(8 - hh); y <= Mathf::CeilToInt(8 + hh * 0.5f); y++) { queue->Set(-1, y, z, OiOr); queue->Set(0, y, z, z < -39 ? OiOr : OiOrS); }
        }
        r->Add("queue", queue, V(0, 14, -11), "corps");
        return Fin(r);
    }
}
