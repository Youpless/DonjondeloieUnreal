// Donjon de l'Oie — duels entre joueurs, combats contre les monstres, pièges.
#include "Partie.h"

namespace DonjonDeLOie::Regles
{
    using Moteur::Texte::De;

    /// Duel : chacun lance son attaque + 1D3 ; le plus haut gagne. Le perdant perd un niveau (deux si l'écart atteint 5)
    /// et de l'or ; le gagnant gagne 1 XP. En embuscade, l'attaquant a +2.
    Signaux Partie::Duel(Ptr<Joueur> a, Ptr<Joueur> b, bool embuscade)
    {
        Ptr<SCombat> c;
        for (int lancer = 1; lancer <= 3; lancer++)
        {
            c = Nouveau<SCombat>();
            c->Duel = true;
            c->Attaquant = a->Id;
            c->Defenseur = b->Id;
            c->FacesDe = 3;
            c->AtkA = a->Attaque();
            c->AtkB = b->Attaque();
            c->Embuscade = embuscade;
            if (embuscade)
            {
                c->BonusA = 2;
                c->RaisonBonusA = "Embuscade";
            }
            c->DeA = Des.D3();
            c->DeB = Des.D3();
            if (c->TotalA() == c->TotalB() && lancer < 3)
            {
                c->Egalite = true;
                co_yield c;
                co_yield Nouveau<SAnnonce>("Égalité ! On relance.", a->Id, Ton::Combat);
                continue;
            }
            c->AttaquantGagne = c->TotalA() > c->TotalB();   // troisième égalité : le défenseur tient bon
            co_yield c;
            break;
        }
        auto gagnant = c->AttaquantGagne ? a : b;
        auto perdant = c->AttaquantGagne ? b : a;
        int ecart = std::abs(c->TotalA() - c->TotalB());
        gagnant->DuelsGagnes++;
        perdant->DuelsPerdus++;
        co_yield Nouveau<SAnnonce>(gagnant->Nom + " remporte le duel contre " + perdant->Nom + " (" + De(std::max(c->TotalA(), c->TotalB())) + " contre " +
                                       De(std::min(c->TotalA(), c->TotalB())) + ").",
                                   gagnant->Id, Ton::Combat, true);

        int vol = std::min(perdant->Or, 5 * std::max(1, ecart));
        if (vol > 0)
        {
            perdant->Or -= vol;
            gagnant->Or += vol;
            gagnant->OrGagne += vol;
            co_yield Nouveau<SStat>(perdant->Id, Stat::Or, -vol);
            co_yield Nouveau<SStat>(gagnant->Id, Stat::Or, vol);
        }
        for (auto& s : PerdreNiveau(perdant, ecart >= 5 ? 2 : 1, "duel perdu contre " + gagnant->Nom)) co_yield s;
        perdant->ProtegeDuel = true;
        protectionNeeCeTour.Add(perdant->Id);
        for (auto& s : GagnerXP(gagnant, 1)) co_yield s;
    }

    Ptr<DefMonstre> Partie::TirerMonstre(const Ptr<Joueur>& j, Zone z)
    {
        Liste<Ptr<DefMonstre>> possibles;
        for (auto& m : Donnees::Monstres)
            if (!m->Boss && !m->Errant && m->Zones.IndexOf(z) >= 0) possibles.Add(m);
        if (possibles.Count() == 0)
            for (auto& m : Donnees::Monstres)
                if (!m->Boss && !m->Errant)
                {
                    possibles.Add(m);
                    break;
                }
        if (possibles.Count() == 0)
            for (auto& m : Donnees::Monstres)
                if (!m->Errant)
                {
                    possibles.Add(m);
                    break;
                }
        if (possibles.Count() == 0) possibles.Add(Donnees::Monstres[0]);
        // Une quête de chasse attire sa proie, si elle vit dans la région.
        if (j->Quete != nullptr && j->Quete->Type == Quete::Genre::Chasser && Des.Reel() < 0.4)
            for (auto& m : possibles)
                if (m->Id == j->Quete->MonstreCible) return m;
        return Des.Choisir(possibles);
    }

