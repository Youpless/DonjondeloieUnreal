// Donjon de l'Oie — arbres : chêne, pin, saule, arbre mort, pin brûlé (voxel de 0,1 m).
// Pièces : « corps » (tronc, racines, branches maîtresses) et « feuillage » (houppier, parent corps, pivot en haut du tronc).
#include "NatureInterne.h"

#include <algorithm>
#include <cmath>

namespace DonjonDeLOie::Modeles::DecorsModeles::Nature
{
    using namespace DonjonDeLOie::Voxel;

    namespace
    {
        // écorces et feuillages (NaEcorce et NaEcorceSombre, qui servent aussi aux plantes, sont dans NatureInterne.h)
        constexpr Color32 NaPinEcorce = Pal::C("7a4a32"), NaPinSillon = Pal::C("4e2e22");
        constexpr Color32 NaMortEcorce = Pal::C("7d7468");
        [[maybe_unused]] constexpr Color32 NaMortSillon = Pal::C("4f4a45");     // (ne sert nulle part, en C# non plus)
        constexpr Color32 NaCharbon = Pal::C("2e2826"), NaCharbonClair = Pal::C("4a403a"), NaCendre = Pal::C("8a817a");
        constexpr Color32 NaSauleEcorce = Pal::C("5f5440"), NaSauleSillon = Pal::C("3e3628");

        void NaAjouterArbre(VoxelRig& rig, const Ptr<VoxelModel>& corps, const Ptr<VoxelModel>& feuillage, Vector3 pivot)
        {
            NaSol(*corps); NaSol(*feuillage);

            rig.Add("corps", corps, Vector3::zero);
            rig.Add("feuillage", feuillage, pivot, "corps");
        }
    }

    // ------------------------------------------------------------------ chêne (3 – 4,5 m)

    Ptr<VoxelRig> Chene(AleaNet& r)
    {
        auto rig = NaRig("chene", 0.1f);
        auto corps = Nouveau<VoxelModel>(); auto feu = Nouveau<VoxelModel>();
        int H = NaI(r, 13, 18);
        float px = NaF(r, -0.06f, 0.06f), pz = NaF(r, -0.06f, 0.06f);
        Vector3 haut = NaTronc(*corps, r, H, 2.7f, 2.0f, px, pz, 0.5f, 7, NaEcorce, NaEcorceSombre);
        int nRacines = NaI(r, 4, 6); float lRacines = NaF(r, 3.5f, 5.0f);
        NaRacines(*corps, r, nRacines, 2.6f, lRacines, NaEcorceSombre);

        // teinte : du vert frais des prés au vert profond du bois
        float t = NaF(r, 0, 1);
        Color32 vClair = NaTeinte(Pal::Mix(Pal::C("7da246"), Pal::C("568e44"), t), r, 0.05f);
        Color32 vBase = NaTeinte(Pal::Mix(Pal::C("587f34"), Pal::C("386a36"), t), r, 0.05f);
        Color32 vSombre = Pal::Mix(Pal::C("3c5e2a"), Pal::C("2a4e2c"), t);
        Color32 vTouffe = Pal::Mix(Pal::C("74983c"), Pal::C("4f8a50"), t);

        // branches maîtresses, chacune portant une masse de feuillage
        int nb = NaI(r, 3, 5); float a0 = NaF(r, 0, 6.28f);
        float largeur = NaF(r, 0.85f, 1.15f);
        for (int i = 0; i < nb; i++)
        {
            float a = a0 + static_cast<float>(i) * 6.2832f / static_cast<float>(nb) + NaF(r, -0.4f, 0.4f);
            float L = NaF(r, 7.0f, 10.0f) * largeur, monte = NaF(r, 0.55f, 1.0f);
            float y0 = static_cast<float>(H) - NaF(r, 1.0f, 4.0f);
            Vector3 b(haut.x + Mathf::Cos(a) * L, y0 + L * monte, haut.z + Mathf::Sin(a) * L);
            corps->Line(haut.x, y0, haut.z, b.x, b.y, b.z, 1.7f, 0.9f, NaEcorce);
            float rx = NaF(r, 6.0f, 8.0f), ry = NaF(r, 5.0f, 6.5f), rz = NaF(r, 6.0f, 8.0f), rot = NaF(r, 0, 1.6f);
            NaMasse(*feu, b.x, b.y + 3.5f, b.z, rx, ry, rz, 2, rot, vBase);
        }
        // masse du sommet, plus une couronne basse qui épaissit la silhouette
        float sx = NaF(r, -1, 1), sy = NaF(r, 13, 16), sz = NaF(r, -1, 1), srx = NaF(r, 8, 10), sry = NaF(r, 6, 7.5f), srz = NaF(r, 8, 10), srot = NaF(r, 0, 1.6f);
        NaMasse(*feu, haut.x + sx, static_cast<float>(H) + sy, haut.z + sz, srx * largeur, sry, srz * largeur, 2, srot, vBase);
        NaMasse(*feu, haut.x, static_cast<float>(H + 5), haut.z, 9.0f * largeur, 4.0f, 9.0f * largeur, 2, 0.4f, vBase);
        NaPeindreFeuillage(*feu, vClair, vSombre, vTouffe, r.Next(100));
        NaStrier(*corps, NaEcorce, NaEcorceSombre, r.Next(100));
        // remettre le tronc taillé par-dessus les branches (garde les côtes au pied)
        AleaNet rPied(1);
        NaTronc(*corps, rPied, 3, 2.7f, 2.0f, 0, 0, 0, 7, NaEcorce, NaEcorceSombre);
        NaAjouterArbre(*rig, corps, feu, Vector3(haut.x, static_cast<float>(H), haut.z));
        return rig;
    }

