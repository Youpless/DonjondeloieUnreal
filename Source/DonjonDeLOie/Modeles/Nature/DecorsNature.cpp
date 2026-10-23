// Donjon de l'Oie — décors de nature : aiguillage, ids par zone et outils de sculpture communs
// (masses de feuillage étagées, troncs à écorce en relief, rochers à facettes, prismes taillés).
// Les modèles eux-mêmes sont dans DecorsNature.Arbres.cpp, .Roches.cpp, .Plantes.cpp, .Ombres.cpp.
#include "NatureInterne.h"

#include <algorithm>
#include <cmath>

namespace DonjonDeLOie::Modeles
{
    using namespace DonjonDeLOie::Voxel;
    using namespace DecorsModeles::Nature;

    /// Ids semés au hasard dans chaque zone (terrain, eau et cases sont faits ailleurs).
    Liste<string> DecorsModeles::DeZone(Regles::Zone z)
    {
        switch (z)
        {
            case Regles::Zone::Village: return {"chene", "buisson", "fleurs", "rocher", "herbes_hautes"};
            case Regles::Zone::Bois: return {"chene", "pin", "buisson", "champignon", "souche", "fougere", "rocher"};
            case Regles::Zone::Marais: return {"saule", "arbre_mort", "roseaux", "champignon_marais", "rocher_mousse", "feu_follet"};
            case Regles::Zone::Cryptes: return {"arbre_mort", "tombe", "croix_pierre", "os", "rocher", "cierge"};
            case Regles::Zone::Pics: return {"rocher", "pic_rocheux", "pin_brule", "lave", "cristal_braise", "os"};
            default: return {"rocher", "stalagmite", "os", "tas_or", "cristal_braise"};
        }
    }

    /// Un élément de nature, ou nullptr si l'id n'en est pas un.
    Ptr<VoxelRig> DecorsModeles::ConstruireNature(const string& id, int graine)
    {
        // la graine déborde comme en C# (le jeu passe des graines quelconques) : calcul en entiers non signés sur 32 bits
        AleaNet r(static_cast<int>(static_cast<uint32_t>(graine) * 7919u + static_cast<uint32_t>(Texte::Longueur(id)) * 131u +
                                   static_cast<uint32_t>(static_cast<unsigned char>(id[0])) * 17u));
        if (id == "chene") return Chene(r);
        if (id == "pin") return Pin(r);
        if (id == "saule") return Saule(r);
        if (id == "arbre_mort") return ArbreMort(r);
        if (id == "pin_brule") return PinBrule(r);
        if (id == "souche") return Souche(r);
        if (id == "buisson") return Buisson(r);
        if (id == "rocher") return Rocher(r, false);
        if (id == "rocher_mousse") return Rocher(r, true);
        if (id == "pic_rocheux") return PicRocheux(r);
        if (id == "stalagmite") return Stalagmite(r);
        if (id == "lave") return Lave(r);
        if (id == "cristal_braise") return CristalBraise(r);
        if (id == "fleurs") return Fleurs(r);
        if (id == "herbes_hautes") return HerbesHautes(r);
        if (id == "champignon") return Champignon(r);
        if (id == "champignon_marais") return ChampignonMarais(r);
        if (id == "fougere") return Fougere(r);
        if (id == "roseaux") return Roseaux(r);
        if (id == "tombe") return Tombe(r);
        if (id == "croix_pierre") return CroixPierre(r);
        if (id == "os") return Os(r);
        if (id == "cierge") return Cierge(r);
        if (id == "tas_or") return TasOr(r);
        if (id == "feu_follet") return FeuFollet(r);
        return nullptr;
    }
}

namespace DonjonDeLOie::Modeles::DecorsModeles::Nature
{
    using namespace DonjonDeLOie::Voxel;

    // ------------------------------------------------------------------ hasard

    float NaF(AleaNet& r, float a, float b) { return a + static_cast<float>(r.NextDouble()) * (b - a); }
    int NaI(AleaNet& r, int a, int b) { return r.Next(a, b + 1); }   // bornes incluses
    bool NaP(AleaNet& r, float p) { return r.NextDouble() < static_cast<double>(p); }

    /// Teinte légèrement décalée (luminosité ± amp, petite dérive vers le chaud ou le froid).
    Color32 NaTeinte(Color32 c, AleaNet& r, float amp)
    {
        float f = 1.0f + NaF(r, -amp, amp), d = NaF(r, -amp, amp) * 0.6f;
        return Color32(static_cast<uint8_t>(Mathf::Clamp(static_cast<float>(c.r) * f * (1 + d), 0.0f, 255.0f)), static_cast<uint8_t>(Mathf::Clamp(static_cast<float>(c.g) * f, 0.0f, 255.0f)),
                       static_cast<uint8_t>(Mathf::Clamp(static_cast<float>(c.b) * f * (1 - d), 0.0f, 255.0f)), 255);
    }

