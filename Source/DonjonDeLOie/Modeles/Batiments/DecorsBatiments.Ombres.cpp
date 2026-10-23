// Donjon de l'Oie — lieux sacrés et lugubres : chapelle des Saules, autel d'Ombrelune, portail des cryptes, mausolée,
// comptoir des fossoyeurs, ruine, ponton du marais.
#include "BatimentsInterne.h"

namespace DonjonDeLOie::Modeles::DecorsModeles::Batiments
{
    using namespace DonjonDeLOie::Voxel;

    namespace
    {
        constexpr Color32 CrPierre = Pal::C("7e848c"), CrPierreSombre = Pal::C("575c66"), CrPierreNoire = Pal::C("33343c");
        constexpr Color32 CrArgent = Pal::C("c8d0d8"), CrMousse = Pal::C("56684a");
        constexpr Color32 ChBlanc = Pal::C("d8d2c4"), ChBlancSombre = Pal::C("b0a898");
        constexpr Color32 MaBois = Pal::C("6a6440"), MaBoisSombre = Pal::C("4a4630"), MaVase = Pal::C("4e5534");
    }

    // ------------------------------------------------------------------ chapelle
    Ptr<VoxelRig> Chapelle(int graine)
    {
        ChantierBat c(graine);
        c.Assise = 3;
        c.Longueur = 5;
        c.Mousse = 0.14f;
        c.CouleurMousse = Pal::C("6a7a44");
        // nef : x −9..8, z −16..12 ; faîtage selon z
        c.Pierre(-11, 0, -18, 10, 1, 14, ChBlancSombre);
        c.MursPierre(-9, 2, -16, 8, 18, 12, 2, ChBlanc);
        // contreforts
        for (int z : {-12, -4, 4})
            for (int x : {-11, 9})
                for (int y = 2; y < 15; y++)
                {
                    int d = (y - 2) / 5;
                    c.Pierre(x + (x < 0 ? d : -d), y, z, x + (x < 0 ? 1 : 1) - (x < 0 ? 0 : 0), y, z + 1, ChBlancSombre);
                }
        c.Pignon(false, 12, -9, 8, 19, 1, ChBlanc, true);
        c.Pignon(false, -16, -9, 8, 19, 1, ChBlanc, true);
        c.Toit(false, -18, 14, -11, 10, 19, 1, 2, Pal::C("5a6878"), 2);
        // clocher-mur sur la façade, avec cloche
        c.Pierre(-4, 28, 11, 3, 39, 12, ChBlanc);
        c.Vide(-2, 31, 11, 1, 36, 12);
        for (int x = -2; x <= 1; x++)
        {
            float dx = x + 0.5f;
            if (dx * dx > 1.2f) c.Vide(x, 37, 11, x, 37, 12);
            else c.Vide(x, 37, 11, x, 37, 12);
        }
        c.Pierre(-5, 40, 10, 4, 40, 13, ChBlancSombre);
        c.Pierre(-1, 41, 11, 0, 44, 12, ChBlanc);   // croix
        c.Pierre(-2, 43, 11, 1, 43, 12, ChBlanc);
        Color32 bronze = Pal::C("b08a3e");
        c.M->Box(-1, 33, 11, 0, 36, 12, bronze);
        c.M->Box(-2, 31, 11, 1, 32, 12, bronze);
        c.M->Set(-1, 30, 11, Pal::FerSombre);
        c.M->Box(-2, 37, 11, 1, 37, 12, Pal::BoisSombre);
        // portail en arc brisé
        FaceBat av = FaceBat::Avant(12);
        c.Porte(av, -3, 2, 6, 14, 2, Pal::C("5e4430"), true, ChBlancSombre);
        // rosace / vitrail au-dessus de la porte (lueur discrète)
        auto vit = c.Lueur("lueur_vitrail");
        for (int x = -3; x <= 2; x++)
            for (int y = 18; y <= 23; y++)
            {
                float dx = x + 0.5f, dy = y - 20.5f;
                float q = dx * dx + dy * dy;
                if (q > 9.5f) continue;
                c.Vide(x, y, 12, x, y, 13);
                if (q < 1.2f || Mathf::Abs(dx) < 0.6f || Mathf::Abs(dy) < 0.6f) c.M->Set(x, y, 11, Pal::C("3a3a46"));
                else
                {
                    c.M->Remove(x, y, 11);
                    c.P->Remove(x, y, 11);
                    vit->Set(x, y, 11, (x + y) % 2 == 0 ? Pal::C("6a9ae0") : Pal::C("e0b860"));
                }
            }
        // baies latérales en lancette (vitrail sombre, pas de lueur)
        for (const FaceBat& f : {FaceBat::Droite(8), FaceBat::Gauche(-9)})
            for (int u0 : {-10, -1, 7})
            {
                int u = f.Nx > 0 ? -u0 - 2 : u0;
                for (int y = 7; y < 15; y++)
                    for (int k = 0; k < 2; k++)
                    {
                        if (y == 14 && k == 1) continue;
                        c.VideF(f, u + k, y, 0);
                        c.SetF(f, u + k, y, -1, (y + k) % 3 == 0 ? Pal::C("4a5a8a") : Pal::C("2e3448"));
                    }
                for (int k = -1; k <= 2; k++) c.SetF(f, u + k, 6, 1, ChBlancSombre);
            }
        // saules au pied ? non : quelques touffes de roseaux et un bénitier
        c.Pierre(5, 2, 14, 6, 4, 15, ChBlancSombre);
        c.M->Set(5, 5, 14, Pal::Eau);
        for (int i = 0; i < 6; i++)
        {
            int x = -10 + i * 4 % 7, z = 14 + i % 2;
            c.M->Box(x, 0, z, x, 2 + i % 3, z, Pal::C("7a8a44"));
        }
        return c.Rig("chapelle", 0.1f);
    }

