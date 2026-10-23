// Donjon de l'Oie — bâtiments et lieux remarquables du plateau : aiguillage, maisons à colombages de Valcreux, taverne.
// Les autres lieux sont dans DecorsBatiments.Valcreux.cpp, .Ombres.cpp (cryptes, marais) et .Pics.cpp (forge, antre du dragon).
// Repère : origine au sol au centre de l'emprise, façade / entrée vers +z.
#include "BatimentsInterne.h"

namespace DonjonDeLOie::Modeles::DecorsModeles::Batiments
{
    using namespace DonjonDeLOie::Voxel;

    namespace
    {
        // (définis plus bas, à leur place dans le C#)
        Ptr<VoxelRig> Maison(int graine);
        Ptr<VoxelRig> Taverne(int graine);
    }
}

namespace DonjonDeLOie::Modeles::DecorsModeles
{
    const Liste<string>& Speciaux()
    {
        static const Liste<string> s = {
            "maison", "tour_garde", "porte_ville", "rempart", "puits", "taverne", "chapelle", "autel_ombrelune", "portail_cryptes",
            "mausolee", "etal", "forge", "comptoir_fossoyeurs", "montagne_antre", "pont", "panneau", "ponton", "ruine", "banniere",
            "etal_marche", "caisses", "tonneaux", "sacs", "charrette_foin",
        };
        return s;
    }

    Ptr<Voxel::VoxelRig> ConstruireBatiment(const string& id, int graine)
    {
        using namespace Batiments;
        if (id == "maison") return Maison(graine);
        if (id == "taverne") return Taverne(graine);
        if (id == "tour_garde") return TourGarde(graine);
        if (id == "porte_ville") return PorteVille(graine);
        if (id == "rempart") return Rempart(graine);
        if (id == "puits") return Puits(graine);
        if (id == "banniere") return Banniere(graine);
        if (id == "panneau") return Panneau(graine);
        if (id == "etal") return Etal(graine);
        if (id == "arene_diable") return AreneDuDiable(graine);
        if (id == "etal_marche") return EtalMarche(graine);
        if (id == "caisses") return Caisses(graine);
        if (id == "tonneaux") return Tonneaux(graine);
        if (id == "sacs") return Sacs(graine);
        if (id == "charrette_foin") return CharretteFoin(graine);
        if (id == "pont") return Pont(graine);
        if (id == "chapelle") return Chapelle(graine);
        if (id == "autel_ombrelune") return AutelOmbrelune(graine);
        if (id == "portail_cryptes") return PortailCryptes(graine);
        if (id == "mausolee") return Mausolee(graine);
        if (id == "comptoir_fossoyeurs") return ComptoirFossoyeurs(graine);
        if (id == "ruine") return Ruine(graine);
        if (id == "ponton") return Ponton(graine);
        if (id == "forge") return Forge(graine);
        if (id == "montagne_antre") return MontagneAntre(graine);
        return nullptr;
    }
}

namespace DonjonDeLOie::Modeles::DecorsModeles::Batiments
{
    // ------------------------------------------------------------------ couleurs de Valcreux
    // (VcPierre, VcPierreSombre, VcEnduit, VcColombage, VcBois, VcTuile, VcArdoise, VcChaume, VcBardeau, VcVolets, VcFleurs,
    // VcFeuilles : dans BatimentsInterne.h, les autres fichiers s'en servent aussi)

    namespace
    {
        // ------------------------------------------------------------------ colombages
        /// Pan de bois sur une façade : enduit, poteaux, sablières, écharpes en diagonale. Rend l'espacement des poteaux.
        int Pans(ChantierBat& c, FaceBat f, int uA, int uB, int ya, int yb, Color32 bois, bool diagonales)
        {
            // (int)Math.Round(x) d'un float x : Mathf::RoundToInt(x) (même arrondi au pair, en double)
            int n = Mathf::Max(1, Mathf::RoundToInt((uB - uA) / 7.0f));
            float s = (uB - uA) / static_cast<float>(n);
            for (int k = 0; k <= n; k++)
            {
                int u = uA + Mathf::RoundToInt(k * s);
                for (int y = ya; y <= yb; y++) c.SetF(f, u, y, 1, Pal::Var(bois, u, y, 1, 0.05f));
            }
            for (int u = uA - 1; u <= uB + 1; u++)
            {
                c.SetF(f, u, ya, 1, Pal::Shade(bois, 0.9f));
                c.SetF(f, u, yb, 1, bois);
            }
            if (!diagonales) return static_cast<int>(s);
            for (int k = 0; k < n; k++)
            {
                if (k != 0 && k != n - 1) continue;
                int ua = uA + Mathf::RoundToInt(k * s), ub = uA + Mathf::RoundToInt((k + 1) * s);
                bool montant = k == 0;
                int len = Mathf::Max(ub - ua, yb - ya);
                for (int t = 0; t <= len; t++)
                {
                    int u = ua + Mathf::RoundToInt(t * (ub - ua) / static_cast<float>(len));
                    int y = montant ? ya + Mathf::RoundToInt(t * (yb - ya) / static_cast<float>(len)) : yb - Mathf::RoundToInt(t * (yb - ya) / static_cast<float>(len));
                    c.SetF(f, u, y, 1, Pal::Shade(bois, 0.92f));
                }
            }
            return static_cast<int>(s);
        }

