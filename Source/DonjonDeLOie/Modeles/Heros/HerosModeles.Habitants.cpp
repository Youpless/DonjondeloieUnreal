// Donjon de l'Oie — les habitants de Valcreux, sur le gabarit des héros (1 voxel = 5 cm) : la foule du marché du
// mini-jeu de l'évadé. Leur allure (teint, cheveux, coiffure, barbe, couvre-chef, bandeau sur l'œil, balafre, habits)
// se tire d'une graine ; le visage se lit sur l'avis de recherche (Portrait : la tête vue de face, 11 × 11 pixels).
#include "HerosInterne.h"

#include "../../Voxel/PixelIcones.h"

#include <climits>
#include <cstdint>
#include <iterator>

namespace DonjonDeLOie::Modeles
{
    // ------------------------------------------------------------------ Allure : ce qui distingue un habitant d'un autre
    // (la structure et Copie sont dans Modeles/HerosModeles.h)

    bool Allure::MemeVisage(const Allure& o) const
    {
        return Peau == o.Peau && Cheveux == o.Cheveux && Coiffure == o.Coiffure && Barbe == o.Barbe && Couvre == o.Couvre
            && (Couvre == 0 || CouleurCouvre == o.CouleurCouvre) && Oeil == o.Oeil && Cicatrice == o.Cicatrice;
    }

    Ptr<Allure> Allure::Hasard(AleaNet& r)
    {
        // (l'initialiseur d'objet de C# : les tirages dans l'ordre du texte, un par instruction)
        auto a = Nouveau<Allure>();
        a->Peau = r.Next(NPeaux);
        a->Cheveux = r.Next(NCheveux);
        a->Coiffure = r.Next(NCoiffures);
        a->Barbe = r.Next(10) < 4 ? 0 : r.Next(NBarbes);
        a->Couvre = r.Next(10) < 4 ? 0 : r.Next(NCouvres);
        a->CouleurCouvre = r.Next(NCouleurs);
        a->Oeil = r.Next(10) == 0 ? 1 + r.Next(2) : 0;
        a->Cicatrice = r.Next(8) == 0;
        a->Tunique = r.Next(NCouleurs);
        a->Pantalon = r.Next(NCouleurs);
        a->Tablier = r.Next(4) == 0;
        a->Objet = r.Next(5) < 2 ? 0 : r.Next(4);
        if (a->Coiffure == 2 && a->Couvre == 4) a->Coiffure = 0;
        return a;
    }

    /// Un sosie : le même visage à un ou deux détails près (le piège pour l'œil trop pressé).
    Ptr<Allure> Allure::Sosie(AleaNet& r) const
    {
        auto a = Copie();
        int n = 1 + r.Next(2);
        for (int i = 0; i < n; i++)
            switch (r.Next(6))
            {
                case 0: a->Cheveux = (a->Cheveux + 1 + r.Next(NCheveux - 1)) % NCheveux; break;
                case 1: a->Barbe = (a->Barbe + 1 + r.Next(NBarbes - 1)) % NBarbes; break;
                case 2: a->Oeil = a->Oeil == 0 ? 1 + r.Next(2) : (r.Next(2) == 0 ? 0 : 3 - a->Oeil); break;
                case 3: a->Couvre = (a->Couvre + 1 + r.Next(NCouvres - 1)) % NCouvres; break;
                case 4: a->Cicatrice = !a->Cicatrice; break;
                default: a->Coiffure = (a->Coiffure + 1 + r.Next(NCoiffures - 1)) % NCoiffures; break;
            }
        a->Tunique = r.Next(NCouleurs); a->Pantalon = r.Next(NCouleurs); a->Tablier = r.Next(4) == 0; a->Objet = r.Next(4);
        return a;
    }
}

namespace DonjonDeLOie::Modeles::HerosModeles
{
    using namespace DonjonDeLOie::Voxel;

    namespace
    {
        constexpr Color32 Peaux[] = {Pal::C("f0c8a0"), Pal::C("d9a57a"), Pal::C("b07850"), Pal::C("7a4f33")};
        constexpr Color32 Chevelures[] = {Pal::C("231a16"), Pal::C("5a3a22"), Pal::C("8a5a30"), Pal::C("b8471f"), Pal::C("e0c070"), Pal::C("b8b2aa")};
        constexpr Color32 Etoffes[] = {Pal::C("b8463a"), Pal::C("3e6aa8"), Pal::C("5e8a3a"), Pal::C("c9a03a"), Pal::C("7a4a8a"), Pal::C("8a6a4a")};

