// Donjon de l'Oie — figurines des cases : un « pion » par type de case, posé sur un coin de la dalle.
// VoxelSize 0,05 ; 0,4 à 0,8 m de haut ; une couleur dominante par type ; pièce « corps » + éventuelles « lueur* ».
#include "../IconesCases.h"

#include "../../Voxel/Pal.h"

#include <algorithm>
#include <cmath>

namespace DonjonDeLOie::Modeles::IconesCases
{
    using namespace DonjonDeLOie::Voxel;

    namespace
    {
        constexpr Color32 Bleu = Pal::C("2f5fa8"), BleuSombre = Pal::C("22457c"), Or = Pal::C("e8b64a"), OrSombre = Pal::C("a77a23"), OrClair = Pal::C("f6d77a");
        constexpr Color32 Rouge = Pal::C("b8352c"), RougeSombre = Pal::C("7e2620"), Violet = Pal::C("7a4ab0"), VioletSombre = Pal::C("4e2e76");
        constexpr Color32 Orange = Pal::C("d9782e"), Brun = Pal::C("7a4e2d"), BrunClair = Pal::C("a0703e");
        constexpr Color32 PierreC = Pal::C("9a958c"), PierreS = Pal::C("6a665f");

        // (définies plus bas, après Construire, comme dans la version C#)
        void Socle(VoxelModel& m, Color32 lisere);
        void Cube(VoxelModel& m, int x0, int y0, int z0, int x1, int y1, int z1, Color32 c);
        void Motif(VoxelModel& m, const Liste<string>& lignes, int x0, int yHaut, int z0, int z1, Color32 a, Color32 b, Color32 c);
        void Ville(VoxelModel& m);
        void Chemin(VoxelModel& m);
        void Monstre(VoxelModel& m);
        void Tresor(VoxelModel& m, VoxelModel& l);
        void Evenement(VoxelModel& m, VoxelModel& l);
        void Malediction(VoxelModel& m, VoxelModel& l);
        void Sanctuaire(VoxelModel& m, VoxelModel& l);
        void Marchand(VoxelModel& m);
        void Taverne(VoxelModel& m);
        void Antre(VoxelModel& m, VoxelModel& l);
    }

    Ptr<VoxelRig> Construire(Regles::TypeCase t)
    {
        auto m = Nouveau<VoxelModel>();
        auto l = Nouveau<VoxelModel>();
        switch (t)
        {
            case Regles::TypeCase::Ville: Ville(*m); break;
            case Regles::TypeCase::Chemin: Chemin(*m); break;
            case Regles::TypeCase::Monstre: Monstre(*m); break;
            case Regles::TypeCase::Tresor: Tresor(*m, *l); break;
            case Regles::TypeCase::Evenement: Evenement(*m, *l); break;
            case Regles::TypeCase::Malediction: Malediction(*m, *l); break;
            case Regles::TypeCase::Sanctuaire: Sanctuaire(*m, *l); break;
            case Regles::TypeCase::Marchand: Marchand(*m); break;
            case Regles::TypeCase::Taverne: Taverne(*m); break;
            case Regles::TypeCase::Antre: Antre(*m, *l); break;
        }
        auto r = Nouveau<VoxelRig>();
        r->Name = "icone_" + Regles::NomEnum(t);
        r->VoxelSize = 0.05f;
        r->Origin = Vector3::zero;
        r->Add("corps", m, Vector3::zero);
        if (l->Count() > 0) r->Add("lueur", l, Vector3::zero, "corps");
        return r;
    }

    namespace
    {
        /// Socle octogonal de pion, deux étages chanfreinés, liseré de la couleur du type.
        void Socle(VoxelModel& m, Color32 lisere)
        {
            for (int x = -5; x <= 4; x++)
                for (int z = -5; z <= 4; z++)
                {
                    int ax = x < 0 ? -x - 1 : x, az = z < 0 ? -z - 1 : z;
                    if (ax + az <= 7) m.Set(x, 0, z, Pal::Shade(PierreS, 0.9f + 0.1f * Pal::Hash01(x, 0, z)));
                    if (ax + az <= 6) m.Set(x, 1, z, ax + az >= 5 || ax == 4 || az == 4 ? lisere : PierreC);
                }
        }

