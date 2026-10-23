// Donjon de l'Oie — la wyverne (Pics de Braise, Antre). Voxel de 6 cm.
#include "MonstresInterne.h"

namespace DonjonDeLOie::Modeles::MonstresModeles
{
    using namespace DonjonDeLOie::Voxel;

    /// Écailles en plaques : rangées décalées, bord inférieur de chaque plaque assombri.
    void Ecailles(VoxelModel& m, Color32 base_, Color32 sombre, Color32 claire, int pas)
    {
        m.Paint([&](int x, int y, int z, Color32 c)
        {
            if (!Meme(c, base_)) return c;
            int rz = Mathf::FloorToInt(z / static_cast<float>(pas));
            int dx = x + (rz % 2 == 0 ? 0 : pas / 2);
            int px = Mathf::FloorToInt(dx / static_cast<float>(pas));
            int lz = z - rz * pas;
            if (lz == 0) return sombre;
            if (lz == pas - 1 && ((px + rz) % 3 == 0)) return claire;
            return c;
        });
    }

    /// Une aile membraneuse : bras (épaule → coude → poignet), doigts, membrane tendue entre eux et le flanc.
    /// sx = −1 (gauche) ou +1 (droite). Les points sont donnés pour le côté droit, ils sont reflétés pour le gauche.
    Ptr<VoxelModel> Aile(float sx, Vector3 epaule, Vector3 coude, Vector3 poignet, const Liste<Vector3>& doigts, const Liste<Vector3>& flanc,
                         float rOs, Color32 os, Color32 membrane, Color32 membraneS, Color32 griffe)
    {
        auto m = Nouveau<VoxelModel>();
        auto f = [&](Vector3 p) { return Vector3(p.x * sx, p.y, p.z); };
        Vector3 e = f(epaule), c = f(coude), w = f(poignet);
        // membrane d'abord (les os passent par-dessus)
        Liste<Vector3> bords(doigts.Length() + 1);
        for (int i = 0; i < doigts.Length(); i++) bords[i] = f(doigts[i]);
        for (int i = 0; i < doigts.Length() - 1; i++) Triangle(*m, w, bords[i], bords[i + 1], membrane);
        // entre le dernier doigt, le poignet, le coude et le flanc
        Liste<Vector3> fl(flanc.Length());
        for (int i = 0; i < flanc.Length(); i++) fl[i] = f(flanc[i]);
        Triangle(*m, w, bords[doigts.Length() - 1], fl[0], membrane);
        Triangle(*m, w, fl[0], c, membrane);
        for (int i = 0; i < fl.Length() - 1; i++) Triangle(*m, c, fl[i], fl[i + 1], membrane);
        Triangle(*m, c, fl[fl.Length() - 1], e, membrane);
        Triangle(*m, e, c, w, membrane);
        // bord de fuite festonné : on creuse des encoches entre les doigts
        for (int i = 0; i < doigts.Length() - 1; i++)
        {
            Vector3 mil = (bords[i] + bords[i + 1]) * 0.5f;
            Vector3 vers = (w - mil).normalized();
            Vector3 centre = mil - vers * 1.5f;
            float rr = (bords[i] - bords[i + 1]).magnitude() * 0.32f;
            m->Carve([&](int x, int y, int z) { return (Vector3(x + 0.5f, y + 0.5f, z + 0.5f) - centre).sqrMagnitude() < rr * rr; });
        }
        // nervures : la membrane s'assombrit près des os
        // os
        m->Line(e.x, e.y, e.z, c.x, c.y, c.z, rOs * 1.3f, rOs * 1.05f, os);
        m->Line(c.x, c.y, c.z, w.x, w.y, w.z, rOs * 1.05f, rOs * 0.85f, os);
        for (int i = 0; i < bords.Length() - 1; i++) m->Line(w.x, w.y, w.z, bords[i].x, bords[i].y, bords[i].z, rOs * 0.7f, rOs * 0.35f, os);
        // nervures : la membrane qui touche un os s'assombrit, le bord de fuite aussi
        m->Paint([&](int x, int y, int z, Color32 col)
        {
            if (!Meme(col, membrane)) return col;
            for (int dx = -1; dx <= 1; dx++)
                for (int dy = -1; dy <= 1; dy++)
                    for (int dz = -1; dz <= 1; dz++)
                    {
                        Color32 o;
                        if (m->TryGet(x + dx, y + dy, z + dz, o) && Meme(o, os)) return membraneS;
                    }
            return col;
        });
        // griffe du pouce au poignet
        Vector3 g = w + Vector3(0.5f * sx, 2.2f, 1.5f);
        m->Line(w.x, w.y, w.z, g.x, g.y, g.z, rOs * 0.6f, 0.4f, griffe);
        return m;
    }

