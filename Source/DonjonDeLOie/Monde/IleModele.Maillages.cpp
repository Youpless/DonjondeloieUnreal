// Donjon de l'Oie — maillages de l'île : terrain en colonnes (par morceaux de 32 × 32 cellules), mer, dalles des cases.
#include "IleModele.h"

#include "../Voxel/PixelIcones.h"

namespace DonjonDeLOie::Monde
{
    using namespace DonjonDeLOie::Regles;
    using namespace DonjonDeLOie::Voxel;

    float IleModele::HC(int x, int z) const
    {
        if (x < 0 || z < 0 || x >= NX || z >= NZ) return -4.0f;
        return H[I(x, z)];
    }

    void IleModele::Quad(MeshData& md, Vector3 a, Vector3 b, Vector3 c, Vector3 d, Vector3 n, Color32 ca, Color32 cb, Color32 cc, Color32 cd)
    {
        // Sens horaire vu de l'extérieur (face avant d'Unity) : on vérifie avec la normale voulue.
        if (Vector3::Dot(Vector3::Cross(b - a, c - a), n) < 0)
        {
            Vector3 t = b; b = d; d = t;
            Color32 tc = cb; cb = cd; cd = tc;
        }
        int i = md.V.Count();
        md.V.Add(a); md.V.Add(b); md.V.Add(c); md.V.Add(d);
        md.N.Add(n); md.N.Add(n); md.N.Add(n); md.N.Add(n);
        md.C.Add(ca); md.C.Add(cb); md.C.Add(cc); md.C.Add(cd);
        md.T.Add(i); md.T.Add(i + 1); md.T.Add(i + 2);
        md.T.Add(i); md.T.Add(i + 2); md.T.Add(i + 3);
    }

    Color32 IleModele::CouleurDessus(int x, int z) const
    {
        int i = I(x, z);
        Color32 c = CouleursMat[Mat[i]];
        float n = Pal::Hash01(x, 7, z);
        if (Mat[i] == 15)
        {
            // prairie fleurie : quelques cellules de fleurs
            if (n > 0.86f) return Pal::C("f2e6a0");
            if (n > 0.72f) return Pal::C("e7a0b8");
            if (n > 0.6f) return Pal::C("f4f1ea");
            c = CouleursMat[0];
        }
        return Pal::Var(c, x, 3, z, 0.06f);
    }

    /// Maillage d'un morceau de terrain (mx, mz : indices du morceau).
    Ptr<MeshData> IleModele::MaillageTerrain(int mx, int mz) const
    {
        auto md = Nouveau<MeshData>();
        int x0 = mx * Morceau, z0 = mz * Morceau;
        for (int z = z0; z < Mathf::Min(NZ, z0 + Morceau); z++)
            for (int x = x0; x < Mathf::Min(NX, x0 + Morceau); x++)
            {
                float h = HC(x, z);
                if (h < -3.0f) continue;
                float ax = X0 + static_cast<float>(x) * Cell, az = Z0 + static_cast<float>(z) * Cell, bx = ax + Cell, bz = az + Cell;
                Color32 col = CouleurDessus(x, z);
                // occlusion des coins : voisins plus hauts
                Color32 c00 = Pal::Shade(col, Occ(x, z, -1, -1, h)); Color32 c10 = Pal::Shade(col, Occ(x, z, 1, -1, h));
                Color32 c11 = Pal::Shade(col, Occ(x, z, 1, 1, h)); Color32 c01 = Pal::Shade(col, Occ(x, z, -1, 1, h));
                Quad(*md, Vector3(ax, h, az), Vector3(ax, h, bz), Vector3(bx, h, bz), Vector3(bx, h, az), Vector3::up, c00, c01, c11, c10);
                // flancs vers les voisins plus bas
                Flanc(*md, x, z, h, 1, 0, Vector3(bx, 0.0f, az), Vector3(bx, 0.0f, bz), Vector3::right);
                Flanc(*md, x, z, h, -1, 0, Vector3(ax, 0.0f, bz), Vector3(ax, 0.0f, az), Vector3::left);
                Flanc(*md, x, z, h, 0, 1, Vector3(bx, 0.0f, bz), Vector3(ax, 0.0f, bz), Vector3::forward);
                Flanc(*md, x, z, h, 0, -1, Vector3(ax, 0.0f, az), Vector3(bx, 0.0f, az), Vector3::back);
            }
        return md;
    }

