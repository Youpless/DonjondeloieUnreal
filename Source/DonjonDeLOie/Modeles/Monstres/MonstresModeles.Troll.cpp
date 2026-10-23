// Donjon de l'Oie — le troll des pics et la wyverne (Pics de Braise). Voxel de 6 cm.
#include "MonstresInterne.h"

namespace DonjonDeLOie::Modeles::MonstresModeles
{
    using namespace DonjonDeLOie::Voxel;

    namespace
    {
        /// Peau de pierre : plaques de teintes voisines par blocs de 2–3 voxels, mousse sur le dessus.
        void PeauDePierre(VoxelModel& m, Color32 base_, Color32 sombre, Color32 claire, Color32 mousse, Color32 mousseS, float tauxMousse, int graine)
        {
            m.Paint([&](int x, int y, int z, Color32 c)
            {
                if (!Meme(c, base_)) return c;
                bool dessus = !m.Has(x, y + 1, z);
                float h = Pal::Hash01((x + graine) / 3, (y + graine) / 3, (z + graine) / 3);
                if (dessus && Pal::Hash01(x / 2 + graine, y / 2, z / 2) < tauxMousse) return Pal::Hash01(x, y, z) > 0.5f ? mousse : mousseS;
                if (h > 0.72f) return sombre;
                if (h < 0.18f) return claire;
                return c;
            });
        }
    }

