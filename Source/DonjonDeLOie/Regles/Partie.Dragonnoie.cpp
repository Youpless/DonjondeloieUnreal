// Donjon de l'Oie — le Dragonnoie : mi-oie, mi-dragon, il erre sur la carte. Au début d'un tour de table sur trois
// (réglable), il prend une grande envolée vers une case au hasard et défie tous ceux qui s'y trouvent. Qui passe sur sa
// case peut l'attaquer ou continuer sa route ; qui s'y arrête doit l'affronter. Le combat se joue comme contre un monstre
// (attaque + 1D6 contre sa force + 1D6). Vaincu, on perd tout son or, jamais de niveau ; vainqueur, on gagne la partie.
#include "Partie.h"

namespace DonjonDeLOie::Regles
{
    using Moteur::Texte::De;

    int Partie::Intervalle() { return std::max(1, Donnees::Reglages->ToursEntreEnvols); }

    // ------------------------------------------------------------------ ses vols

    /// Au début de la partie : il se pose loin de la case de départ.
    Signaux Partie::ArriveeDragonnoie()
    {
        if (Donnees::Dragonnoie() == nullptr) co_return;
        Liste<int> possibles;
        for (int i = 0; i < Plateau.Nombre(); i++)
        {
            if (i == Plateau.Depart) continue;
            int d = Plateau.DistanceEnAvant(Plateau.Depart, i, 60);
            if (d < 0 || d >= 9) possibles.Add(i);           // hors d'atteinte au premier tour
        }
        if (possibles.Count() == 0)
            for (int i = 0; i < Plateau.Nombre(); i++)
                if (i != Plateau.Depart) possibles.Add(i);
        if (possibles.Count() == 0) co_return;
        CaseDragonnoie = Des.Choisir(possibles);
        TourEnvol = Tour + Intervalle();
        co_yield Nouveau<SEnvol>(-1, CaseDragonnoie);
        co_yield Nouveau<SAnnonce>("Le Dragonnoie fond sur l'île et se pose sur « " + Plateau[CaseDragonnoie]->Libelle() + " ». Il change de perchoir tous les " +
                                       De(Intervalle()) + " tours. Qui le terrasse gagne la partie !",
                                   -1, Ton::Combat, true);
    }

    /// Au début d'un tour de table sur N : une grande envolée vers une case au hasard, puis il défie ceux qui y sont.
    Signaux Partie::EnvolDragonnoie()
    {
        TourEnvol = Tour + Intervalle();
        int de = CaseDragonnoie;
        int vers = CaseAtterrissage(de);
        if (vers < 0) co_return;
        CaseDragonnoie = vers;
        co_yield Nouveau<SEnvol>(de, vers);
        auto defies = JoueursSur(vers);
        Liste<string> noms;
        for (auto& j : defies) noms.Add(j->Nom);
        co_yield Nouveau<SAnnonce>("Grande envolée ! Le Dragonnoie se pose sur « " + Plateau[vers]->Libelle() + " »" +
                                       (defies.Count() > 0 ? " et défie " + Moteur::Texte::Joindre(", ", noms) + " !" : string(".")),
                                   -1, Ton::Combat, true);
        for (auto& j : defies)
        {
            for (auto& s : CombatDragonnoie(j)) co_yield s;
            if (Fini) co_return;
        }
    }

    /// Une case loin de la sienne (à plus de 12 m), jamais la case de départ.
    int Partie::CaseAtterrissage(int depuis)
    {
        Liste<int> loin, autres;
        for (int i = 0; i < Plateau.Nombre(); i++)
        {
            if (i == Plateau.Depart || i == depuis) continue;
            autres.Add(i);
            if (depuis < 0)
            {
                loin.Add(i);
                continue;
            }
            float dx = Plateau[i]->X - Plateau[depuis]->X, dz = Plateau[i]->Z - Plateau[depuis]->Z;
            if (dx * dx + dz * dz >= 12.0f * 12.0f) loin.Add(i);
        }
        if (loin.Count() > 0) return Des.Choisir(loin);
        return autres.Count() > 0 ? Des.Choisir(autres) : -1;
    }

    // ------------------------------------------------------------------ rencontres

    /// En passant sur sa case : l'attaquer (le déplacement s'arrête là) ou continuer sa route.
    Signaux Partie::PasserDragonnoie(Ptr<Joueur> j, int pasRestants, std::function<void(bool)> attaque)
    {
        auto m = Dragonnoie();
        auto q = Nouveau<SQuestion>(j->Id, GenreQuestion::Dragonnoie, m->Nom,
                                    "Le Dragonnoie vous barre la route. Il vous reste " + De(pasRestants) + " case" + (pasRestants > 1 ? "s" : "") +
                                        " à parcourir.");
        q->Monstre = m;
        q->Ajouter("Attaquer le Dragonnoie", "combat", "Victoire : la partie est à vous. Défaite : il vous arrache tout votre or.");
        q->Ajouter("Continuer sa route", "passer", "Le contourner et finir votre déplacement.");
        q->ParDefaut = 1;
        co_yield q;
        attaque(q->CodeChoisi() == "combat");
    }

    /// S'arrêter sur sa case, quelle qu'en soit la raison (dé, recul, téléportation, fuite) : il faut l'affronter.
    Signaux Partie::ArretDragonnoie(Ptr<Joueur> j, std::function<void(bool)> combat)
    {
        bool la = Dragonnoie() != nullptr && !Fini && j->Case == CaseDragonnoie;
        if (combat) combat(la);
        if (!la) co_return;
        co_yield Nouveau<SAnnonce>(j->Nom + " s'arrête devant le Dragonnoie : pas moyen de l'éviter !", j->Id, Ton::Combat, true);
        for (auto& s : CombatDragonnoie(j)) co_yield s;
    }

    /// Le combat. Vainqueur : la partie est gagnée (et son nid avec). Vaincu : tout son or perdu, aucun niveau.
    Signaux Partie::CombatDragonnoie(Ptr<Joueur> j)
    {
        auto m = Dragonnoie();
        if (m == nullptr || Fini) co_return;
        auto c = CombatContreMonstre(j, m);
        co_yield c;
        if (c->AttaquantGagne)
        {
            j->MonstresTues++;
            VaincuDragonnoie = true;
            co_yield Nouveau<SAnnonce>(j->Nom + " terrasse le Dragonnoie !", j->Id, Ton::Combat, true);
            if (OrDragonnoie > 0)
                for (auto& s : GagnerOr(j, OrDragonnoie, "le nid du Dragonnoie")) co_yield s;
            OrDragonnoie = 0;
            Fini = true;
            Gagnant = j->Id;
            {
                auto e = Nouveau<SEffet>(Effet::Victoire);
                e->Joueur = j->Id;
                e->Texte = "dragonnoie";
                co_yield e;
            }
            co_yield Nouveau<SFinPartie>(j->Id);
        }
        else
        {
            int perte = j->Or;
            co_yield Nouveau<SAnnonce>("Le Dragonnoie terrasse " + j->Nom +
                                           (perte > 0 ? " et lui arrache toute sa bourse : −" + De(perte) + " or." : string(", qui n'avait heureusement pas un sou.")),
                                       j->Id, Ton::Mauvais, true);
            if (perte > 0)
            {
                j->Or = 0;
                OrDragonnoie += perte;
                {
                    auto e = Nouveau<SEffet>(Effet::OrDragonnoie);
                    e->Joueur = j->Id;
                    e->Case = CaseDragonnoie;
                    e->Texte = De(perte);
                    co_yield e;
                }
                co_yield Nouveau<SStat>(j->Id, Stat::Or, -perte);
            }
        }
    }
}
