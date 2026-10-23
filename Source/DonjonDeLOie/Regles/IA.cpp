// Donjon de l'Oie — les bots : ils répondent aux mêmes questions que les joueurs humains.
#include "IA.h"

#include <cfloat>

namespace DonjonDeLOie::Regles
{
    // ------------------------------------------------------------------ probabilités

    double IA::ChanceDuel(int atkA, int atkB)
    {
        int gagne = 0, egal = 0;
        for (int a = 1; a <= 3; a++)
            for (int b = 1; b <= 3; b++)
            {
                int d = atkA + a - (atkB + b);
                if (d > 0) gagne++;
                else if (d == 0) egal++;
            }
        double pw = gagne / 9.0, pt = egal / 9.0;
        return pw * (1 + pt + pt * pt);
    }

    double IA::ChanceMonstre(int atk, int force)
    {
        int ok = 0;
        for (int a = 1; a <= 6; a++)
            for (int b = 1; b <= 6; b++)
                if (atk + a >= force + b) ok++;
        return ok / 36.0;
    }

    // ------------------------------------------------------------------ réponse

    int IA::Repondre(Partie& p, SQuestion& q)
    {
        auto j = p.Joueurs[q.Joueur];
        if (dernierTour != p.TourJoueurs || dernierJoueur != j->Id)
        {
            dernierTour = p.TourJoueurs;
            dernierJoueur = j->Id;
            actionsCeTour = 0;
        }
        int rep;
        switch (q.Genre)
        {
            case GenreQuestion::Action: rep = Action(p, j, q); break;
            case GenreQuestion::Embranchement: rep = Embranchement(p, j, q); break;
            case GenreQuestion::Rodeur: rep = MeilleureCase(p, j, q); break;
            case GenreQuestion::Defi: rep = Defi(p, j, q); break;
            case GenreQuestion::Monstre: rep = Monstre(j, q); break;
            case GenreQuestion::Boutique: rep = Boutique(p, j, q); break;
            case GenreQuestion::Quetes: rep = Quetes(p, j, q); break;
            case GenreQuestion::ChoixCase: rep = ChoixCase(p, j, q); break;
            case GenreQuestion::ChoixCible: rep = ChoixCible(p, j, q); break;
            case GenreQuestion::ChoixEvenement: rep = ChoixEvenement(p, j, q); break;
            case GenreQuestion::Dragonnoie: rep = Index(q, VeutAffronterDragonnoie(p, j) ? "combat" : "passer"); break;
            case GenreQuestion::Evade: rep = Index(q, r.NextDouble() < ChanceEvade ? "trouve" : "rate"); break;
            default: rep = q.ParDefaut; break;
        }
        if (rep < 0 || rep >= q.Options.Count() || !q.Actives[rep]) rep = PremiereActive(q, q.ParDefaut);
        return rep;
    }

    int IA::PremiereActive(const SQuestion& q, int prefere)
    {
        if (prefere >= 0 && prefere < q.Options.Count() && q.Actives[prefere]) return prefere;
        for (int i = 0; i < q.Options.Count(); i++)
            if (q.Actives[i]) return i;
        return 0;
    }

    int IA::Index(const SQuestion& q, const string& code)
    {
        for (int i = 0; i < q.Codes.Count(); i++)
            if (q.Codes[i] == code && q.Actives[i]) return i;
        return -1;
    }

    /// Adversaires qui arriveront sur ma case ou juste derrière en moins de « portee » pas.
    Liste<Ptr<Joueur>> IA::Poursuivants(const Partie& p, const Ptr<Joueur>& j, int portee)
    {
        Liste<Ptr<Joueur>> l;
        for (auto& o : p.Joueurs)
        {
            if (o == j) continue;
            int d = p.Plateau.DistanceEnAvant(o->Case, j->Case, portee);
            if (d >= 1 && d <= portee) l.Add(o);
        }
        return l;
    }