    // ------------------------------------------------------------------ autel d'Ombrelune
    Ptr<VoxelRig> AutelOmbrelune(int graine)
    {
        ChantierBat c(graine);
        c.Assise = 2;
        c.Longueur = 4;
        c.Mousse = 0.08f;
        c.CouleurMousse = CrMousse;
        // trois marches
        c.Pierre(-10, 0, -8, 9, 0, 7, CrPierreSombre);
        c.Pierre(-8, 1, -7, 7, 1, 5, CrPierreNoire);
        // table d'autel
        c.Pierre(-6, 2, -3, 5, 7, 2, CrPierreNoire);
        c.Pierre(-7, 8, -4, 6, 8, 3, CrPierreSombre);
        for (int x = -6; x <= 5; x++)
        {
            c.M->Set(x, 8, 4, CrArgent);
            c.M->Set(x, 3, 3, Pal::Shade(CrArgent, 0.8f));
        }
        // stèle et croissant de lune d'argent
        c.Pierre(-4, 9, -4, 3, 17, -3, CrPierreNoire);
        c.Pierre(-3, 18, -4, 2, 18, -3, CrPierreNoire);
        for (int x = -5; x <= 4; x++)
            for (int y = 13; y <= 24; y++)
            {
                float dx = x + 0.5f, dy = y - 18.5f;
                bool dansA = dx * dx + dy * dy <= 5.2f * 5.2f;
                float ex = dx - 2.2f, ey = dy + 0.6f;
                bool dansB = ex * ex + ey * ey <= 4.2f * 4.2f;
                if (dansA && !dansB)
                {
                    c.M->Set(x, y, -2, Pal::Var(CrArgent, x, y, 0, 0.06f));
                    c.M->Set(x, y, -3, Pal::Shade(CrArgent, 0.8f));
                }
            }
        // rune centrale sur l'autel
        auto lu = c.Lueur("lueur_cierges");
        // cierges de chaque côté
        const int cierges[][3] = {{-6, -3, 5}, {5, -3, 5}, {-5, 1, 3}, {4, 1, 3}, {-9, 4, 6}, {8, 4, 6}};
        for (const auto& k : cierges)
        {
            int yb = k[1] < 0 ? 9 : k[1] == 1 ? 9 : 1;
            int x = k[0], z = k[1] < 2 ? k[1] : 4, h = k[2];
            if (k[1] == 4)
            {
                yb = 1;
                z = 5;
            }
            c.M->Box(x, yb, z, x, yb + h - 1, z, Pal::C("e8e0cc"));
            c.M->Set(x, yb + h, z, Pal::Noir);
            lu->Set(x, yb + h + 1, z, Pal::C("ffcf7a"));
        }
        // dalle d'argent incrustée au sol devant
        for (int x = -2; x <= 1; x++) c.M->Set(x, 1, 6, CrArgent);
        // deux piliers brisés derrière
        for (int x : {-9, 7})
        {
            c.Pierre(x, 1, -7, x + 2, 14 + (x > 0 ? 4 : 0), -5, CrPierre);
            c.Rond(x + 1.5f, -6.0f, 0, 20, 1.5f);
        }
        return c.Rig("autel_ombrelune", 0.1f);
    }