        void Enduit(ChantierBat& c, int x0, int y0, int z0, int x1, int y1, int z1)
        {
            for (int x = x0; x <= x1; x++)
                for (int y = y0; y <= y1; y++)
                    for (int z = z0; z <= z1; z++)
                    {
                        bool bord = x <= x0 + 1 || x >= x1 - 1 || z <= z0 + 1 || z >= z1 - 1;
                        if (bord) c.M->Set(x, y, z, Pal::Var(VcEnduit, x / 2, y / 2, z / 2, 0.04f));
                    }
        }

        void Jardiniere(ChantierBat& c, FaceBat f, int u0, int w, int y)
        {
            for (int u = u0 - 1; u <= u0 + w; u++)
            {
                c.SetF(f, u, y, 1, VcBois);
                c.SetF(f, u, y, 2, Pal::Shade(VcBois, 0.85f));
                Color32 fl = ChantierBat::H(u, y, 3) < 0.45f ? VcFleurs : ChantierBat::H(u, y, 4) < 0.5f ? Pal::C("e8c84a") : VcFeuilles;
                c.SetF(f, u, y + 1, 2, fl);
                if (ChantierBat::H(u, y, 9) < 0.4f) c.SetF(f, u, y + 1, 1, VcFeuilles);
            }
        }

