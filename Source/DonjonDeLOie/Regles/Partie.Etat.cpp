// Donjon de l'Oie — la partie : l'état des joueurs (soins, blessures, K.O., or, expérience, niveaux, amulette).
#include "Partie.h"

namespace DonjonDeLOie::Regles
{
    using Moteur::Texte::De;

    Signaux Partie::Soigner(Ptr<Joueur> j, int n, string raison)
    {
        int avant = j->PV;
        j->PV = std::min(j->PVMax(), j->PV + n);
        if (j->PV != avant)
        {
            co_yield Nouveau<SStat>(j->Id, Stat::PV, j->PV - avant);
            if (!raison.empty())
            {
                auto e = Nouveau<SEffet>(Effet::Soin);
                e->Joueur = j->Id;
                e->Texte = raison;
                co_yield e;
            }
        }
    }

    Signaux Partie::Blesser(Ptr<Joueur> j, int n, string raison)
    {
        if (n <= 0) co_return;
        j->PV -= n;
        co_yield Nouveau<SStat>(j->Id, Stat::PV, -n);
        if (j->PV <= 0)
            for (auto& s : Ko(j, raison)) co_yield s;
    }

    Signaux Partie::Ko(Ptr<Joueur> j, string raison)
    {
        j->KO++;
        int perte = j->Or / 2;
        {
            auto e = Nouveau<SEffet>(Effet::KO);
            e->Joueur = j->Id;
            e->Texte = raison;
            co_yield e;
        }
        co_yield Nouveau<SAnnonce>(j->Nom + " est K.O. (" + raison + ") ! Retour à " + Plateau[Plateau.Depart]->Libelle() + ", la moitié de son or perdue.",
                                   j->Id, Ton::Mauvais, true);
        if (perte > 0)
        {
            j->Or -= perte;
            co_yield Nouveau<SStat>(j->Id, Stat::Or, -perte);
        }
        j->Embuscade = nullptr;
        j->Case = Plateau.Depart;
        co_yield Nouveau<STeleport>(j->Id, Plateau.Depart, "K.O.");
        int pvAvant = j->PV;
        j->PV = std::max(1, j->PVMax() / 2);
        co_yield Nouveau<SStat>(j->Id, Stat::PV, j->PV - pvAvant);
        if (j == JoueurCourant()) tourFini = true;
    }

    Signaux Partie::GagnerOr(Ptr<Joueur> j, int n, string raison)
    {
        if (n <= 0) co_return;
        j->Or += n;
        j->OrGagne += n;
        co_yield Nouveau<SStat>(j->Id, Stat::Or, n);
        if (!raison.empty()) co_yield Nouveau<SAnnonce>(j->Nom + " : +" + De(n) + " or (" + raison + ").", j->Id, Ton::Bon);
    }

    Signaux Partie::PerdreOr(Ptr<Joueur> j, int n, string raison)
    {
        n = std::min(n, j->Or);
        if (n <= 0) co_return;
        j->Or -= n;
        co_yield Nouveau<SStat>(j->Id, Stat::Or, -n);
        if (!raison.empty()) co_yield Nouveau<SAnnonce>(j->Nom + " : −" + De(n) + " or (" + raison + ").", j->Id, Ton::Mauvais);
    }

    Signaux Partie::GagnerXP(Ptr<Joueur> j, int n)
    {
        if (n <= 0 || Fini) co_return;
        j->XP += n;
        co_yield Nouveau<SStat>(j->Id, Stat::XP, n);
        while (j->XP >= Donnees::XPParNiveau() && !Fini)
        {
            j->XP -= Donnees::XPParNiveau();
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

    Signaux Partie::PerdreNiveau(Ptr<Joueur> j, int n, string raison)
    {
        int avant = j->Niveau;
        j->Niveau = std::max(1, j->Niveau - n);
        int perdu = avant - j->Niveau;
        if (perdu <= 0)
        {
            if (j->XP > 0)
            {
                int x = j->XP;
                j->XP = 0;
                co_yield Nouveau<SStat>(j->Id, Stat::XP, -x);
            }
            co_yield Nouveau<SAnnonce>(j->Nom + " est déjà au niveau 1 : il perd son expérience (" + raison + ").", j->Id, Ton::Mauvais);
            co_return;
        }
        j->PV = std::min(j->PV, j->PVMax());
        co_yield Nouveau<SStat>(j->Id, Stat::Niveau, -perdu);
        {
            auto e = Nouveau<SEffet>(Effet::NiveauPerdu);
            e->Joueur = j->Id;
            e->Texte = raison;
            co_yield e;
        }
        co_yield Nouveau<SAnnonce>(j->Nom + " perd " + (perdu > 1 ? De(perdu) + " niveaux" : string("un niveau")) + " (" + raison + ") → niveau " +
                                       De(j->Niveau) + ".",
                                   j->Id, Ton::Mauvais, true);
    }

    Signaux Partie::Amulette(Ptr<Joueur> j, std::function<void(bool)> bloque)
    {
        if (j->A(EffetObjet::Amulette))
        {
            j->RetirerAccessoire(EffetObjet::Amulette);
            bloque(true);
            {
                auto e = Nouveau<SEffet>(Effet::SortBloque);
                e->Joueur = j->Id;
                co_yield e;
            }
            co_yield Nouveau<SAnnonce>("L'amulette de " + j->Nom + " absorbe le maléfice et se brise.", j->Id, Ton::Bon);
        }
        else bloque(false);
    }
}