    namespace
    {
        /// Une gargouille assise (tournée vers +z) sur un socle en (x, y, z).
        void Gargouille(ChantierBat& c, int x, int y, int z, Color32 col)
        {
            Color32 s = Pal::Shade(col, 0.82f);
            c.M->Box(x - 1, y, z - 1, x + 1, y + 2, z + 1, col);            // corps accroupi
            c.M->Box(x - 1, y + 3, z, x + 1, y + 4, z + 2, col);            // tête
            c.M->Set(x, y + 3, z + 3, s);                                   // museau, cornes
            c.M->Set(x - 1, y + 5, z, s);
            c.M->Set(x + 1, y + 5, z, s);
            c.M->Set(x - 1, y + 4, z + 2, Pal::Noir);
            c.M->Set(x + 1, y + 4, z + 2, Pal::Noir);
            c.M->Box(x - 2, y + 1, z - 2, x - 2, y + 4, z - 1, s);          // ailes repliées
            c.M->Box(x + 2, y + 1, z - 2, x + 2, y + 4, z - 1, s);
            c.M->Set(x - 1, y, z + 2, s);
            c.M->Set(x + 1, y, z + 2, s);
        }
    }

    // ------------------------------------------------------------------ portail des cryptes
    Ptr<VoxelRig> PortailCryptes(int graine)
    {
        ChantierBat c(graine);
        c.Assise = 3;
        c.Longueur = 4;
        c.Mousse = 0.18f;
        c.CouleurMousse = CrMousse;
        // tertre de pierre et terre dans lequel s'enfonce l'escalier
        c.Pierre(-13, 0, -14, 12, 2, 6, CrPierreSombre);
        // tertre en gradins de terre et d'herbe rase, dans lequel l'arche est enchâssée
        Color32 terre = Pal::C("5e5a4e"), herbe = Pal::C("5f6e4c");
        for (int k = 0; k < 6; k++)
            for (int x = -15 + k * 2; x <= 14 - k * 2; x++)
                for (int z = -16 + k; z <= 0 - k / 2; z++)
                    for (int y = 3 + k * 3; y <= 5 + k * 3; y++)
                        c.M->Set(x, y, z, y == 5 + k * 3 ? Pal::Var(herbe, x, y, z, 0.08f) : Pal::Var(terre, x / 2, y, z / 2, 0.08f));
        // escalier qui descend vers −z (sous le tertre) : on creuse et on pose des marches
        for (int k = 0; k < 9; k++)
        {
            int z = 6 - k * 2, y = 2 - k;
            c.Vide(-5, y + 1, z - 1, 4, 13, z);
            if (y >= 0) c.Pierre(-5, y, z - 1, 4, y, z, Pal::Shade(CrPierre, 0.95f - k * 0.06f));
        }
        c.Vide(-5, 0, -14, 4, 13, -1);
        c.Pierre(-6, 0, -14, 5, 0, -14, CrPierreNoire);
        // le fond est noir
        c.M->Box(-5, -6, -13, 4, 15, -13, Pal::Noir);
        // murets latéraux de l'escalier
        for (int x : {-7, 6})
            for (int z = -1; z <= 7; z++)
            {
                int hh = 2 + (7 - z) / 2;
                c.Pierre(x, 0, z, x + 1, hh, z, CrPierreSombre);
            }
        // arche à double rouleau
        for (int x = -9; x <= 8; x++)
            for (int y = 3; y <= 24; y++)
            {
                // Math.Max(0, dy) : version float
                float dx = x + 0.5f, dy = y - 14.0f;
                float q = dx * dx + Mathf::Max(0.0f, dy) * Mathf::Max(0.0f, dy);
                if (q <= 5.2f * 5.2f) continue;
                if (q <= 9.0f * 9.0f)
                    for (int z = -1; z <= 1; z++)
                    {
                        float ang = Mathf::Atan2(Mathf::Max(0.0f, dy), dx);
                        int k = Mathf::FloorToInt(ang * 5.0f);
                        Color32 col = dy > 0 ? Pal::Shade(CrPierre, ChantierBat::Mod(k, 2) == 0 ? 1.1f : 0.95f) : CrPierre;
                        if (dy > 0) c.M->Set(x, y, z, col);
                        else c.Pierre(x, y, z, x, y, z, col);
                    }
            }
        // crâne sculpté en clé de voûte
        c.M->Box(-2, 21, 2, 1, 23, 2, Pal::Os);
        c.M->Set(-2, 22, 3, Pal::Noir);
        c.M->Set(1, 22, 3, Pal::Noir);
        c.M->Box(-1, 20, 2, 0, 20, 2, Pal::Os);
        // grille de fer entrouverte
        for (int x = -5; x <= 0; x++)
            for (int y = 3; y <= 17; y++)
            {
                float dx = x + 0.5f, dy = y - 14.0f;
                if (dy > 0 && dx * dx + dy * dy > 5.2f * 5.2f) continue;
                if (ChantierBat::Mod(x, 2) == 0 || y == 4 || y == 12) c.M->Set(x, y, 0, Pal::FerSombre);
                if (y == 17 && ChantierBat::Mod(x, 2) == 0) c.M->Set(x, y + 1, 0, Pal::Fer);
            }
        for (int k = 0; k < 6; k++)
            for (int y = 3; y <= 15; y++)
                if (k % 2 == 0 || y == 4 || y == 12) c.M->Set(1 + (k * 2) / 3, y, 1 + k, Pal::FerSombre);   // vantail droit pivoté
        // gargouilles sur des socles aux angles
        for (int x : {-11, 10})
        {
            c.Pierre(x - 1, 0, 2, x + 1, 9, 4, CrPierreSombre);
            Gargouille(c, x, 10, 3, CrPierre);
        }
        // quelques os épars
        c.Trait(-9, 0.5f, 9, -6, 0.5f, 10, 0.5f, Pal::Os);
        c.M->Set(-10, 0, 9, Pal::Os);
        c.M->Set(-5, 0, 10, Pal::Os);
        c.M->Box(7, 0, 9, 8, 1, 10, Pal::Os);
        c.M->Set(7, 1, 11, Pal::Noir);
        return c.Rig("portail_cryptes", 0.1f);
    }