        void Cube(VoxelModel& m, int x0, int y0, int z0, int x1, int y1, int z1, Color32 c) { m.Box(x0, y0, z0, x1, y1, z1, c); }

        /// Motif 2D sur un plan z (lignes du haut vers le bas), '#' = couleur a, 'o' = couleur b, '+' = couleur c.
        void Motif(VoxelModel& m, const Liste<string>& lignes, int x0, int yHaut, int z0, int z1, Color32 a, Color32 b, Color32 c)
        {
            for (int r = 0; r < lignes.Length(); r++)
                for (int k = 0; k < Texte::Longueur(lignes[r]); k++)
                {
                    char ch = lignes[r][static_cast<size_t>(k)]; if (ch == '.') continue;
                    Color32 col = ch == '#' ? a : ch == 'o' ? b : c;
                    for (int z = z0; z <= z1; z++) m.Set(x0 + k, yHaut - r, z, col);
                }
        }

        // ------------------------------------------------------------------ Ville : mât et bannière de Valcreux
        void Ville(VoxelModel& m)
        {
            Socle(m, Or);
            Cube(m, -1, 2, -1, 0, 15, 0, Pal::BoisSombre);
            Cube(m, -1, 15, -1, 0, 15, 0, Or);
            Cube(m, -5, 14, 0, 4, 14, 0, Pal::BoisSombre);                       // traverse
            m.Set(-6, 14, 0, Or); m.Set(5, 14, 0, Or); m.Set(-1, 16, 0, Or); m.Set(0, 16, -1, Or); m.Set(-1, 16, -1, Or); m.Set(0, 16, 0, Or); m.Set(0, 17, 0, Or);
            const Liste<string> b =
            {
                "+++++++++",
                "+#######+",
                "+#.ooo.#+",
                "+#oooo.#+",
                "+#.ooo.#+",
                "+#..oo.#+",
                "+#.oooo#+",
                "+#o.o.o#+",
                "+#######+",
                "+###.###+",
                "++#...#++",
                ".+.....+.",
            };
            Liste<string> lignes(b.Length());
            for (int i = 0; i < b.Length(); i++) lignes[i] = Texte::Remplacer(b[i], ".", i == 2 || i == 3 || i == 4 || i == 5 || i == 6 || i == 7 ? "#" : ".");
            Motif(m, lignes, -5, 13, 1, 1, Bleu, Or, OrSombre);
            for (int x = -5; x <= 3; x++)
                for (int y = 2; y <= 13; y++)
                    if (m.Has(x, y, 1)) { Color32 c; m.TryGet(x, y, 1, c); m.Set(x, y, 2, c == Bleu ? BleuSombre : c); }
            // le lion d'or ressort d'un voxel sur la face avant
            for (int x = -4; x <= 2; x++)
                for (int y = 6; y <= 11; y++) { Color32 c; if (m.TryGet(x, y, 1, c) && c == Or) m.Set(x, y, 3, Or); }
        }