    // ------------------------------------------------------------------ outils de modèle

    bool NaEgal(Color32 a, Color32 b) { return a.r == b.r && a.g == b.g && a.b == b.b; }

    /// Éclaircit les voxels à ciel ouvert, assombrit ceux qui regardent le sol (lecture des volumes de loin).
    void NaDessusDessous(VoxelModel& m, float dessus, float dessous)
    {
        m.Paint([&](int x, int y, int z, Color32 c)
        {
            if (!m.Has(x, y + 1, z)) return Pal::Shade(c, dessus);
            if (!m.Has(x, y - 1, z) && y > 0) return Pal::Shade(c, dessous);
            return c;
        });
    }

    /// Retire tout ce qui passe sous le sol.
    void NaSol(VoxelModel& m) { m.Carve([](int x, int y, int z) { return y < 0; }); }

    Ptr<VoxelRig> NaRig(const string& id, float taille)
    {
        auto rig = Nouveau<VoxelRig>();
        rig->Name = id;
        rig->VoxelSize = taille;
        rig->Origin = Vector3::zero;
        return rig;
    }

    // ------------------------------------------------------------------ feuillage

    /// Masse de feuillage taillée : sections octogonales, étagée par paliers de « marche » voxels, dessous aplati.
    void NaMasse(VoxelModel& m, float cx, float cy, float cz, float rx, float ry, float rz, int marche, float rot, Color32 c)
    {
        int y0 = static_cast<int>(std::floor(static_cast<double>(cy - ry * 0.75f))), y1 = static_cast<int>(std::ceil(static_cast<double>(cy + ry)));
        float cs = Mathf::Cos(rot), sn = Mathf::Sin(rot);
        for (int y = y0; y <= y1; y++)
        {
            // palier : la hauteur est arrondie à la marche, d'où des terrasses nettes
            float yq = static_cast<float>(std::floor(static_cast<double>((static_cast<float>(y) - cy) / static_cast<float>(marche) + 0.5f))) * static_cast<float>(marche);
            float t = yq >= 0 ? yq / ry : yq / (ry * 0.75f);
            if (t > 1.0f || t < -1.0f) continue;
            float s = Mathf::Sqrt(1.0f - t * t);
            if (t < 0) s = Mathf::Min(1.0f, s * 1.08f);
            int xa = static_cast<int>(std::floor(static_cast<double>(cx - rx - 1))), xb = static_cast<int>(std::ceil(static_cast<double>(cx + rx + 1)));
            int za = static_cast<int>(std::floor(static_cast<double>(cz - rz - 1))), zb = static_cast<int>(std::ceil(static_cast<double>(cz + rz + 1)));
            for (int x = xa; x <= xb; x++)
                for (int z = za; z <= zb; z++)
                {
                    float dx = static_cast<float>(x) + 0.5f - cx, dz = static_cast<float>(z) + 0.5f - cz;
                    float u = Mathf::Abs((dx * cs + dz * sn) / rx), v = Mathf::Abs((-dx * sn + dz * cs) / rz);
                    float d = Mathf::Max(Mathf::Max(u, v), (u + v) * 0.74f);
                    if (d <= s) m.Set(x, y, z, c);
                }
        }
    }

    /// Peint un feuillage : dessus clair, dessous sombre, quelques touffes d'un second ton.
    void NaPeindreFeuillage(VoxelModel& m, Color32 clair, Color32 sombre, Color32 touffe, int graine)
    {
        m.Paint([&](int x, int y, int z, Color32 c)
        {
            bool haut = !m.Has(x, y + 1, z), bas = !m.Has(x, y - 1, z);
            float h = Pal::Hash01(x / 2 + graine, y / 2, z / 2);
            if (haut) return h < 0.3f ? Pal::Mix(clair, touffe, 0.5f) : clair;
            if (bas) return sombre;
            if (h < 0.18f) return touffe;
            return c;
        });
    }

    // ------------------------------------------------------------------ bois