    // ------------------------------------------------------------------ mausolée
    Ptr<VoxelRig> Mausolee(int graine)
    {
        ChantierBat c(graine);
        c.Assise = 3;
        c.Longueur = 5;
        c.Mousse = 0.15f;
        c.CouleurMousse = CrMousse;
        c.Pierre(-13, 0, -13, 12, 1, 12, CrPierreSombre);
        c.Pierre(-11, 2, -11, 10, 2, 10, CrPierre);
        c.Pierre(-8, 3, -9, 7, 18, 6, CrPierre);   // cella
        // colonnes en façade et sur les côtés
        for (int k = 0; k < 4; k++)
        {
            float x = -9.5f + k * 6.33f;
            c.PierreCyl(x, 9.0f, 3, 18, 1.5f, Pal::Shade(CrPierre, 1.12f));
            // (int)Math.Floor(x) : Mathf::FloorToInt(x)
            c.Pierre(Mathf::FloorToInt(x) - 1, 3, 8, Mathf::FloorToInt(x) + 1, 3, 10, CrPierreSombre);
            c.Pierre(Mathf::FloorToInt(x) - 1, 18, 8, Mathf::FloorToInt(x) + 1, 18, 10, CrPierreSombre);
        }
        c.Pierre(-11, 19, -11, 10, 21, 11, CrPierreSombre);   // entablement
        for (int x = -11; x <= 10; x += 2) c.M->Set(x, 20, 12, CrPierreNoire);
        // toit pointu (pyramide à quatre pans) en dalles
        for (int k = 0; k < 14; k++)
        {
            int r = 11 - k * 11 / 14;
            for (int x = -r - 1; x <= r; x++)
                for (int z = -r - 1; z <= r; z++)
                {
                    bool bord = x == -r - 1 || x == r || z == -r - 1 || z == r;
                    if (!bord && k < 13) continue;
                    c.M->Set(x, 22 + k, z, Pal::Shade(Pal::C("5a6270"), (k % 2 == 0 ? 1.0f : 0.88f) * (0.94f + 0.1f * ChantierBat::H(x / 3, k, z / 3))));
                }
        }
        c.Pierre(-1, 36, -1, 0, 38, 0, CrPierreSombre);
        c.M->Box(-1, 39, -1, 0, 41, 0, CrArgent);
        c.M->Box(-2, 40, -1, 1, 40, 0, CrArgent);
        // porte de bronze et épitaphe
        FaceBat av = FaceBat::Avant(6);
        c.Porte(av, -3, 3, 6, 11, 1, Pal::C("6a5a3a"), true, CrPierreSombre);
        for (int x = -4; x <= 3; x++) c.M->Set(x, 16, 7, ChantierBat::Mod(x, 2) == 0 ? CrPierreNoire : CrPierreSombre);
        // urnes de part et d'autre de l'entrée
        for (int x : {-12, 11})
        {
            c.Pierre(x - 1, 2, 10, x + 1, 4, 12, CrPierreSombre);
            c.M->Ellipsoid(x + 0.5f, 6.5f, 11.5f, 1.6f, 2.0f, 1.6f, Pal::C("6e7a84"));
            c.M->Set(x, 9, 11, Pal::C("6e7a84"));
        }
        return c.Rig("mausolee", 0.1f);
    }