        // ------------------------------------------------------------------ Chemin : poteau indicateur
        void Chemin(VoxelModel& m)
        {
            Socle(m, PierreC);
            Cube(m, -1, 2, -1, 0, 14, 0, Brun);
            Cube(m, -1, 15, -1, 0, 15, 0, Pal::Shade(Brun, 0.8f));
            // deux flèches : une vers +x (en haut), une vers −x
            Color32 p1 = Pal::C("c08a50"); Color32 p2 = Pal::C("a8743e");
            Cube(m, 1, 11, 0, 6, 13, 0, p1); m.Set(7, 12, 0, p1); Cube(m, 6, 11, 0, 6, 13, 0, p1);
            m.Set(7, 11, 0, p1); m.Set(7, 13, 0, p1); m.Set(8, 12, 0, p1);
            Cube(m, -7, 7, -1, -2, 9, -1, p2); m.Set(-8, 8, -1, p2); m.Set(-8, 7, -1, p2); m.Set(-8, 9, -1, p2); m.Set(-9, 8, -1, p2);
            // chapeau de bois et lanterne éteinte, pierre de borne au pied
            Cube(m, -2, 16, -2, 1, 16, 1, Pal::Shade(Brun, 0.75f)); Cube(m, -1, 17, -1, 0, 17, 0, Pal::Shade(Brun, 0.75f));
            Cube(m, -3, 2, 1, -2, 4, 2, PierreS); m.Set(-3, 5, 1, PierreC);
            // inscriptions sombres
            m.Set(2, 12, 1, Pal::BoisSombre); m.Set(4, 12, 1, Pal::BoisSombre); m.Set(-4, 8, 0, Pal::BoisSombre); m.Set(-6, 8, 0, Pal::BoisSombre);
            m.Set(2, 12, -1, Pal::BoisSombre); m.Set(4, 12, -1, Pal::BoisSombre);
            // cailloux au pied
            m.Set(2, 2, 2, PierreS); m.Set(-3, 2, 2, PierreC); m.Set(-2, 2, 3, PierreS);
        }

        // ------------------------------------------------------------------ Monstre : épées croisées devant un écu rouge à tête de mort
        void Monstre(VoxelModel& m)
        {
            Socle(m, Rouge);
            // écu (forme de blason), bord de fer
            for (int x = -6; x <= 5; x++)
                for (int y = 3; y <= 17; y++)
                {
                    int ax = x < 0 ? -x - 1 : x;
                    int bas = (y - 3) * 2 / 3;
                    if (bas < 5 && ax > bas + 1) continue;
                    bool bord = ax == 5 || y == 17 || (bas < 5 && ax >= bas);
                    m.Set(x, y, -1, bord ? Pal::FerSombre : RougeSombre);
                    m.Set(x, y, 0, bord ? Pal::Fer : Rouge);
                }
            // tête de mort
            const Liste<string> crane =
            {
                ".####.",
                "######",
                "#o##o#",
                "######",
                ".#oo#.",
                ".#.#..",
            };
            Motif(m, crane, -3, 14, 1, 1, Pal::Os, Pal::Noir, Pal::Os);
            m.Set(-2, 12, 1, Pal::Noir); m.Set(1, 12, 1, Pal::Noir);
            // deux épées croisées derrière l'écu : lames qui dépassent en haut, gardes et poignées en bas
            for (int sg = -1; sg <= 1; sg += 2)
            {
                float fsg = static_cast<float>(sg);      // (sg converti en réel, comme le fait C# dans les produits ci-dessous)
                m.Line(fsg * 6.5f + 0.5f, 3.5f, -2.5f, fsg * -6.5f + 0.5f, 20.5f, -2.5f, 1.1f, 0.7f, Pal::Acier);
                m.Line(fsg * -3.0f + 0.5f, 16.0f, -2.5f, fsg * -6.8f + 0.5f, 21.0f, -2.5f, 0.6f, Pal::C("e4eaee"));
                float gx = fsg * 5.3f + 0.5f, gy = 5.6f;
                m.Line(gx - 2.4f, gy - 2.0f * fsg * -1.0f, -1.5f, gx + 2.4f, gy + 2.0f * fsg * -1.0f, -1.5f, 0.8f, Or);
                m.Line(gx + fsg * 0.6f, gy - 0.8f, -1.5f, fsg * 7.5f + 0.5f, 2.6f, -1.5f, 0.6f, Pal::Cuir);
                m.Set(Mathf::FloorToInt(fsg * 7.8f + 0.5f), 2, -2, Or);
            }
        }