    /// Tronc vertical taillé : centre qui ondule, évasement au pied, écorce à côtes (sillons creusés, deux tons).
    /// Rend le centre du tronc à la hauteur yHaut.
    Vector3 NaTronc(VoxelModel& m, AleaNet& r, int yHaut, float r0, float r1, float penteX, float penteZ,
                    float ondule, int cotes, Color32 ecorce, Color32 sillon)
    {
        float ph = NaF(r, 0, 6.28f), tor = NaF(r, -0.04f, 0.04f);
        Vector3 c = Vector3::zero;
        for (int y = 0; y <= yHaut; y++)
        {
            float t = static_cast<float>(y) / static_cast<float>(yHaut);
            float cx = penteX * static_cast<float>(y) + Mathf::Sin(static_cast<float>(y) * 0.21f + ph) * ondule, cz = penteZ * static_cast<float>(y) + Mathf::Cos(static_cast<float>(y) * 0.17f + ph) * ondule;
            float rr = Mathf::Lerp(r0, r1, t) * (1.0f + 0.55f * Mathf::Exp(static_cast<float>(-y) / 2.2f));
            int xa = static_cast<int>(std::floor(static_cast<double>(cx - rr - 1))), xb = static_cast<int>(std::ceil(static_cast<double>(cx + rr + 1)));
            int za = static_cast<int>(std::floor(static_cast<double>(cz - rr - 1))), zb = static_cast<int>(std::ceil(static_cast<double>(cz + rr + 1)));
            for (int x = xa; x <= xb; x++)
                for (int z = za; z <= zb; z++)
                {
                    float dx = static_cast<float>(x) + 0.5f - cx, dz = static_cast<float>(z) + 0.5f - cz;
                    float d = Mathf::Sqrt(dx * dx + dz * dz);
                    if (d > rr) continue;
                    float ang = Mathf::Atan2(dz, dx) / 6.2832f + static_cast<float>(y) * tor + Pal::Hash01(0, y / 3, 7) * 0.08f;
                    float g = ang * static_cast<float>(cotes) - static_cast<float>(std::floor(static_cast<double>(ang * static_cast<float>(cotes))));
                    bool bord = d > rr - 1.1f;
                    if (bord && rr > 2.2f && g < 0.22f) continue;              // sillon creusé
                    m.Set(x, y, z, (bord && g < 0.42f) || !bord ? sillon : ecorce);
                }
            c = Vector3(cx, static_cast<float>(y), cz);
        }
        return c;
    }

    /// Racines : traits effilés qui partent du tronc et plongent dans le sol.
    void NaRacines(VoxelModel& m, AleaNet& r, int n, float rTronc, float longueur, Color32 c)
    {
        float a0 = NaF(r, 0, 6.28f);
        for (int i = 0; i < n; i++)
        {
            float a = a0 + static_cast<float>(i) * 6.2832f / static_cast<float>(n) + NaF(r, -0.35f, 0.35f);
            float L = longueur * NaF(r, 0.7f, 1.15f), ca = Mathf::Cos(a), sa = Mathf::Sin(a);
            float hy = NaF(r, 2.5f, 4.5f) * rTronc / 2.5f;
            m.Line(ca * rTronc * 0.4f, hy, sa * rTronc * 0.4f, ca * (rTronc + L * 0.5f), hy * 0.35f, sa * (rTronc + L * 0.5f), rTronc * 0.55f, rTronc * 0.38f, c);
            m.Line(ca * (rTronc + L * 0.5f), hy * 0.35f, sa * (rTronc + L * 0.5f), ca * (rTronc + L), -0.5f, sa * (rTronc + L), rTronc * 0.38f, 0.6f, c);
        }
    }

    /// Écorce striée sur des branches (traits) : sillons verticaux courts, deux tons.
    void NaStrier(VoxelModel& m, Color32 ecorce, Color32 sillon, int graine)
    {
        m.Paint([&](int x, int y, int z, Color32 c)
        {
            float h = Pal::Hash01(x, (y + static_cast<int>(Pal::Hash01(x, graine, z) * 5)) / 4, z);
            return h < 0.38f ? sillon : ecorce;
        });
    }

    // ------------------------------------------------------------------ pierre