    namespace
    {
        void Cercueil(ChantierBat& c, int x, int y, int z, bool debout);   // (défini après ComptoirFossoyeurs, comme en C#)
    }

    // ------------------------------------------------------------------ comptoir des fossoyeurs
    Ptr<VoxelRig> ComptoirFossoyeurs(int graine)
    {
        ChantierBat c(graine);
        c.Assise = 2;
        c.Longueur = 4;
        c.Mousse = 0.2f;
        c.CouleurMousse = CrMousse;
        Color32 bs = Pal::C("4e4038"), bs2 = Pal::C("3a302a");
        c.Pierre(-12, 0, -10, 11, 2, 9, CrPierreSombre);
        // murs de planches verticales
        c.Bois(-11, 3, -9, 10, 18, -8, bs, 'x');
        c.Bois(-11, 3, 7, 10, 18, 8, bs, 'x');
        c.Bois(-11, 3, -9, -10, 18, 8, bs, 'z');
        c.Bois(9, 3, -9, 10, 18, 8, bs, 'z');
        for (int x : {-12, 11})
            for (int z : {-10, 9}) c.Poutre(x, 0, z, x, 19, z, bs2);
        c.Toit(true, -13, 12, -12, 11, 19, 1, 2, Pal::C("4a4a50"), 3);
        c.Pignon(true, -11, -9, 8, 19, 1, bs, false);
        c.Pignon(true, 10, -9, 8, 19, 1, bs, false);
        // auvent avant et comptoir
        FaceBat av = FaceBat::Avant(8);
        for (int x = -9; x <= 5; x++)
            for (int y = 7; y <= 13; y++) c.VideF(av, x, y, 0);
        for (int x = -9; x <= 5; x++)
            for (int y = 7; y <= 13; y++) c.VideF(av, x, y, -1);
        c.M->Box(-9, 7, 7, 5, 13, 7, Pal::Noir);
        c.Bois(-10, 6, 9, 6, 6, 11, Pal::C("5a4a3e"), 'x', 1);
        for (int x = -10; x <= 6; x++)
            for (int z = 9; z <= 13; z++) c.M->Set(x, 15 - (z - 9) / 2, z, Pal::Shade(Pal::C("4a4a50"), z % 2 == 0 ? 1.0f : 0.86f));
        c.Poutre(-10, 0, 13, -10, 13, 13, bs2);
        c.Poutre(6, 0, 13, 6, 13, 13, bs2);
        c.Porte(av, 7, 3, 3, 13, 1, bs2, false, bs2);
        // pelles, cercueils
        c.Trait(7.5f, 8.5f, 10.0f, 8.5f, 0.5f, 11.0f, 0.55f, Pal::Bois);
        c.M->Box(8, 0, 11, 9, 2, 11, Pal::FerSombre);
        Cercueil(c, -14, 0, -2, true);
        Cercueil(c, 12, 0, -6, false);
        Cercueil(c, 13, 0, 2, false);
        Cercueil(c, 11, 0, 13, true);   // dressé près de l'entrée, bien visible
        // croix de bois rudimentaire, lanterne
        c.Poutre(-14, 0, 10, -14, 7, 10, bs);
        c.Poutre(-16, 5, 10, -12, 5, 10, bs);
        auto lu = c.Lueur("lueur_lanterne");
        c.Poutre(-8, 14, 12, -8, 14, 12, Pal::FerSombre);
        c.Poutre(-9, 10, 12, -7, 10, 13, Pal::FerSombre);
        c.Poutre(-9, 13, 12, -7, 13, 13, Pal::FerSombre);
        lu->Box(-9, 11, 12, -7, 12, 13, Pal::C("b8e090"));
        return c.Rig("comptoir_fossoyeurs", 0.1f);
    }