        // (définies plus bas, après Habitant, comme dans la version C#)
        void Coiffure(VoxelModel& t, int style, Color32 c);
        void Barbe(VoxelModel& t, int style, Color32 c);
        void CouvreChef(VoxelModel& t, int genre, Color32 c);
    }

    /// Un habitant de Valcreux, d'après son allure.
    Ptr<VoxelRig> Habitant(const Allure& al)
    {
        Atelier a;
        Color32 peau = Peaux[al.Peau % static_cast<int>(std::size(Peaux))], cheveux = Chevelures[al.Cheveux % static_cast<int>(std::size(Chevelures))];
        Color32 tunique = Etoffes[al.Tunique % static_cast<int>(std::size(Etoffes))], tuniqueS = Sh(tunique, 0.72f);
        Color32 pantalon = Sh(Etoffes[al.Pantalon % static_cast<int>(std::size(Etoffes))], 0.6f), souliers = Pal::C("3a2a20");

        // ---- corps : tunique ceinturée, tablier éventuel
        auto& k = *a.Corps;
        Taille(k, -5, 6, -3, 4, 15, 3, tunique, AretesV | AretesHaut);
        Peint(k, -5, 6, -3, 4, 6, 3, tuniqueS);
        k.Box(-5, 9, -3, 4, 9, 3, Pal::Cuir); k.Set(-1, 9, 4, Pal::OrSombre);
        Peint(k, -2, 13, 3, 1, 15, 3, Sh(tunique, 1.15f));                          // col
        if (al.Tablier) { k.Box(-4, 5, 4, 3, 13, 4, Pal::C("e8e0c8")); k.Box(-3, 14, 4, 2, 14, 4, Pal::C("e8e0c8")); }
        if (al.Objet == 2) { Taille(k, -4, 8, -6, 3, 14, -4, Pal::C("c9b07a"), AretesV); k.Box(-1, 14, -5, 0, 15, -4, Pal::C("8a7048")); }

        // ---- jambes et bras
        Jambe(*a.JambeD, pantalon, souliers, 2, Sh(souliers, 0.7f));
        Miroir(*a.JambeD, *a.JambeG);
        Bras(*a.BrasD, tunique, peau);
        Miroir(*a.BrasD, *a.BrasG);
        if (al.Objet == 1)
        {
            // panier d'osier au bras droit, garni de pommes
            Taille(*a.BrasD, 4, 1, -1, 9, 4, 3, Pal::C("b08a4e"), AretesV | AretesBas);
            for (int x = 5; x <= 8; x++)
                for (int z = 0; z <= 2; z++) a.BrasD->Set(x, 5, z, (x + z) % 2 == 0 ? Pal::C("c23a2a") : Pal::C("9ab03a"));
        }
        if (al.Objet == 3) a.BrasD->Line(6, 0, 1, 6, 22, 1, 0.6f, Pal::C("7a5a3a"));   // bâton de marche

        // ---- tête
        auto& t = *a.Tete;
        if (al.Couvre == 4)
            Capuche(t, Etoffes[al.CouleurCouvre % static_cast<int>(std::size(Etoffes))], Sh(Etoffes[al.CouleurCouvre % static_cast<int>(std::size(Etoffes))], 0.65f), 1);
        TeteNue(t, peau, al.Coiffure == 2 && al.Couvre == 0 ? Sh(peau, 0.7f) : Sh(cheveux, 0.9f));
        if (al.Couvre != 4) Coiffure(t, al.Coiffure, cheveux);
        Barbe(t, al.Barbe, al.Cheveux == 5 ? cheveux : Sh(cheveux, 1.05f));
        if (al.Oeil > 0)
        {
            int x = al.Oeil == 1 ? -2 : 1;                        // 1 : son œil gauche (à droite de l'image, vu de face), 2 : le droit
            t.Box(x - 1, 19, 3, x + 1, 20, 4, Pal::C("16110f"));
            for (int xx = -4; xx <= 3; xx++)
                if (!(xx >= x - 1 && xx <= x + 1)) t.Set(xx, 21, 3, Pal::C("2a201c"));
        }
        if (al.Cicatrice) { t.Set(1, 18, 4, Pal::C("8e3a2e")); t.Set(2, 17, 4, Pal::C("8e3a2e")); t.Set(2, 18, 4, Pal::C("b8584a")); }
        CouvreChef(t, al.Couvre, Etoffes[al.CouleurCouvre % static_cast<int>(std::size(Etoffes))]);

        auto rig = Assembler("habitant", a);
        return rig;
    }

