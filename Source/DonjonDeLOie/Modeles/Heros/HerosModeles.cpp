// Donjon de l'Oie — les six héros, en voxels sculptés (style Battlegrade).
// Repère commun à toutes les pièces : 1 voxel = 5 cm, origine au sol entre les pieds, le héros regarde vers +z,
// sa droite est +x. Gabarit commun (voxels, côté droit ; la gauche est en miroir x ↔ −1 − x) :
// jambe x 1..4, y 0..8 (pivot hanche y 8) ; buste x −5..4, y 8..15, z −3..3 ; bras x 5..7, y 4..15
// (pivot épaule y 14,5, main y 4..7) ; tête x −4..3, y 16..22, visage en z = 3, yeux en y = 19.
// Chaque classe est construite dans son propre fichier (HerosModeles.<Classe>.cpp).
#include "HerosInterne.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace DonjonDeLOie::Modeles::HerosModeles
{
    using namespace DonjonDeLOie::Voxel;

    Ptr<VoxelRig> Construire(Regles::ClasseHeros c, Color32 couleur)
    {
        switch (c)
        {
            case Regles::ClasseHeros::Guerrier: return Guerrier(couleur);
            case Regles::ClasseHeros::Mage: return Mage(couleur);
            case Regles::ClasseHeros::Voleur: return Voleur(couleur);
            case Regles::ClasseHeros::Clerc: return Clerc(couleur);
            case Regles::ClasseHeros::Rodeur: return Rodeur(couleur);
            default: return Barbare(couleur);
        }
    }

    // ------------------------------------------------------------------ atelier : les pièces d'un héros

    // (l'atelier, la structure Atelier, est dans HerosInterne.h : tous les fichiers de la famille s'en servent)

    Ptr<VoxelRig> Assembler(const string& nom, const Atelier& a)
    {
        auto r = Nouveau<VoxelRig>();
        r->Name = nom;
        r->VoxelSize = 0.05f;
        r->Origin = Vector3::zero;
        r->Add("corps", a.Corps, a.PCorps);
        r->Add("tete", a.Tete, a.PTete, "corps");
        r->Add("brasG", a.BrasG, a.PBrasG, "corps");
        r->Add("brasD", a.BrasD, a.PBrasD, "corps");
        r->Add("jambeG", a.JambeG, a.PJambeG, "corps");
        r->Add("jambeD", a.JambeD, a.PJambeD, "corps");
        if (a.Arme->Count() > 0) r->Add("arme", a.Arme, a.PArme, "brasD");
        if (a.Bouclier->Count() > 0) r->Add("bouclier", a.Bouclier, a.PBouclier, "brasG");
        if (a.Cape->Count() > 0) r->Add("cape", a.Cape, a.PCape, "corps");
        if (a.Lueur->Count() > 0) r->Add(a.NomLueur, a.Lueur, a.PArme, "arme");
        return r;
    }

    // ------------------------------------------------------------------ outils de sculpture

    /// Boîte taillée : la boîte pleine, puis les arêtes choisies retirées (chanfrein d'un voxel).
    /// AretesV : les 4 arêtes verticales ; AretesHaut / AretesBas : le pourtour de la face du dessus / du dessous ;
    /// AretesAvant : le pourtour de la face avant (+z).
    void Taille(VoxelModel& m, int x0, int y0, int z0, int x1, int y1, int z1, Color32 c, int aretes)
    {
        m.Box(x0, y0, z0, x1, y1, z1, c);
        for (int x = x0; x <= x1; x++)
            for (int y = y0; y <= y1; y++)
                for (int z = z0; z <= z1; z++)
                {
                    bool bx = x == x0 || x == x1, by0 = y == y0, by1 = y == y1, bz = z == z0 || z == z1;
                    bool enleve = false;
                    if ((aretes & AretesV) != 0 && bx && bz) enleve = true;
                    if ((aretes & AretesHaut) != 0 && by1 && (bx || bz)) enleve = true;
                    if ((aretes & AretesBas) != 0 && by0 && (bx || bz)) enleve = true;
                    if ((aretes & AretesAvant) != 0 && z == z1 && (bx || by0 || by1)) enleve = true;
                    if (enleve) m.Remove(x, y, z);
                }
    }

    /// Repeint les voxels existants d'une boîte.
    void Peint(VoxelModel& m, int x0, int y0, int z0, int x1, int y1, int z1, Color32 c)
    {
        if (x0 > x1) std::swap(x0, x1);
        if (y0 > y1) std::swap(y0, y1);
        if (z0 > z1) std::swap(z0, z1);
        for (int x = x0; x <= x1; x++)
            for (int y = y0; y <= y1; y++)
                for (int z = z0; z <= z1; z++)
                    if (m.Has(x, y, z)) m.Set(x, y, z, c);
    }

    /// Retire les voxels d'une boîte.
    void Vide(VoxelModel& m, int x0, int y0, int z0, int x1, int y1, int z1)
    {
        for (int x = std::min(x0, x1); x <= std::max(x0, x1); x++)
            for (int y = std::min(y0, y1); y <= std::max(y0, y1); y++)
                for (int z = std::min(z0, z1); z <= std::max(z0, z1); z++) m.Remove(x, y, z);
    }

    /// Copie tournée d'un modèle autour d'un pivot (échantillonnage inverse : pas de trous).
    Ptr<VoxelModel> Pivoter(const VoxelModel& src, Vector3 pivot, Quaternion q)
    {
        auto res = Nouveau<VoxelModel>();
        Vector3Int a, b;
        if (!src.Bounds(a, b)) return res;
        // boîte englobante de la copie tournée
        Vector3 mn(1e9f, 1e9f, 1e9f);
        Vector3 mx = -mn;
        for (int i = 0; i < 8; i++)
        {
            Vector3 p(static_cast<float>((i & 1) == 0 ? a.x : b.x + 1), static_cast<float>((i & 2) == 0 ? a.y : b.y + 1),
                      static_cast<float>((i & 4) == 0 ? a.z : b.z + 1));
            Vector3 r = q * (p - pivot) + pivot;
            mn = Vector3::Min(mn, r);
            mx = Vector3::Max(mx, r);
        }
        Quaternion inv = Quaternion::Inverse(q);
        for (int x = static_cast<int>(std::floor(static_cast<double>(mn.x))) - 1; x <= static_cast<int>(std::ceil(static_cast<double>(mx.x))); x++)
            for (int y = static_cast<int>(std::floor(static_cast<double>(mn.y))) - 1; y <= static_cast<int>(std::ceil(static_cast<double>(mx.y))); y++)
                for (int z = static_cast<int>(std::floor(static_cast<double>(mn.z))) - 1; z <= static_cast<int>(std::ceil(static_cast<double>(mx.z))); z++)
                {
                    Vector3 s = inv * (Vector3(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f, static_cast<float>(z) + 0.5f) - pivot) + pivot;
                    Color32 c;
                    if (src.TryGet(static_cast<int>(std::floor(static_cast<double>(s.x))), static_cast<int>(std::floor(static_cast<double>(s.y))),
                                   static_cast<int>(std::floor(static_cast<double>(s.z))), c))
                        res->Set(x, y, z, c);
                }
        return res;
    }

    /// Copie miroir d'un modèle (x ↔ −1 − x) : la pièce droite devient la gauche.
    void Miroir(const VoxelModel& src, VoxelModel& dst)
    {
        for (auto& kv : src.Tous()) dst.Set(-1 - kv.Key.x, kv.Key.y, kv.Key.z, kv.Value);
    }

    /// Symétrise un modèle construit sur sa moitié droite (x ≥ 0).
    void Symetrise(VoxelModel& m)
    {
        m.Carve([&](int x, int y, int z) { return x < 0; });
        m.MirrorX(0);
    }


    // ------------------------------------------------------------------ gabarit commun : tête, jambes, bras

    /// Tête nue taillée : x −4..3, y 16..22, z −3..3 ; visage sur le plan z = 3, yeux en y = 19.
    void TeteNue(VoxelModel& t, Color32 peau, Color32 sourcils)
    {
        Taille(t, -4, 16, -3, 3, 22, 3, peau, AretesV | AretesHaut | AretesBas);
        t.Box(-4, 16, 3, 3, 16, 3, peau);                         // mâchoire franche devant
        t.Remove(-4, 16, 3); t.Remove(3, 16, 3);
        t.Box(-5, 18, 0, -5, 19, 0, Pal::PeauOmbre); t.Box(4, 18, 0, 4, 19, 0, Pal::PeauOmbre);   // oreilles
        Visage(t, 19, 3, sourcils, true);
    }

    /// Jambe droite (x 1..4) : chausse jusqu'à y 7 (le bas du buste couvre la hanche), botte jusqu'à hautBotte, pointe du pied en z = 3.
    void Jambe(VoxelModel& j, Color32 chausse, Color32 botte, int hautBotte, Color32 semelle)
    {
        Taille(j, 1, 2, -1, 4, 7, 1, chausse, AretesV);
        Taille(j, 1, 0, -2, 4, hautBotte, 2, botte, AretesV);
        j.Box(1, 0, 3, 4, 1, 3, botte); j.Remove(1, 1, 3); j.Remove(4, 1, 3);
        j.Box(1, 0, -2, 4, 0, 3, semelle);
    }

    /// Bras droit (x 5..7) : manche jusqu'au poignet (y 8), main y 4..7 (z −1..2), un peu grosse.
    void Bras(VoxelModel& b, Color32 manche, Color32 main)
    {
        Taille(b, 5, 8, -1, 7, 15, 1, manche, AretesV | AretesHaut);
        Taille(b, 5, 4, -1, 7, 7, 2, main, AretesV | AretesBas);
        Peint(b, 5, 4, -1, 7, 4, 2, Sh(main, 0.85f));
    }


    /// Capuche taillée (section octogonale qui se resserre en haut), ouverte devant sur le visage.
    /// À appeler avant de dessiner la tête : elle évide l'emplacement de la tête. pointe : longueur de la pointe dans la nuque.
    void Capuche(VoxelModel& t, Color32 c, Color32 cS, int pointe)
    {
        for (int y = 16; y <= 25; y++)
        {
            float h = y <= 21 ? 5.5f : y == 22 ? 5.0f : y == 23 ? 4.5f : y == 24 ? 3.5f : 2.0f;
            for (int x = -6; x <= 5; x++)
                for (int z = -7; z <= 6; z++)
                {
                    float dx = Mathf::Abs(static_cast<float>(x) + 0.5f), dz = Mathf::Abs(static_cast<float>(z) + 0.5f + (y >= 23 ? 0.6f : 0.0f));
                    if (dx <= h && dz <= h && dx + dz <= h * 1.45f) t.Set(x, y, z, y == 25 ? cS : c);
                }
        }
        // ouverture du visage et place de la tête
        Vide(t, -4, 16, -3, 3, 22, 3);
        Vide(t, -3, 16, 3, 2, 21, 6); Vide(t, -4, 16, 3, 3, 18, 6);
        // rebord sombre autour de l'ouverture (l'ombre de la capuche)
        t.Paint([&](int x, int y, int z, Color32 col) { return (z >= 3 && y <= 22) ? cS : col; });
        for (int i = 0; i < pointe; i++) t.Box(-1, 21 - i * 2, -6 - i, 0, 23 - i * 2, -6 - i, i == pointe - 1 ? cS : c);
    }

    Color32 Sh(Color32 c, float f) { return Pal::Shade(c, f); }

    /// Visage commun : yeux (2 voxels sombres), nez, sourcils. z = plan du visage.
    void Visage(VoxelModel& t, int yYeux, int z, Color32 sourcils, bool nez)
    {
        Color32 oeil = Pal::C("221c1c");
        t.Set(-2, yYeux, z, oeil); t.Set(1, yYeux, z, oeil);
        // une étincelle claire au-dessus de l'œil côté soleil, pour l'expression
        if (sourcils.a > 0) { t.Box(-3, yYeux + 1, z, -2, yYeux + 1, z, sourcils); t.Box(1, yYeux + 1, z, 2, yYeux + 1, z, sourcils); }
        if (nez) { t.Box(-1, yYeux - 1, z + 1, 0, yYeux, z + 1, Pal::PeauOmbre); t.Box(-1, yYeux - 1, z + 1, 0, yYeux - 1, z + 1, Pal::Peau); }
    }
}
