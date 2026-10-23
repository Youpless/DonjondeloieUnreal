// Donjon de l'Oie — le Guerrier : armure de plaques taillée, heaume à nasal et plumet, épée longue, grand écu,
// tabard à la couleur du joueur.
#include "HerosInterne.h"

namespace DonjonDeLOie::Modeles::HerosModeles
{
    using namespace DonjonDeLOie::Voxel;

    Ptr<VoxelRig> Guerrier(Color32 coul)
    {
        Atelier a;
        Color32 acier = Pal::Acier, fer = Pal::Fer, ferS = Pal::FerSombre, or_ = Pal::Or;
        [[maybe_unused]] Color32 coulS = Sh(coul, 0.72f);     // (inutilisée, comme dans la version C#)

        // ---- corps : jupe de plaques, cuirasse, tabard, ceinture
        auto& k = *a.Corps;
        Taille(k, -6, 5, -4, 5, 6, 4, fer, AretesV);                      // lame basse, évasée
        Taille(k, -5, 7, -3, 4, 8, 3, acier, AretesV);                    // lame haute
        Taille(k, -4, 9, -3, 3, 10, 3, acier, AretesV);                   // taille
        Taille(k, -5, 11, -3, 4, 15, 3, acier, AretesV | AretesHaut);     // poitrine
        Taille(k, -4, 11, 4, 3, 14, 4, acier, AretesV);                   // plastron bombé
        Taille(k, -4, 11, -4, 3, 14, -4, fer, AretesV);                   // dossière
        k.Box(-3, 15, -2, 2, 16, 2, ferS);                                // gorgerin
        Peint(k, -6, 6, -4, 5, 6, 4, ferS);
        // tabard devant et derrière (couleur du joueur), bordé d'or en bas
        k.Box(-3, 3, 5, 2, 13, 5, coul); k.Box(-4, 3, -5, 3, 14, -5, coul); k.Box(-4, 15, -4, 3, 15, -4, coul);
        k.Box(-3, 14, 4, 2, 14, 4, coul); k.Box(-3, 14, -4, 2, 14, -4, coul);
        Vide(k, -3, 3, 5, -3, 3, 5); Vide(k, 2, 3, 5, 2, 3, 5); Vide(k, -4, 3, -5, -4, 3, -5); Vide(k, 3, 3, -5, 3, 3, -5);
        Peint(k, -2, 3, 5, 1, 3, 5, or_); Peint(k, -3, 3, -5, 2, 3, -5, or_);
        k.Box(-4, 11, 5, -4, 13, 5, coul); k.Box(3, 11, 5, 3, 13, 5, coul);   // épaulement du tabard
        // emblème : une épée d'or stylisée sur la poitrine
        k.Box(-1, 10, 6, 0, 13, 6, or_); k.Box(-2, 12, 6, 1, 12, 6, or_);
        k.Box(-1, 10, -6, 0, 13, -6, or_); k.Box(-2, 12, -6, 1, 12, -6, or_);
        // ceinture de cuir et boucle
        k.Box(-5, 9, -4, 4, 9, 4, Pal::CuirSombre); Vide(k, -5, 9, -4, -5, 9, -4); Vide(k, 4, 9, -4, 4, 9, -4); Vide(k, -5, 9, 4, -5, 9, 4); Vide(k, 4, 9, 4, 4, 9, 4);
        k.Box(-5, 9, 5, 4, 9, 5, Pal::CuirSombre); k.Box(-1, 8, 6, 0, 10, 6, or_);
        k.Box(-5, 9, -5, 4, 9, -5, Pal::CuirSombre);
        // rivets d'or sur la jupe
        for (int x = -5; x <= 4; x += 3) { k.Set(x, 7, 4, or_); k.Set(x, 7, -4, or_); }

        // ---- tête et heaume à nasal
        auto& t = *a.Tete;
        TeteNue(t, Pal::Peau, Pal::Cheveux);
        t.Box(-3, 16, 4, 2, 16, 4, Pal::Barbe); t.Remove(-3, 16, 4); t.Remove(2, 16, 4);   // barbe courte
        t.Box(-2, 17, 4, 1, 17, 4, Pal::Barbe);                                            // moustache
        t.Remove(-1, 17, 4); t.Remove(0, 17, 4);
        Taille(t, -5, 18, -4, 4, 24, 4, acier, AretesV | AretesHaut);
        Vide(t, -3, 18, 4, 2, 20, 4); Vide(t, -4, 18, -3, 3, 20, 3);      // ouverture du visage
        TeteNue(t, Pal::Peau, Pal::Cheveux);
        Vide(t, -4, 21, -3, 3, 22, 3);                                    // le crâne est sous le heaume
        t.Box(-4, 21, -3, 3, 23, 3, acier);
        t.Box(-1, 18, 4, 0, 23, 4, acier);                                // nasal
        t.Box(-1, 18, 5, 0, 18, 5, Pal::Peau);                            // bout du nez sous le nasal
        t.Box(-5, 21, 5, 4, 21, 5, fer); t.Remove(-5, 21, 5); t.Remove(4, 21, 5);   // bandeau du front
        t.Box(-5, 17, -4, 4, 18, -4, fer);                                 // couvre-nuque
        Peint(t, -5, 21, -4, 4, 21, 4, fer);
        t.Box(-1, 25, -3, 0, 25, 3, fer);                                  // crête
        t.Box(-1, 26, -4, 0, 27, 2, coul); t.Box(-1, 23, -5, 0, 26, -5, coul); t.Box(-1, 26, 3, 0, 26, 3, coul);   // plumet
        t.Box(-1, 27, -2, 0, 27, 0, Sh(coul, 1.15f));
        t.Set(-5, 22, 0, or_); t.Set(4, 22, 0, or_);

        // ---- bras droit : spalière en lames, canon, gantelet
        auto& bd = *a.BrasD;
        Bras(bd, fer, ferS);
        Peint(bd, 5, 8, -1, 7, 9, 1, acier);                               // canon d'avant-bras
        Taille(bd, 5, 10, -2, 7, 11, 1, acier, AretesV);                   // cubitière
        Taille(bd, 4, 14, -3, 8, 16, 3, acier, AretesV | AretesHaut);      // spalière
        Taille(bd, 5, 12, -3, 8, 13, 3, fer, AretesV);
        Peint(bd, 4, 13, -3, 8, 13, 3, coul); bd.Set(8, 15, 0, or_); bd.Set(8, 12, 2, or_); bd.Set(8, 12, -2, or_);
        bd.Box(4, 4, 2, 7, 5, 2, ferS);
        Miroir(bd, *a.BrasG);

        // ---- jambes : chausses de mailles, grèves, genouillères, solerets
        auto& jd = *a.JambeD;
        Jambe(jd, ferS, fer, 4, Pal::Noir);
        Taille(jd, 1, 5, 2, 4, 6, 2, acier, AretesV);                      // genouillère
        Peint(jd, 1, 0, -2, 4, 1, 3, acier);
        Miroir(jd, *a.JambeG);

        // ---- épée longue (pièce arme, tenue lame levée vers l'avant)
        auto e = Nouveau<VoxelModel>();
        e->Box(6, 1, 0, 7, 2, 1, or_);                                     // pommeau
        e->Box(6, 3, 0, 6, 7, 1, Pal::CuirSombre);                         // fusée (dans le poing)
        e->Box(4, 8, -1, 9, 8, 2, or_); e->Set(3, 9, 0, or_); e->Set(10, 9, 0, or_); e->Set(3, 9, 1, or_); e->Set(10, 9, 1, or_); e->Box(5, 9, 0, 8, 9, 1, Pal::OrSombre);   // garde
        e->Box(5, 10, 0, 7, 20, 1, acier);                                 // lame
        e->Box(6, 10, 0, 6, 18, 1, fer);                                   // gorge
        e->Box(6, 21, 0, 6, 22, 1, acier); e->Set(5, 21, 0, acier); e->Set(7, 21, 0, acier);
        auto arme = Pivoter(*e, a.PArme, Quaternion::AngleAxis(22.0f, Vector3::right));
        a.Arme->Merge(*arme, 0, 0, 0);

        // ---- grand écu (bouclier), sur l'avant-bras gauche, tourné vers l'avant-gauche
        auto s = Nouveau<VoxelModel>();
        int sx0 = -12, sx1 = -3;
        for (int y = 2; y <= 14; y++)
        {
            int retrait = y >= 7 ? 0 : (7 - y + 1) / 2 * 1 + (y <= 3 ? 1 : 0);
            if (y <= 2) retrait = 4;
            for (int x = sx0 + retrait; x <= sx1 - retrait; x++)
            {
                bool bord = x == sx0 + retrait || x == sx1 - retrait || y == 14 || y == 2;
                s->Set(x, y, 4, bord ? fer : coul);
                s->Set(x, y, 3, bord ? ferS : Pal::BoisSombre);
            }
        }
        s->Remove(sx0, 14, 4); s->Remove(sx1, 14, 4); s->Remove(sx0, 14, 3); s->Remove(sx1, 14, 3);
        // chevron d'or
        for (int i = 0; i < 4; i++) { s->Set(-8 - i, 10 - i, 5, or_); s->Set(-7 + i, 10 - i, 5, or_); s->Set(-8 - i, 11 - i, 5, or_); s->Set(-7 + i, 11 - i, 5, or_); }
        s->Box(-8, 5, 5, -7, 6, 5, Pal::OrSombre);
        s->Box(-8, 7, 2, -7, 9, 2, Pal::CuirSombre);                       // énarmes
        auto ecu = Pivoter(*s, Vector3(-7.0f, 8.0f, 2.5f), Quaternion::AngleAxis(-12.0f, Vector3::up));
        a.Bouclier->Merge(*ecu, 0, 0, 0);

        return Assembler("heros_Guerrier", a);
    }
}
