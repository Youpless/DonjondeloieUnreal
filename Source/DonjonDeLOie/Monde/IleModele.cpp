// Donjon de l'Oie — l'île, calculée sans Unity : relief en colonnes (cellules de 50 cm, marches de 25 cm), zones,
// chemin entre les cases, dalles des cases, emplacements du décor et des lieux, lampes.
// La vue Unity (IleVue) et le rendu hors Unity (outils d'aperçu) lisent le même modèle.
#include "IleModele.h"

namespace DonjonDeLOie::Monde
{
    using namespace DonjonDeLOie::Regles;
    using namespace DonjonDeLOie::Voxel;

    /// options : la carte (marge, décor, graine, décors posés à la main) ; null pour les réglages de la carte standard.
    IleModele::IleModele(const Ptr<Regles::Plateau>& p, const Ptr<Regles::CarteDef>& options)
    {
        Plateau = p;
        Options = options != nullptr ? options : Nouveau<CarteDef>();
        graine = Options->Graine;
        float minX = 1e9f, maxX = -1e9f, minZ = 1e9f, maxZ = -1e9f, cx = 0, cz = 0; int n = 0;
        for (auto& c : p->Cases)
        {
            minX = Mathf::Min(minX, c->X); maxX = Mathf::Max(maxX, c->X); minZ = Mathf::Min(minZ, c->Z); maxZ = Mathf::Max(maxZ, c->Z);
            cx += c->X; cz += c->Z; n++;
        }
        if (p->Nombre() == 0) { minX = minZ = -10; maxX = maxZ = 10; }
        Centre = n > 0 ? Vector3(cx / static_cast<float>(n), 0.0f, cz / static_cast<float>(n)) : Vector3::zero;
        float bord = Mathf::Max(4.0f, Options->Marge) + 12.0f;
        X0 = Mathf::Floor((minX - bord) / Cell) * Cell; Z0 = Mathf::Floor((minZ - bord) / Cell) * Cell;
        NX = Mathf::Min(TailleMax, static_cast<int>(std::ceil(static_cast<double>((maxX + bord - X0) / Cell))));
        NZ = Mathf::Min(TailleMax, static_cast<int>(std::ceil(static_cast<double>((maxZ + bord - Z0) / Cell))));
        H = Liste<float>(NX * NZ); Mat = Liste<uint8_t>(NX * NZ); ZoneCell = Liste<Zone>(NX * NZ); Dist = Liste<float>(NX * NZ); Cote = Liste<float>(NX * NZ);
        Construire();
    }

    float IleModele::CX(int x) const { return X0 + (static_cast<float>(x) + 0.5f) * Cell; }
    float IleModele::CZ(int z) const { return Z0 + (static_cast<float>(z) + 0.5f) * Cell; }
    float IleModele::LargeurM() const { return static_cast<float>(NX) * Cell; }
    float IleModele::ProfondeurM() const { return static_cast<float>(NZ) * Cell; }

    float IleModele::HauteurEn(float x, float z) const
    {
        int ix = Mathf::Clamp(static_cast<int>((x - X0) / Cell), 0, NX - 1), iz = Mathf::Clamp(static_cast<int>((z - Z0) / Cell), 0, NZ - 1);
        return H[I(ix, iz)];
    }

    Zone IleModele::ZoneEn(float x, float z) const
    {
        int ix = Mathf::Clamp(static_cast<int>((x - X0) / Cell), 0, NX - 1), iz = Mathf::Clamp(static_cast<int>((z - Z0) / Cell), 0, NZ - 1);
        return ZoneCell[I(ix, iz)];
    }

    float IleModele::DistanceChemin(float x, float z) const
    {
        int ix = Mathf::Clamp(static_cast<int>((x - X0) / Cell), 0, NX - 1), iz = Mathf::Clamp(static_cast<int>((z - Z0) / Cell), 0, NZ - 1);
        return Dist[I(ix, iz)];
    }

    // ------------------------------------------------------------------ bruit

    float IleModele::Bruit(float x, float z, int o) const
    {
        return Mathf::PerlinNoise(x + static_cast<float>(graine) * 13.7f + static_cast<float>(o) * 71.3f, z + static_cast<float>(graine) * 5.3f + static_cast<float>(o) * 37.9f);
    }

