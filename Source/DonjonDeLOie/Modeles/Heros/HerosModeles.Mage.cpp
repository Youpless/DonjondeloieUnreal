// Donjon de l'Oie — le Mage : robe longue évasée, grand chapeau pointu à large bord, longue barbe,
// bâton noueux surmonté d'un cristal lumineux. Robe et chapeau à la couleur du joueur.
#include "HerosInterne.h"

#include <iterator>

namespace DonjonDeLOie::Modeles::HerosModeles
{
    using namespace DonjonDeLOie::Voxel;

    Ptr<VoxelRig> Mage(Color32 coul)
    {
        Atelier a;
        Color32 coulS = Sh(coul, 0.7f), coulC = Sh(coul, 1.18f);
        Color32 doublure = Pal::C("3a3046"), barbe = Pal::C("dcd6cc"), barbeS = Pal::C("b5aea3"), or_ = Pal::Or;

        // ---- corps : robe en tronc de cône taillé (du sol au buste), étole d'or, ceinture de corde
        auto& k = *a.Corps;
        for (int y = 0; y <= 15; y++)
        {
            // demi-largeur en x et profondeur en z selon la hauteur : évasée en bas
            int hx = y <= 2 ? 6 : y <= 6 ? 5 : y <= 11 ? 5 : 5;
            int hz = y <= 2 ? 5 : y <= 6 ? 4 : 3;
            if (y >= 14) { hx = 4; }
            for (int x = -hx; x < hx; x++)
                for (int z = -hz; z <= hz; z++)
                {
                    bool coin = (x == -hx || x == hx - 1) && (z == -hz || z == hz);
                    if (coin) continue;
                    k.Set(x, y, z, coul);
                }
        }
        // ourlet sombre et bordure d'or en bas, plis verticaux
        Peint(k, -7, 0, -6, 6, 0, 6, coulS);
        Peint(k, -7, 1, -6, 6, 1, 6, or_);
        for (int x = -5; x <= 4; x += 3) { Peint(k, x, 2, 5, x, 6, 5, coulS); Peint(k, x, 2, -5, x, 6, -5, coulS); }
        // pan avant ouvert sur la doublure
        k.Box(-1, 2, 5, 0, 13, 4, doublure); k.Box(-1, 7, 4, 0, 13, 4, doublure);
        Peint(k, -1, 1, 5, 0, 1, 5, or_);
        k.Box(-2, 2, 6, -2, 6, 6, or_); k.Box(1, 2, 6, 1, 6, 6, or_);
        Vide(k, -1, 2, 6, 0, 6, 6);
        // ceinture de corde et nœud
        k.Box(-5, 9, -4, 4, 9, 4, Pal::Sable); Vide(k, -5, 9, -4, -5, 9, -4); Vide(k, 4, 9, -4, 4, 9, -4); Vide(k, -5, 9, 4, -5, 9, 4); Vide(k, 4, 9, 4, 4, 9, 4);
        k.Box(1, 6, 5, 1, 8, 5, Pal::Sable); k.Set(2, 6, 5, Pal::Sable);
        // col en V d'or sur les épaules
        k.Box(-4, 14, -3, 3, 15, 3, coulS);
        for (int i = 0; i < 3; i++) { k.Set(-2 - i, 15 - i, 4, or_); k.Set(1 + i, 15 - i, 4, or_); }
        // bourse et parchemin à la ceinture
        Taille(k, -6, 6, 1, -5, 8, 3, Pal::Cuir, AretesBas);
        k.Box(4, 5, -2, 5, 9, -2, Pal::Os); k.Box(4, 7, -2, 5, 7, -2, Pal::Rouge);

        // ---- tête : visage âgé, sourcils broussailleux, nez fort, longue barbe en pointe
        auto& t = *a.Tete;
        TeteNue(t, Pal::Peau, barbe);
        t.Box(-1, 18, 4, 0, 19, 5, Pal::PeauOmbre); t.Box(-1, 18, 5, 0, 18, 5, Pal::Peau);   // nez fort
        // barbe : bloc taillé devant le bas du visage, qui descend en pointe sur la robe
        t.Box(-4, 16, 3, 3, 18, 4, barbe); t.Box(-3, 18, 4, 2, 18, 4, barbe);
        t.Remove(-1, 18, 4); t.Remove(0, 18, 4);
        t.Box(-2, 17, 5, 1, 17, 5, barbeS);                                         // moustache
        t.Box(-4, 13, 4, 3, 15, 5, barbe); t.Box(-3, 10, 4, 2, 12, 5, barbe); t.Box(-2, 8, 5, 1, 9, 5, barbe); t.Box(-1, 7, 5, 0, 7, 5, barbeS);
        t.Remove(-4, 13, 5); t.Remove(3, 13, 5);
        Peint(t, -4, 13, 5, 3, 13, 5, barbeS);
        t.Box(-4, 16, 0, -4, 18, 2, barbe); t.Box(3, 16, 0, 3, 18, 2, barbe);          // favoris
        t.Box(-5, 19, -3, -4, 20, 0, barbeS); t.Box(3, 19, -3, 4, 20, 0, barbeS);      // cheveux sous le chapeau
        t.Box(-4, 17, -4, 3, 20, -4, barbe);                                           // nuque
        // chapeau : large bord, cône en escalier qui se casse vers l'arrière
        Taille(t, -8, 21, -7, 7, 21, 7, coul, AretesV);
        Vide(t, -8, 21, -6, -8, 21, -6); Vide(t, 7, 21, -6, 7, 21, -6); Vide(t, -8, 21, 6, -8, 21, 6); Vide(t, 7, 21, 6, 7, 21, 6);
        Vide(t, -7, 21, -7, -7, 21, -7); Vide(t, 6, 21, -7, 6, 21, -7); Vide(t, -7, 21, 7, -7, 21, 7); Vide(t, 6, 21, 7, 6, 21, 7);
        Peint(t, -8, 21, -7, 7, 21, 7, coulS);
        t.Box(-6, 22, -5, 5, 22, 5, coul);
        Taille(t, -5, 22, -4, 4, 23, 4, Sh(coul, 0.6f), AretesV);                     // ruban
        t.Box(-1, 22, 5, 0, 23, 5, or_);                                                // boucle d'or
        int cz = 0;
        const int demi[] = {4, 3, 3, 2, 2, 1};
        for (int i = 0; i < static_cast<int>(std::size(demi)); i++)
        {
            int y = 24 + i, h = demi[i];
            if (i >= 3) cz = -(i - 2) / 2;                                              // la pointe se casse vers l'arrière
            for (int x = -h; x < h; x++)
                for (int z = -h + 1; z <= h - 1; z++) t.Set(x, y, z + cz, i % 4 == 3 ? coulC : coul);
            if (h <= 1) t.Box(-1, y, cz, 0, y, cz, coul);
        }
        t.Box(-1, 29, cz - 1, 0, 29, cz - 2, coul); t.Box(-1, 28, cz - 3, 0, 28, cz - 3, coulS);
        t.Set(1, 25, 3, or_); t.Set(-2, 26, 2, or_);                                    // étoiles brodées

        // ---- bras : manches larges évasées, mains osseuses
        auto& bd = *a.BrasD;
        Bras(bd, coul, Pal::Peau);
        Taille(bd, 4, 7, -2, 8, 10, 2, coul, AretesV);                                  // manche évasée
        Peint(bd, 4, 7, -2, 8, 7, 2, or_);
        Vide(bd, 5, 7, -1, 7, 8, 1);
        Bras(bd, Sh(coul, 0.9f), Pal::Peau);
        Peint(bd, 5, 8, -1, 7, 8, 1, doublure);
        Taille(bd, 4, 13, -3, 7, 15, 3, coulS, AretesHaut);                              // épaule de la cape
        Miroir(bd, *a.BrasG);

        // ---- jambes : courtes, cachées sous la robe (seuls les bouts des souliers dépassent)
        auto& jd = *a.JambeD;
        jd.Box(1, 3, -1, 3, 7, 1, doublure);
        jd.Box(1, 0, 4, 3, 1, 6, Pal::CuirSombre); jd.Box(1, 1, 7, 2, 1, 7, Pal::CuirSombre);
        jd.Box(1, 2, -1, 3, 2, 1, Pal::CuirSombre);
        Miroir(jd, *a.JambeG);

        // ---- bâton noueux et cristal (le cristal : lueur_baton, enfant de l'arme)
        auto b = Nouveau<VoxelModel>();
        Color32 bois = Pal::C("6b4a30"), boisS = Pal::C("4a3220");
        b->Line(6.5f, 0, 0.5f, 6.5f, 23, 0.5f, 0.8f, bois);
        b->Set(6, 12, 1, boisS); b->Set(7, 3, 0, boisS); b->Set(5, 17, 0, boisS); b->Set(7, 9, 1, boisS);   // nœuds
        b->Box(5, 15, 0, 5, 15, 0, bois); b->Box(8, 10, 0, 8, 10, 0, bois);
        // tête du bâton : racines qui enserrent le cristal
        b->Line(6.5f, 23, 0.5f, 4.5f, 27, 0.5f, 0.6f, boisS);
        b->Line(6.5f, 23, 0.5f, 8.5f, 27, 0.5f, 0.6f, boisS);
        b->Line(6.5f, 23, 0.5f, 6.5f, 27, -1.5f, 0.6f, boisS);
        b->Line(6.5f, 23, 0.5f, 6.5f, 27, 2.5f, 0.6f, boisS);
        b->Box(5, 4, 0, 7, 5, 1, Pal::Cuir);                                                 // poignée de cuir
        auto cristal = Nouveau<VoxelModel>();
        Color32 cr = Pal::C("8fe3ff"), crS = Pal::C("4fb0e0");
        cristal->Box(5, 25, -1, 7, 28, 2, cr); cristal->Box(6, 29, 0, 6, 29, 1, cr); cristal->Box(6, 24, 0, 6, 24, 1, crS);
        Peint(*cristal, 5, 25, -1, 5, 28, 2, crS); cristal->Remove(5, 28, -1); cristal->Remove(7, 28, 2); cristal->Remove(5, 25, 2); cristal->Remove(7, 25, -1);
        Quaternion q = Quaternion::AngleAxis(8.0f, Vector3::right) * Quaternion::AngleAxis(-4.0f, Vector3::forward);
        a.Arme->Merge(*Pivoter(*b, a.PArme, q), 0, 0, 0);
        a.Lueur->Merge(*Pivoter(*cristal, a.PArme, q), 0, 0, 0);
        a.NomLueur = "lueur_baton";
        // le bâton est tenu un peu plus loin du corps, la main droite le serre
        return Assembler("heros_Mage", a);
    }
}