    // ------------------------------------------------------------------ troll des pics (~2,2 m voûté, voxel 0,06)
    Ptr<VoxelRig> Troll()
    {
        auto r = NouveauRig("troll", 0.06f);
        Color32 peau = C("7c8574"), peauS = C("565e52"), peauC = C("9ba08c"), mousse = C("5f7a34"), mousseS = C("47602a");
        Color32 ongle = C("2e2a26"), dent = C("d8cba0"), bouche = C("2a1614"), oeil = C("ffb02a"), pagne = C("5a4432"), pagneS = C("3f2f22");
        Color32 roc = C("8a8378"), rocS = C("5f5a53"), rocC = C("aaa395"), corde = C("8f7a52");

        auto corps = Nouveau<VoxelModel>();
        // Tronc très penché vers l'avant : bassin bas en arrière, dos en bosse, épaules projetées devant.
        ProfilY(*corps, 0, {12, -2, 6.4f, 5, 15, -1.5f, 7.6f, 6, 19, -0.5f, 8.4f, 6.6f, 23, 0.8f, 9, 7, 27, 2.6f, 9.4f, 7, 30, 4.2f, 9.2f, 6.4f, 32, 5.4f, 7.8f, 5.4f, 33, 6.2f, 5.4f, 4}, 0.42f, peau);
        corps->Ellipsoid(0, 28.5f, -1.5f, 7.6f, 6, 5, peau);                              // bosse dorsale
        corps->Ellipsoid(0, 17, 3.5f, 6.4f, 5, 4.4f, peau);                                // panse
        corps->Paint([&](int x, int y, int z, Color32 c) { return (Meme(c, peau) && z >= 5 && y >= 13 && y <= 21 && std::abs(x + 0.5f) < 5) ? peauC : c; });
        // pics et plaques de roche le long de l'échine
        for (int k = 0; k < 7; k++)
        {
            float z = -5 + k * 1.6f;
            int h = Haut(*corps, k % 2 == 0 ? -1 : 0, Mathf::FloorToInt(z), 0, 60);
            if (h == INT_MIN) continue;
            float l = 3.2f - std::abs(k - 3) * 0.5f;
            corps->Line(k % 2 == 0 ? -0.5f : 0.8f, h, z, k % 2 == 0 ? -0.8f : 1.2f, h + l, z - 1.8f, 1.5f, 0.5f, k % 3 == 0 ? rocC : rocS);
        }
        PeauDePierre(*corps, peau, peauS, peauC, mousse, mousseS, 0.38f, 3);
        // Pagne de peau rapiécée et ceinture de corde
        ProfilY(*corps, 0, {7, -1.5f, 6.8f, 5.6f, 13, -1.8f, 6.8f, 5.4f}, 0.35f, pagne);
        corps->Carve([&](int x, int y, int z) { return y < 11 && Meme(Couleur(*corps, x, y, z), pagne) && (Pal::Hash01(x / 2, 1, z / 2) * 5 + 6 > y + 1 + (std::abs(x + 0.5f) < 2.5f ? 3 : 0)); });
        corps->Paint([&](int x, int y, int z, Color32 c) { return Meme(c, pagne) && Pal::Hash01(x / 2, y / 3, z) > 0.6f ? pagneS : c; });
        for (int x = -7; x <= 6; x++)
        {
            int zf = Avant(*corps, x, 13, -12, 14);
            if (zf != INT_MIN) corps->Set(x, 13, zf + 1, corde);
        }
        corps->Line(4, 12.5f, 5, 4.5f, 8, 5.5f, 0.6f, dent); corps->Box(4, 7, 5, 5, 7, 5, dent);       // os pendu
        corps->Line(-5, 12.5f, 4, -5.5f, 9, 4.5f, 0.6f, corde); BoiteC(*corps, -7, 6, 3, -4, 9, 6, rocC, 1); // bourse de cailloux
        r->Add("corps", corps, V(0, 13, 0));

        // Jambes courtes et arquées, pieds larges à orteils griffus
        for (int s = 0; s < 2; s++)
        {
            float sx = s == 0 ? -1.0f : 1.0f;
            auto j = Nouveau<VoxelModel>();
            Chaine(*j, {sx * 4, 12, -2, sx * 6, 7, 0.5f, sx * 5.4f, 3, -1.5f}, 3.6f, 2.6f, peau);
            int px = Mathf::FloorToInt(sx * 5.4f);
            BoiteC(*j, px - 3, 0, -4, px + 3, 3, 3, peauS, 1);
            for (int t = -1; t <= 1; t++) j->Box(px + t * 2, 0, 4, px + t * 2 + (t == 1 ? 0 : 1), 1, 4, ongle);
            PeauDePierre(*j, peau, peauS, peauC, mousse, mousseS, 0.25f, 9 + s);
            r->Add(s == 0 ? "jambeG" : "jambeD", j, V(sx * 4, 12.5f, -1.5f), "corps");
        }

        // Tête petite, basse, enfoncée entre les épaules : front fuyant, gros nez, crocs relevés.
        auto tete = Nouveau<VoxelModel>();
        ProfilY(*tete, 0, {27, 10, 3.2f, 3, 29, 10.8f, 3.8f, 3.4f, 32, 10.6f, 3.6f, 3.2f, 34, 9.6f, 2.6f, 2.6f}, 0.45f, peau);
        BoiteC(*tete, -4, 32, 12, 3, 33, 14, peauS, 0);                                    // arcade-visière
        tete->Line(-0.5f, 32, 14.5f, -0.5f, 29.5f, 15.5f, 1.5f, 1.1f, peauC);                // gros nez tombant
        tete->Line(0.5f, 32, 14.5f, 0.5f, 29.5f, 15.5f, 1.5f, 1.1f, peauC);
        for (int s = 0; s < 2; s++)
        {
            float sx = s == 0 ? -1.0f : 1.0f;
            Triangle(*tete, V(sx * 3.4f, 30, 10), V(sx * 3.4f, 32.5f, 11), V(sx * 7, 32, 8), peau);   // oreilles déchiquetées
        }
        PeauDePierre(*tete, peau, peauS, peauC, mousse, mousseS, 0.55f, 21);
        auto lueur = Nouveau<VoxelModel>();
        Oeil(*tete, *lueur, -3, 31, Avant(*tete, -3, 31, 0, 20), oeil); Oeil(*tete, *lueur, 2, 31, Avant(*tete, 2, 31, 0, 20), oeil);
        r->Add("tete", tete, V(0, 29, 9), "corps");
        r->Add("lueur_yeux", lueur, V(0, 29, 9), "tete");

        auto mach = Nouveau<VoxelModel>();
        BoiteC(*mach, -4, 25, 10, 3, 28, 15, peau, 1);
        PeauDePierre(*mach, peau, peauS, peauC, mousse, mousseS, 0.1f, 4);
        mach->Box(-3, 28, 15, 2, 28, 15, bouche);
        mach->Line(-3.5f, 27.5f, 15.5f, -3.7f, 30.6f, 15.6f, 0.8f, 0.45f, dent);
        mach->Line(2.5f, 27.5f, 15.5f, 2.7f, 30.6f, 15.6f, 0.8f, 0.45f, dent);
        mach->Set(-1, 29, 15, dent); mach->Set(0, 29, 15, dent);
        Soustraire(*tete, *mach);
        r->Add("machoire", mach, V(0, 28, 11), "tete");

        // Bras immenses : les poings touchent le sol devant lui. Épaules en rochers moussus.
        for (int s = 0; s < 2; s++)
        {
            float sx = s == 0 ? -1.0f : 1.0f;
            auto b = Nouveau<VoxelModel>();
            Chaine(*b, {sx * 9, 29, 4, sx * 12, 20, 4.5f, sx * 12.6f, 13, 8, sx * 12.4f, 7, 10.5f}, 4.4f, 3, peau);
            b->Ellipsoid(sx * 12.3f, 14, 7.5f, 3.4f, 3, 3.4f, peau);                         // coude noueux
            int hx = Mathf::FloorToInt(sx * 12.4f);
            BoiteC(*b, hx - 3, 0, 8, hx + 3, 6, 14, peauS, 2);                                // poing énorme, phalanges au sol
            for (int f = 0; f < 4; f++) b->Box(hx - 3 + f * 2 - (f == 3 ? 1 : 0), 0, 14, hx - 3 + f * 2 - (f == 3 ? 1 : 0), 1, 15, ongle);
            BoiteC(*b, Mathf::FloorToInt(sx * 9) - 4, 28, 0, Mathf::FloorToInt(sx * 9) + 4, 33, 8, peau, 2);   // épaule-rocher
            PeauDePierre(*b, peau, peauS, peauC, mousse, mousseS, 0.5f, 13 + s);
            if (s == 0) { b->Line(-10, 32, 3, -11.5f, 37, 1.5f, 1.5f, 0.4f, rocC); b->Line(-7.5f, 32, 5, -8, 35.5f, 4, 1.1f, 0.3f, roc); b->Line(-11, 31, 6, -13.5f, 33.5f, 6, 1, 0.3f, rocS); }
            r->Add(s == 0 ? "brasG" : "brasD", b, V(sx * 9, 29, 4), "corps");
        }

        // Massue de pierre : un menhir taillé ficelé sur un tronc, tenu dans le poing droit, tête posée au sol devant.
        auto arme = Nouveau<VoxelModel>();
        arme->Line(12.5f, 3.5f, 11, 13.5f, 4, 19, 1.7f, 1.5f, Pal::BoisSombre);
        for (int z = 18; z <= 32; z++)
        {
            float t = (z - 18) / 14.0f;
            float w = 3 + static_cast<float>(::Moteur::SinNet(static_cast<double>(t * 3.0f))) * 1.8f;
            Section(*arme, 2, z, 14, 4 + w * 0.15f, w, w * 0.95f, w * 0.55f, roc);
        }
        arme->Carve([&](int x, int y, int z) { return y < 0; });
        arme->Paint([&](int x, int y, int z, Color32 c) { return !Meme(c, roc) ? c : (Pal::Hash01(x / 2, y / 3, z / 2) > 0.65f ? rocS : (Pal::Hash01(x, y, z) > 0.85f ? rocC : c)); });
        arme->Paint([&](int x, int y, int z, Color32 c) { return (Meme(c, roc) && !arme->Has(x, y + 1, z) && Pal::Hash01(x, y, z * 3) > 0.45f) ? mousse : c; });
        arme->Box(11, 2, 18, 17, 7, 19, corde);
        arme->Carve([&](int x, int y, int z) { return z == 18 || z == 19 ? !(std::abs(x - 14) <= 3 && y >= 1 && y <= 7 && (std::abs(x - 14) == 3 || y == 1 || y == 7)) && Meme(Couleur(*arme, x, y, z), corde) : false; });
        arme->Line(16.5f, 7, 26, 18.5f, 10, 27, 1, 0.3f, rocS);                            // éclats saillants
        arme->Line(11, 7.5f, 29, 9.5f, 10, 29.5f, 1, 0.3f, rocS);
        r->Add("arme", arme, V(12.4f, 3.5f, 11), "brasD");
        return Fin(r);
    }
}