    // ------------------------------------------------------------------ pin (4 – 6 m)

    namespace
    {
        /// Étage de pin : disque à pans dont les bords retombent (cône aplati), dents irrégulières.
        void NaEtagePin(VoxelModel& m, float cx, float cy, float cz, float rayon, float h, int pans, float rot, int graine, Color32 c)
        {
            int y0 = static_cast<int>(std::floor(static_cast<double>(cy))), y1 = static_cast<int>(std::ceil(static_cast<double>(cy + h)));
            for (int y = y0; y <= y1; y++)
            {
                // rayon quantifié par marches de 2 voxels : des gradins nets au lieu de stries
                float t = static_cast<float>(std::floor(static_cast<double>((static_cast<float>(y) - cy) / 2.0f))) * 2.0f / h;
                float rr = rayon * (1.0f - t) + 0.8f;
                int xa = static_cast<int>(std::floor(static_cast<double>(cx - rr - 1))), xb = static_cast<int>(std::ceil(static_cast<double>(cx + rr + 1)));
                int za = static_cast<int>(std::floor(static_cast<double>(cz - rr - 1))), zb = static_cast<int>(std::ceil(static_cast<double>(cz + rr + 1)));
                for (int x = xa; x <= xb; x++)
                    for (int z = za; z <= zb; z++)
                    {
                        float dx = static_cast<float>(x) + 0.5f - cx, dz = static_cast<float>(z) + 0.5f - cz;
                        float ang = Mathf::Atan2(dz, dx) - rot;
                        // polygone à « pans » côtés : distance au bord du polygone
                        float sect = 6.2832f / static_cast<float>(pans);
                        float aa = ang - sect * static_cast<float>(std::floor(static_cast<double>(ang / sect + 0.5f)));
                        float d = Mathf::Sqrt(dx * dx + dz * dz) * Mathf::Cos(aa) / Mathf::Cos(sect * 0.5f);
                        // pointes : le bord avance aux coins des pans
                        float dent = (Pal::Hash01(static_cast<int>((ang + 7) * 3), graine, y0) - 0.5f) * 1.6f;
                        if (d <= rr + (t < 0.25f ? dent : 0.0f)) m.Set(x, y, z, c);
                    }
            }
            // le dessous retombe : on creuse l'intérieur du premier niveau pour un bord pendant
            for (int x = static_cast<int>(cx - rayon); x <= static_cast<int>(cx + rayon); x++)
                for (int z = static_cast<int>(cz - rayon); z <= static_cast<int>(cz + rayon); z++)
                {
                    float dx = static_cast<float>(x) + 0.5f - cx, dz = static_cast<float>(z) + 0.5f - cz;
                    if (dx * dx + dz * dz < (rayon * 0.6f) * (rayon * 0.6f)) m.Remove(x, y0, z);
                }
        }
    }