    float IleModele::Fbm(float x, float z, int o) const
    {
        return Bruit(x, z, o) * 0.55f + Bruit(x * 2.1f, z * 2.1f, o + 1) * 0.3f + Bruit(x * 4.3f, z * 4.3f, o + 2) * 0.15f;
    }

    float IleModele::Lisse(float a, float b, float v)
    {
        float t = Mathf::Clamp01((v - a) / (b - a));
        return t * t * (3 - 2 * t);
    }

    /// < 1 sur terre (0 au cœur de l'île), 1 au rivage, > 1 en mer — calculé depuis le tracé du chemin (voir CalculerCote).
    float IleModele::Ellipse(float x, float z) const
    {
        int ix = static_cast<int>(std::floor(static_cast<double>((x - X0) / Cell))), iz = static_cast<int>(std::floor(static_cast<double>((z - Z0) / Cell)));
        if (ix < 0 || iz < 0 || ix >= NX || iz >= NZ)
        {
            float dx = Mathf::Max(0.0f, Mathf::Max(X0 - x, x - (X0 + LargeurM()))), dz = Mathf::Max(0.0f, Mathf::Max(Z0 - z, z - (Z0 + ProfondeurM())));
            return 1.4f + Mathf::Sqrt(dx * dx + dz * dz) / 30.0f;
        }
        return Cote[I(ix, iz)];
    }

    /// La terre = une bande de « Marge » mètres autour du chemin (bord bruité) + tout ce que les boucles entourent.
    /// Puis, pour chaque cellule, la distance au rivage : Cote = 1 − d/30 sur terre, 1 + d/30 en mer.
    void IleModele::CalculerCote()
    {
        float marge = Mathf::Max(3.0f, Options->Marge);
        Liste<bool> terre(NX * NZ);
        for (int z = 0; z < NZ; z++)
            for (int x = 0; x < NX; x++)
            {
                float px = CX(x), pz = CZ(z);
                float m = marge * (0.8f + 0.45f * Fbm(px * 0.05f, pz * 0.05f, 3));
                terre[I(x, z)] = Dist[I(x, z)] < m;
            }
        // ce que l'on ne peut pas rejoindre depuis le bord de la grille sans traverser la terre est enclos : c'est de la terre
        Liste<bool> mer(NX * NZ);
        Liste<int> file;                                        // la file (Queue<int>) : ajout au bout, lecture dans l'ordre
        int tete = 0;
        for (int x = 0; x < NX; x++) { Semer(x, 0, terre, mer, file); Semer(x, NZ - 1, terre, mer, file); }
        for (int z = 0; z < NZ; z++) { Semer(0, z, terre, mer, file); Semer(NX - 1, z, terre, mer, file); }
        while (file.Count() - tete > 0)
        {
            int k = file[tete++]; int x = k % NX, z = k / NX;
            if (x > 0) Semer(x - 1, z, terre, mer, file);
            if (x < NX - 1) Semer(x + 1, z, terre, mer, file);
            if (z > 0) Semer(x, z - 1, terre, mer, file);
            if (z < NZ - 1) Semer(x, z + 1, terre, mer, file);
        }
        for (int i = 0; i < terre.Count(); i++) terre[i] = !mer[i];
        // distance au rivage (deux passes de chanfrein), en mètres
        Liste<float> dTerre = Chanfrein(terre, true); Liste<float> dMer = Chanfrein(terre, false);
        for (int i = 0; i < Cote.Count(); i++) Cote[i] = terre[i] ? 1.0f - dTerre[i] / 30.0f : 1.0f + dMer[i] / 30.0f;
    }

    void IleModele::Semer(int x, int z, const Liste<bool>& terre, Liste<bool>& mer, Liste<int>& file) const
    {
        int k = I(x, z);
        if (terre[k] || mer[k]) return;
        mer[k] = true; file.Add(k);
    }

