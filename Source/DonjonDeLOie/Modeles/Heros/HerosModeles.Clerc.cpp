// Donjon de l'Oie — le Clerc : cotte de mailles sous un surplis blanc, mitre, soleil d'or sur la poitrine,
// masse d'armes, petit écu rond ; étole et mitre à la couleur du joueur.
#include "HerosInterne.h"

namespace DonjonDeLOie::Modeles::HerosModeles
{
    using namespace DonjonDeLOie::Voxel;

    Ptr<VoxelRig> Clerc(Color32 coul)
    {
        Atelier a;
        [[maybe_unused]] Color32 coulS = Sh(coul, 0.7f);     // (inutilisée, comme dans la version C#)
        Color32 blanc = Pal::C("ece6d8"), blancS = Pal::C("cbc3b0"), maille = Pal::C("7d858c"), or_ = Pal::Or;
        [[maybe_unused]] Color32 orS = Pal::OrSombre;         // (inutilisée aussi)

        // ---- corps : surplis blanc jusqu'aux genoux sur une cotte de mailles, étole à la couleur du joueur
        auto& k = *a.Corps;
        Taille(k, -6, 3, -4, 5, 7, 4, blanc, AretesV);                     // jupe du surplis
        Taille(k, -5, 8, -3, 4, 10, 3, blanc, AretesV);
        Taille(k, -5, 11, -3, 4, 15, 3, blanc, AretesV | AretesHaut);
        Taille(k, -4, 11, 4, 3, 14, 4, blanc, AretesV);
        Peint(k, -6, 3, -4, 5, 3, 4, or_);                                   // galon d'or
        Peint(k, -6, 4, -4, 5, 4, 4, blancS);
        for (int x = -5; x <= 4; x += 3) { Peint(k, x, 5, 4, x, 7, 4, blancS); Peint(k, x, 5, -4, x, 7, -4, blancS); }
        k.Box(-5, 2, -3, 4, 2, 3, maille);                                   // ourlet de mailles qui dépasse
        // étole : deux bandes verticales devant (de l'épaule au bas), une en V dans le dos
        for (int cote = 0; cote < 2; cote++)
        {
            int x0 = cote == 0 ? -4 : 2;
            k.Box(x0, 3, 5, x0 + 1, 13, 5, coul); k.Box(x0, 14, 4, x0 + 1, 15, 4, coul);
            k.Box(x0, 2, 5, x0 + 1, 2, 5, or_);
            k.Box(x0, 4, 6, x0 + 1, 4, 6, or_);                               // petite croix brodée en bas
            k.Set(x0, 5, 6, or_);
        }
        k.Box(-4, 15, -4, 3, 15, -4, coul); k.Box(-3, 11, -4, 2, 14, -4, coul);
        k.Box(-2, 6, -5, 1, 13, -5, coul); Peint(k, -2, 6, -5, 1, 6, -5, or_);
        // ceinture de corde
        k.Box(-5, 9, -4, 4, 9, 4, Pal::Sable); Vide(k, -5, 9, 4, -5, 9, 4); Vide(k, 4, 9, 4, 4, 9, 4); Vide(k, -5, 9, -4, -5, 9, -4); Vide(k, 4, 9, -4, 4, 9, -4);
        k.Box(-1, 9, 5, 0, 9, 5, Pal::Sable);
        // soleil sacré d'or sur la poitrine (losange + rayons)
        k.Box(-2, 11, 5, 1, 14, 5, or_); k.Remove(-2, 11, 5); k.Remove(1, 11, 5); k.Remove(-2, 14, 5); k.Remove(1, 14, 5);
        k.Box(-1, 12, 6, 0, 13, 6, Pal::C("fff0b0"));
        k.Set(-1, 15, 5, or_); k.Set(0, 15, 5, or_); k.Set(-1, 10, 5, or_); k.Set(0, 10, 5, or_);
        // col de mailles
        k.Box(-3, 15, -3, 2, 16, 3, maille);
        // livre de prières à la hanche
        Taille(k, -7, 5, -1, -6, 8, 2, Pal::C("6a2a2a"), AretesV); Peint(k, -7, 6, -1, -7, 7, 2, or_);

        // ---- tête : visage doux, rasé, mitre haute
        auto& t = *a.Tete;
        TeteNue(t, Pal::Peau, Pal::Cheveux);
        t.Box(-1, 17, 3, 0, 17, 3, Pal::PeauOmbre);                            // bouche
        t.Box(-5, 17, -3, -4, 20, 1, Pal::Cheveux); t.Box(3, 17, -3, 4, 20, 1, Pal::Cheveux);
        t.Box(-4, 17, -4, 3, 21, -4, Pal::Cheveux);
        // mitre : bandeau d'or puis deux pans qui montent en pointe (vue de face), plus étroite de profil
        Taille(t, -5, 21, -4, 4, 22, 4, or_, AretesV);
        for (int y = 23; y <= 29; y++)
        {
            // de face : arc brisé ; de profil : étroite et légèrement évasée vers le haut
            int hx = y <= 26 ? 4 : y == 27 ? 3 : y == 28 ? 2 : 1;
            int hz = y <= 25 ? 2 : 3;
            for (int x = -hx; x < hx; x++)
                for (int z = -hz; z <= hz - 1; z++) t.Set(x, y, z, coul);
        }

        Vide(t, -4, 28, -1, 3, 29, -1);                                     // fente entre les deux pans (de profil)
        // orfrois : bandes d'or verticale et horizontale, gemme
        Peint(t, -1, 23, 2, 0, 29, 2, or_); Peint(t, -4, 23, -3, 3, 23, 2, or_);
        Peint(t, -1, 23, -3, 0, 29, -3, or_);
        t.Box(-1, 25, 3, 0, 25, 3, Pal::C("c0303a"));
        // fanons qui tombent dans la nuque
        t.Box(-3, 17, -5, -2, 21, -5, coul); t.Box(1, 17, -5, 2, 21, -5, coul);
        t.Set(-3, 16, -5, or_); t.Set(2, 16, -5, or_);

        // ---- bras : manches blanches larges, gantelets de mailles
        auto& bd = *a.BrasD;
        Bras(bd, blanc, Pal::Peau);
        Taille(bd, 4, 8, -2, 7, 10, 2, blanc, AretesV); Peint(bd, 4, 8, -2, 7, 8, 2, or_);
        Vide(bd, 5, 8, -1, 6, 8, 1); bd.Box(5, 8, -1, 7, 8, 1, maille);
        Taille(bd, 4, 13, -3, 7, 16, 3, Pal::Acier, AretesHaut | AretesV);    // petite épaulière
        Peint(bd, 4, 13, -3, 7, 13, 3, or_);
        Miroir(bd, *a.BrasG);

        // ---- jambes : chausses grises, sandales-bottes de cuir clair
        auto& jd = *a.JambeD;
        Jambe(jd, maille, Pal::Cuir, 3, Pal::CuirSombre);
        Miroir(jd, *a.JambeG);

        // ---- masse d'armes à ailettes (arme)
        auto e = Nouveau<VoxelModel>();
        e->Box(6, 1, 0, 7, 2, 1, or_);
        e->Box(6, 3, 0, 7, 12, 1, Pal::Bois); e->Box(6, 3, 0, 7, 7, 1, Pal::CuirSombre);
        e->Box(5, 13, -1, 8, 17, 2, Pal::Acier);
        e->Box(4, 14, 0, 9, 16, 1, Pal::Fer); e->Box(6, 14, -2, 7, 16, 3, Pal::Fer);   // ailettes
        e->Box(6, 18, 0, 7, 18, 1, or_); e->Box(5, 12, -1, 8, 12, 2, or_);
        a.Arme->Merge(*Pivoter(*e, a.PArme, Quaternion::AngleAxis(30.0f, Vector3::right)), 0, 0, 0);

        // ---- petit écu rond (bouclier) : disque d'acier, umbo d'or, rayons de couleur du joueur
        auto s = Nouveau<VoxelModel>();
        for (int x = -12; x <= -2; x++)
            for (int y = 2; y <= 12; y++)
            {
                float dx = static_cast<float>(x) + 0.5f + 7.0f, dy = static_cast<float>(y) + 0.5f - 7.0f; float d2 = dx * dx + dy * dy;
                if (d2 > 30.0f) continue;
                Color32 c = d2 > 20.0f ? Pal::Fer : (Mathf::Abs(dx) < 0.6f || Mathf::Abs(dy) < 0.6f ? Pal::Blanc : coul);
                s->Set(x, y, 4, c); s->Set(x, y, 3, d2 > 20.0f ? Pal::FerSombre : Pal::BoisSombre);
            }
        s->Box(-8, 6, 5, -7, 7, 5, or_); s->Set(-8, 7, 6, Pal::C("fff0b0"));
        s->Set(-12, 7, 5, Pal::Acier); s->Set(-2, 7, 5, Pal::Acier); s->Set(-7, 12, 5, Pal::Acier); s->Set(-7, 2, 5, Pal::Acier);
        s->Box(-8, 6, 2, -6, 7, 2, Pal::CuirSombre);
        a.Bouclier->Merge(*Pivoter(*s, Vector3(-7.0f, 7.0f, 2.5f), Quaternion::AngleAxis(-35.0f, Vector3::up)), -1, 0, -1);

        return Assembler("heros_Clerc", a);
    }
}