    Signaux Partie::CombatMonstre(Ptr<Joueur> j, Ptr<DefMonstre> m, bool fuitePossible)
    {
        auto q = Nouveau<SQuestion>(j->Id, GenreQuestion::Monstre, m->Nom,
                                    m->Description + "\nForce " + De(m->Force) + " · dégâts " + De(m->Degats) + " · butin " + De(m->Or) + " or, " + De(m->XP) + " XP");
        q->Monstre = m;
        q->Ajouter("Combattre", "combat", "Votre attaque + 1D6 contre sa force + 1D6. L'égalité vous sourit.");
        if (fuitePossible && Plateau[j->Case]->Precedents.Count() > 0) q->Ajouter("Fuir", "fuir", "Reculer d'une case, sans butin.");
        co_yield q;
        if (q->CodeChoisi() == "fuir")
        {
            int de = j->Case, vers = Plateau[j->Case]->Precedents[0];
            j->Case = vers;
            co_yield Nouveau<SPas>(j->Id, de, vers);
            co_yield Nouveau<SAnnonce>(j->Nom + " fuit devant " + m->Nom + ".", j->Id, Ton::Neutre);
            for (auto& s : ArretDragonnoie(j, nullptr)) co_yield s;   // … droit sur le Dragonnoie ?
            co_return;
        }

        auto c = CombatContreMonstre(j, m);
        co_yield c;

        if (c->AttaquantGagne)
        {
            j->MonstresTues++;
            co_yield Nouveau<SAnnonce>(j->Nom + " terrasse " + (m->Boss ? m->Nom : "un " + Moteur::Texte::Minuscules(m->Nom)) + " !", j->Id, Ton::Combat, m->Boss);
            for (auto& s : GagnerOr(j, m->Or, "")) co_yield s;
            if (j->Quete != nullptr && j->Quete->Type == Quete::Genre::Chasser && j->Quete->MonstreCible == m->Id)
            {
                j->Quete->Fait++;
                if (j->Quete->Fait >= j->Quete->Nombre)
                    for (auto& s : FinirQuete(j)) co_yield s;
            }
            for (auto& s : ProgresQuete(j, Quete::Genre::Vaincre)) co_yield s;
            for (auto& s : GagnerXP(j, m->XP)) co_yield s;
        }
        else
        {
            int degats = std::max(1, m->Degats - j->Defense());
            co_yield Nouveau<SAnnonce>(m->Nom + " repousse " + j->Nom + " : −" + De(degats) + " PV.", j->Id, Ton::Mauvais);
            for (auto& s : Blesser(j, degats, "vaincu par " + m->Nom)) co_yield s;
        }
    }

    Ptr<SCombat> Partie::CombatContreMonstre(const Ptr<Joueur>& j, const Ptr<DefMonstre>& m)
    {
        auto c = Nouveau<SCombat>();
        c->Duel = false;
        c->Attaquant = j->Id;
        c->Monstre = m;
        c->FacesDe = 6;
        c->AtkA = j->Attaque();
        c->AtkB = m->Force;
        Liste<string> raisons;
        if (j->Classe == ClasseHeros::Guerrier)
        {
            c->BonusA += 1;
            raisons.Add("Vétéran");
        }
        if (j->Classe == ClasseHeros::Mage)
        {
            c->BonusA += 1;
            raisons.Add("Trait de feu");
        }
        if (j->Benediction > 0)
        {
            c->BonusA += j->Benediction;
            raisons.Add("Bénédiction");
            j->Benediction = 0;
        }
        c->RaisonBonusA = raisons.Count() > 0 ? Moteur::Texte::Joindre(", ", raisons) : string();
        c->DeA = Des.D6();
        c->DeB = Des.D6();
        c->AttaquantGagne = c->TotalA() >= c->TotalB();
        return c;
    }

    Signaux Partie::DeclencherPiege(Ptr<Joueur> j, Ptr<Piege> p)
    {
        Pieges.Remove(p);
        auto poseur = Joueurs[p->Proprietaire];
        poseur->PiegesDeclenches++;
        {
            auto e = Nouveau<SEffet>(Effet::PiegeDeclenche);
            e->Joueur = poseur->Id;
            e->Cible = j->Id;
            e->Case = p->Case;
            e->Objet = p->Objet->Id;
            co_yield e;
        }
        co_yield Nouveau<SAnnonce>(j->Nom + " tombe dans un piège de " + poseur->Nom + " : " + p->Objet->Nom + " !", j->Id, Ton::Mauvais, true);
        switch (p->Objet->Effet)
        {
            case EffetObjet::PiegeLoups:
                j->ToursImmobilise = std::max(j->ToursImmobilise, 1);
                for (auto& s : Blesser(j, p->Objet->Bonus, "piège à loups")) co_yield s;
                break;
            case EffetObjet::PiegeFosse:
                for (auto& s : Blesser(j, p->Objet->Bonus, "fosse à pieux")) co_yield s;
                break;
            case EffetObjet::PiegeFilet:
            {
                int vol = std::min(j->Or, p->Objet->Bonus);
                if (vol > 0)
                {
                    j->Or -= vol;
                    poseur->Or += vol;
                    poseur->OrGagne += vol;
                    co_yield Nouveau<SStat>(j->Id, Stat::Or, -vol);
                    co_yield Nouveau<SStat>(poseur->Id, Stat::Or, vol);
                }
                break;
            }
            case EffetObjet::PiegeRune:
            {
                bool bloque = false;
                for (auto& s : Amulette(j, [&bloque](bool r) { bloque = r; })) co_yield s;
                if (!bloque)
                    for (auto& s : PerdreNiveau(j, std::max(1, p->Objet->Bonus), "rune d'oubli")) co_yield s;
                break;
            }
            default: break;
        }
    }
}