        // ------------------------------------------------------------------ Trésor : coffre entrouvert débordant d'or
        void Tresor(VoxelModel& m, VoxelModel& l)
        {
            Socle(m, Or);
            Color32 b = Pal::C("8a5530"); Color32 bS = Pal::C("5e3a22");
            Cube(m, -5, 2, -3, 4, 7, 3, b);
            for (int x = -5; x <= 4; x++)
                for (int y = 2; y <= 7; y++)
                    if (y % 3 == 1) { m.Set(x, y, 3, Pal::Shade(b, 0.82f)); m.Set(x, y, -3, Pal::Shade(b, 0.82f)); }
            for (int x : {-5, -2, 1, 4})
                for (int y = 2; y <= 7; y++) { m.Set(x, y, 3, OrSombre); m.Set(x, y, -3, OrSombre); }
            for (int z = -3; z <= 3; z++)
                for (int y = 2; y <= 7; y++) { m.Set(-5, y, z, z == 0 || y == 2 ? OrSombre : bS); m.Set(4, y, z, z == 0 || y == 2 ? OrSombre : bS); }
            Cube(m, -1, 5, 4, 0, 6, 4, Or); m.Set(-1, 5, 4, Pal::Noir);   // serrure
            // couvercle bombé, ouvert vers l'arrière (incliné)
            for (int x = -5; x <= 4; x++)
                for (int k = 0; k < 7; k++)
                {
                    int y = 8 + k, z = -3 - (k < 2 ? 0 : 1) - (k > 4 ? 1 : 0);
                    Color32 c = (x == -5 || x == 4 || x == -2 || x == 1) ? OrSombre : b;
                    m.Set(x, y, z, c); m.Set(x, y, z - 1, Pal::Shade(c, 0.85f));
                }
            // l'or qui déborde : dôme de pièces, quelques pièces au sol, une gemme
            for (int x = -4; x <= 3; x++)
                for (int z = -2; z <= 2; z++)
                {
                    float dx = (static_cast<float>(x) + 0.5f) / 4.5f, dz = (static_cast<float>(z) + 0.5f) / 3.2f; float d = dx * dx + dz * dz;
                    int hh = 8 + Mathf::RoundToInt(3.0f * (1.0f - d));
                    for (int y = 8; y <= hh; y++) m.Set(x, y, z, Pal::Hash01(x, y, z) < 0.3f ? OrClair : Or);
                }
            m.Set(-1, 12, 0, OrClair); m.Set(0, 12, 0, Or); m.Set(1, 11, 1, Pal::C("c8344a")); m.Set(-3, 10, 1, Pal::C("3a8ac8"));
            const int pieces[][2] = {{3, 4}, {-4, 5}, {5, 1}, {1, 5}};
            for (auto& p : pieces) { m.Set(p[0], 2, p[1], Or); }
            Cube(m, -6, 2, 3, -5, 3, 4, Or); m.Set(-6, 4, 4, OrClair);
            // reflets discrets
            l.Set(0, 12, -1, Pal::C("ffe08a")); l.Set(-3, 11, 0, Pal::C("ffe08a"));
        }

