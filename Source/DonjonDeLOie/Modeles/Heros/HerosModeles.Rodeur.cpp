// Donjon de l'Oie — le Rôdeur : grande cape à capuche à la couleur du joueur, cuir vert et brun,
// carquois dans le dos, arc long, bottes hautes.
#include "HerosInterne.h"

namespace DonjonDeLOie::Modeles::HerosModeles
{
    using namespace DonjonDeLOie::Voxel;

    Ptr<VoxelRig> Rodeur(Color32 coul)
    {
        Atelier a;
        Color32 coulS = Sh(coul, 0.68f);
        Color32 vert = Pal::C("4f6b38"), vertS = Pal::C("3a5029"), brun = Pal::C("6e4a2e"), brunS = Pal::C("4b311e"), lin = Pal::C("c9b48a");

        // ---- corps : tunique verte, plastron de cuir, ceinture, carquois (avec le corps, dans le dos)
        auto& k = *a.Corps;
        Taille(k, -5, 5, -3, 4, 9, 3, vert, AretesV);
        Peint(k, -5, 5, -3, 4, 5, 3, vertS);
        Vide(k, -1, 5, 3, 0, 6, 3);                                          // tunique fendue
        Taille(k, -5, 10, -3, 4, 15, 3, vert, AretesV | AretesHaut);
        Taille(k, -4, 10, 4, 3, 14, 4, brun, AretesV);                        // plastron de cuir
        Peint(k, -4, 10, 4, 3, 10, 4, brunS);
        for (int y = 11; y <= 13; y += 2) { k.Set(-1, y, 5, lin); k.Set(0, y, 5, lin); }   // lacets
        k.Box(-5, 8, -4, 4, 8, 4, brunS); Vide(k, -5, 8, 4, -5, 8, 4); Vide(k, 4, 8, 4, 4, 8, 4); Vide(k, -5, 8, -4, -5, 8, -4); Vide(k, 4, 8, -4, 4, 8, -4);
        k.Box(-1, 8, 5, 0, 8, 5, Pal::Fer);
        Taille(k, 3, 5, 3, 5, 7, 4, brun, AretesBas);                          // sacoche
        k.Box(-6, 4, 0, -6, 8, 0, brunS); k.Box(-7, 4, 0, -7, 6, 0, Pal::Acier); // couteau de chasse
        // baudrier du carquois
        for (int i = 0; i <= 7; i++) { int x = -4 + i, y = 15 - i; k.Set(x, y, 5, brunS); k.Set(x, y, -4, brunS); }
        // carquois dans le dos, incliné, et flèches à empennage couleur du joueur
        auto q = Nouveau<VoxelModel>();
        Taille(*q, -2, 7, -8, 1, 16, -6, brun, AretesV);
        Peint(*q, -2, 7, -8, 1, 7, -6, brunS); Peint(*q, -2, 15, -8, 1, 16, -6, brunS);
        Peint(*q, -2, 11, -8, 1, 11, -6, lin);
        for (int i = 0; i < 4; i++) { int x = -2 + i; q->Box(x, 17, -7 + (i % 2), x, 18, -7 + (i % 2), Pal::Bois); q->Box(x, 19, -7 + (i % 2), x, 20, -7 + (i % 2), i % 2 == 0 ? Pal::Blanc : lin); }
        k.Merge(*Pivoter(*q, Vector3(0, 12, -6), Quaternion::AngleAxis(-25.0f, Vector3::forward)), 0, 0, 0);

        // ---- cape à capuche (pièce cape) : longue, couleur du joueur, bord inférieur en dents
        auto& c = *a.Cape;
        for (int y = 3; y <= 15; y++)
        {
            int h = y >= 14 ? 6 : 7, zb = -5 - (15 - y) / 5;
            for (int x = -h; x < h; x++)
            {
                if (y <= 4 && ((x + 7) % 3 == 0)) continue;
                c.Set(x, y, zb, y <= 4 ? coulS : coul);
            }
            if (y >= 9) { c.Set(-h - 1, y, zb + 1, coul); c.Set(h, y, zb + 1, coul); c.Set(-h - 1, y, zb + 2, coulS); c.Set(h, y, zb + 2, coulS); }
        }
        // le carquois dépasse par une fente : on évide la cape où il passe
        c.Carve([&](int x, int y, int z) { return y >= 12 && x >= -3 && x <= 3 && z >= -8; });
        c.Box(-7, 15, -5, 6, 16, 4, coul); Vide(c, -5, 15, -3, 4, 16, 3);      // col de la cape sur les épaules
        Vide(c, -7, 15, 4, -7, 16, 4); Vide(c, 6, 15, 4, 6, 16, 4);
        Peint(c, -7, 16, -5, 6, 16, 4, coulS);
        c.Box(-1, 15, 5, 0, 15, 5, Pal::Or);                                     // fermail

        // ---- tête : capuche relevée (couleur du joueur), mèches, barbe de quelques jours
        auto& t = *a.Tete;
        TeteNue(t, Pal::Peau, Pal::Cheveux);
        Peint(t, -4, 16, 3, 3, 16, 3, Pal::C("9a6c4c"));                         // barbe naissante
        t.Box(-1, 17, 3, 0, 17, 3, Pal::PeauOmbre);
        t.Clear();
        Capuche(t, coul, coulS, 3);
        TeteNue(t, Pal::Peau, Pal::Cheveux);
        Peint(t, -4, 16, 3, 3, 16, 3, Pal::C("9a6c4c")); t.Box(-1, 17, 3, 0, 17, 3, Pal::PeauOmbre);
        t.Box(-3, 21, 3, 2, 22, 3, Pal::Cheveux); t.Set(-3, 20, 3, Pal::Cheveux); t.Set(2, 20, 3, Pal::Cheveux);   // mèches
        t.Box(-2, 20, 3, -2, 20, 3, Pal::Cheveux); t.Box(1, 20, 3, 1, 20, 3, Pal::Cheveux);

        // ---- bras : manches vertes, brassard d'archer, gants
        auto& bd = *a.BrasD;
        Bras(bd, vert, Pal::Peau);
        Taille(bd, 4, 7, -2, 7, 10, 2, brun, AretesV); Peint(bd, 4, 7, -2, 7, 7, 2, brunS);
        Peint(bd, 5, 4, -1, 7, 6, 2, brunS);
        Taille(bd, 4, 13, -3, 7, 15, 3, brun, AretesHaut);
        Miroir(bd, *a.BrasG);

        // ---- jambes : chausses brunes, bottes hautes à revers
        auto& jd = *a.JambeD;
        Jambe(jd, brunS, brun, 6, Pal::Noir);
        Taille(jd, 1, 6, -2, 4, 7, 2, Pal::C("8a5e3a"), AretesV);
        jd.Box(1, 3, 3, 4, 3, 3, brunS);
        Miroir(jd, *a.JambeG);

        // ---- arc long (arme), tenu à la verticale en main droite, corde tendue
        auto b = Nouveau<VoxelModel>();
        Color32 bois = Pal::C("7d5434"), boisC = Pal::C("9a6a40");
        int yb = -3, yh = 17;                                   // extrémités de l'arc
        for (int y = yb; y <= yh; y++)
        {
            float u = (static_cast<float>(y) - 6.5f) / (static_cast<float>(yh - yb) * 0.5f);         // −1 … 1
            float zc = 2.5f - 3.6f * u * u;                     // courbure vers l'avant au centre
            int zi = Mathf::FloorToInt(zc);
            b->Box(6, y, zi, 7, y, zi, Mathf::Abs(u) > 0.85f ? boisC : bois);
            if (Mathf::Abs(u) < 0.45f) b->Box(6, y, zi + 1, 7, y, zi + 1, bois);
        }
        b->Box(6, 4, 2, 7, 8, 4, brunS);                         // poignée
        b->Box(6, yh + 1, -1, 7, yh + 1, -1, Pal::Os); b->Box(6, yb - 1, -1, 7, yb - 1, -1, Pal::Os);   // pointes de corne
        for (int y = yb; y <= yh; y++)
            if (!b->Has(6, y, -1)) b->Set(6, y, -1, lin);                                           // corde
        a.Arme->Merge(*Pivoter(*b, a.PArme, Quaternion::AngleAxis(-8.0f, Vector3::right) * Quaternion::AngleAxis(-8.0f, Vector3::forward)), 0, 0, 0);

        return Assembler("heros_Rodeur", a);
    }
}