    Ptr<VoxelRig> Pin(AleaNet& r)
    {
        auto rig = NaRig("pin", 0.1f);
        auto corps = Nouveau<VoxelModel>(); auto feu = Nouveau<VoxelModel>();
        int Htot = NaI(r, 40, 58);
        int H = static_cast<int>(static_cast<float>(Htot) * NaF(r, 0.86f, 0.92f));
        float px = NaF(r, -0.035f, 0.035f), pz = NaF(r, -0.035f, 0.035f);
        [[maybe_unused]] Vector3 haut = NaTronc(*corps, r, H, 1.9f, 0.9f, px, pz, 0.25f, 6, NaPinEcorce, NaPinSillon);   // (haut ne sert pas, en C# non plus)
        int nRacines = NaI(r, 3, 5); float lRacines = NaF(r, 2.5f, 3.5f);
        NaRacines(*corps, r, nRacines, 1.9f, lRacines, NaPinSillon);
        NaStrier(*corps, NaPinEcorce, NaPinSillon, r.Next(100));
        AleaNet rPied(2);
        NaTronc(*corps, rPied, 3, 1.9f, 1.5f, 0, 0, 0, 6, NaPinEcorce, NaPinSillon);

        Color32 vBase = NaTeinte(Pal::C("2f5f3c"), r, 0.07f);
        Color32 vClair = Pal::Shade(Pal::Mix(vBase, Pal::C("6f9a58"), 0.35f), 1.08f);
        Color32 vSombre = Pal::Shade(vBase, 0.62f);
        Color32 vTouffe = Pal::Mix(vBase, Pal::C("3b6a4e"), 0.6f);

        int nEtages = NaI(r, 4, 6);
        float yBas = static_cast<float>(Htot) * NaF(r, 0.22f, 0.3f), largeur = NaF(r, 10.0f, 12.5f);
        float pasY = (static_cast<float>(Htot) - yBas - 6) / (static_cast<float>(nEtages) - 0.4f);
        float rot = NaF(r, 0, 1);
        for (int i = 0; i < nEtages; i++)
        {
            float t = static_cast<float>(i) / static_cast<float>(nEtages - 1);
            float y = yBas + static_cast<float>(i) * pasY, rayon = Mathf::Lerp(largeur, 3.2f, Mathf::Pow(t, 0.9f)) * NaF(r, 0.92f, 1.06f);
            float fx = px * y, fz = pz * y;
            float ex = NaF(r, -0.6f, 0.6f), ez = NaF(r, -0.6f, 0.6f), eh = NaF(r, 1.35f, 1.55f);
            int pans = NaI(r, 7, 9), ge = r.Next(1000);
            NaEtagePin(*feu, fx + ex, y, fz + ez, rayon, pasY * eh, pans, rot + static_cast<float>(i) * 0.7f, ge, vBase);
        }
        // pointe
        feu->Line(px * static_cast<float>(Htot - 8), static_cast<float>(Htot - 8), pz * static_cast<float>(Htot - 8), px * static_cast<float>(Htot), static_cast<float>(Htot),
                  pz * static_cast<float>(Htot), 2.2f, 0.6f, vBase);
        NaPeindreFeuillage(*feu, vClair, vSombre, vTouffe, r.Next(100));
        NaAjouterArbre(*rig, corps, feu, Vector3(px * yBas, yBas, pz * yBas));
        return rig;
    }

    // ------------------------------------------------------------------ saule du marais (3 – 4 m)