        /// Une maison complète : rez-de-chaussée de pierre (ou soubassement), colombages, toit à deux pans (faîtage selon x).
        void Batisse(ChantierBat& c, int x0, int x1, int z0, int z1, int hRdc, bool etage, int hEtage, Color32 toit, int style,
                     Color32 volet, bool cheminee, int porteU, bool arche)
        {
            FaceBat avant = FaceBat::Avant(z1);
            [[maybe_unused]] FaceBat arriere = FaceBat::Arriere(z0);   // (jamais employée, comme en C#)
            FaceBat droite = FaceBat::Droite(x1);
            FaceBat gauche = FaceBat::Gauche(x0);
            int soub = etage ? hRdc - 1 : 2;
            c.MursPierre(x0, 0, z0, x1, soub, z1, 2, VcPierre);
            c.Pierre(x0 - 1, 0, z0 - 1, x1 + 1, 0, z1 + 1, VcPierreSombre);
            int ya = soub + 1, yb = etage ? hRdc + hEtage - 1 : hRdc - 1;
            int ex = etage ? 1 : 0;   // encorbellement de l'étage
            int ea = etage ? hRdc : ya;
            if (etage)
            {
                Enduit(c, x0, ea, z0 - ex, x1, yb, z1 + ex);
                for (int x = x0 - 1; x <= x1 + 1; x++)
                {
                    c.Poutre(x, ea - 1, z1 + 1, x, ea - 1, z1 + 2, VcColombage);
                    c.Poutre(x, ea - 1, z0 - 2, x, ea - 1, z0 - 1, VcColombage);
                }
            }
            else Enduit(c, x0, ya, z0, x1, yb, z1);
            FaceBat av2 = FaceBat::Avant(z1 + ex);
            FaceBat ar2 = FaceBat::Arriere(z0 - ex);
            int s = Pans(c, av2, x0, x1, ea, yb, VcColombage, true);
            Pans(c, ar2, -x1, -x0, ea, yb, VcColombage, true);
            Pans(c, droite, -(z1 + ex), -(z0 - ex), ea, yb, VcColombage, false);
            Pans(c, gauche, z0 - ex, z1 + ex, ea, yb, VcColombage, false);
            if (!etage) Pans(c, avant, x0, x1, ya, yb, VcColombage, true);
            // toit
            int v0 = z0 - ex - 2, v1 = z1 + ex + 2;
            c.Pignon(true, x0, z0 - ex, z1 + ex, yb, 1, VcEnduit, false);
            c.Pignon(true, x1, z0 - ex, z1 + ex, yb, 1, VcEnduit, false);
            int pas = (v1 - v0 + 2) / 2, zc = (z0 + z1) / 2;
            for (int y = yb; y < yb + pas - 2; y++)
            {
                c.Poutre(x0 - 1, y, zc, x0 - 1, y, zc, VcColombage);
                c.Poutre(x1 + 1, y, zc, x1 + 1, y, zc, VcColombage);
            }
            for (int z = z0 - ex; z <= z1 + ex; z++)
            {
                c.Poutre(x0 - 1, yb + 1, z, x0 - 1, yb + 1, z, VcColombage);
                c.Poutre(x1 + 1, yb + 1, z, x1 + 1, yb + 1, z, VcColombage);
            }
            c.Toit(true, x0 - 2, x1 + 2, v0, v1, yb, 1, 2, toit, style);
            // bandeaux de rive (pignons)
            if (style != 1)
                for (int d = 0; d < pas; d++)
                {
                    int yh = yb + d;
                    for (int x : {x0 - 3, x1 + 3})
                    {
                        c.Poutre(x, yh, v0 + d, x, yh, v0 + d, VcColombage);
                        c.Poutre(x, yh, v1 - d, x, yh, v1 - d, VcColombage);
                    }
                }
            // fenêtres de l'étage (ou de la façade en colombage)
            int largF = 4, hF = etage ? 6 : 6;
            int yF = etage ? ea + 2 : ya + 4;
            for (int u = x0 + s / 2 - 1; u + largF <= x1 - 1; u += s)
            {
                if (!etage && Mathf::Abs(u + largF / 2 - porteU) < 6) continue;
                c.Fenetre(av2, u - 1, yF, largF, hF, VcColombage, volet, true);
                if (ChantierBat::H(u, yF, 7) < 0.6f) Jardiniere(c, av2, u - 1, largF, yF - 2);
            }
            for (int u = -x1 + s / 2; u + 4 <= -x0 - 1; u += s * 2) c.Fenetre(ar2, u - 1, yF, 4, hF, VcColombage, volet, false);
            c.Fenetre(droite, -(zc + 2), yF, 4, hF, VcColombage, volet, true);
            c.Fenetre(gauche, zc - 2, yF, 4, hF, VcColombage, volet, true);
            if (etage)
            {
                c.Fenetre(avant, x0 + 3, 5, 4, 5, VcPierreSombre, volet, false);
                if (x1 - porteU > 10) c.Fenetre(avant, x1 - 7, 5, 4, 5, VcPierreSombre, volet, false);
                c.Fenetre(gauche, zc - 2, 5, 4, 5, VcPierreSombre, volet, false);
            }
            // porte
            c.Porte(avant, porteU - 3, 1, 6, 15, 2, Pal::C("6e4a2e"), arche, etage ? VcPierreSombre : VcColombage);
            if (!etage)
                for (int u = porteU - 4; u <= porteU + 3; u++) c.SetF(avant, u, 16, 2, VcColombage);
            // cheminée de pierre contre le pignon droit
            if (cheminee)
            {
                int yt = yb + pas + 4;
                c.Pierre(x1 + 1, 0, zc - 3, x1 + 4, yt, zc + 1, VcPierre);
                c.Pierre(x1 + 1, yt - 2, zc - 4, x1 + 5, yt - 1, zc + 2, VcPierreSombre);
                c.Vide(x1 + 2, yt - 1, zc - 2, x1 + 3, yt, zc);
                c.M->Set(x1 + 2, yt - 2, zc - 2, Pal::Noir);
                c.M->Set(x1 + 3, yt - 2, zc - 1, Pal::Noir);
            }
        }

