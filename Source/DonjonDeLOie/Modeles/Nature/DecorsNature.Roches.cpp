// Donjon de l'Oie — roches : rocher (teinte selon la zone), rocher moussu, pic rocheux, stalagmite,
// coulée de lave, cristal de braise.
#include "NatureInterne.h"

#include <cmath>

namespace DonjonDeLOie::Modeles::DecorsModeles::Nature
{
    using namespace DonjonDeLOie::Voxel;

    namespace
    {
        constexpr Color32 NaLaveClaire = Pal::C("ffb347"), NaLave = Pal::C("ff6a1e"), NaLaveSombre = Pal::C("c23a14");
        constexpr Color32 NaBasalte = Pal::C("3a3236"), NaBasalteClair = Pal::C("554a4c"), NaRocheRouge = Pal::C("6e4438");
    }

    /// Jeux de tons de pierre : 0 gris chaud (prés, bois), 1 gris bleuté (cryptes), 2 volcanique (pics), 3 roche d'antre.
    Liste<Color32> NaTonsPierre(int famille, AleaNet& r)
    {
        Color32 a, b, c, d;
        switch (famille)
        {
            case 1: a = Pal::C("7a8088"); b = Pal::C("8e949b"); c = Pal::C("666c75"); d = Pal::C("a2a6aa"); break;
            case 2: a = Pal::C("5a4744"); b = Pal::C("6e544c"); c = Pal::C("463a3a"); d = Pal::C("806458"); break;
            case 3: a = Pal::C("5a5058"); b = Pal::C("6c6068"); c = Pal::C("483f48"); d = Pal::C("82746e"); break;
            default: a = Pal::C("8a867e"); b = Pal::C("9a958b"); c = Pal::C("7a766f"); d = Pal::C("a8a397"); break;
        }
        float f = NaF(r, -0.06f, 0.06f);
        return {Pal::Shade(a, 1 + f), Pal::Shade(b, 1 + f), Pal::Shade(c, 1 + f), Pal::Shade(d, 1 + f), Pal::Shade(a, 0.92f + f)};
    }

    // ------------------------------------------------------------------ rocher (0,6 – 1,8 m)

    /// La graine choisit aussi la famille de teinte (le même id sert dans plusieurs zones) : on garde une pierre
    /// chaude pour la plupart, et parfois une variante plus sombre. Le rocher moussu est celui du marais.
    Ptr<VoxelRig> Rocher(AleaNet& r, bool mousse)
    {
        auto rig = NaRig(mousse ? "rocher_mousse" : "rocher", 0.1f);
        auto m = Nouveau<VoxelModel>();
        int famille = 0;
        Liste<Color32> tons = NaTonsPierre(famille, r);
        float taille = NaF(r, 0.6f, 1.7f) * 10.0f;           // hauteur visée en voxels
        float large = taille * NaF(r, 0.55f, 0.8f);
        Color32 strate = Pal::Shade(tons[0], 0.78f);
        // bloc principal posé à moitié enfoncé, et un ou deux éclats
        float fz = NaF(r, 0.65f, 0.95f); int nPlans = NaI(r, 6, 8); float pasStrate = NaF(r, 3.0f, 4.5f);
        NaRoche(*m, r, Vector3(0, taille * 0.35f, 0), Vector3(large, taille * 0.72f, large * fz), nPlans, tons, pasStrate, strate);
        int n = NaI(r, 1, 2);
        for (int i = 0; i < n; i++)
        {
            float a = NaF(r, 0, 6.28f), s = NaF(r, 0.35f, 0.55f);
            int nEclat = NaI(r, 6, 8);
            NaRoche(*m, r, Vector3(Mathf::Cos(a) * large * 0.95f, taille * 0.15f, Mathf::Sin(a) * large * 0.9f),
                    Vector3(large * s, taille * s * 0.8f, large * s), nEclat, tons, 0, strate);
        }
        // cailloux au pied
        for (int i = 0; i < 3; i++)
        {
            float a = NaF(r, 0, 6.28f); int x = static_cast<int>(Mathf::Cos(a) * (large + 2)), z = static_cast<int>(Mathf::Sin(a) * (large + 2));
            int ex = NaI(r, 0, 1), ez = NaI(r, 0, 1);
            m->Box(x, 0, z, x + ex, 0, z + ez, tons[2]);
        }
        NaSol(*m);
        if (mousse)
        {
            Color32 vm = NaTeinte(Pal::C("5f7234"), r, 0.06f); Color32 vm2 = Pal::Mix(vm, Pal::C("8a8f48"), 0.4f);
            m->Paint([&](int x, int y, int z, Color32 c)
            {
                bool dessus = !m->Has(x, y + 1, z);
                float h = Pal::Hash01(x / 3, y / 4, z / 3), h2 = Pal::Hash01(x, y, z);
                if (dessus && y > 2 && h < 0.75f) return h2 < 0.25f ? vm2 : vm;
                if (!dessus && !m->Has(x, y + 2, z) && y > 3 && h < 0.45f) return vm;   // la mousse déborde sur les bords
                if (y <= 1 && h < 0.5f) return Pal::Mix(c, Pal::C("4a4a2c"), 0.6f);
                return c;
            });
            // quelques brins
            for (auto& kv : m->Clone()->Tous())
            {
                Vector3Int p = kv.Key;
                if (!m->Has(p.x, p.y + 1, p.z) && Pal::Hash01(p.x, p.y, p.z) < 0.05f) m->Set(p.x, p.y + 1, p.z, vm2);
            }
        }
        else NaDessusDessous(*m, 1.06f, 0.85f);

        rig->Add("corps", m, Vector3::zero);
        return rig;
    }