    namespace
    {
        void Coiffure(VoxelModel& t, int style, Color32 c)
        {
            Color32 cS = Sh(c, 0.8f);
            if (style == 2)
            {
                // chauve : une couronne de cheveux autour de l'arrière du crâne
                for (int x = -4; x <= 3; x++) { t.Set(x, 19, -4, cS); t.Set(x, 20, -4, c); }
                for (int z = -3; z <= 0; z++) { t.Set(-5, 20, z, c); t.Set(4, 20, z, c); }
                return;
            }
            // calotte : le dessus, l'arrière et les tempes, deux mèches sur le front
            t.Box(-4, 23, -3, 3, 23, 2, c);
            Peint(t, -4, 22, -3, 3, 22, 3, c);
            t.Box(-4, 18, -4, 3, 22, -4, c);
            t.Box(-5, 20, -3, -5, 22, 1, cS); t.Box(4, 20, -3, 4, 22, 1, cS);
            t.Set(-1, 21, 3, c); t.Set(2, 21, 3, c);
            if (style == 1) { t.Box(-5, 14, -4, 4, 18, -4, c); t.Box(-5, 16, -3, -5, 18, 0, cS); t.Box(4, 16, -3, 4, 18, 0, cS); }   // longs
            if (style == 3) t.Ellipsoid(-0.5f, 23.5f, -4.0f, 2.2f, 2.2f, 2.0f, c);                                               // chignon
            if (style == 4)
                for (int x = -4; x <= 3; x += 2) { t.Set(x, 24, 0, c); t.Set(x + 1, 24, -2, cS); t.Set(x, 25, -1, c); }       // épis
        }

        void Barbe(VoxelModel& t, int style, Color32 c)
        {
            if (style == 0) return;
            Color32 cS = Sh(c, 0.82f);
            if (style >= 1) { t.Box(-3, 17, 4, 2, 17, 4, c); t.Remove(-1, 17, 4); t.Remove(0, 17, 4); t.Set(-1, 17, 4, cS); t.Set(0, 17, 4, cS); }   // moustache
            if (style >= 2) { t.Box(-4, 16, 3, 3, 16, 4, c); t.Box(-4, 16, 0, -4, 18, 3, cS); t.Box(3, 16, 0, 3, 18, 3, cS); t.Box(-2, 15, 3, 1, 15, 4, c); }
            if (style >= 3) { t.Box(-3, 13, 3, 2, 15, 4, c); t.Box(-2, 12, 3, 1, 12, 4, cS); }
        }

        void CouvreChef(VoxelModel& t, int genre, Color32 c)
        {
            Color32 cS = Sh(c, 0.7f);
            switch (genre)
            {
                case 1:     // bonnet de laine, revers clair
                    t.Box(-5, 21, -4, 4, 21, 4, Sh(c, 1.2f)); Vide(t, -5, 21, -4, -5, 21, -4); Vide(t, 4, 21, 4, 4, 21, 4);
                    t.Box(-4, 22, -3, 3, 24, 3, c); t.Box(-3, 25, -2, 2, 25, 2, cS);
                    break;
                case 2:     // chapeau de paille à large bord
                {
                    Color32 paille = Pal::C("d8b860"); Color32 pailleS = Pal::C("a88a3e");
                    for (int x = -8; x <= 7; x++)
                        for (int z = -7; z <= 7; z++)
                            if ((static_cast<float>(x) + 0.5f) * (static_cast<float>(x) + 0.5f) + (static_cast<float>(z) + 0.5f) * (static_cast<float>(z) + 0.5f) <= 7.4f * 7.4f)
                                t.Set(x, 23, z, (x + z) % 3 == 0 ? pailleS : paille);
                    t.Box(-4, 24, -3, 3, 25, 3, paille); t.Box(-4, 24, -3, 3, 24, 3, c);   // ruban
                    break;
                }
                case 3:     // foulard noué
                    t.Box(-5, 21, -4, 4, 23, 2, c); t.Box(-4, 24, -3, 3, 24, 1, c); t.Box(-1, 18, -5, 0, 21, -5, cS);
                    break;
                case 5:     // béret penché
                    t.Box(-5, 23, -4, 4, 23, 3, c); t.Box(-4, 24, -3, 2, 24, 2, c); t.Set(0, 25, 0, cS);
                    break;
            }
        }
    }