        /// Enseigne en fer forgé, pendue à une potence perpendiculaire à la façade (plan x).
        void Enseigne(ChantierBat& c, int x, int y, int z, int motif)
        {
            c.Poutre(x, y, z, x, y, z + 8, Pal::FerSombre);
            c.Trait(x + 0.5f, y - 3.5f, z + 0.5f, x + 0.5f, y + 0.5f, z + 4.5f, 0.5f, Pal::FerSombre);
            c.M->Set(x, y - 1, z + 3, Pal::FerSombre);
            c.M->Set(x, y - 1, z + 7, Pal::FerSombre);
            Color32 planche = Pal::C("7a5232");
            for (int yy = y - 9; yy <= y - 2; yy++)
                for (int zz = z + 2; zz <= z + 8; zz++)
                {
                    bool bord = yy == y - 9 || yy == y - 2 || zz == z + 2 || zz == z + 8;
                    if ((yy == y - 9 || yy == y - 2) && (zz == z + 2 || zz == z + 8)) continue;
                    c.M->Set(x, yy, zz, bord ? Pal::Shade(planche, 0.75f) : planche);
                }
            // le motif, peint sur les deux faces (x − 1 et x + 1)
            Liste<int> m;
            Color32 col;
            if (motif == 0)   // pain
            {
                col = Pal::C("e0b86a");
                m = {0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0};
            }
            else if (motif == 1)   // clé
            {
                col = Pal::Or;
                m = {0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1};
            }
            else if (motif == 2)   // fer à cheval
            {
                col = Pal::Acier;
                m = {0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0};
            }
            else   // chope
            {
                col = Pal::C("e8b04a");
                m = {1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0};
            }
            for (int i = 0; i < 21; i++)
            {
                if (m[i] == 0) continue;
                int col7 = i % 7, row = i / 7;
                [[maybe_unused]] int zz = z + 2 + col7, yy = y - 4 - row * 1 - (row == 2 ? 1 : 0);   // (calculés, jamais lus, comme en C#)
                if (motif != 3)
                {
                    c.M->Set(x - 1, y - 4 - row, z + 2 + col7, col);
                    c.M->Set(x + 1, y - 4 - row, z + 2 + col7, col);
                }
            }
            if (motif == 3)   // chope : corps doré, mousse blanche, anse
                for (int side = -1; side <= 1; side += 2)
                {
                    for (int yy = y - 8; yy <= y - 5; yy++)
                        for (int zz = z + 3; zz <= z + 5; zz++) c.M->Set(x + side, yy, zz, Pal::C("d99a32"));
                    for (int zz = z + 3; zz <= z + 5; zz++) c.M->Set(x + side, y - 4, zz, Pal::Blanc);
                    c.M->Set(x + side, y - 3, z + 4, Pal::Blanc);
                    c.M->Set(x + side, y - 7, z + 6, Pal::C("d99a32"));
                    c.M->Set(x + side, y - 6, z + 7, Pal::C("d99a32"));
                    c.M->Set(x + side, y - 5, z + 6, Pal::C("d99a32"));
                }
        }
    }

    void Tonneau(ChantierBat& c, int x, int y, int z, int r, int h, bool couche)
    {
        Color32 b = Pal::C("8a5a34");
        if (!couche)
        {
            for (int yy = y; yy < y + h; yy++)
            {
                float rr = r + (yy == y || yy == y + h - 1 ? -0.6f : (yy - y > 0 && yy - y < h - 1 ? 0.3f : 0.0f));
                Color32 col = yy == y + 1 || yy == y + h - 2 ? Pal::FerSombre : b;
                auto t = Nouveau<VoxelModel>();
                t->Cylinder(static_cast<float>(x), static_cast<float>(z), yy, yy, rr, col);
                for (const VoxelLu& kv : t->Tous())
                    c.M->Set(kv.Key.x, kv.Key.y, kv.Key.z, kv.Value == col && col == b ? Pal::Shade(b, 0.88f + 0.2f * ChantierBat::H(kv.Key.x, 0, kv.Key.z)) : kv.Value);
            }
            auto cou = Nouveau<VoxelModel>();
            cou->Cylinder(static_cast<float>(x), static_cast<float>(z), y + h - 1, y + h - 1, r - 0.8f, Pal::C("6a4228"));
            c.M->Merge(*cou, 0, 0, 0);
        }
        else
        {
            for (int xx = x - h / 2; xx < x - h / 2 + h; xx++)
            {
                int k = xx - (x - h / 2);
                float rr = r + (k == 0 || k == h - 1 ? -0.6f : 0.3f);
                Color32 col = k == 1 || k == h - 2 ? Pal::FerSombre : b;
                for (int yy = y; yy <= y + 2 * r + 1; yy++)
                    for (int zz = z - r - 1; zz <= z + r + 1; zz++)
                    {
                        float dy = yy + 0.5f - (y + r), dz = zz + 0.5f - z;
                        if (dy * dy + dz * dz <= rr * rr)
                            c.M->Set(xx, yy, zz, k == 0 || k == h - 1 ? Pal::C("6a4228") : col == b ? Pal::Shade(b, 0.88f + 0.2f * ChantierBat::H(0, yy, zz)) : col);
                    }
            }
        }
    }

    namespace
    {
        // ------------------------------------------------------------------ maison
        Ptr<VoxelRig> Maison(int graine) { return DecorsModeles::Maison(graine, 99, true); }
    }
}