    /// Rocher à facettes : un ellipsoïde tranché par des plans au hasard ; chaque facette a son ton, strates inclinées.
    void NaRoche(VoxelModel& m, AleaNet& r, Vector3 c, Vector3 rad, int nPlans, const Liste<Color32>& tons, float pasStrate, Color32 strate)
    {
        Liste<Vector3> normales(nPlans); Liste<float> dist(nPlans);
        // des facettes plutôt latérales, bien réparties autour de l'axe
        float a0 = NaF(r, 0, 6.28f);
        for (int k = 0; k < nPlans; k++)
        {
            float ak = a0 + static_cast<float>(k) * 6.2832f / static_cast<float>(std::max(1, nPlans - 1)) + NaF(r, -0.3f, 0.3f);
            float ny = NaF(r, -0.2f, 0.9f);
            Vector3 n(Mathf::Cos(ak), ny, Mathf::Sin(ak));
            if (k == 0)   // un dessus à peu près plat
            {
                float nx = NaF(r, -0.3f, 0.3f), nz = NaF(r, -0.3f, 0.3f);
                n = Vector3(nx, 1, nz);
            }
            normales[k] = n.normalized(); dist[k] = k == 0 ? NaF(r, 0.42f, 0.62f) : NaF(r, 0.52f, 0.7f);
        }
        float incX = NaF(r, -0.25f, 0.25f), incZ = NaF(r, -0.25f, 0.25f);
        int x0 = static_cast<int>(std::floor(static_cast<double>(c.x - rad.x))), x1 = static_cast<int>(std::ceil(static_cast<double>(c.x + rad.x)));
        int y0 = std::max(0, static_cast<int>(std::floor(static_cast<double>(c.y - rad.y)))), y1 = static_cast<int>(std::ceil(static_cast<double>(c.y + rad.y)));
        int z0 = static_cast<int>(std::floor(static_cast<double>(c.z - rad.z))), z1 = static_cast<int>(std::ceil(static_cast<double>(c.z + rad.z)));
        for (int x = x0; x <= x1; x++)
            for (int y = y0; y <= y1; y++)
                for (int z = z0; z <= z1; z++)
                {
                    Vector3 q((static_cast<float>(x) + 0.5f - c.x) / rad.x, (static_cast<float>(y) + 0.5f - c.y) / rad.y, (static_cast<float>(z) + 0.5f - c.z) / rad.z);
                    if (q.sqrMagnitude() > 1.0f) continue;
                    int face = -1; float meilleur = -9.0f; bool dedans = true;
                    for (int k = 0; k < nPlans; k++)
                    {
                        float e = Vector3::Dot(q, normales[k]) - dist[k];
                        if (e > 0)
                        {
                            dedans = false;
                            break;
                        }
                        if (e > meilleur)
                        {
                            meilleur = e;
                            face = k;
                        }
                    }
                    if (!dedans) continue;
                    Color32 col = tons[(face + 1) % tons.Length()];
                    if (pasStrate > 0)
                    {
                        float s = (static_cast<float>(y) + incX * static_cast<float>(x) + incZ * static_cast<float>(z)) / pasStrate;
                        if (s - static_cast<float>(std::floor(static_cast<double>(s))) < 0.22f) col = Pal::Mix(col, strate, 0.35f);
                    }
                    m.Set(x, y, z, col);
                }
    }

    /// Prisme taillé à K pans entre a et b (rayon r0 → r1), orienté librement. Chaque pan prend un ton de « tons ».
    void NaPrisme(VoxelModel& m, Vector3 a, Vector3 b, float r0, float r1, int K, float rot, const Liste<Color32>& tons)
    {
        Vector3 ax = b - a; float L = ax.magnitude();
        if (L < 0.01f) return;
        Vector3 u = ax / L;
        Vector3 e1 = Vector3::Cross(u, Mathf::Abs(u.y) < 0.9f ? Vector3::up : Vector3::right).normalized();
        Vector3 e2 = Vector3::Cross(u, e1);
        float rm = Mathf::Max(r0, r1) + 1;
        int xa = static_cast<int>(std::floor(static_cast<double>(Mathf::Min(a.x, b.x) - rm))), xb = static_cast<int>(std::ceil(static_cast<double>(Mathf::Max(a.x, b.x) + rm)));
        int ya = static_cast<int>(std::floor(static_cast<double>(Mathf::Min(a.y, b.y) - rm))), yb = static_cast<int>(std::ceil(static_cast<double>(Mathf::Max(a.y, b.y) + rm)));
        int za = static_cast<int>(std::floor(static_cast<double>(Mathf::Min(a.z, b.z) - rm))), zb = static_cast<int>(std::ceil(static_cast<double>(Mathf::Max(a.z, b.z) + rm)));
        for (int x = xa; x <= xb; x++)
            for (int y = ya; y <= yb; y++)
                for (int z = za; z <= zb; z++)
                {
                    Vector3 p = Vector3(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f, static_cast<float>(z) + 0.5f) - a;
                    float t = Vector3::Dot(p, u) / L;
                    if (t < 0 || t > 1) continue;
                    Vector3 q = p - u * (t * L);
                    float qx = Vector3::Dot(q, e1), qy = Vector3::Dot(q, e2), rr = Mathf::Lerp(r0, r1, t);
                    int face = 0; float meilleur = -99.0f; bool dedans = true;
                    for (int k = 0; k < K; k++)
                    {
                        float ph = rot + static_cast<float>(k) * 6.2832f / static_cast<float>(K);
                        float e = qx * Mathf::Cos(ph) + qy * Mathf::Sin(ph) - rr;
                        if (e > 0)
                        {
                            dedans = false;
                            break;
                        }
                        if (e > meilleur)
                        {
                            meilleur = e;
                            face = k;
                        }
                    }
                    if (dedans) m.Set(x, y, z, tons[face % tons.Length()]);
                }
    }
}