    // ------------------------------------------------------------------ wyverne (~2,4 m d'envergure, voxel 0,06)
    Ptr<VoxelRig> Wyverne()
    {
        auto r = NouveauRig("wyverne", 0.06f);
        Color32 ec = C("6f7a3c"), ecS = C("4b5428"), ecC = C("97a052"), ventre = C("c9b07a"), ventreS = C("a08a5a");
        Color32 membrane = C("a07a40"), membraneS = C("6e5230"), corne = C("2f2a24"), dard = C("d9c58a"), oeil = C("ffd84a");
        Color32 bouche = C("6a2424"), dent = C("ece2c4"), venin = C("b6f04a");

        auto corps = Nouveau<VoxelModel>();
        // Tronc incliné vers l'avant, poitrail haut ; la queue part de l'arrière.
        ProfilZ(*corps, 0, {-9, 13.5f, 3.4f, 3.2f, -5, 14.5f, 4.8f, 4.6f, 0, 16.5f, 5.4f, 5.2f, 3, 18.5f, 4.8f, 4.8f, 5, 20.5f, 3.4f, 3.6f}, 0.45f, ec);
        corps->Paint([&](int x, int y, int z, Color32 c) { return (Meme(c, ec) && y <= 14 + (z + 9) * 0.35f && std::abs(x + 0.5f) < 3.2f) ? ventre : c; });
        corps->Paint([&](int x, int y, int z, Color32 c) { return (Meme(c, ventre) && z % 2 == 0) ? ventreS : c; });
        Ecailles(*corps, ec, ecS, ecC, 3);
        // Cou serpentin : chaîne en S, qui s'affine, avec crête
        Liste<float> cou = {0, 21, 3, 0, 25, 4, 0, 28, 3.5f, 0, 30.5f, 5, 0, 31.5f, 8};
        Chaine(*corps, cou, 3.2f, 2.2f, ec);
        Chaine(*corps, {0, 20, 5, 0, 24, 6.5f, 0, 27.5f, 6, 0, 29.5f, 7.5f}, 2.2f, 1.6f, ventre);
        for (int k = 0; k < 7; k++)
        {
            float t = k / 6.0f;
            [[maybe_unused]] float y = Lerp(16, 30, t) + (k > 3 ? 1.5f : 3), z = Lerp(-6, 4, t) - (k > 3 ? 0 : 2);   // (y ne sert pas, comme en C#)
            int h = Haut(*corps, 0, static_cast<int>(z), 0, 60);
            if (h == INT_MIN) continue;
            corps->Line(0, h, z, 0, h + 2.5f - k * 0.15f, z - 1.8f, 0.8f, 0.4f, corne);
        }
        r->Add("corps", corps, V(0, 16, 0));

        // Tête : longue, en coin, deux cornes rabattues, mâchoire séparée.
        auto tete = Nouveau<VoxelModel>();
        ProfilZ(*tete, 0, {5, 32.5f, 2.6f, 2.6f, 8, 33, 3, 2.6f, 12, 32.6f, 2.2f, 1.8f, 15, 32.2f, 1.4f, 1.3f}, 0.4f, ec);
        Ecailles(*tete, ec, ecS, ecC, 2);
        for (int s = 0; s < 2; s++)
        {
            float sx = s == 0 ? -1.0f : 1.0f;
            Chaine(*tete, {sx * 1.8f, 34.5f, 7, sx * 2.6f, 36.5f, 4, sx * 3, 36.5f, 0.5f}, 1, 0.4f, corne);
            tete->Box(s == 0 ? -3 : 2, 34, 9, s == 0 ? -2 : 1, 34, 11, ecS);           // arcade
            tete->Set(s == 0 ? -1 : 0, 33, 15, corne);                                 // naseau
        }
        auto lueur = Nouveau<VoxelModel>();
        Oeil(*tete, *lueur, -3, 33, 10, oeil); Oeil(*tete, *lueur, 2, 33, 10, oeil);
        r->Add("tete", tete, V(0, 32, 6), "corps");
        r->Add("lueur_yeux", lueur, V(0, 32, 6), "tete");

        auto mach = Nouveau<VoxelModel>();
        ProfilZ(*mach, 0, {6, 30.4f, 2.2f, 1.2f, 14, 30.8f, 1.2f, 0.8f}, 0.3f, ventre);
        for (int z = 8; z <= 13; z += 2) { mach->Set(-2, 31, z, dent); mach->Set(1, 31, z, dent); }
        mach->Box(-1, 31, 8, 0, 31, 13, bouche);
        mach->Set(-1, 30, 14, venin);
        Soustraire(*tete, *mach);
        r->Add("machoire", mach, V(0, 31, 7), "tete");

        // Jambes : cuisses fortes, jarret, serres à trois doigts + ergot.
        for (int s = 0; s < 2; s++)
        {
            float sx = s == 0 ? -1.0f : 1.0f;
            auto j = Nouveau<VoxelModel>();
            j->Ellipsoid(sx * 4.4f, 13.5f, -1, 2.6f, 4, 3.8f, ec);
            Chaine(*j, {sx * 4.6f, 12, 0, sx * 5, 8, -4, sx * 4.8f, 2, -1}, 2.4f, 1.3f, ec);
            Ecailles(*j, ec, ecS, ecC, 2);
            int px = Mathf::FloorToInt(sx * 4.8f);
            for (int t = -1; t <= 1; t++) j->Line(px + 0.5f, 1.2f, -1, px + 0.5f + t * 1.8f, 0.6f, 3.5f, 0.75f, ecS);
            for (int t = -1; t <= 1; t++) j->Line(px + 0.5f + t * 1.8f, 0.6f, 3.5f, px + 0.5f + t * 2.0f, 0.5f, 5, 0.5f, corne);
            j->Line(px + 0.5f, 1.2f, -1.5f, px + 0.5f, 0.5f, -3.5f, 0.6f, corne);
            r->Add(s == 0 ? "jambeG" : "jambeD", j, V(sx * 4.4f, 14, -1), "corps");
        }

        // Ailes : à demi ouvertes, relevées.
        Liste<Vector3> doigts = {V(21, 31, -9), V(19.5f, 21, -13), V(14.5f, 13, -11.5f), V(8.5f, 12, -8.5f)};
        Liste<Vector3> flanc = {V(4, 13, -6.5f), V(3.6f, 16, -3)};
        for (int s = 0; s < 2; s++)
        {
            float sx = s == 0 ? -1.0f : 1.0f;
            auto a = Aile(sx, V(3.6f, 21, 1), V(9, 29.5f, 0), V(14, 35, -3.5f), doigts, flanc, 1.2f, ecS, membrane, membraneS, corne);
            r->Add(s == 0 ? "aileG" : "aileD", a, V(sx * 3.6f, 21, 1), "corps");
        }

        // Queue : longue, ondulée, terminée par un dard en lame.
        auto queue = Nouveau<VoxelModel>();
        Liste<float> qp = {0, 14, -8, 0, 12, -14, 1.5f, 9, -20, 3, 7.5f, -26, 1.5f, 8, -31, -1, 10, -35};
        Chaine(*queue, qp, 3, 0.9f, ec);
        Ecailles(*queue, ec, ecS, ecC, 2);
        for (int k = 0; k < 7; k++)
        {
            int z = -10 - k * 4; int h = Haut(*queue, k < 4 ? (k < 2 ? 0 : 1) : 1, z, 0, 40);
            if (h != INT_MIN) queue->Line(0.5f + (k < 2 ? -0.5f : 1), h, z, 0.5f + (k < 2 ? -0.5f : 1), h + 1.6f, z - 1, 0.6f, 0.3f, corne);
        }
        // dard : losange plat vertical, pointe vers l'arrière-haut
        for (int z = -41; z <= -34; z++)
        {
            float t = (z + 41) / 7.0f;
            float hh = t < 0.4f ? t * 7 : (1 - t) * 4.7f;
            for (int y = Mathf::FloorToInt(10.5f - hh); y <= Mathf::CeilToInt(10.5f + hh * 0.6f); y++)
            {
                queue->Set(-1, y + static_cast<int>((-34 - z) * 0.35f), z, dard);
                queue->Set(0, y + static_cast<int>((-34 - z) * 0.35f), z, z < -38 ? dard : ecS);
            }
        }
        queue->Set(-1, 12, -42, venin); queue->Set(-1, 13, -43, dard);
        r->Add("queue", queue, V(0, 14, -8), "corps");
        return Fin(r);
    }
}