    /// Distance (m) de chaque cellule « dedans » à la cellule la plus proche « dehors ».
    Liste<float> IleModele::Chanfrein(const Liste<bool>& terre, bool dedansEstTerre) const
    {
        const float A = Cell, D = Cell * 1.4142f, Inf = 1e6f;
        Liste<float> d(NX * NZ);
        for (int i = 0; i < d.Count(); i++) d[i] = terre[i] == dedansEstTerre ? Inf : 0.0f;
        for (int z = 0; z < NZ; z++)
            for (int x = 0; x < NX; x++)
            {
                int k = I(x, z); if (d[k] == 0) continue;
                float v = d[k];
                if (x > 0) v = Mathf::Min(v, d[k - 1] + A);
                if (z > 0) v = Mathf::Min(v, d[k - NX] + A);
                if (x > 0 && z > 0) v = Mathf::Min(v, d[k - NX - 1] + D);
                if (x < NX - 1 && z > 0) v = Mathf::Min(v, d[k - NX + 1] + D);
                d[k] = v;
            }
        for (int z = NZ - 1; z >= 0; z--)
            for (int x = NX - 1; x >= 0; x--)
            {
                int k = I(x, z); if (d[k] == 0) continue;
                float v = d[k];
                if (x < NX - 1) v = Mathf::Min(v, d[k + 1] + A);
                if (z < NZ - 1) v = Mathf::Min(v, d[k + NX] + A);
                if (x < NX - 1 && z < NZ - 1) v = Mathf::Min(v, d[k + NX + 1] + D);
                if (x > 0 && z < NZ - 1) v = Mathf::Min(v, d[k + NX - 1] + D);
                d[k] = v;
            }
        for (int i = 0; i < d.Count(); i++)
            if (d[i] >= Inf) d[i] = 30.0f;
        return d;
    }

    // ------------------------------------------------------------------ construction

    void IleModele::Construire()
    {
        const Regles::Plateau& pl = *Plateau;
        Liste<float> tmp(NX * NZ);
        Montagne = PlacerMontagne();

        // 1. distance au chemin, zone la plus proche
        for (int z = 0; z < NZ; z++)
            for (int x = 0; x < NX; x++)
            {
                float px = CX(x), pz = CZ(z);
                float best = 1e9f;
                for (int c = 0; c < pl.Nombre(); c++)
                    for (int s : pl[c]->Suivants)
                    {
                        float d = DistSeg(px, pz, pl[c]->X, pl[c]->Z, pl[s]->X, pl[s]->Z);
                        if (d < best) best = d;
                    }
                Dist[I(x, z)] = best;
                float bz = 1e9f; Zone zo = Zone::Village;
                for (int c = 0; c < pl.Nombre(); c++)
                {
                    float dx = px - pl[c]->X, dz = pz - pl[c]->Z;
                    float d = Mathf::Sqrt(dx * dx + dz * dz) + (Bruit(px * 0.18f, pz * 0.18f, 10 + static_cast<int>(pl[c]->Zone)) - 0.5f) * 5.0f;
                    if (pl[c]->Raccourci) d += 4.0f;      // la grande boucle donne le ton, les raccourcis colorent leurs abords
                    if (d < bz) { bz = d; zo = pl[c]->Zone; }
                }
                float dm = Mathf::Sqrt((px - Montagne.x) * (px - Montagne.x) + (pz - Montagne.z) * (pz - Montagne.z));
                if (aMontagne && dm < 10.0f + Bruit(px * 0.2f, pz * 0.2f, 20) * 3.0f) zo = Zone::Antre;
                else if (zo == Zone::Antre)
                {
                    // loin de la montagne, le raccourci de l'antre traverse les Pics
                    zo = Zone::Pics;
                }
                ZoneCell[I(x, z)] = zo;
            }

        CalculerCote();

        // 2. relief brut
        for (int z = 0; z < NZ; z++)
            for (int x = 0; x < NX; x++)
            {
                float px = CX(x), pz = CZ(z);
                tmp[I(x, z)] = Relief(px, pz, ZoneCell[I(x, z)], Dist[I(x, z)]);
            }
        // 3. lissage (deux passes) pour fondre les zones
        for (int passe = 0; passe < 2; passe++)
        {
            Liste<float> b = tmp;                               // (float[])tmp.Clone()
            for (int z = 1; z < NZ - 1; z++)
                for (int x = 1; x < NX - 1; x++)
                {
                    float s = 0;
                    for (int dz = -1; dz <= 1; dz++)
                        for (int dx = -1; dx <= 1; dx++) s += b[I(x + dx, z + dz)];
                    tmp[I(x, z)] = s / 9.0f;
                }
        }
        // 4. hauteur des cases
        Cases = Liste<Vector3>(pl.Nombre());
        Liste<float> hc(pl.Nombre());
        for (int c = 0; c < pl.Nombre(); c++)
        {
            float h = Echantillon(tmp, pl[c]->X, pl[c]->Z);
            h = Mathf::Max(0.5f, Mathf::Min(h, pl[c]->Zone == Zone::Pics || pl[c]->Zone == Zone::Antre ? 2.5f : 1.5f));
            hc[c] = Quant(h);
        }
        // 5. le chemin aplanit le terrain, la montagne s'élève
        for (int z = 0; z < NZ; z++)
            for (int x = 0; x < NX; x++)
            {
                int i = I(x, z);
                float px = CX(x), pz = CZ(z);
                float h = tmp[i];
                float cible = HauteurChemin(px, pz, hc);
                float d = Dist[i];
                if (d < 1.7f) h = cible;
                else if (d < 3.6f) h = Mathf::Lerp(cible, h, Lisse(1.7f, 3.6f, d));
                H[i] = Quant(h);
            }
        for (int c = 0; c < pl.Nombre(); c++)
            Cases[c] = Vector3(pl[c]->X, hc[c] + EpaisseurDalle, pl[c]->Z);

        // 6. matières
        for (int z = 0; z < NZ; z++)
            for (int x = 0; x < NX; x++)
            {
                int i = I(x, z);
                Mat[i] = Matiere(CX(x), CZ(z), ZoneCell[i], H[i], Dist[i]);
            }

        PlacerLieux();
        Semer();
    }