    int IA::Action(Partie& p, const Ptr<Joueur>& j, SQuestion& q)
    {
        actionsCeTour++;
        int de = Index(q, "de");
        if (actionsCeTour > 4) return de;

        // Se soigner.
        if (j->PV * 10 <= j->PVMax() * 4)
        {
            int i = Index(q, "objet:elixir");
            if (i < 0) i = Index(q, "objet:potion");
            if (i >= 0) return i;
        }
        auto meneur = MeneurAdverse(p, j);
        // Sorts contre le meneur.
        if (meneur != nullptr)
        {
            int m = Index(q, "objet:malediction");
            if (m >= 0 && meneur->Niveau >= std::max(3, j->Niveau)) return m;
            int f = Index(q, "objet:foudre");
            if (f >= 0)
            {
                for (auto& o : p.Joueurs)
                    if (o != j && o->PV <= std::max(1, 5 - o->Defense()) && o->Niveau >= j->Niveau - 1) return f;
                if (meneur->Niveau >= j->Niveau + 2 && r.NextDouble() < 0.5) return f;
            }
            int l = Index(q, "objet:lenteur");
            if (l >= 0 && meneur->Niveau >= j->Niveau + 1 && r.NextDouble() < 0.6) return l;
        }
        // Pièges si quelqu'un suit.
        auto suiveurs = Poursuivants(p, j, 8);
        if (suiveurs.Count() > 0)
            for (const char* code : {"objet:rune", "objet:loups", "objet:fosse", "objet:filet"})
            {
                int i = Index(q, code);
                if (i >= 0 && (string(code) != "objet:rune" || r.NextDouble() < 0.8)) return i;
            }
        // Embuscade : une proie arrive, je suis plus fort qu'elle (ou ma quête l'exige).
        int emb = Index(q, "embuscade");
        if (emb >= 0 && EmbuscadePourQuete(p, j)) return emb;
        if (emb >= 0)
        {
            for (auto& o : Poursuivants(p, j, 6))
            {
                if (o->ProtegeDuel) continue;
                double c = ChanceDuel(j->Attaque() + 2, o->Attaque());
                bool vaut = (o->Niveau > j->Niveau && o->Niveau >= 3) || (o == meneur && o->Niveau >= 4);
                if (vaut && c >= 0.7 && r.NextDouble() < 0.4) return emb;
            }
        }
        if (j->PV * 2 <= j->PVMax() && j->CompterSac(TypeObjet::Potion) == 0)
        {
            int rap = Index(q, "objet:rappel");
            if (rap >= 0 && p.Plateau[j->Case]->Type != TypeCase::Ville) return rap;
        }
        return de;
    }

    Ptr<Joueur> IA::MeneurAdverse(const Partie& p, const Ptr<Joueur>& j)
    {
        Ptr<Joueur> m;
        for (auto& o : p.Joueurs)
            if (o != j && (m == nullptr || o->Niveau > m->Niveau || (o->Niveau == m->Niveau && o->XP > m->XP))) m = o;
        return m;
    }

    int IA::Embranchement(Partie& p, const Ptr<Joueur>& j, SQuestion& q)
    {
        int principal = -1, raccourci = -1;
        for (int i = 0; i < q.Cases.Count(); i++)
        {
            if (q.Cases[i] < 0) continue;
            if (p.Plateau[q.Cases[i]]->Raccourci) raccourci = i;
            else principal = i;
        }
        if (raccourci < 0) return std::max(0, principal);
        auto z = p.Plateau[q.Cases[raccourci]]->Zone;
        double vie = j->PV / static_cast<double>(j->PVMax());
        int atk = j->Attaque() + BonusMonstre(*j);
        bool ok;
        if (z == Zone::Antre) ok = ChanceMonstre(atk, 12) >= 0.5 && vie >= 0.7;
        else if (z == Zone::Cryptes) ok = ChanceMonstre(atk, 6) >= 0.55 && vie >= 0.5;
        else ok = ChanceMonstre(atk, 4) >= 0.55 && vie >= 0.5;
        // Une quête vers un lieu du raccourci l'emporte.
        if (j->Quete != nullptr && j->Quete->CaseCible >= 0 && p.Plateau[j->Quete->CaseCible]->Raccourci &&
            p.Plateau.DistanceEnAvant(q.Cases[raccourci], j->Quete->CaseCible, 6) >= 0)
            ok = true;
        return ok ? raccourci : (principal >= 0 ? principal : 0);
    }