        // ------------------------------------------------------------------ Événement : pierre levée à rune « ? »
        void Evenement(VoxelModel& m, VoxelModel& l)
        {
            Socle(m, Bleu);
            Color32 p = Pal::C("8c8e94"); Color32 pS = Pal::C("62656c");
            for (int y = 2; y <= 17; y++)
            {
                int w = y < 15 ? 4 : y < 16 ? 3 : 2;
                int dec = 0;
                for (int x = -w; x < w; x++)
                    for (int z = -2; z <= 1; z++)
                    {
                        bool arete = (x == -w || x == w - 1) && (z == -2 || z == 1);
                        if (arete && y > 3) continue;   // chanfrein
                        m.Set(x + dec - (y > 13 ? 0 : 0), y, z, Pal::Shade((y / 3) % 2 == 0 ? p : Pal::Mix(p, pS, 0.4f), 0.95f + 0.1f * Pal::Hash01(x, y / 3, z)));
                    }
            }
            m.Set(-3, 2, 2, Pal::C("5d8a3c")); m.Set(-4, 3, 1, Pal::C("5d8a3c")); m.Set(3, 2, 2, Pal::C("6f9a43"));
            // le point d'interrogation, gravé et lumineux, sur la face avant (z = 2)
            const Liste<string> q =
            {
                ".####.",
                "##..##",
                "....##",
                "...##.",
                "..##..",
                "..##..",
                "......",
                "..##..",
                "..##..",
            };
            for (int r = 0; r < q.Length(); r++)
                for (int k = 0; k < 6; k++)
                    if (q[r][static_cast<size_t>(k)] == '#') { l.Set(-3 + k, 15 - r, 2, Pal::C("6ab0ff")); m.Set(-3 + k, 15 - r, 1, Pal::C("1e3a66")); }
        }

        // ------------------------------------------------------------------ Malédiction : crâne violet sur une pique, fumée figée
        void Malediction(VoxelModel& m, VoxelModel& l)
        {
            Socle(m, Violet);
            Cube(m, -1, 2, -1, 0, 9, 0, Pal::BoisSombre);
            m.Set(-1, 2, 1, Pal::BoisSombre); m.Set(1, 2, -1, Pal::BoisSombre); m.Set(-2, 2, -1, Pal::BoisSombre);
            // crâne : boîte chanfreinée 8 × 7 × 7, mâchoire
            Color32 os = Pal::C("b8a0d0"); Color32 osS = Pal::C("8a6aa8");
            for (int x = -4; x <= 3; x++)
                for (int y = 11; y <= 17; y++)
                    for (int z = -3; z <= 3; z++)
                    {
                        int ax = x < 0 ? -x - 1 : x;
                        bool coin = (ax == 3 && (y == 17 || z == -3 || z == 3)) || (y == 17 && (z == -3 || z == 3));
                        if (coin) continue;
                        m.Set(x, y, z, Pal::Shade(os, 0.94f + 0.08f * Pal::Hash01(x, y, z)));
                    }
            Cube(m, -3, 9, -1, 2, 10, 3, osS);
            for (int x = -3; x <= 2; x += 1)
                if (x % 2 == 0) m.Set(x, 10, 4, Pal::Os); else m.Set(x, 10, 4, osS);
            // orbites et nez creusés, yeux lumineux
            for (int x : {-3, 1}) { Cube(m, x, 13, 3, x + 1, 14, 3, Pal::Noir); l.Set(x, 13, 4, Pal::C("c070ff")); l.Set(x + 1, 13, 4, Pal::C("c070ff")); }
            m.Set(-1, 12, 3, Pal::Noir); m.Set(0, 12, 3, Pal::Noir);
            // fêlure
            m.Set(1, 17, 0, osS); m.Set(1, 16, 1, osS); m.Set(2, 16, 2, osS);
            // fumée figée : trois volutes en blocs de 2 × 2 qui s'enroulent autour de la pique et s'échappent du crâne
            for (int v = 0; v < 3; v++)
                for (int k = 0; k < 7; k++)
                {
                    float a = static_cast<float>(v) * 2.1f + static_cast<float>(k) * 0.7f; float r = 4.6f - static_cast<float>(k) * 0.25f;
                    int x = Mathf::FloorToInt(Mathf::Cos(a) * r), z = Mathf::FloorToInt(Mathf::Sin(a) * r);
                    int y = 2 + k * 2 + v;
                    if (y >= 8 && y <= 18 && x >= -5 && x <= 4 && z >= -4 && z <= 4) continue;
                    Color32 c = Pal::Shade((k + v) % 2 == 0 ? Violet : VioletSombre, 1.0f + static_cast<float>(k) * 0.04f);
                    m.Box(x, y, z, x + 1, y + 1, z + 1, c);
                }
            Cube(m, -2, 18, -2, 0, 19, 0, Violet); Cube(m, -1, 20, -1, 1, 21, 0, Pal::Shade(Violet, 1.15f)); Cube(m, -2, 22, 0, -1, 23, 1, Pal::Shade(Violet, 1.3f));
        }