    Ptr<VoxelRig> Saule(AleaNet& r)
    {
        auto rig = NaRig("saule", 0.1f);
        auto corps = Nouveau<VoxelModel>(); auto feu = Nouveau<VoxelModel>();
        int H = NaI(r, 15, 20);
        float a = NaF(r, 0, 6.28f), pente = NaF(r, 0.1f, 0.22f);
        float px = Mathf::Cos(a) * pente, pz = Mathf::Sin(a) * pente;
        Vector3 haut = NaTronc(*corps, r, H, 3.2f, 2.3f, px, pz, 0.8f, 6, NaSauleEcorce, NaSauleSillon);
        int nRacines = NaI(r, 5, 7); float lRacines = NaF(r, 4.0f, 6.0f);
        NaRacines(*corps, r, nRacines, 3.0f, lRacines, NaSauleSillon);

        Color32 vBase = NaTeinte(Pal::C("6b7a3a"), r, 0.06f);
        Color32 vClair = Pal::Mix(vBase, Pal::C("a3a85a"), 0.4f);
        Color32 vSombre = Pal::Shade(Pal::Mix(vBase, Pal::C("3c4428"), 0.5f), 0.85f);
        Color32 vTouffe = Pal::Mix(vBase, Pal::C("55642e"), 0.7f);

        // branches qui s'arquent
        int nb = NaI(r, 4, 6); float a0 = NaF(r, 0, 6.28f);
        Liste<Vector3> bouts(nb);
        for (int i = 0; i < nb; i++)
        {
            float ab = a0 + static_cast<float>(i) * 6.2832f / static_cast<float>(nb) + NaF(r, -0.3f, 0.3f), L = NaF(r, 7, 10);
            float by = static_cast<float>(H) + NaF(r, 5, 8);
            Vector3 b(haut.x + Mathf::Cos(ab) * L, by, haut.z + Mathf::Sin(ab) * L);
            corps->Line(haut.x, static_cast<float>(H - 2), haut.z, b.x, b.y, b.z, 1.6f, 0.8f, NaSauleEcorce);
            bouts[i] = b;
            float rx = NaF(r, 6, 7.5f), rz = NaF(r, 6, 7.5f), rot = NaF(r, 0, 1.5f);
            NaMasse(*feu, b.x, b.y + 2, b.z, rx, 3.5f, rz, 2, rot, vBase);
        }
        NaMasse(*feu, haut.x, static_cast<float>(H + 9), haut.z, 9, 4.5f, 9, 2, 0.3f, vBase);
        // rideaux qui pendent : lanières sous le bord du houppier
        Liste<Vector3Int> fins;
        for (auto& kv : feu->Tous())
        {
            Vector3Int p = kv.Key;
            if (feu->Has(p.x, p.y - 1, p.z)) continue;
            float dx = static_cast<float>(p.x) + 0.5f - haut.x, dz = static_cast<float>(p.z) + 0.5f - haut.z;
            float d = Mathf::Sqrt(dx * dx + dz * dz);
            if (d < 7.5f) continue;
            if (Pal::Hash01(p.x / 2, 3, p.z / 2) < 0.42f && Pal::Hash01(p.x, 4, p.z) < 0.7f) fins.Add(p);
        }
        for (auto& p : fins)
        {
            int L = static_cast<int>(NaF(r, 0.2f, 0.6f) * static_cast<float>(p.y - 4));
            L = std::max(3, L);
            for (int k = 1; k <= L; k++) feu->Set(p.x, p.y - k, p.z, k > L - 2 ? vClair : vTouffe);
        }
        NaPeindreFeuillage(*feu, vClair, vSombre, vTouffe, r.Next(100));
        // la mousse et les lanières gardent leur ton pendant
        for (auto& p : fins)
            for (int k = 1; k < 3; k++)
                if (feu->Has(p.x, p.y - k, p.z)) feu->Set(p.x, p.y - k, p.z, vTouffe);
        NaStrier(*corps, NaSauleEcorce, NaSauleSillon, r.Next(100));
        AleaNet rPied(3);
        NaTronc(*corps, rPied, 3, 3.2f, 2.8f, 0, 0, 0, 6, NaSauleEcorce, NaSauleSillon);
        // mousse au pied
        corps->Paint([&](int x, int y, int z, Color32 c) { return y < 3 && !corps->Has(x, y + 1, z) ? Pal::C("5d6b34") : c; });
        NaAjouterArbre(*rig, corps, feu, Vector3(haut.x, static_cast<float>(H), haut.z));
        return rig;
    }

    // ------------------------------------------------------------------ branches nues (arbre mort, pin brûlé)

    namespace
    {
        /// Branche ramifiée récursive : un trait effilé, puis 1 à 3 rameaux qui s'écartent.
        void NaBranche(VoxelModel& m, AleaNet& r, Vector3 a, Vector3 dir, float L, float rayon, int prof, Color32 c)
        {
            Vector3 b = a + dir * L;
            m.Line(a.x, a.y, a.z, b.x, b.y, b.z, rayon, Mathf::Max(0.5f, rayon * 0.68f), c);
            if (prof <= 0 || rayon < 0.6f) return;
            int n = NaI(r, 1, prof >= 2 ? 3 : 2);
            for (int i = 0; i < n; i++)
            {
                float dx = NaF(r, -0.8f, 0.8f), dy = NaF(r, -0.15f, 0.55f), dz = NaF(r, -0.8f, 0.8f);
                Vector3 d2 = (dir + Vector3(dx, dy, dz)).normalized();
                float l2 = L * NaF(r, 0.55f, 0.8f);
                NaBranche(m, r, b, d2, l2, rayon * 0.68f, prof - 1, c);
            }
        }
    }