    double IA::ValeurCase(Partie& p, const Ptr<Joueur>& j, int c)
    {
        auto& k = p.Plateau[c];
        double v = 0;
        int atk = j->Attaque() + BonusMonstre(*j);
        double vie = j->PV / static_cast<double>(j->PVMax());
        switch (k->Type)
        {
            case TypeCase::Tresor: v = 3; break;
            case TypeCase::Sanctuaire: v = vie < 0.6 ? 4 : 0.5; break;
            case TypeCase::Ville: v = 2 + (vie < 0.6 ? 2 : 0) + (j->Or >= 40 ? 2 : 0); break;
            case TypeCase::Marchand: v = j->Or >= 30 ? 3 : 0; break;
            case TypeCase::Taverne: v = j->Quete == nullptr ? 2 : 0.5; break;
            case TypeCase::Evenement: v = 1; break;
            case TypeCase::Malediction: v = -3; break;
            case TypeCase::Monstre: v = ChanceMonstre(atk, ForceTypique(k->Zone)) >= 0.6 && vie > 0.4 ? 3 : -2; break;
            case TypeCase::Antre: v = ChanceMonstre(atk, 12) >= 0.6 ? 6 : -6; break;
            default: break;
        }
        if (j->Quete != nullptr && j->Quete->CaseCible == c) v += 6;
        v += BonusCaseQuete(p, j, c);
        if (c == p.CaseDragonnoie && p.Dragonnoie() != nullptr) v = VeutAffronterDragonnoie(p, j) ? 9 : -4 - j->Or / 10.0;   // s'y arrêter, c'est le combattre
        if (p.PiegeSur(c) != nullptr && p.PiegeSur(c)->Proprietaire == j->Id) v -= 0.5;
        for (auto& o : p.JoueursSur(c, j))
            if (!o->ProtegeDuel && ChanceDuel(j->Attaque(), o->Attaque()) >= 0.65) v += 2;
        return v;
    }

    int IA::ForceTypique(Zone z)
    {
        switch (z)
        {
            case Zone::Village: return 2;
            case Zone::Bois: return 3;
            case Zone::Marais: return 5;
            case Zone::Cryptes: return 6;
            case Zone::Pics: return 7;
            default: return 10;
        }
    }

    int IA::MeilleureCase(Partie& p, const Ptr<Joueur>& j, SQuestion& q)
    {
        int best = q.ParDefaut;
        double bv = -DBL_MAX;
        for (int i = 0; i < q.Options.Count(); i++)
        {
            if (q.Cases[i] < 0) continue;
            double v = ValeurCase(p, j, q.Cases[i]) + r.NextDouble() * 0.2;
            if (v > bv)
            {
                bv = v;
                best = i;
            }
        }
        return best;
    }

    int IA::Defi(Partie& p, const Ptr<Joueur>& j, SQuestion& q)
    {
        int pourQuete = DefiPourQuete(p, j, q);
        if (pourQuete >= 0) return pourQuete;
        int best = q.ParDefaut;
        double bv = 0;
        for (int i = 0; i < q.Options.Count(); i++)
        {
            if (q.Cibles[i] < 0) continue;
            auto o = p.Joueurs[q.Cibles[i]];
            double c = ChanceDuel(j->Attaque(), o->Attaque());
            if (c < 0.6) continue;
            double v = c + 0.15 * (o->Niveau - j->Niveau) + (o->Or >= 20 ? 0.1 : 0);
            if (o->Niveau <= 1 && o->XP == 0) v -= 0.3;
            if (v > bv)
            {
                bv = v;
                best = i;
            }
        }
        return best;
    }

    /// Affronter le Dragonnoie ? Le vaincre gagne la partie, perdre ne coûte que l'or : on tente sa chance d'autant plus
    /// volontiers que la bourse est légère, ou qu'un rival est à un niveau de la victoire.
    bool IA::VeutAffronterDragonnoie(const Partie& p, const Ptr<Joueur>& j)
    {
        auto m = p.Dragonnoie();
        if (m == nullptr) return false;
        double c = ChanceMonstre(j->Attaque() + BonusMonstre(*j), m->Force);
        double seuil = j->Or <= 3 ? 0.06 : (j->Or <= 20 ? 0.17 : 0.3);
        for (auto& o : p.Joueurs)
            if (o != j && o->Niveau >= p.NiveauCible - 1) seuil = std::min(seuil, 0.1);
        return c >= seuil;
    }