    /// Derrière la case Antre (s'il y en a une), du côté opposé au chemin, là où la montagne ne recouvre aucune route.
    Vector3 IleModele::PlacerMontagne()
    {
        const Regles::Plateau& pl = *Plateau;
        int antre = -1;
        for (int i = 0; i < pl.Nombre(); i++)
            if (pl[i]->Type == TypeCase::Antre) antre = i;
        aMontagne = antre >= 0;
        if (!aMontagne) return Vector3(1e5f, 0.0f, 1e5f);
        Vector3 pa(pl[antre]->X, 0.0f, pl[antre]->Z);
        Vector3 best = pa; float bestScore = -1e9f;
        for (int a = 0; a < 24; a++)
        {
            float ang = static_cast<float>(a) * Mathf::Pi * 2.0f / 24.0f;
            Vector3 dir(Mathf::Cos(ang), 0.0f, Mathf::Sin(ang));
            Vector3 p = pa + dir * 7.5f;
            float libre = 1e9f;
            for (int c = 0; c < pl.Nombre(); c++)
                for (int s : pl[c]->Suivants)
                {
                    if (c == antre || s == antre) continue;
                    libre = Mathf::Min(libre, DistSeg(p.x, p.z, pl[c]->X, pl[c]->Z, pl[s]->X, pl[s]->Z));
                }
            // préférer : loin des autres routes, vers le cœur de l'île, et pas vers la caméra (sud)
            Vector3 versCentre = Centre - pa; versCentre.y = 0;
            float score = Mathf::Min(libre, 9.0f) * 2.0f + (versCentre.sqrMagnitude() > 1.0f ? Vector3::Dot(dir, versCentre.normalized()) * 3.0f : 0.0f) + dir.z * 1.5f;
            if (score > bestScore) { bestScore = score; best = p; }
        }
        return best;
    }

    float IleModele::Quant(float h) { return Mathf::Round(h / Marche) * Marche; }

    float IleModele::Echantillon(const Liste<float>& t, float x, float z) const
    {
        int ix = Mathf::Clamp(static_cast<int>((x - X0) / Cell), 0, NX - 1), iz = Mathf::Clamp(static_cast<int>((z - Z0) / Cell), 0, NZ - 1);
        return t[I(ix, iz)];
    }

    float IleModele::DistSeg(float px, float pz, float ax, float az, float bx, float bz)
    {
        float vx = bx - ax, vz = bz - az, l2 = vx * vx + vz * vz;
        float t = l2 > 0 ? Mathf::Clamp01(((px - ax) * vx + (pz - az) * vz) / l2) : 0.0f;
        float dx = ax + vx * t - px, dz = az + vz * t - pz;
        return Mathf::Sqrt(dx * dx + dz * dz);
    }

