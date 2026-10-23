// Donjon de l'Oie — la nuit : elle tombe après chaque cinquième tour de table (réglable). Un événement l'occupe, tiré
// parmi ceux qui peuvent avoir lieu : le voleur de la nuit (un joueur perd la moitié de son or), le trou de ver (deux
// joueurs échangent leur place), la bonne nuit (un joueur gagne un niveau, jamais celui de la victoire) et l'arène du
// diable (un tournoi : chaque défaite coûte un niveau, le vainqueur gagne un niveau et de l'or).
#include "Partie.h"

namespace DonjonDeLOie::Regles
{
    using Moteur::Texte::De;

    string NomEnum(EvenementNuit v)
    {
        static const char* n[] = {"Voleur", "TrouDeVer", "BonneNuit", "Arene"};
        return n[static_cast<int>(v)];
    }

    int Partie::IntervalleNuits() { return std::max(1, Donnees::Reglages->ToursEntreNuits); }

    bool Partie::NuitAvantLeTour(int tour) { return Donnees::Reglages->NuitsActives && tour > 1 && (tour - 1) % IntervalleNuits() == 0; }

    int Partie::ProchaineNuit() const
    {
        if (!Donnees::Reglages->NuitsActives) return -1;
        int t = Tour + 1;
        if (Courant == 0 && tourDerniereNuit != Tour && NuitAvantLeTour(Tour)) t = Tour;   // elle tombe tout de suite
        while (!NuitAvantLeTour(t)) t++;
        return t - 1;
    }

    Signaux Partie::Nuit()
    {
        tourDerniereNuit = Tour;
        Nuits++;
        co_yield Nouveau<SNuit>(true, Tour - 1, Nuits);
        co_yield Nouveau<SAnnonce>("Fin du tour " + De(Tour - 1) + " : la nuit tombe sur l'île…", -1, Ton::Systeme, true);

        Liste<EvenementNuit> possibles;
        auto& rg = *Donnees::Reglages;
        if (rg.NuitVoleur && Joueurs.Exists([](const Ptr<Joueur>& j) { return j->Or >= 2; })) possibles.Add(EvenementNuit::Voleur);
        if (rg.NuitTrouDeVer && PaireSurDeuxCases().Count() > 0) possibles.Add(EvenementNuit::TrouDeVer);
        if (rg.NuitBonneNuit && Joueurs.Exists([this](const Ptr<Joueur>& j) { return j->Niveau + 1 < NiveauCible; })) possibles.Add(EvenementNuit::BonneNuit);
        if (rg.NuitArene && Joueurs.Count() >= 2) possibles.Add(EvenementNuit::Arene);
        if (possibles.Count() > 0)
        {
            DerniereNuit = Des.Choisir(possibles);
            switch (DerniereNuit)
            {
                case EvenementNuit::Voleur:
                    for (auto& s : NuitVoleur()) co_yield s;
                    break;
                case EvenementNuit::TrouDeVer:
                    for (auto& s : NuitTrouDeVer()) co_yield s;
                    break;
                case EvenementNuit::BonneNuit:
                    for (auto& s : NuitBonneNuit()) co_yield s;
                    break;
                default:
                    for (auto& s : NuitArene()) co_yield s;
                    break;
            }
            if (Fini) co_return;          // l'arène peut couronner un vainqueur en pleine nuit
        }
        co_yield Nouveau<SNuit>(false, Tour - 1, Nuits);
        co_yield Nouveau<SAnnonce>("Le jour se lève.", -1, Ton::Systeme);
    }

    // ------------------------------------------------------------------ le voleur de la nuit