    namespace
    {
        /// Un cercueil (forme d'hexagone allongé) : couché au sol (debout = false) ou dressé contre un mur (debout = true, axe y).
        void Cercueil(ChantierBat& c, int x, int y, int z, bool debout)
        {
            Color32 b = Pal::C("5a4636");
            int L = 13;
            for (int k = 0; k < L; k++)
            {
                int w = k < 4 ? 2 + k / 2 : 4 - (k - 4) / 4;
                for (int a = -w; a < w; a++)
                    for (int h = 0; h < 3; h++)
                    {
                        Color32 col = h == 2 ? Pal::Shade(b, 1.1f) : b;
                        if (h == 2 && a == -1 && (k == 4 || k == 5)) col = Pal::FerSombre;
                        if (h == 2 && a == 0 && k >= 3 && k <= 6) col = Pal::FerSombre;
                        if (debout) c.M->Set(x + h, y + L - 1 - k, z + a, col);
                        else c.M->Set(x + a, y + h, z + k - L / 2, col);
                    }
            }
        }
    }

    // ------------------------------------------------------------------ ruine
    Ptr<VoxelRig> Ruine(int graine)
    {
        ChantierBat c(graine);
        c.Assise = 3;
        c.Longueur = 5;
        c.Mousse = 0.3f;
        Color32 col = Pal::C("8f8a7e");
        c.Pierre(-14, 0, -3, 13, 22, 2, col);
        // arche brisée
        for (int x = -6; x <= 5; x++)
            for (int y = 0; y <= 24; y++)
            {
                float dx = x + 0.5f, dy = y - 13.0f;
                if (dy < 0 || dx * dx + dy * dy <= 6.0f * 6.0f) c.Vide(x, y, -4, x, y, 3);
            }
        // profil cassé en haut (dents irrégulières) selon le bruit
        c.P->Carve([&](int x, int y, int z)
        {
            float bord = 22 - Mathf::Abs(x + 0.5f) * 0.25f;
            float n = ChantierBat::H(ChantierBat::Div(x, 3), 3, graine) * 9.0f;
            float lim = x < 0 ? bord - n - 2 : bord - n * 0.6f + (x > 8 ? -8 : 2);
            return y > lim;
        });
        c.P->Carve([&](int x, int y, int z) { return x > 9 && y > 6 + (13 - x); });
        // voussoirs restants de l'arche (côté gauche seulement)
        for (int x = -9; x <= 0; x++)
            for (int y = 13; y <= 22; y++)
            {
                float dx = x + 0.5f, dy = y - 13.0f;
                float q = dx * dx + dy * dy;
                if (q > 6.0f * 6.0f && q <= 8.5f * 8.5f && Mathf::Atan2(dy, dx) > 1.9f)
                    for (int z = -3; z <= 2; z++)
                        c.M->Set(x, y, z, Pal::Shade(col, ChantierBat::Mod(Mathf::FloorToInt(Mathf::Atan2(dy, dx) * 6.0f), 2) == 0 ? 1.1f : 0.98f));
            }
        // blocs tombés au sol
        AleaNet& R = c.R;
        for (int i = 0; i < 9; i++)
        {
            int x = -15 + R.Next(30), z = 4 + R.Next(8) - (i % 3 == 0 ? 14 : 0);
            int w = 2 + R.Next(3), h = 1 + R.Next(3);
            if (Mathf::Abs(x) < 6 && z > -5 && z < 5) continue;
            c.Pierre(x, 0, z, x + w, h, z + 2, Pal::Shade(col, 0.9f + 0.15f * static_cast<float>(R.NextDouble())));
        }
        // lierre et herbes
        for (int y = 0; y < 20; y++)
            for (int x = -14; x <= -7; x++)
                if (ChantierBat::H(x, y, 5) < 0.55f - y * 0.025f) c.M->Set(x, y, 3, ChantierBat::H(x, y, 6) < 0.3f ? Pal::C("4a6e30") : Pal::C("5d8a3c"));
        return c.Rig("ruine", 0.1f);
    }