    float IleModele::HauteurChemin(float px, float pz, const Liste<float>& hc) const
    {
        const Regles::Plateau& pl = *Plateau;
        float best = 1e9f, h = 0.5f;
        for (int c = 0; c < pl.Nombre(); c++)
            for (int s : pl[c]->Suivants)
            {
                float ax = pl[c]->X, az = pl[c]->Z, bx = pl[s]->X, bz = pl[s]->Z;
                float vx = bx - ax, vz = bz - az, l2 = vx * vx + vz * vz;
                float t = l2 > 0 ? Mathf::Clamp01(((px - ax) * vx + (pz - az) * vz) / l2) : 0.0f;
                float dx = ax + vx * t - px, dz = az + vz * t - pz, d = dx * dx + dz * dz;
                if (d < best) { best = d; h = Mathf::Lerp(hc[c], hc[s], Lisse(0.25f, 0.75f, t)); }
            }
        return h;
    }

    float IleModele::Relief(float x, float z, Zone zo, float d) const
    {
        float e = Ellipse(x, z);
        float terre = 0.55f + (Fbm(x * 0.07f, z * 0.07f, 0) - 0.5f) * 0.7f;
        float f = Fbm(x * 0.11f, z * 0.11f, 4);
        switch (zo)
        {
            case Zone::Village: terre += 0.1f * f; break;
            case Zone::Bois: terre += 0.25f + 1.0f * f * Lisse(2.0f, 7.0f, d); break;
            case Zone::Marais: terre += -1.05f + 0.9f * f; break;
            case Zone::Cryptes: terre += 0.35f + 0.6f * f * Lisse(2.0f, 6.0f, d); break;
            case Zone::Pics:
            {
                float crete = 1.0f - Mathf::Abs(Fbm(x * 0.09f, z * 0.09f, 6) * 2.0f - 1.0f);
                terre += 0.5f + 3.2f * Lisse(2.5f, 11.0f, d) * (0.3f + crete * 0.8f);
                break;
            }
            case Zone::Antre: terre += 0.9f + 1.8f * Lisse(3.0f, 10.0f, d) * f; break;
        }
        // le centre de l'île forme un plateau
        terre += 0.7f * Lisse(0.7f, 0.3f, e);
        // les falaises et la plage
        float plage = Lisse(0.9f, 1.03f, e);
        float h = Mathf::Lerp(terre, -1.4f, plage);
        if (e > 1.08f) h = -1.6f - (e - 1.08f) * 4.0f;
        return h;
    }

    // Matières : 0 herbe claire, 1 herbe de forêt, 2 mousse olive, 3 boue, 4 herbe morte, 5 terre sombre, 6 cendre, 7 roche,
    // 8 roche sombre, 9 sable, 10 fond mouillé, 11 chemin de terre, 12 pavés, 13 chemin de cendre, 14 chemin de boue, 15 fleurs
    uint8_t IleModele::Matiere(float x, float z, Zone zo, float h, float d) const
    {
        float e = Ellipse(x, z);
        if (h < NiveauEau - 0.01f) return static_cast<uint8_t>(e > 0.95f ? 10 : (zo == Zone::Marais ? 3 : 10));
        if (e > 0.93f && h < 0.6f) return 9;
        float bruit = Bruit(x * 0.6f, z * 0.6f, 30);
        float grosBruit = Bruit(x * 0.17f, z * 0.17f, 31);
        if (d < 0.75f + (bruit - 0.5f) * 0.3f)
        {
            switch (zo)
            {
                case Zone::Village: return 12;
                case Zone::Cryptes: return 12;
                case Zone::Marais: return 14;
                case Zone::Pics:
                case Zone::Antre: return 13;
                default: return 11;
            }
        }
        switch (zo)
        {
            case Zone::Village: return static_cast<uint8_t>(bruit > 0.78f && d > 2.5f ? 15 : 0);
            case Zone::Bois: return static_cast<uint8_t>(grosBruit > 0.7f ? 0 : 1);
            case Zone::Marais: return static_cast<uint8_t>(grosBruit > 0.66f ? 3 : 2);
            case Zone::Cryptes: return static_cast<uint8_t>(grosBruit > 0.68f ? 5 : 4);
            case Zone::Pics: return static_cast<uint8_t>(h > 2.2f && grosBruit > 0.45f ? 7 : (grosBruit > 0.72f ? 8 : 6));
            default: return static_cast<uint8_t>(grosBruit > 0.6f ? 8 : 7);
        }
    }
}