    Signaux Partie::NuitVoleur()
    {
        auto v = Des.Choisir(Joueurs.FindAll([](const Ptr<Joueur>& j) { return j->Or >= 2; }));
        int vol = v->Or / 2;
        v->Or -= vol;
        {
            auto e = Nouveau<SEffet>(Effet::VolNocturne);
            e->Joueur = v->Id;
            e->Texte = De(vol);
            co_yield e;
        }
        co_yield Nouveau<SStat>(v->Id, Stat::Or, -vol);
        co_yield Nouveau<SAnnonce>("Un voleur se glisse dans le campement de " + v->Nom + " : la moitié de sa bourse disparaît (−" + De(vol) + " or).", v->Id,
                                   Ton::Mauvais, true);
    }

    // ------------------------------------------------------------------ le trou de ver

    /// Deux joueurs sur deux cases différentes, tirés au hasard parmi toutes les paires possibles (vide s'il n'y en a pas).
    Liste<Ptr<Joueur>> Partie::PaireSurDeuxCases()
    {
        Liste<Liste<Ptr<Joueur>>> paires;
        for (int a = 0; a < Joueurs.Count(); a++)
            for (int b = a + 1; b < Joueurs.Count(); b++)
                if (Joueurs[a]->Case != Joueurs[b]->Case) paires.Add(Liste<Ptr<Joueur>>{Joueurs[a], Joueurs[b]});
        return paires.Count() > 0 ? Des.Choisir(paires) : Liste<Ptr<Joueur>>();
    }

    Signaux Partie::NuitTrouDeVer()
    {
        auto p = PaireSurDeuxCases();
        auto a = p[0], b = p[1];
        int ca = a->Case, cb = b->Case;
        {
            auto e = Nouveau<SEffet>(Effet::TrouDeVer);
            e->Joueur = a->Id;
            e->Cible = b->Id;
            e->Case = ca;
            co_yield e;
        }
        co_yield Nouveau<SAnnonce>("Un trou de ver s'ouvre dans la nuit : " + a->Nom + " et " + b->Nom + " échangent leur place !", -1, Ton::Systeme, true);
        for (auto& s : Teleporter(a, cb, false, "Trou de ver")) co_yield s;
        if (Fini) co_return;
        for (auto& s : Teleporter(b, ca, false, "Trou de ver")) co_yield s;
    }

    // ------------------------------------------------------------------ la bonne nuit

    Signaux Partie::NuitBonneNuit()
    {
        auto j = Des.Choisir(Joueurs.FindAll([this](const Ptr<Joueur>& o) { return o->Niveau + 1 < NiveauCible; }));
        {
            auto e = Nouveau<SEffet>(Effet::BonneNuit);
            e->Joueur = j->Id;
            co_yield e;
        }
        co_yield Nouveau<SAnnonce>(j->Nom + " dort comme un loir et se réveille plus fort.", j->Id, Ton::Bon, true);
        for (auto& s : GagnerNiveau(j, false)) co_yield s;
    }

    // ------------------------------------------------------------------ l'arène du diable

