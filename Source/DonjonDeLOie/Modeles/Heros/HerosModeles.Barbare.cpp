// Donjon de l'Oie — le Barbare : torse nu taillé, fourrure sur les épaules, casque à cornes, tresses,
// grande hache à deux mains posée sur l'épaule ; pagne et ceinture à la couleur du joueur.
#include "HerosInterne.h"

namespace DonjonDeLOie::Modeles::HerosModeles
{
    using namespace DonjonDeLOie::Voxel;

    Ptr<VoxelRig> Barbare(Color32 coul)
    {
        Atelier a;
        a.PBrasD = Vector3(7.5f, 14.5f, 0.5f); a.PBrasG = Vector3(-7.5f, 14.5f, 0.5f);
        a.PArme = Vector3(7.5f, 5.5f, 0.5f);
        Color32 coulS = Sh(coul, 0.68f);
        Color32 peau = Pal::C("d79a72"), peauS = Pal::C("ad7352"), peauC = Pal::C("eab38c");
        Color32 four = Pal::C("8a6a4a"), fourS = Pal::C("5e452e"), fourC = Pal::C("b0906a"), roux = Pal::C("6a4024"), rouxS = Pal::C("4a2a16");

        // ---- corps : torse en V, pectoraux et abdos en plaques, ceinture et pagne couleur joueur
        auto& k = *a.Corps;
        Taille(k, -5, 6, -3, 4, 9, 3, Pal::CuirSombre, AretesV);                // hanches (culotte de cuir)
        Taille(k, -5, 10, -3, 4, 11, 3, peau, AretesV);                          // taille
        Taille(k, -6, 12, -3, 5, 16, 3, peau, AretesV | AretesHaut);             // poitrine large
        // pectoraux : deux plaques en saillie, séparées au centre
        Taille(k, -5, 13, 4, -1, 15, 4, peau, AretesV); Taille(k, 0, 13, 4, 4, 15, 4, peau, AretesV);
        Peint(k, -5, 13, 4, 4, 13, 4, peauS);
        k.Set(-3, 14, 5, peauS); k.Set(2, 14, 5, peauS);
        // abdos : 2 × 2 plaques
        for (int r = 0; r < 2; r++) { int y = 10 + r; k.Box(-3, y, 4, -2, y, 4, r == 0 ? peauC : peau); k.Box(1, y, 4, 2, y, 4, r == 0 ? peauC : peau); }
        Peint(k, -1, 10, 3, 0, 12, 3, peauS);
        Peint(k, -6, 12, -3, -6, 15, 3, peauS); Peint(k, 5, 12, -3, 5, 15, 3, peauS);
        // dos : omoplates
        Taille(k, -5, 13, -4, -1, 15, -4, peau, AretesV); Taille(k, 0, 13, -4, 4, 15, -4, peau, AretesV);
        Peint(k, -1, 10, -3, 0, 15, -3, peauS);
        // cicatrice
        k.Set(2, 15, 5, Pal::C("c2786a")); k.Set(1, 14, 5, Pal::C("c2786a")); k.Set(3, 13, 4, Pal::C("c2786a"));
        // large ceinture à la couleur du joueur, boucle de fer
        Taille(k, -6, 8, -4, 5, 9, 4, coul, AretesV); Peint(k, -6, 8, -4, 5, 8, 4, coulS);
        k.Box(-2, 7, 5, 1, 10, 5, Pal::Fer); k.Box(-1, 8, 5, 0, 9, 5, Pal::FerSombre); k.Box(-1, 8, 6, 0, 9, 6, Pal::Acier);
        // pagne devant et derrière (couleur joueur), bord frangé
        k.Box(-4, 1, 4, 3, 7, 4, coul); k.Box(-4, 1, -4, 3, 7, -4, coul); k.Box(-3, 2, 5, 2, 7, 5, coul); k.Box(-3, 2, -5, 2, 7, -5, coul);
        for (int x = -4; x <= 3; x += 2) { k.Remove(x, 1, 4); k.Remove(x, 1, -4); }
        Peint(k, -4, 2, -5, 3, 2, 5, coulS);
        k.Set(-2, 5, 6, Pal::Os); k.Set(1, 5, 6, Pal::Os); k.Set(-2, 4, 6, Pal::Os); k.Set(1, 4, 6, Pal::Os);  // crocs cousus
        // fourrure sur les épaules (bloc taillé, touffes)
        Taille(k, -8, 15, -5, 7, 18, 4, four, AretesV | AretesHaut);
        Vide(k, -4, 15, -3, 3, 18, 3);
        for (int x = -8; x <= 7; x++)
            for (int z = -5; z <= 4; z++)
                if (k.Has(x, 15, z) && ((x + z) & 1) == 0) k.Remove(x, 15, z);
        Peint(k, -8, 18, -5, 7, 18, 4, fourC);
        for (int x = -7; x <= 6; x += 3) { Peint(k, x, 16, -5, x, 17, 4, fourS); }
        Vide(k, -4, 15, 3, 3, 16, 4);                                            // la fourrure dégage le haut des pectoraux
        // le cou passe dans la fourrure
        k.Box(-3, 16, -2, 2, 17, 2, peau);
        // une ceinture-bandoulière de fourrure ne cache pas le torse : rien de plus

        // ---- tête : mâchoire carrée, barbe rousse en deux tresses, casque à cornes
        auto& t = *a.Tete;
        Taille(t, -4, 16, -3, 3, 22, 3, peau, AretesV | AretesHaut);
        t.Box(-5, 18, 0, -5, 19, 0, peauS); t.Box(4, 18, 0, 4, 19, 0, peauS);
        Visage(t, 19, 3, rouxS, true);
        t.Box(-3, 20, 4, 2, 20, 4, rouxS); t.Remove(-1, 20, 4); t.Remove(0, 20, 4);   // sourcils froncés
        // barbe : couvre le bas du visage, puis deux tresses
        t.Box(-4, 15, 3, 3, 17, 4, roux); t.Box(-3, 18, 4, 2, 18, 4, roux); t.Remove(-1, 18, 4); t.Remove(0, 18, 4);
        t.Box(-2, 17, 5, 1, 17, 5, rouxS);                                            // moustache
        Vide(t, -1, 16, 4, 0, 16, 4); t.Set(-1, 16, 4, Pal::C("5a2a1c")); t.Set(0, 16, 4, Pal::C("5a2a1c"));   // bouche
        t.Box(-3, 12, 5, -2, 14, 5, roux); t.Box(1, 12, 5, 2, 14, 5, roux);
        t.Box(-3, 13, 5, -2, 13, 5, coul); t.Box(1, 13, 5, 2, 13, 5, coul);       // rubans
        t.Box(-3, 11, 5, -2, 11, 5, rouxS); t.Box(1, 11, 5, 2, 11, 5, rouxS);
        // cheveux : longue tresse dans le dos
        t.Box(-4, 17, -4, 3, 21, -4, roux); t.Box(-1, 10, -5, 0, 17, -5, roux);
        for (int y = 11; y <= 16; y += 2) t.Box(-1, y, -5, 0, y, -5, rouxS);
        t.Box(-1, 9, -5, 0, 10, -5, coul);
        // casque : calotte de fer à bandes, nasal, cornes d'os recourbées
        Taille(t, -5, 21, -4, 4, 24, 4, Pal::Fer, AretesV | AretesHaut);
        Peint(t, -5, 21, -4, 4, 21, 4, Pal::FerSombre);
        Peint(t, -1, 21, -4, 0, 24, 4, Pal::Acier);
        t.Box(-1, 18, 4, 0, 21, 4, Pal::Fer); t.Box(-1, 19, 5, 0, 20, 5, Pal::Fer);   // nasal
        for (int sx = 0; sx < 2; sx++)
        {
            float s = static_cast<float>(sx == 0 ? -1 : 1); float x0 = sx == 0 ? -5.0f : 5.0f;
            t.Line(x0, 23, 0.5f, x0 + 3.5f * s, 25, 0.5f, 1.3f, 1.0f, Pal::Os);
            t.Line(x0 + 3.5f * s, 25, 0.5f, x0 + 5.0f * s, 28, 1.5f, 1.0f, 0.7f, Pal::Os);
            t.Line(x0 + 5.0f * s, 28, 1.5f, x0 + 4.5f * s, 29.5f, 2.5f, 0.7f, 0.45f, Pal::C("cfc2a2"));
        }
        for (int x = -5; x <= 4; x += 3) t.Set(x, 22, 5, Pal::Acier);                 // rivets

        // ---- bras : énormes, nus, bracelets de cuir, poings serrés
        auto& bd = *a.BrasD;
        Taille(bd, 6, 8, -2, 9, 16, 2, peau, AretesV | AretesHaut);               // biceps / épaule
        Taille(bd, 6, 12, 2, 8, 14, 2, peauC, AretesV);                           // biceps en saillie
        Peint(bd, 6, 8, -2, 9, 8, 2, peauS);
        Taille(bd, 6, 7, -2, 9, 11, 2, coul, AretesV);                             // bandes d'avant-bras (couleur du joueur)
        Peint(bd, 6, 9, -2, 9, 9, 2, coulS); Peint(bd, 6, 7, -2, 9, 7, 2, Pal::Fer);
        Taille(bd, 6, 3, -2, 9, 6, 3, peau, AretesV | AretesBas);                 // poing
        Peint(bd, 6, 3, 3, 9, 6, 3, peauS);
        Taille(bd, 5, 15, -3, 9, 17, 3, four, AretesHaut);                         // fourrure d'épaule
        Peint(bd, 5, 17, -3, 9, 17, 3, fourC);
        Miroir(bd, *a.BrasG);

        // ---- jambes : culotte de cuir, bandes molletières, bottes fourrées
        auto& jd = *a.JambeD;
        Jambe(jd, Pal::CuirSombre, fourS, 4, Pal::Noir);
        Taille(jd, 0, 4, -2, 4, 5, 2, four, AretesV); Peint(jd, 0, 5, -2, 4, 5, 2, fourC);
        for (int y = 6; y <= 7; y++) Peint(jd, 1, y, -1, 4, y, 1, y == 6 ? coul : coulS);
        Miroir(jd, *a.JambeG);

        // ---- grande hache à deux tranchants, manche posé contre l'épaule
        auto e = Nouveau<VoxelModel>();
        e->Box(7, 0, 0, 8, 22, 1, Pal::Bois); e->Box(7, 3, 0, 8, 7, 1, Pal::CuirSombre);
        for (int y = 9; y <= 22; y += 4) e->Box(7, y, 0, 8, y, 1, Pal::BoisSombre);
        e->Box(6, 23, 0, 9, 23, 1, Pal::Fer); e->Box(7, -1, 0, 8, -1, 1, Pal::Fer);
        // lames en croissant de part et d'autre
        for (int side = 0; side < 2; side++)
        {
            int dir = side == 0 ? 1 : -1, xb = side == 0 ? 9 : 6;
            for (int i = 0; i < 6; i++)
            {
                int h = 2 + i; int x = xb + dir * i;
                e->Box(x, 19 - h / 2 - i / 2, 0, x, 20 + h / 2 + i / 2, 1, i == 5 ? Pal::Acier : Pal::Fer);
            }
        }
        e->Box(6, 19, 0, 9, 21, 1, Pal::FerSombre);
        e->Box(7, 24, 0, 8, 25, 1, Pal::Acier);
        a.Arme->Merge(*Pivoter(*e, a.PArme, Quaternion::AngleAxis(-24.0f, Vector3::right) * Quaternion::AngleAxis(14.0f, Vector3::forward)), 0, 0, 0);

        return Assembler("heros_Barbare", a);
    }
}
