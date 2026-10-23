// Donjon de l'Oie — Ignathor, le dragon rouge, seigneur de l'Antre. Voxel de 8 cm (~5 m du museau à la queue enroulée).
#include "MonstresInterne.h"

namespace DonjonDeLOie::Modeles::MonstresModeles
{
    using namespace DonjonDeLOie::Voxel;

    // Les couleurs du dragon (DrRouge… DrOeil) sont dans MonstresInterne.h : DragonMembres s'en sert aussi.

    /// Écailles du dragon : rangées de plaques qui se chevauchent (bord haut éclairé, joint bas sombre), en quinconce.
    void PlaquesDragon(VoxelModel& m)
    {
        m.Paint([&](int x, int y, int z, Color32 c)
        {
            if (!Meme(c, DrRouge)) return c;
            float ax = std::abs(x + 0.5f);
            int ligne = Mathf::FloorToInt((y + ax * 0.6f) / 3.0f);
            int lv = Mathf::FloorToInt(y + ax * 0.6f) - ligne * 3;
            int col = Mathf::FloorToInt((z + (ligne % 2) * 2.5f) / 5.0f);
            int lz = Mathf::FloorToInt(z + (ligne % 2) * 2.5f) - col * 5;
            if (lv == 0) return DrRougeS;
            if (lz == 0 && lv == 1) return DrRougeS;
            if (lv == 2 && lz >= 2 && lz <= 3) return DrRougeC;
            if (Pal::Hash01(ligne, col, 7) > 0.9f) return DrRougeT;
            return c;
        });
    }

    namespace
    {
        /// Ventre doré en bandes transversales.
        void Ventre(VoxelModel& m, const std::function<bool(int, int, int)>& dedans)
        {
            m.Paint([&](int x, int y, int z, Color32 c)
            {
                if (!Meme(c, DrRouge) || !dedans(x, y, z)) return c;
                if (((z % 3) + 3) % 3 == 0) return DrOrS;
                return DrOr;
            });
        }
    }