    /// Un tournoi dans un ordre tiré au sort : le champion reste dans l'arène et affronte le suivant. Chaque défaite
    /// coûte un niveau ; le dernier champion gagne un niveau (même celui de la victoire) et de l'or.
    Signaux Partie::NuitArene()
    {
        auto ordre = Joueurs.ToArray();
        for (int i = ordre.Count() - 1; i > 0; i--)
        {
            int k = Des.Entre(0, i);
            auto t = ordre[i];
            ordre[i] = ordre[k];
            ordre[k] = t;
        }
        Liste<string> ids, noms;
        for (auto& j : ordre)
        {
            ids.Add(De(j->Id));
            noms.Add(j->Nom);
        }
        {
            auto e = Nouveau<SEffet>(Effet::AreneDebut);
            e->Texte = Moteur::Texte::Joindre(",", ids);
            co_yield e;
        }
        co_yield Nouveau<SAnnonce>("L'arène du diable surgit des ténèbres ! Tournoi nocturne, dans cet ordre : " + Moteur::Texte::Joindre(", ", noms) + ".", -1,
                                   Ton::Combat, true);
        auto champion = ordre[0];
        for (int k = 1; k < ordre.Count(); k++)
        {
            auto defi = ordre[k];
            co_yield Nouveau<SAnnonce>(defi->Nom + " entre dans l'arène face à " + champion->Nom + ".", defi->Id, Ton::Combat);
            Ptr<SCombat> c;
            for (auto& s : DuelArene(defi, champion, [&c](Ptr<SCombat> r) { c = r; })) co_yield s;
            auto gagnant = c->AttaquantGagne ? defi : champion;
            auto perdant = c->AttaquantGagne ? champion : defi;
            gagnant->DuelsGagnes++;
            perdant->DuelsPerdus++;
            co_yield Nouveau<SAnnonce>(gagnant->Nom + " l'emporte sur " + perdant->Nom + " (" + De(std::max(c->TotalA(), c->TotalB())) + " contre " +
                                           De(std::min(c->TotalA(), c->TotalB())) + ").",
                                       gagnant->Id, Ton::Combat, true);
            for (auto& s : PerdreNiveau(perdant, 1, "vaincu dans l'arène du diable")) co_yield s;
            champion = gagnant;
        }
        {
            auto e = Nouveau<SEffet>(Effet::AreneFin);
            e->Joueur = champion->Id;
            co_yield e;
        }
        co_yield Nouveau<SAnnonce>(champion->Nom + " remporte l'arène du diable !", champion->Id, Ton::Combat, true);
        for (auto& s : GagnerOr(champion, Donnees::Reglages->OrArene, "l'arène du diable")) co_yield s;
        for (auto& s : GagnerNiveau(champion, true)) co_yield s;
    }

    /// Un duel de l'arène : attaque + 1D3 chacun ; deux relances en cas d'égalité, puis le champion tient bon.
    Signaux Partie::DuelArene(Ptr<Joueur> defi, Ptr<Joueur> champion, std::function<void(Ptr<SCombat>)> resultat)
    {
        Ptr<SCombat> c;
        for (int lancer = 1; lancer <= 3; lancer++)
        {
            c = Nouveau<SCombat>();
            c->Duel = true;
            c->Arene = true;
            c->Attaquant = defi->Id;
            c->Defenseur = champion->Id;
            c->FacesDe = 3;
            c->AtkA = defi->Attaque();
            c->AtkB = champion->Attaque();
            c->DeA = Des.D3();
            c->DeB = Des.D3();
            if (c->TotalA() == c->TotalB() && lancer < 3)
            {
                c->Egalite = true;
                co_yield c;
                co_yield Nouveau<SAnnonce>("Égalité ! On relance.", defi->Id, Ton::Combat);
                continue;
            }
            c->AttaquantGagne = c->TotalA() > c->TotalB();
            co_yield c;
            break;
        }
        resultat(c);
    }

    // ------------------------------------------------------------------ un niveau gagné d'un coup

    /// Un niveau de plus, sans passer par l'expérience. « peutGagner » faux : jamais le niveau de la victoire.
    Signaux Partie::GagnerNiveau(Ptr<Joueur> j, bool peutGagner)
    {
        if (Fini || (!peutGagner && j->Niveau + 1 >= NiveauCible)) co_return;
        j->Niveau++;
        j->NiveauMax = std::max(j->NiveauMax, j->Niveau);
        j->PV = std::min(j->PVMax(), j->PV + 2);
        co_yield Nouveau<SStat>(j->Id, Stat::Niveau, 1);
        {
            auto e = Nouveau<SEffet>(Effet::NiveauGagne);
            e->Joueur = j->Id;
            co_yield e;
        }
        co_yield Nouveau<SAnnonce>(j->Nom + " passe au niveau " + De(j->Niveau) + " !", j->Id, Ton::Bon, true);
        if (j->Niveau >= NiveauCible)
        {
            Fini = true;
            Gagnant = j->Id;
            {
                auto e = Nouveau<SEffet>(Effet::Victoire);
                e->Joueur = j->Id;
                co_yield e;
            }
            co_yield Nouveau<SFinPartie>(j->Id);
        }
    }
}