    float IleModele::Occ(int x, int z, int dx, int dz, float h) const
    {
        int n = 0;
        if (HC(x + dx, z) > h + 0.01f) n++;
        if (HC(x, z + dz) > h + 0.01f) n++;
        if (HC(x + dx, z + dz) > h + 0.01f) n++;
        return n == 0 ? 1.0f : n == 1 ? 0.86f : n == 2 ? 0.74f : 0.66f;
    }

    void IleModele::Flanc(MeshData& md, int x, int z, float h, int dx, int dz, Vector3 p, Vector3 q, Vector3 n) const
    {
        float hn = HC(x + dx, z + dz);
        if (hn >= h - 0.001f) return;
        hn = Mathf::Max(hn, -3.5f);
        int i = I(x, z);
        Color32 flanc = CouleursFlanc[Mat[i]];
        Color32 dessus = CouleursMat[Mat[i] == 15 ? 0 : Mat[i]];
        int k = 0;
        for (float y = h; y > hn + 0.001f; y -= Marche, k++)
        {
            float y0 = Mathf::Max(hn, y - Marche);
            Color32 c;
            if ((k == 0 && Mat[i] <= 2) || (k == 0 && Mat[i] == 15)) c = Pal::Shade(dessus, 0.82f);   // liseré d'herbe
            else
            {
                c = Pal::Var(flanc, x * 3 + dx, Mathf::RoundToInt(y * 4), z * 3 + dz, 0.08f);
                if (Mathf::RoundToInt(y * 4) % 3 == 0) c = Pal::Shade(c, 0.9f);                       // strates
            }
            float bas = y0 <= hn + 0.001f ? 0.78f : 1.0f;                                             // pied du flanc plus sombre
            Color32 cb = Pal::Shade(c, bas * 0.92f);
            Quad(md, Vector3(p.x, y0, p.z), Vector3(p.x, y, p.z), Vector3(q.x, y, q.z), Vector3(q.x, y0, q.z), n, cb, c, c, cb);
        }
    }

    /// La mer : une grande grille ondulée, plus claire et transparente près des côtes.
    Ptr<MeshData> IleModele::MaillageMer() const
    {
        auto md = Nouveau<MeshData>();
        // une grande grille centrée sur l'île, qui déborde largement de la grille du terrain
        float cx = X0 + LargeurM() / 2.0f, cz = Z0 + ProfondeurM() / 2.0f;
        float demi = Mathf::Max(LargeurM(), ProfondeurM()) / 2.0f + 100.0f;
        const float pas = 2.0f;
        int n = static_cast<int>(demi * 2 / pas);
        Color32 peu = Pal::C("5aa5b8"); Color32 prof = Pal::C("1d4a73");
        for (int z = 0; z <= n; z++)
            for (int x = 0; x <= n; x++)
            {
                float px = cx - demi + static_cast<float>(x) * pas, pz = cz - demi + static_cast<float>(z) * pas;
                float e = Ellipse(px, pz);
                float t = Mathf::Clamp01((e - 0.95f) / 0.3f);
                Color32 c = Pal::Mix(peu, prof, t);
                c.a = static_cast<uint8_t>(255 * Mathf::Lerp(0.55f, 0.94f, t));
                md->V.Add(Vector3(px, NiveauEau - 0.05f, pz));
                md->N.Add(Vector3::up);
                md->C.Add(c);
            }
        for (int z = 0; z < n; z++)
            for (int x = 0; x < n; x++)
            {
                int a = z * (n + 1) + x, b = a + 1, c = a + n + 1, d = c + 1;
                // a(x,z) c(x,z+1) d(x+1,z+1) b(x+1,z) : horaire vu du dessus
                md->T.Add(a); md->T.Add(c); md->T.Add(d);
                md->T.Add(a); md->T.Add(d); md->T.Add(b);
            }
        return md;
    }