    int IA::Monstre(const Ptr<Joueur>& j, SQuestion& q)
    {
        auto m = q.Monstre;
        int fuir = Index(q, "fuir");
        if (fuir < 0 || m == nullptr) return Index(q, "combat");
        double c = ChanceMonstre(j->Attaque() + BonusMonstre(*j), m->Force);
        int degats = std::max(1, m->Degats - j->Defense());
        bool survit = j->PV > degats;
        bool combat = m->Boss ? (c >= 0.55 && survit) : (c >= 0.45 || (survit && c >= 0.3));
        if (j->Quete != nullptr && j->Quete->MonstreCible == m->Id && survit && c >= 0.3) combat = true;
        return combat ? Index(q, "combat") : fuir;
    }

    int IA::Boutique(Partie& p, const Ptr<Joueur>& j, SQuestion& q)
    {
        (void)p;
        int partir = Index(q, "partir");
        int best = partir;
        double bv = 0;
        for (int i = 0; i < q.Options.Count(); i++)
        {
            if (!q.Actives[i] || q.Objets[i] == nullptr) continue;
            auto o = q.Objets[i];
            int prix = q.Prix[i];
            int reste = j->Or - prix;
            double v = 0;
            switch (o->Type)
            {
                case TypeObjet::Arme: v = 10 + 3 * (o->Bonus - (j->Arme != nullptr ? j->Arme->Bonus : 0)); break;
                case TypeObjet::Armure: v = 6 + 2 * (o->Bonus - (j->Armure != nullptr ? j->Armure->Bonus : 0)); break;
                case TypeObjet::Accessoire:
                    if (o->Effet == EffetObjet::Amulette) v = 5;
                    else if (o->Effet == EffetObjet::Bottes) v = 4;
                    else v = 4;
                    if (reste < 15) v -= 4;
                    break;
                case TypeObjet::Potion:
                    v = j->CompterSac(TypeObjet::Potion) < 2 ? (o->Effet == EffetObjet::Soin ? 5 : 3) : 0;
                    break;
                case TypeObjet::Piege:
                    v = j->CompterSac(TypeObjet::Piege) < (j->Classe == ClasseHeros::Voleur ? 3 : 1) ? 3 + (j->Classe == ClasseHeros::Voleur ? 2 : 0) : 0;
                    if (o->Effet == EffetObjet::PiegeRune) v += 1;
                    if (reste < 10) v -= 3;
                    break;
                case TypeObjet::Parchemin:
                    if (o->Effet == EffetObjet::SortRappel) v = 0.5;
                    else v = j->CompterSac(TypeObjet::Parchemin) < (j->Classe == ClasseHeros::Mage ? 3 : 1) ? 3 + (j->Classe == ClasseHeros::Mage ? 2 : 0) : 0;
                    if (o->Effet == EffetObjet::SortMalediction) v += 1.5;
                    if (reste < 10) v -= 3;
                    break;
            }
            // Une arme reste la priorité : on ne dilapide pas l'or si une meilleure arme est presque à portée.
            if (o->Type != TypeObjet::Arme && reste < 25 && ProchaineArme(*j) != nullptr && ProchaineArme(*j)->Prix <= j->Or + 20) v -= 2.5;
            if (v > bv)
            {
                bv = v;
                best = i;
            }
        }
        return best < 0 ? q.Options.Count() - 1 : best;
    }

    Ptr<DefObjet> IA::ProchaineArme(const Joueur& j)
    {
        Ptr<DefObjet> best;
        for (auto& o : Donnees::Objets)
            if (o->Type == TypeObjet::Arme && (j.Arme == nullptr || o->Bonus > j.Arme->Bonus) && (best == nullptr || o->Prix < best->Prix)) best = o;
        return best;
    }
}