        // ------------------------------------------------------------------ Sanctuaire : cristal blanc-bleu sur un petit autel
        void Sanctuaire(VoxelModel& m, VoxelModel& l)
        {
            Socle(m, Pal::C("dde4ea"));
            Color32 b = Pal::C("e8e4da"); Color32 bS = Pal::C("bcb6aa"); Color32 ar = Pal::C("c8d0d8");
            Cube(m, -4, 2, -4, 3, 3, 3, bS);
            Cube(m, -3, 4, -3, 2, 7, 2, b);
            Cube(m, -4, 8, -4, 3, 8, 3, bS);
            for (int x = -3; x <= 2; x++) { m.Set(x, 6, 3, ar); m.Set(x, 6, -4, ar); }
            for (int z = -3; z <= 2; z++) { m.Set(3, 6, z, ar); m.Set(-4, 6, z, ar); }
            // le cristal : grand prisme hexagonal à pointe, et deux éclats penchés ; deux tons de facette
            Color32 c1 = Pal::C("dff2ff"), c2 = Pal::C("9ccff5");
            for (int y = 9; y <= 22; y++)
            {
                float r = y <= 18 ? 2.3f : 2.3f * static_cast<float>(22 - y) / 4.0f + 0.3f;
                for (int x = -3; x <= 2; x++)
                    for (int z = -3; z <= 2; z++)
                    {
                        float dx = std::abs(static_cast<float>(x) + 0.5f), dz = std::abs(static_cast<float>(z) + 0.5f);
                        if (std::max(dx, (dx + dz * 1.7f) * 0.6f) > r) continue;
                        l.Set(x, y, z, x - z < 0 ? c2 : c1);
                    }
            }
            l.Line(2.5f, 9.5f, 0.5f, 5.0f, 15.0f, 1.5f, 1.2f, 0.5f, c2);
            l.Line(-3.0f, 9.5f, -1.5f, -5.5f, 13.5f, -2.5f, 1.0f, 0.4f, c1);
        }

        // ------------------------------------------------------------------ Marchand : sac de pièces, balance, petite caisse
        void Marchand(VoxelModel& m)
        {
            Socle(m, Orange);
            // caisse
            Color32 cb = Pal::C("a8784a");
            Cube(m, 0, 2, -4, 5, 6, 1, cb);
            for (int y = 2; y <= 6; y++) { m.Set(0, y, 2, Pal::Shade(cb, 0.8f)); m.Set(5, y, 2, Pal::Shade(cb, 0.8f)); }
            for (int x = 0; x <= 5; x++) { m.Set(x, 4, 2, Pal::Shade(cb, 0.85f)); m.Set(x, 2, 2, Pal::Shade(cb, 0.8f)); m.Set(x, 6, 2, Pal::Shade(cb, 0.8f)); }
            // balance posée sur la caisse
            Cube(m, 2, 7, -2, 3, 7, -1, OrSombre);
            Cube(m, 2, 8, -2, 2, 15, -2, Or);
            Cube(m, -2, 15, -2, 7, 15, -2, Or); m.Set(2, 16, -2, Or);
            for (int x : {-2, 7})
            {
                m.Set(x, 14, -2, OrSombre); m.Set(x, 13, -2, OrSombre);
                Cube(m, x - 1, 12, -3, x + 1, 12, -1, Or);
            }
            m.Set(-2, 13, -2, OrSombre); m.Set(-2, 14, -2, OrSombre);
            m.Set(7, 13, -2, OrClair);   // une pièce sur le plateau
            // sac de pièces devant, orange, noué, pièces débordantes
            Color32 sac = Orange; Color32 sacS = Pal::C("a85a22");
            m.Ellipsoid(-2.5f, 5.2f, 2.0f, 3.4f, 3.4f, 3.0f, sac);
            m.Paint([&](int x, int y, int z, Color32 c) { return c == sac && Pal::Hash01(x, y, z) < 0.3f ? sacS : c; });
            Cube(m, -3, 9, 1, -2, 9, 2, Pal::C("6a3a1a"));
            Cube(m, -4, 10, 0, -1, 10, 3, sac); m.Set(-3, 11, 1, Or); m.Set(-2, 11, 2, OrClair);
            // la marque « pièce » cousue sur le sac
            Cube(m, -4, 4, 5, -2, 6, 5, Or); m.Set(-3, 5, 5, OrSombre);
            // pièces au sol
            m.Set(1, 2, 4, Or); m.Set(2, 2, 3, OrClair); m.Set(-5, 2, -2, Or);
        }