    // ------------------------------------------------------------------ pic rocheux des Pics de Braise (3 – 6 m)

    Ptr<VoxelRig> PicRocheux(AleaNet& r)
    {
        auto rig = NaRig("pic_rocheux", 0.1f);
        auto m = Nouveau<VoxelModel>();
        Liste<Color32> tons = NaTonsPierre(2, r);
        Liste<Color32> tonsSombres = {tons[2], Pal::Shade(tons[2], 0.85f), tons[0], NaBasalteClair};
        float H = NaF(r, 30, 52), pied = Mathf::Min(5.8f, H * NaF(r, 0.12f, 0.14f));
        float a = NaF(r, 0, 6.28f), inc = NaF(r, 0.05f, 0.2f);
        Vector3 sommet(Mathf::Cos(a) * H * inc, H, Mathf::Sin(a) * H * inc);
        int pans = NaI(r, 5, 7); float rot = NaF(r, 0, 3);
        NaPrisme(*m, Vector3(0, -2, 0), sommet, pied, 1.2f, pans, rot, Liste<Color32>{tons[0], tons[1], tons[2], tons[3], tons[4]});
        // aiguilles secondaires
        int n = NaI(r, 1, 3);
        for (int i = 0; i < n; i++)
        {
            float b = NaF(r, 0, 6.28f), d = pied * NaF(r, 0.8f, 1.1f), h = H * NaF(r, 0.25f, 0.5f);
            Vector3 base0(Mathf::Cos(b) * d, -2, Mathf::Sin(b) * d);
            Vector3 pointe = base0 + Vector3(Mathf::Cos(b) * h * 0.18f, h, Mathf::Sin(b) * h * 0.18f);
            float rAiguille = pied * NaF(r, 0.4f, 0.6f); int pansAiguille = NaI(r, 4, 6); float rotAiguille = NaF(r, 0, 3);
            NaPrisme(*m, base0, pointe, rAiguille, 0.8f, pansAiguille, rotAiguille, tonsSombres);
        }
        // éboulis au pied
        for (int i = 0; i < 4; i++)
        {
            float b = NaF(r, 0, 6.28f), d = pied * NaF(r, 1.1f, 1.5f);
            NaRoche(*m, r, Vector3(Mathf::Cos(b) * d, 1, Mathf::Sin(b) * d), Vector3(3.2f, 2.6f, 3.2f), 6, tonsSombres, 0, tons[2]);
        }
        // strates inclinées et cendre sur les replats
        float sx = NaF(r, -0.2f, 0.2f), sz = NaF(r, -0.2f, 0.2f);
        Color32 cendre = Pal::C("8c8580");
        m->Paint([&](int x, int y, int z, Color32 c)
        {
            float s = (static_cast<float>(y) + sx * static_cast<float>(x) + sz * static_cast<float>(z)) / 5.0f;
            if (s - static_cast<float>(std::floor(static_cast<double>(s))) < 0.2f) c = Pal::Mix(c, NaRocheRouge, 0.55f);
            if (!m->Has(x, y + 1, z) && y < 8) c = Pal::Mix(c, cendre, 0.5f);
            return c;
        });
        NaSol(*m);
        rig->Add("corps", m, Vector3::zero);
        return rig;
    }