    Ptr<VoxelRig> ArbreMort(AleaNet& r)
    {
        auto rig = NaRig("arbre_mort", 0.1f);
        auto corps = Nouveau<VoxelModel>(); auto bra = Nouveau<VoxelModel>();
        int H = NaI(r, 12, 17);
        float a = NaF(r, 0, 6.28f), pente = NaF(r, 0.04f, 0.16f);
        float tMort = NaF(r, 0, 1);
        Color32 ecorce = NaTeinte(Pal::Mix(NaMortEcorce, Pal::C("6b5a48"), tMort), r, 0.06f);
        Color32 sillon = Pal::Shade(ecorce, 0.62f);
        Vector3 haut = NaTronc(*corps, r, H, 2.6f, 1.6f, Mathf::Cos(a) * pente, Mathf::Sin(a) * pente, 1.0f, 5, ecorce, sillon);
        int nRacines = NaI(r, 4, 6); float lRacines = NaF(r, 4.0f, 6.0f);
        NaRacines(*corps, r, nRacines, 2.4f, lRacines, sillon);
        // un creux sombre dans le tronc
        float ac = NaF(r, 0, 6.28f);
        int yc = NaI(r, 4, 8);
        corps->Ellipsoid(Mathf::Cos(ac) * 2.4f + Mathf::Cos(a) * pente * static_cast<float>(yc), static_cast<float>(yc), Mathf::Sin(ac) * 2.4f + Mathf::Sin(a) * pente * static_cast<float>(yc),
                         1.1f, 2.2f, 1.1f, Pal::C("231e1c"));
        // branches maîtresses (corps) et ramure (feuillage, qui grince au vent)
        int nb = NaI(r, 2, 4); float a0 = NaF(r, 0, 6.28f);
        for (int i = 0; i < nb; i++)
        {
            float ab = a0 + static_cast<float>(i) * 6.2832f / static_cast<float>(nb) + NaF(r, -0.5f, 0.5f);
            float dirY = NaF(r, 0.5f, 1.3f);
            Vector3 dir = Vector3(Mathf::Cos(ab), dirY, Mathf::Sin(ab)).normalized();
            float baisse = NaF(r, 0, 4);
            Vector3 depart(haut.x, static_cast<float>(H) - baisse, haut.z);
            float L = NaF(r, 5, 8);
            corps->Line(depart.x, depart.y, depart.z, depart.x + dir.x * L, depart.y + dir.y * L, depart.z + dir.z * L, 1.6f, 1.1f, ecorce);
            float lRamure = NaF(r, 6, 9);
            NaBranche(*bra, r, depart + dir * L, (dir + Vector3(0, 0.3f, 0)).normalized(), lRamure, 1.1f, 3, ecorce);
        }
        // une flèche cassée au sommet
        float fx = NaF(r, -0.3f, 0.3f), fz = NaF(r, -0.3f, 0.3f), lFleche = NaF(r, 6, 10);
        NaBranche(*bra, r, Vector3(haut.x, static_cast<float>(H), haut.z), Vector3(fx, 1, fz).normalized(), lFleche, 1.5f, 2, ecorce);
        NaStrier(*corps, ecorce, sillon, r.Next(100));
        NaStrier(*bra, ecorce, Pal::Shade(ecorce, 0.8f), r.Next(100));
        AleaNet rPied(4);
        NaTronc(*corps, rPied, 3, 2.6f, 2.3f, 0, 0, 0, 5, ecorce, sillon);
        NaAjouterArbre(*rig, corps, bra, Vector3(haut.x, static_cast<float>(H), haut.z));
        return rig;
    }

    // ------------------------------------------------------------------ pin brûlé des Pics (3,5 – 5 m)