    Ptr<VoxelRig> Dragon()
    {
        auto r = NouveauRig("dragon", 0.08f);

        // ---------------------------------------------------------- corps : torse, cou, crête
        auto corps = Nouveau<VoxelModel>();
        // Torse : poitrail haut et large, reins plus bas, hanches marquées.
        ProfilZ(*corps, 0, {-15, 13, 5, 5, -11, 14, 7.6f, 6.6f, -5, 14.5f, 8.4f, 7.2f, 0, 15, 8.8f, 7.4f, 5, 16, 9.4f, 8, 9, 17.5f, 8.4f, 7.6f, 12, 19, 6.4f, 6.2f}, 0.42f, DrRouge);
        // épaules saillantes et hanches (masses où s'attachent les pattes)
        for (int s = 0; s < 2; s++)
        {
            float sx = s == 0 ? -1.0f : 1.0f;
            corps->Ellipsoid(sx * 8, 17, 7, 4, 5, 5, DrRouge);
            corps->Ellipsoid(sx * 7.5f, 15, -9, 4, 5, 5.5f, DrRouge);
        }
        // Cou : épais, en col de cygne relevé
        Liste<float> cou = {0, 19, 10, 0, 23.5f, 15, 0, 29, 17.5f, 0, 34, 19.5f, 0, 37.5f, 22.5f};
        Chaine(*corps, cou, 6.2f, 4, DrRouge);
        Chaine(*corps, {0, 15.5f, 13, 0, 21, 18, 0, 27, 20.5f, 0, 32.5f, 23}, 4.2f, 3.2f, DrRouge);
        PlaquesDragon(*corps);
        Ventre(*corps, [&](int x, int y, int z) { return (y <= 11 + std::max(0, z) * 0.3f && std::abs(x + 0.5f) < 6) || (z >= 12 && std::abs(x + 0.5f) < 3.6f && z >= y * 0.62f + 1.5f); });
        // Crête dorsale : épines d'or en lames, plus hautes au garrot
        for (int k = 0; k < 15; k++)
        {
            float z;
            [[maybe_unused]] float h0;   // (h0 ne sert pas, comme en C#)
            if (k < 5) { float t = k / 4.0f; z = Lerp(21, 13, t); h0 = 0; }
            else { float t = (k - 5) / 9.0f; z = Lerp(10, -14, t); h0 = 0; }
            int zi = Mathf::FloorToInt(z);
            int h = Haut(*corps, 0, zi, 0, 80);
            if (h == INT_MIN) continue;
            float haut = k < 5 ? 2.4f + k * 0.5f : 4.6f - std::abs(k - 7.5f) * 0.35f;
            for (int y = 0; y < static_cast<int>(haut) + 1; y++)
            {
                int zz = zi - y / 2;
                corps->Set(-1, h + y, zz, y >= haut - 1 ? DrOrC : DrOr); corps->Set(0, h + y, zz, y >= haut - 1 ? DrOrC : DrOrS);
                if (y < haut / 2) { corps->Set(-1, h + y, zz - 1, DrOrS); corps->Set(0, h + y, zz - 1, DrOrS); }
            }
        }
        // Cicatrices de batailles et pointes de lances brisées fichées dans le flanc
        corps->Line(8.5f, 18, 1, 12.5f, 21, 2.5f, 0.5f, Pal::BoisSombre); corps->Set(8, 18, 1, Pal::FerSombre);
        corps->Line(-8.5f, 15, -3, -12.5f, 16.5f, -4.5f, 0.5f, Pal::BoisSombre);
        r->Add("corps", corps, V(0, 15, 0));

        // ---------------------------------------------------------- tête et mâchoire
        auto tete = Nouveau<VoxelModel>();
        // Crâne massif, front plat, museau en coin et long.
        ProfilZ(*tete, 0, {18, 38.5f, 4.6f, 4.6f, 22, 39, 5.6f, 4.4f, 26, 38.2f, 4.6f, 3.4f, 31, 37.6f, 3.4f, 2.6f, 34, 37.2f, 2.8f, 2.2f}, 0.38f, DrRouge);
        // collerette de joues : plaques en éventail derrière les mâchoires
        for (int s = 0; s < 2; s++)
            for (int k = 0; k < 3; k++)
            {
                float sx = s == 0 ? -1.0f : 1.0f;
                Triangle(*tete, V(sx * 4.8f, 36 + k * 2.2f, 21), V(sx * 4.8f, 38 + k * 2.2f, 19), V(sx * (9 - k * 0.6f), 35 + k * 3.2f, 15.5f + k), k == 1 ? DrRougeS : DrRouge);
            }
        // arcades sourcilières lourdes, pommettes saillantes
        for (int s = 0; s < 2; s++)
        {
            float sx = s == 0 ? -1.0f : 1.0f;
            tete->Line(sx * 3.8f, 41.5f, 21, sx * 3, 41, 27.5f, 1.4f, 0.9f, DrRougeS);
            tete->Line(sx * 5.2f, 36.5f, 21, sx * 4, 36.5f, 27, 1.3f, 0.8f, DrRouge);
            // cornes principales : longues, rabattues vers l'arrière puis relevées
            Chaine(*tete, {sx * 3.6f, 41.5f, 21, sx * 5.4f, 44.5f, 17, sx * 6.6f, 45.5f, 12, sx * 7, 48.5f, 8.5f}, 1.8f, 0.5f, DrCorne);
            // cornes secondaires et piquants de joue
            Chaine(*tete, {sx * 5.4f, 38.5f, 20, sx * 8.4f, 39.5f, 16.5f, sx * 9.4f, 41.5f, 14.5f}, 1.1f, 0.4f, DrCorne);
            Chaine(*tete, {sx * 5, 35.5f, 20, sx * 7.8f, 34, 17, sx * 8.6f, 33.5f, 15}, 0.9f, 0.35f, DrCorneC);
            tete->Set(s == 0 ? -2 : 1, 38, 34, DrRougeT);                                   // naseaux
            tete->Set(s == 0 ? -2 : 1, 39, 33, DrRougeT);
            tete->Line(sx * 1.4f, 39.5f, 30.5f, sx * 1.6f, 41, 29.5f, 0.6f, 0.4f, DrCorneC);  // petites cornes nasales
        }
        PlaquesDragon(*tete);
        tete->Paint([&](int x, int y, int z, Color32 c) { return (Meme(c, DrRouge) || Meme(c, DrRougeC)) && y >= 41 && z >= 20 && z <= 28 && std::abs(x + 0.5f) < 2.2f ? DrRougeS : c; });
        // yeux enfoncés sous l'arcade, lueur
        auto lueurYeux = Nouveau<VoxelModel>();
        for (int s = 0; s < 2; s++)
        {
            int ex = s == 0 ? -5 : 4, ex2 = s == 0 ? -4 : 3;
            for (int z = 24; z <= 26; z++) { tete->Remove(ex, 39, z); lueurYeux->Set(ex, 39, z, DrOeil); }
            tete->Remove(ex2, 39, 26); lueurYeux->Set(ex2, 39, 26, DrOeil);
            for (int z = 23; z <= 27; z++) tete->Set(ex + (s == 0 ? -1 : 1), 40, z, DrRougeT);   // paupière lourde
            tete->Set(ex, 39, 23, DrRougeT);
        }
        // rangée de dents supérieures visibles hors de la gueule
        for (int z = 24; z <= 33; z += 2) { tete->Set(-3, 35, z, DrDent); tete->Set(2, 35, z, DrDent); }
        tete->Set(-3, 34, 30, DrDent); tete->Set(2, 34, 30, DrDent);   // crocs
        r->Add("tete", tete, V(0, 37, 20), "corps");
        r->Add("lueur_yeux", lueurYeux, V(0, 37, 20), "tete");

        auto mach = Nouveau<VoxelModel>();
        ProfilZ(*mach, 0, {19, 33, 4, 2.4f, 24, 32.6f, 3.8f, 1.8f, 30, 33, 2.6f, 1.3f, 34, 33.4f, 2, 1}, 0.35f, DrRouge);
        PlaquesDragon(*mach);
        Ventre(*mach, [&](int x, int y, int z) { return y <= 32; });
        for (int s = 0; s < 2; s++)
        {
            float sx = s == 0 ? -1.0f : 1.0f;
            Chaine(*mach, {sx * 3.4f, 33.5f, 20, sx * 5.6f, 31.5f, 17.5f, sx * 6, 29.5f, 16}, 1, 0.35f, DrCorne);   // barbillons cornus
            for (int z = 23; z <= 33; z += 2) mach->Set(s == 0 ? -3 : 2, 34, z, DrDent);
        }
        mach->Line(-0.5f, 33.5f, 20, -0.5f, 30.5f, 21, 0.9f, 0.4f, DrCorne);
        mach->Line(0.5f, 33.5f, 20, 0.5f, 30.5f, 21, 0.9f, 0.4f, DrCorne);
        Soustraire(*tete, *mach);
        r->Add("machoire", mach, V(0, 35, 21), "tete");
        // braise au fond de la gueule (entre crâne et mâchoire), et fumerolles aux naseaux
        auto gueule = Nouveau<VoxelModel>();
        for (int z = 20; z <= 31; z++)
            for (int x = -2; x <= 1; x++)
                for (int y = 34; y <= 35; y++)
                    if (!tete->Has(x, y, z) && !mach->Has(x, y, z)) gueule->Set(x, y, z, DrFeu);
        gueule->Set(-1, 36, 22, DrFeu); gueule->Set(0, 36, 22, DrFeu);
        Soustraire(*tete, *gueule);
        r->Add("lueur_gueule", gueule, V(0, 37, 20), "tete");

        AjouterPattesDragon(r);
        AjouterAilesDragon(r);
        AjouterQueueDragon(r);
        return Fin(r);
    }
}