        // ------------------------------------------------------------------ Taverne : enseigne à chope suspendue, tonnelet
        void Taverne(VoxelModel& m)
        {
            Socle(m, Brun);
            Color32 bs = Pal::C("5e3d26");
            Cube(m, -5, 2, -2, -4, 17, -1, bs);           // potence
            Cube(m, -5, 17, -2, 4, 17, -1, bs);
            m.Line(-4.0f, 13.5f, -1.5f, -0.5f, 17.0f, -1.5f, 0.5f, bs);
            m.Set(-1, 16, -2, Pal::FerSombre); m.Set(3, 16, -2, Pal::FerSombre);
            // enseigne : planche brune, bord sombre
            Color32 pl = BrunClair;
            for (int x = -2; x <= 5; x++)
                for (int y = 8; y <= 15; y++)
                {
                    bool bord = x == -2 || x == 5 || y == 8 || y == 15;
                    if ((x == -2 || x == 5) && (y == 8 || y == 15)) continue;
                    m.Set(x, y, -2, bord ? Pal::Shade(pl, 0.7f) : pl); m.Set(x, y, -1, bord ? Pal::Shade(pl, 0.7f) : pl);
                }
            // chope en relief sur les deux faces
            for (int z : {0, -3})
            {
                Cube(m, -1, 9, z, 2, 12, z, Pal::C("e0a43a"));
                Cube(m, -1, 13, z, 2, 13, z, Pal::Blanc); m.Set(0, 14, z, Pal::Blanc); m.Set(-1, 14, z, Pal::Blanc);
                m.Set(3, 10, z, Pal::C("e0a43a")); m.Set(4, 11, z, Pal::C("e0a43a")); m.Set(3, 12, z, Pal::C("e0a43a")); m.Set(4, 10, z, Pal::C("e0a43a"));
                m.Set(0, 10, z, Pal::C("b8802a")); m.Set(1, 11, z, Pal::C("b8802a"));
            }
            // tonnelet couché devant, sur le socle
            Color32 tb = Pal::C("8a5a34");
            for (int x = -1; x <= 4; x++)
                for (int y = 2; y <= 7; y++)
                    for (int z = 0; z <= 5; z++)
                    {
                        float dy = static_cast<float>(y) + 0.5f - 4.5f, dz = static_cast<float>(z) + 0.5f - 3.0f; float r = (x == -1 || x == 4) ? 2.4f : 2.9f;
                        if (dy * dy + dz * dz > r * r) continue;
                        Color32 c = x == 0 || x == 3 ? Pal::FerSombre : (x == -1 || x == 4 ? Pal::C("6a4228") : Pal::Shade(tb, 0.9f + 0.15f * Pal::Hash01(0, y, z)));
                        m.Set(x, y, z, c);
                    }
            m.Set(4, 4, 3, Pal::BoisSombre); m.Set(5, 4, 3, Pal::BoisSombre);   // bonde
        }