    // ------------------------------------------------------------------ dalles

    Color32 IleModele::CouleurType(TypeCase t)
    {
        switch (t)
        {
            case TypeCase::Ville: return Pal::C("3d64b0");
            case TypeCase::Chemin: return Pal::C("8d8a84");
            case TypeCase::Monstre: return Pal::C("b8392f");
            case TypeCase::Tresor: return Pal::C("e2ad3c");
            case TypeCase::Evenement: return Pal::C("3f86c9");
            case TypeCase::Malediction: return Pal::C("7a3fa8");
            case TypeCase::Sanctuaire: return Pal::C("e6edf2");
            case TypeCase::Marchand: return Pal::C("e07b2e");
            case TypeCase::Taverne: return Pal::C("8b5e3c");
            default: return Pal::C("7a1f1a");
        }
    }

    string IleModele::IconeType(TypeCase t)
    {
        switch (t)
        {
            case TypeCase::Ville: return "chateau";
            case TypeCase::Chemin: return "chemin";
            case TypeCase::Monstre: return "epees_croisees";
            case TypeCase::Tresor: return "coffre";
            case TypeCase::Evenement: return "point_interrogation";
            case TypeCase::Malediction: return "crane";
            case TypeCase::Sanctuaire: return "cristal";
            case TypeCase::Marchand: return "sac";
            case TypeCase::Taverne: return "chope";
            default: return "dragon";
        }
    }

    /// Dalle d'une case : pierre taillée chanfreinée, liseré de la couleur du type, icône gravée au centre. Dessus à y = 0.
    Ptr<MeshData> IleModele::MaillageDalle(TypeCase t)
    {
        Ptr<MeshData> md;
        if (dalles.TryGetValue(t, md)) return md;
        VoxelModel m;
        const int R = 9, ch = 3;
        Color32 pierre = Pal::C("a39d92"); Color32 pierreBord = Pal::C("c4bdb0"); Color32 pierreBas = Pal::C("6e6961");
        Color32 coul = CouleurType(t);
        for (int x = -R; x <= R; x++)
            for (int z = -R; z <= R; z++)
            {
                int ax = Mathf::Abs(x), az = Mathf::Abs(z);
                if (ax + az > 2 * R - ch) continue;                         // coins chanfreinés
                bool bord = ax == R || az == R || ax + az == 2 * R - ch;
                bool lisere = !bord && (ax == R - 1 || az == R - 1 || ax + az == 2 * R - ch - 1 || (ax + az == 2 * R - ch - 2 && (ax >= R - 2 || az >= R - 2)));
                m.Set(x, 0, z, pierreBas);
                m.Set(x, 1, z, Pal::Var(pierre, x, 1, z, 0.06f));
                Color32 dessus;
                if (bord) dessus = Pal::Var(pierreBord, x, 2, z, 0.05f);
                else if (lisere) dessus = coul;
                else dessus = Pal::Var(Pal::Mix(pierre, coul, 0.18f), x, 2, z, 0.05f);
                m.Set(x, 2, z, dessus);
            }
        // icône gravée : le contour creuse, les couleurs affleurent
        string ic = IconeType(t);
        int n = PixelIcones::Taille, o = n / 2;
        for (int px = 0; px < n; px++)
            for (int py = 0; py < n; py++)
            {
                Color32 c = PixelIcones::Pixel(ic, px, py);
                if (c.a == 0) continue;
                int x = px - o, z = py - o;
                bool contour = c.r < 40 && c.g < 40 && c.b < 40;
                if (contour) { m.Remove(x, 2, z); m.Set(x, 1, z, Pal::C("3a3430")); }
                else m.Set(x, 2, z, c);
            }
        md = VoxelMesher::Build(m, Vector3(0.5f, 3.0f, 0.5f), 0.1f, 0.02f, true);
        dalles[t] = md;
        return md;
    }

    // Remise à zéro des variables statiques (une partie relancée dans Unreal repart comme un lancement d'Unity).
    const bool IleModele::remiseAZero = Boucle::AuReinitialiser([] { IleModele::dalles.Clear(); });
}