    // ------------------------------------------------------------------ stalagmite de l'antre (1,5 – 3 m)

    Ptr<VoxelRig> Stalagmite(AleaNet& r)
    {
        auto rig = NaRig("stalagmite", 0.1f);
        auto m = Nouveau<VoxelModel>();
        Liste<Color32> tons = NaTonsPierre(3, r);
        float H = NaF(r, 15, 30), pied = H * NaF(r, 0.15f, 0.2f);
        float a = NaF(r, 0, 6.28f), inc = NaF(r, 0.0f, 0.12f);
        Vector3 sommet(Mathf::Cos(a) * H * inc, H, Mathf::Sin(a) * H * inc);
        // en anneaux empilés : chaque tranche un peu décalée, comme du calcaire qui a coulé
        int nAnneaux = NaI(r, 4, 6);
        for (int i = 0; i < nAnneaux; i++)
        {
            float t0 = static_cast<float>(i) / static_cast<float>(nAnneaux), t1 = static_cast<float>(i + 1) / static_cast<float>(nAnneaux);
            Vector3 p0 = Vector3::Lerp(Vector3(0, -1, 0), sommet, t0); Vector3 p1 = Vector3::Lerp(Vector3(0, -1, 0), sommet, t1 + 0.04f);
            float r0 = pied * (1 - t0 * 0.9f) * NaF(r, 0.95f, 1.1f), r1 = pied * (1 - t1 * 0.9f) * 0.88f + 0.6f;
            NaPrisme(*m, p0, p1, r0, r1, 6, static_cast<float>(i) * 0.5f, Liste<Color32>{tons[i % 4], tons[(i + 1) % 4], tons[(i + 2) % 4]});
        }
        // petite sœur
        if (NaP(r, 0.75f))
        {
            float b = a + NaF(r, 2, 4), d = pied * 1.3f, h = H * NaF(r, 0.3f, 0.5f);
            NaPrisme(*m, Vector3(Mathf::Cos(b) * d, -1, Mathf::Sin(b) * d), Vector3(Mathf::Cos(b) * d, h, Mathf::Sin(b) * d), pied * 0.5f, 0.6f, 5, 0,
                     Liste<Color32>{tons[1], tons[2]});
        }
        // la pointe est plus claire (calcaire mouillé), le pied plus sombre
        m->Paint([&](int x, int y, int z, Color32 c) { return static_cast<float>(y) > H * 0.8f ? Pal::Mix(c, tons[3], 0.5f) : (y < 3 ? Pal::Shade(c, 0.85f) : c); });
        NaSol(*m);
        rig->Add("corps", m, Vector3::zero);
        return rig;
    }

    // ------------------------------------------------------------------ coulée de lave (voxel de 0,1 m)