    // ------------------------------------------------------------------ ponton
    Ptr<VoxelRig> Ponton(int graine)
    {
        ChantierBat c(graine);
        AleaNet& R = c.R;
        int L = 15;
        // planches transversales (axe x), un peu disjointes et de travers, sur deux longerons
        for (int z = -L; z < L; z++)
        {
            if (ChantierBat::Mod(z, 3) == 2) continue;
            int k = ChantierBat::Div(z, 3);
            if (z > L - 4 && k % 2 == 0) continue;   // planche manquante au bout
            int dec = static_cast<int>(ChantierBat::H(k, 3, graine) * 3) - 1;
            int y = 5 + (ChantierBat::H(k, 4, graine) < 0.2f ? -1 : 0);
            Color32 col = Pal::Shade(MaBois, 0.85f + 0.25f * ChantierBat::H(k, 7, graine));
            for (int x = -6 + dec; x <= 5 + dec; x++) c.M->Set(x, y, z, col);
        }
        for (int x : {-5, 4}) c.Poutre(x, 4, -L, x, 4, L - 1, MaBoisSombre);
        // pilotis plantés dans la vase
        for (int z = -L; z < L; z += 8)
            for (int x : {-6, 5})
            {
                int h = 7 + R.Next(3);
                c.Poutre(x, -2, z, x, h, z, MaBoisSombre);
                c.M->Set(x, h + 1, z, Pal::Shade(MaBoisSombre, 0.8f));
                for (int dx = -1; dx <= 1; dx++)
                    for (int dz = -1; dz <= 1; dz++) c.M->Set(x + dx, 0, z + dz, MaVase);
            }
        // corde et lanterne éteinte au bout, roseaux
        c.Trait(-5.5f, 8.5f, -L + 0.5f, -5.5f, 8.5f, -L + 8.5f, 0.45f, Pal::C("a89060"));
        c.M->Set(5, 10, L - 3, Pal::FerSombre);
        c.M->Box(5, 7, L - 3, 5, 9, L - 3, Pal::C("6a6a4a"));
        for (int i = 0; i < 14; i++)
        {
            // (le « ? : » et les déclarateurs séparés par des virgules gardent l'ordre des tirages)
            int x = R.Next(2) == 0 ? -8 - R.Next(3) : 7 + R.Next(3), z = -L + R.Next(2 * L), h = 4 + R.Next(6);
            c.M->Box(x, 0, z, x, h, z, Pal::C(i % 3 == 0 ? "8a8a4a" : "6e7a3a"));
            if (i % 4 == 0)
            {
                c.M->Set(x, h + 1, z, Pal::C("5a3a22"));
                c.M->Set(x, h + 2, z, Pal::C("5a3a22"));
            }
        }
        // barque amarrée (petite)
        auto barque = Nouveau<VoxelModel>();
        barque->Ellipsoid(11.5f, 3.5f, 0.5f, 3.0f, 3.0f, 7.0f, Pal::Shade(MaBois, 0.9f));
        barque->Carve([&](int x, int y, int z)
                      { return y > 3 || y < 1 || (y >= 2 && (x + 0.5f - 11.5f) * (x + 0.5f - 11.5f) / 4.0f + (z + 0.5f - 0.5f) * (z + 0.5f - 0.5f) / 36.0f < 1.0f); });
        barque->Box(10, 3, 0, 13, 3, 0, MaBoisSombre);
        c.M->Merge(*barque, 0, 0, 0);
        c.Trait(8.5f, 4.5f, 2.0f, 5.5f, 5.5f, 4.0f, 0.4f, Pal::C("a89060"));
        return c.Rig("ponton", 0.1f);
    }
}