namespace DonjonDeLOie::Modeles::DecorsModeles
{
    /// Une maison de Valcreux. demiXMax borne sa demi-largeur (en voxels) pour l'aligner dans une rue ; sans
    /// « abords », ni tonneau ni bûches au pied (les maisons serrées du diorama).
    Ptr<Voxel::VoxelRig> Maison(int graine, int demiXMax, bool abords)
    {
        using namespace Batiments;
        ChantierBat c(graine);
        AleaNet& R = c.R;
        int demiX = Mathf::Min(demiXMax, 12 + R.Next(3)), demiZ = 10 + R.Next(2);
        bool etage = R.Next(3) > 0;
        int style = R.Next(4);
        Color32 toit = style == 0 ? VcTuile : style == 1 ? VcChaume : style == 2 ? VcArdoise : VcBardeau;
        if (style == 0) toit = Pal::Shade(toit, 0.9f + 0.2f * static_cast<float>(R.NextDouble()));
        Color32 volet = VcVolets[R.Next(static_cast<int>(std::size(VcVolets)))];
        int porteU = R.Next(2) == 0 ? -demiX + 7 : demiX - 8;
        c.Miroir = R.Next(2) == 0;
        Batisse(c, -demiX, demiX - 1, -demiZ, demiZ - 1, etage ? 16 : 19, etage, 10, toit, style, volet, true, porteU, etage);
        if (R.Next(2) == 0) Enseigne(c, porteU > 0 ? porteU - 6 : porteU + 6, etage ? 15 : 17, demiZ, R.Next(3));
        // quelques détails au pied : tonneau, caisse, bûches
        if (!abords) return c.Rig("maison", 0.1f);
        if (R.Next(2) == 0) Tonneau(c, porteU > 0 ? -demiX + 4 : demiX - 5, 0, demiZ + 3, 2, 6, false);
        else c.Bois(demiX - 7, 0, demiZ + 1, demiX - 3, 3, demiZ + 4, Pal::C("9a7048"), 'y');
        for (int i = 0; i < 4; i++)
            c.Trait(-demiX - 2.5f, 0.5f + i * 1.0f, static_cast<float>(-demiZ + 2 + (i % 2)), -demiX - 2.5f, 0.5f + i * 1.0f,
                    static_cast<float>(-demiZ + 8 + (i % 2)), 0.6f, Pal::C(i % 2 == 0 ? "8a6a46" : "7a5a3a"));
        return c.Rig("maison", 0.1f);
    }
}

namespace DonjonDeLOie::Modeles::DecorsModeles::Batiments
{
    namespace
    {
        // ------------------------------------------------------------------ taverne
        Ptr<VoxelRig> Taverne(int graine)
        {
            ChantierBat c(graine);
            Batisse(c, -18, 17, -12, 11, 16, true, 11, VcTuile, 0, Pal::C("6e5a3a"), true, -4, true);
            // seconde aile basse à gauche, toit de chaume
            Enseigne(c, 6, 16, 12, 3);
            // lanterne près de la porte
            auto lu = c.Lueur("lueur_lanterne");
            c.Poutre(-9, 13, 12, -9, 13, 14, Pal::FerSombre);
            c.Poutre(-10, 10, 14, -8, 10, 15, Pal::FerSombre);
            c.Poutre(-10, 13, 14, -8, 13, 15, Pal::FerSombre);
            lu->Box(-10, 11, 14, -8, 12, 15, Pal::C("ffb85a"));
            // tonneaux devant, banc
            Tonneau(c, 10, 0, 16, 3, 8, false);
            Tonneau(c, 15, 0, 15, 3, 8, false);
            Tonneau(c, 12, 8, 15, 2, 7, true);
            Tonneau(c, -15, 0, 15, 3, 8, false);
            c.Bois(-14, 3, 13, -10, 3, 14, Pal::C("8b6040"), 'x', 1);   // banc
            c.Poutre(-14, 0, 13, -14, 2, 14, VcColombage);
            c.Poutre(-10, 0, 13, -10, 2, 14, VcColombage);
            // auvent de bardeaux au-dessus de la porte
            for (int z = 12; z <= 15; z++)
                for (int x = -8; x <= 1; x++) c.M->Set(x, 17 - (z - 12) / 2, z, Pal::Shade(VcBardeau, z % 2 == 0 ? 1.0f : 0.88f));
            c.Poutre(-8, 0, 15, -8, 15, 15, VcColombage);
            c.Poutre(1, 0, 15, 1, 15, 15, VcColombage);
            return c.Rig("taverne", 0.1f);
        }
    }
}