        // ------------------------------------------------------------------ Antre : crâne de dragon cornu sur un tas d'or
        void Antre(VoxelModel& m, VoxelModel& l)
        {
            Socle(m, RougeSombre);
            // tas d'or
            for (int x = -6; x <= 5; x++)
                for (int z = -6; z <= 5; z++)
                {
                    float dx = (static_cast<float>(x) + 0.5f) / 6.2f, dz = (static_cast<float>(z) + 0.5f) / 6.2f; float d = dx * dx + dz * dz; if (d > 1) continue;
                    int hh = 2 + Mathf::RoundToInt(4.0f * (1.0f - d));
                    for (int y = 2; y <= hh; y++) m.Set(x, y, z, Pal::Hash01(x, y, z) < 0.25f ? OrClair : Pal::Hash01(x, y + 1, z) < 0.5f ? Or : OrSombre);
                }
            // crâne de dragon : boîte crânienne, museau long et effilé vers +z, os patiné
            Color32 os = Pal::C("d6c8aa"); Color32 osS = Pal::C("a08e74"); Color32 corne = Pal::C("3e302c");
            Cube(m, -3, 6, -5, 2, 11, -1, os);
            Cube(m, -2, 7, 0, 1, 10, 3, os);
            Cube(m, -2, 7, 4, 1, 9, 7, os);
            Cube(m, -1, 7, 8, 0, 8, 8, os);
            Cube(m, -3, 11, -1, 2, 11, 1, osS);            // arcades sourcilières en surplomb
            Cube(m, -1, 12, -5, 0, 12, -1, osS);           // crête
            m.Set(-1, 13, -4, osS); m.Set(0, 13, -3, osS);
            Cube(m, -3, 6, 0, -3, 6, 6, osS); Cube(m, 2, 6, 0, 2, 6, 6, osS);   // mâchoire
            Cube(m, -2, 5, 1, 1, 5, 7, Pal::Shade(osS, 0.85f));
            // orbites sur les côtés, au fond une braise
            for (int x : {-3, 2}) { Cube(m, x, 9, -1, x, 10, 0, Pal::Noir); m.Remove(x, 9, -1); l.Set(x, 9, -1, Pal::C("ff5a2a")); }
            m.Set(-2, 9, 7, Pal::Noir); m.Set(1, 9, 7, Pal::Noir);   // narines
            for (int z = 1; z <= 7; z += 2) { m.Set(-2, 6, z, Pal::Blanc); m.Set(1, 6, z, Pal::Blanc); }
            // grandes cornes recourbées vers l'arrière puis vers le haut
            for (int sg = -1; sg <= 1; sg += 2)
            {
                float x0 = sg < 0 ? -3.0f : 3.0f;
                float fsg = static_cast<float>(sg);
                m.Line(x0, 10.5f, -3.5f, x0 + fsg * 2.5f, 13.5f, -7.0f, 1.2f, 0.9f, corne);
                m.Line(x0 + fsg * 2.5f, 13.5f, -7.0f, x0 + fsg * 3.0f, 19.5f, -6.0f, 0.9f, 0.45f, corne);
                m.Line(x0, 7.5f, -3.5f, x0 + fsg * 1.5f, 5.5f, -6.5f, 0.6f, 0.4f, corne);
            }
            // pièces éparses et une épée plantée
            Cube(m, 4, 2, 2, 4, 9, 2, Pal::Acier); Cube(m, 3, 9, 2, 5, 9, 2, Pal::FerSombre); m.Set(4, 10, 2, Pal::Cuir); m.Set(4, 11, 2, Or);
            m.Set(-5, 3, 4, Or); m.Set(5, 3, -3, OrClair);
        }
    }
}