    Ptr<VoxelRig> PinBrule(AleaNet& r)
    {
        auto rig = NaRig("pin_brule", 0.1f);
        auto corps = Nouveau<VoxelModel>(); auto bra = Nouveau<VoxelModel>(); auto braise = Nouveau<VoxelModel>();
        int H = NaI(r, 34, 48);
        float a = NaF(r, 0, 6.28f), pente = NaF(r, 0.0f, 0.07f);
        float px = Mathf::Cos(a) * pente, pz = Mathf::Sin(a) * pente;
        Vector3 haut = NaTronc(*corps, r, H, 2.4f, 1.0f, px, pz, 0.3f, 6, NaCharbonClair, NaCharbon);
        int nRacines = NaI(r, 3, 5);
        NaRacines(*corps, r, nRacines, 2.0f, 3.0f, NaCharbon);
        NaStrier(*corps, NaCharbonClair, NaCharbon, r.Next(100));
        AleaNet rPied(5);
        NaTronc(*corps, rPied, 3, 2.0f, 1.8f, 0, 0, 0, 6, NaCharbonClair, NaCharbon);
        // l'écorce rousse survit en bas, côté abrité
        float abri = NaF(r, 0, 6.28f);
        corps->Paint([&](int x, int y, int z, Color32 c)
        {
            float an = Mathf::Atan2(static_cast<float>(z), static_cast<float>(x)) - abri; float co = Mathf::Cos(an);
            if (static_cast<float>(y) < static_cast<float>(H) * 0.35f && co > 0.4f && Pal::Hash01(x, y / 2, z) < 0.7f) return Pal::Mix(NaPinEcorce, NaCharbonClair, 0.45f);
            return c;
        });
        // moignons de branches qui pointent vers le bas, de moins en moins longs vers le haut
        int n = NaI(r, 8, 13);
        for (int i = 0; i < n; i++)
        {
            float t = NaF(r, 0.3f, 0.95f), y = static_cast<float>(H) * t, ab = NaF(r, 0, 6.28f);
            float L = Mathf::Lerp(8.0f, 3.0f, t) * NaF(r, 0.6f, 1.1f);
            Vector3 dep(px * y, y, pz * y);
            float chute = NaF(r, 0.1f, 0.5f);
            Vector3 fin = dep + Vector3(Mathf::Cos(ab) * L, -L * chute, Mathf::Sin(ab) * L);
            bra->Line(dep.x, dep.y, dep.z, fin.x, fin.y, fin.z, 0.9f, 0.5f, NaCharbon);
            bra->Set(static_cast<int>(std::floor(static_cast<double>(fin.x))), static_cast<int>(std::floor(static_cast<double>(fin.y))), static_cast<int>(std::floor(static_cast<double>(fin.z))),
                     NaCendre);
        }
        // tête cassée, pointe de cendre
        float tx = NaF(r, -2, 2), ty = NaF(r, 3, 5), tz = NaF(r, -2, 2);
        bra->Line(haut.x, static_cast<float>(H), haut.z, haut.x + tx, static_cast<float>(H) + ty, haut.z + tz, 0.8f, 0.5f, NaCendre);
        // quelques braises qui couvent dans les fentes
        int nbr = NaI(r, 2, 4);
        for (int i = 0; i < nbr; i++)
        {
            float y = NaF(r, 3, static_cast<float>(H) * 0.5f), ab = NaF(r, 0, 6.28f),
                  rr = Mathf::Lerp(2.0f, 0.7f, y / static_cast<float>(H)) * (1.0f + 0.55f * Mathf::Exp(-y / 2.2f)) + 0.1f;
            int bx = static_cast<int>(std::floor(static_cast<double>(px * y + Mathf::Cos(ab) * rr))), bz = static_cast<int>(std::floor(static_cast<double>(pz * y + Mathf::Sin(ab) * rr))),
                by = static_cast<int>(y);
            braise->Set(bx, by, bz, Pal::C("ff7a2a"));
            if (NaP(r, 0.6f)) braise->Set(bx, by + 1, bz, Pal::C("e0521c"));
        }
        NaSol(*corps); NaSol(*bra);
        rig->Add("corps", corps, Vector3::zero);
        rig->Add("feuillage", bra, Vector3(haut.x * 0.3f, static_cast<float>(H) * 0.3f, haut.z * 0.3f), "corps");
        if (braise->Count() > 0) rig->Add("lueurBraises", braise, Vector3::zero, "corps");
        return rig;
    }
}