    Ptr<VoxelRig> Lave(AleaNet& r)
    {
        auto rig = NaRig("lave", 0.1f);
        auto croute = Nouveau<VoxelModel>(); auto feu = Nouveau<VoxelModel>();
        float R = NaF(r, 5.5f, 8), allonge = NaF(r, 1.0f, 1.5f), rot = NaF(r, 0, 3.14f);
        float cs = Mathf::Cos(rot), sn = Mathf::Sin(rot);
        int ph = r.Next(1000);
        for (int x = -16; x <= 16; x++)
            for (int z = -16; z <= 16; z++)
            {
                float u = (static_cast<float>(x) * cs + static_cast<float>(z) * sn) / (R * allonge), v = (static_cast<float>(-x) * sn + static_cast<float>(z) * cs) / R;
                float bord = 1.0f + (Pal::Hash01(x / 2 + ph, 0, z / 2) - 0.5f) * 0.35f;
                float d = Mathf::Sqrt(u * u + v * v);
                if (d > bord) continue;
                // plaques de croûte noire séparées par des fissures (réseau de Voronoï grossier)
                float best = 9, second = 9;
                for (int k = 0; k < 9; k++)
                {
                    float kx = (Pal::Hash01(k, ph, 1) - 0.5f) * 2.2f * R * allonge, kz = (Pal::Hash01(k, ph, 2) - 0.5f) * 2.2f * R;
                    float e = Mathf::Sqrt((static_cast<float>(x) - kx) * (static_cast<float>(x) - kx) + (static_cast<float>(z) - kz) * (static_cast<float>(z) - kz));
                    if (e < best)
                    {
                        second = best;
                        best = e;
                    }
                    else if (e < second) second = e;
                }
                bool fissure = second - best < 0.9f || (d > bord - 0.1f && Pal::Hash01(x, 7, z) < 0.5f);
                int h = d < 0.6f ? 2 : 1;
                if (fissure)
                {
                    // la lave est à la surface, un peu en retrait
                    feu->Set(x, h - 1, z, second - best < 0.35f ? NaLaveClaire : NaLave);
                    for (int y = 0; y < h - 1; y++) croute->Set(x, y, z, NaLaveSombre);
                }
                else
                {
                    Color32 c = Pal::Hash01(x, 1, z) < 0.25f ? NaBasalteClair : NaBasalte;
                    for (int y = 0; y < h; y++) croute->Set(x, y, z, Pal::Shade(c, y == h - 1 ? 1.0f : 0.8f));
                    // bord rougeoyant des plaques
                    if (second - best < 2.2f) croute->Set(x, h - 1, z, Pal::C("6a2a1e"));
                }
            }
        // quelques blocs de croûte soulevés
        int nb = NaI(r, 2, 3);
        for (int i = 0; i < nb; i++)
        {
            float a = NaF(r, 0, 6.28f), d = NaF(r, 0.2f, 0.6f) * R;
            NaRoche(*croute, r, Vector3(Mathf::Cos(a) * d, 2, Mathf::Sin(a) * d), Vector3(2.4f, 2.2f, 2.4f), 6, Liste<Color32>{NaBasalte, NaBasalteClair, Pal::Shade(NaBasalte, 0.8f)}, 0,
                    NaBasalte);
        }
        for (auto& kv : croute->Tous()) feu->Remove(kv.Key.x, kv.Key.y, kv.Key.z);
        rig->Add("corps", croute, Vector3::zero);
        rig->Add("lueurLave", feu, Vector3::zero, "corps");
        return rig;
    }

    // ------------------------------------------------------------------ cristal de braise (voxel de 0,05 m)

    Ptr<VoxelRig> CristalBraise(AleaNet& r)
    {
        auto rig = NaRig("cristal_braise", 0.05f);
        auto gangue = Nouveau<VoxelModel>(); auto coeur = Nouveau<VoxelModel>();
        int famille = NaP(r, 0.5f) ? 2 : 3;
        Liste<Color32> tons = NaTonsPierre(famille, r);
        NaRoche(*gangue, r, Vector3(0, 1, 0), Vector3(7, 4, 6), 7, tons, 0, tons[2]);
        Liste<Color32> verre = {Pal::C("8a2a20"), Pal::C("a83a22"), Pal::C("6e2018")};
        int n = NaI(r, 3, 5);
        for (int i = 0; i < n; i++)
        {
            float a = NaF(r, 0, 6.28f), inc = i == 0 ? NaF(r, 0, 0.2f) : NaF(r, 0.35f, 0.75f);
            float L = i == 0 ? NaF(r, 14, 19) : NaF(r, 7, 12), rr = i == 0 ? 2.6f : NaF(r, 1.4f, 2.0f);
            Vector3 dir = Vector3(Mathf::Cos(a) * inc, 1, Mathf::Sin(a) * inc).normalized();
            Vector3 p0(Mathf::Cos(a) * 1.5f, 2, Mathf::Sin(a) * 1.5f);
            Vector3 p1 = p0 + dir * (L * 0.78f); Vector3 p2 = p0 + dir * L;
            // fût en verre sombre (corps) ; arêtes et pointe incandescentes (lueur)
            NaPrisme(*gangue, p0, p1, rr, rr * 0.9f, 6, a, verre);
            NaPrisme(*coeur, p1 - dir * 0.6f, p2, rr * 0.92f, 0.4f, 6, a, Liste<Color32>{Pal::C("ff8a30"), Pal::C("ffb050"), Pal::C("e8601e")});
            // cœur visible par une fente
            coeur->Line(p0.x + dir.x * 2, p0.y + dir.y * 2, p0.z + dir.z * 2, p1.x, p1.y, p1.z, 0.5f, Pal::C("ff9a3a"));
        }
        for (auto& kv : coeur->Tous()) gangue->Remove(kv.Key.x, kv.Key.y, kv.Key.z);
        NaSol(*gangue); NaSol(*coeur);
        rig->Add("corps", gangue, Vector3::zero);
        rig->Add("lueurCristal", coeur, Vector3::zero, "corps");
        return rig;
    }
}