    /// L'évadé d'une quête : son allure vient de la graine de la quête, et son surnom se voit (le Borgne porte un
    /// bandeau, la Rousse a les cheveux roux, le Balafré une balafre) ; une évadée n'a pas de barbe.
    Ptr<Allure> AllureEvade(int graine, const string& nom, bool feminin)
    {
        AleaNet r(static_cast<int>(static_cast<uint32_t>(graine) * 31u + 7u));     // graine * 31 + 7, qui boucle comme en C#
        auto a = Allure::Hasard(r);
        string n = Texte::Minuscules(nom);
        if (feminin) { a->Barbe = 0; if (a->Coiffure == 2 || a->Coiffure == 4) a->Coiffure = r.Next(2) == 0 ? 1 : 3; }
        a->Oeil = Texte::Contient(n, "borgne") ? 1 + r.Next(2) : 0;
        a->Cicatrice = Texte::Contient(n, "balafr") || (a->Cicatrice && r.Next(2) == 0);
        if (Texte::Contient(n, "rousse") || Texte::Contient(n, "roux"))
        {
            a->Cheveux = 3;                                         // et qu'on les voie : pas de capuche, des cheveux longs ou un chignon
            if (a->Couvre == 4) a->Couvre = 0;
            if (a->Coiffure != 1 && a->Coiffure != 3) a->Coiffure = r.Next(2) == 0 ? 1 : 3;
        }
        if (a->Couvre == 4 && a->Coiffure == 2) a->Coiffure = 0;
        return a;
    }

    /// L'icône du portrait d'un évadé (calculée une fois, puis gardée parmi les icônes dynamiques).
    string IconePortrait(int graine, const string& nom, bool feminin)
    {
        string id = "portrait_" + std::to_string(graine);
        if (!PixelIcones::Dynamiques().ContainsKey(id)) PixelIcones::Dynamiques()[id] = Portrait(*Habitant(*AllureEvade(graine, nom, feminin)));
        return id;
    }

    /// La tête vue de face, en 11 × 11 pixels (de y 14 à 24) : l'avis de recherche. Ligne 0 en bas ; vue de face, la
    /// droite du personnage (x positifs, côté du bras droit) est à gauche de l'image, comme sur une photo.
    Liste<Color32> Portrait(const VoxelRig& rig)
    {
        constexpr int N = 11;
        Liste<Color32> px(N * N);
        Liste<int> prof(N * N);
        for (int i = 0; i < prof.Length(); i++) prof[i] = INT_MIN;
        for (const char* nom : {"tete", "corps"})
        {
            auto p = rig.Get(nom);
            if (p == nullptr) continue;
            for (auto& kv : p->Model->Tous())
            {
                int x = 5 - kv.Key.x, y = kv.Key.y - 14;
                if (x < 0 || x >= N || y < 0 || y >= N) continue;
                int i = y * N + x;
                if (kv.Key.z <= prof[i]) continue;
                prof[i] = kv.Key.z;
                px[i] = kv.Value;
            }
        }
        // un peu de relief : ce qui est en retrait s'assombrit
        for (int i = 0; i < px.Length(); i++)
            if (prof[i] != INT_MIN && prof[i] < 3) px[i] = Pal::Shade(px[i], Mathf::Clamp(0.78f + 0.07f * (static_cast<float>(prof[i]) + 0.5f), 0.55f, 1.0f));
        return px;
    }
}
