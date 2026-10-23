// Donjon de l'Oie — le Voleur : cuir sombre ajusté, capuche rabattue et petite cape à la couleur du joueur,
// foulard sur le bas du visage, deux dagues, ceinture de bourses.
#include "HerosInterne.h"

namespace DonjonDeLOie::Modeles::HerosModeles
{
    using namespace DonjonDeLOie::Voxel;

    Ptr<VoxelRig> Voleur(Color32 coul)
    {
        Atelier a;
        Color32 coulS = Sh(coul, 0.68f);
        Color32 cuir = Pal::C("4a3a33"), cuirS = Pal::C("2f2522"), cuirC = Pal::C("6a5243"), foulard = Pal::C("2a2628");
        Color32 acier = Pal::Acier, fer = Pal::Fer;

        // ---- corps : justaucorps de cuir, plus étroit que le guerrier, coutures, baudrier
        auto& k = *a.Corps;
        Taille(k, -5, 6, -3, 4, 9, 3, cuir, AretesV);
        Taille(k, -4, 10, -2, 3, 11, 2, cuir, AretesV);
        Taille(k, -5, 12, -3, 4, 15, 3, cuir, AretesV | AretesHaut);
        // pans de la veste en bas (fendus devant)
        k.Box(-5, 5, -3, 4, 5, 3, cuirS); Vide(k, -1, 5, 3, 0, 6, 3);
        Peint(k, -5, 6, -3, 4, 6, 3, cuirS);
        // couture centrale et lacets
        Peint(k, -1, 7, 3, 0, 15, 3, cuirS);
        for (int y = 8; y <= 14; y += 2) { k.Set(-2, y, 4, cuirC); k.Set(1, y, 4, cuirC); }
        // baudrier en diagonale (épaule droite → hanche gauche), boucle
        for (int i = 0; i <= 7; i++) { int x = 4 - i, y = 15 - i; k.Set(x, y, 4, Pal::CuirSombre); k.Set(x - 1, y, 4, Pal::CuirSombre); k.Set(x, y, -4, Pal::CuirSombre); k.Set(x - 1, y, -4, Pal::CuirSombre); }
        k.Set(1, 12, 5, Pal::Or);
        // ceinture et bourses
        k.Box(-5, 8, -4, 4, 8, 4, Pal::Cuir); Vide(k, -5, 8, 4, -5, 8, 4); Vide(k, 4, 8, 4, 4, 8, 4); Vide(k, -5, 8, -4, -5, 8, -4); Vide(k, 4, 8, -4, 4, 8, -4);
        k.Box(-1, 8, 5, 0, 8, 5, Pal::Or);
        Taille(k, 2, 5, 4, 4, 7, 5, Pal::Cuir, AretesBas); k.Box(3, 8, 5, 3, 8, 5, Pal::Sable);
        Taille(k, -6, 5, 1, -5, 7, 3, Pal::Cuir, AretesBas); k.Set(-6, 7, 2, Pal::Sable);
        Taille(k, -4, 5, -6, -1, 7, -5, Pal::CuirSombre, AretesBas); k.Set(-3, 6, -6, Pal::Or);
        Taille(k, 5, 5, -2, 5, 7, 0, Pal::Cuir, AretesBas);
        // foulard noué au cou, couleur du joueur (écharpe), pans qui tombent dans le dos
        k.Box(-4, 15, -3, 3, 16, 3, coul); Vide(k, -4, 16, -3, -4, 16, -3); Vide(k, 3, 16, -3, 3, 16, -3);
        k.Box(-2, 10, -4, 1, 14, -4, coul); k.Box(-1, 9, -4, 0, 9, -4, coulS);

        // ---- cape courte (pièce cape, pivot aux épaules) : couleur du joueur
        auto& c = *a.Cape;
        for (int y = 6; y <= 15; y++)
        {
            int h = y >= 14 ? 5 : 6, zb = -4 - (15 - y) / 4;
            for (int x = -h; x < h; x++) { c.Set(x, y, zb, coul); }
            if (y >= 12) { c.Set(-h - 1, y, zb + 1, coul); c.Set(h, y, zb + 1, coul); }
        }
        // bas de cape découpé en dents
        for (int x = -6; x < 6; x += 3) { c.Remove(x, 6, -6); c.Remove(x + 1, 6, -6); }
        for (int x = -6; x < 6; x++)
            if (c.Has(x, 7, -6)) c.Set(x, 7, -6, coulS);
        Peint(c, -7, 15, -6, 6, 15, -3, coulS);

        // ---- tête : capuche à la couleur du joueur, foulard sombre sur le nez, yeux perçants
        auto& t = *a.Tete;
        TeteNue(t, Pal::Peau, Pal::C("2a1d16"));
        // foulard (masque) : couvre bas du visage et nez
        t.Box(-4, 16, 3, 3, 18, 4, foulard); t.Remove(-4, 16, 4); t.Remove(3, 16, 4);
        t.Box(-1, 18, 5, 0, 18, 5, foulard);
        t.Clear();
        Capuche(t, coul, coulS, 2);
        TeteNue(t, Pal::Peau, Pal::C("2a1d16"));
        t.Box(-4, 16, 3, 3, 18, 4, foulard); t.Remove(-4, 16, 4); t.Remove(3, 16, 4); t.Box(-1, 18, 5, 0, 18, 5, foulard);
        Peint(t, -4, 16, 4, 3, 16, 4, Sh(foulard, 1.3f));
        t.Box(-4, 21, 3, 3, 22, 3, coulS);                                            // ombre de la capuche sur le front
        t.Box(-3, 20, 3, 2, 20, 3, Pal::C("2a1d16"));                                 // sourcils froncés, regard dans l'ombre
        t.Remove(-1, 20, 3); t.Remove(0, 20, 3); t.Set(-1, 20, 3, Pal::Peau); t.Set(0, 20, 3, Pal::Peau);
        Visage(t, 19, 3, Color32(0, 0, 0, 0), false);
        t.Set(-2, 19, 3, Pal::C("e8e0c8")); t.Set(1, 19, 3, Pal::C("e8e0c8")); t.Set(-3, 19, 3, Pal::C("221c1c")); t.Set(2, 19, 3, Pal::C("221c1c"));

        // ---- bras : brassards de cuir, bras droit dague en main (arme), gauche dague intégrée
        auto& bd = *a.BrasD;
        Bras(bd, cuir, Pal::Peau);
        Peint(bd, 5, 4, -1, 7, 7, 2, cuirS);                                          // gants
        Taille(bd, 4, 8, -2, 7, 11, 2, cuirC, AretesV);                               // brassard
        Peint(bd, 4, 9, -2, 7, 9, 2, cuirS);
        Taille(bd, 4, 13, -2, 7, 15, 2, coul, AretesHaut);                            // épaule de capuchon
        Miroir(bd, *a.BrasG);
        // dague gauche, lame vers l'avant, tenue à revers
        auto dg = Nouveau<VoxelModel>();
        dg->Box(-8, 5, 3, -6, 5, 3, Pal::Or); dg->Box(-7, 4, 3, -7, 6, 3, Pal::Or);
        dg->Box(-7, 5, 4, -7, 5, 9, acier); dg->Box(-7, 4, 4, -7, 4, 8, fer); dg->Set(-7, 5, 10, acier);
        dg->Box(-7, 5, 0, -7, 5, 2, Pal::CuirSombre);
        a.BrasG->Merge(*dg, 0, 0, 0);

        // ---- jambes : chausses ajustées, bottes souples à revers
        auto& jd = *a.JambeD;
        Jambe(jd, cuirS, cuir, 4, Pal::Noir);
        Taille(jd, 1, 4, -2, 4, 5, 2, cuirC, AretesV);                                // revers
        jd.Box(4, 5, 0, 4, 6, 1, Pal::CuirSombre); jd.Box(5, 5, 0, 5, 6, 0, fer);      // couteau à la botte
        Miroir(jd, *a.JambeG);

        // ---- dague droite (arme), lame vers le haut et l'avant
        auto e = Nouveau<VoxelModel>();
        e->Box(6, 2, 0, 7, 2, 1, Pal::Or);
        e->Box(6, 3, 0, 7, 7, 1, Pal::CuirSombre);
        e->Box(4, 8, 0, 9, 8, 1, Pal::Or); e->Set(4, 9, 0, Pal::Or); e->Set(9, 9, 1, Pal::Or);
        e->Box(6, 9, 0, 7, 14, 1, acier); e->Box(6, 9, 0, 6, 13, 1, fer); e->Box(6, 15, 0, 7, 15, 0, acier); e->Set(6, 16, 0, acier);
        a.Arme->Merge(*Pivoter(*e, a.PArme, Quaternion::AngleAxis(55.0f, Vector3::right)), 0, 0, 0);

        return Assembler("heros_Voleur", a);
    }
}
