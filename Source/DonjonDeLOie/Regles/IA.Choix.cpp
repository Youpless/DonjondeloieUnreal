// Donjon de l'Oie — les bots : où poser une embuscade ou un piège, quelle option prendre devant un événement, qui viser.
#include "IA.h"

#include <cfloat>

namespace DonjonDeLOie::Regles
{
    int IA::ChoixCase(Partie& p, const Ptr<Joueur>& j, SQuestion& q)
    {
        if (q.Contexte == "embuscade")
        {
            // Couvrir la case par où arrive la proie.
            for (int i = 0; i < q.Cases.Count(); i++)
                if (q.Cases[i] >= 0 && p.Plateau[j->Case]->Precedents.Contains(q.Cases[i])) return i;
        }
        else if (q.Contexte == "piege")
        {
            // Devant soi, là où les suiveurs passeront ; sinon sur sa case.
            int surMoi = -1;
            for (int i = 0; i < q.Cases.Count(); i++)
            {
                if (q.Cases[i] < 0) continue;
                if (q.Cases[i] == j->Case) surMoi = i;
                else if (p.Plateau[j->Case]->Suivants.Contains(q.Cases[i])) return i;
            }
            if (surMoi >= 0) return surMoi;
        }
        else if (q.Contexte == "rappel")
        {
            for (int i = 0; i < q.Cases.Count(); i++)
                if (q.Cases[i] == p.Plateau.Depart) return i;
        }
        for (int i = 0; i < q.Cases.Count(); i++)
            if (q.Cases[i] >= 0) return i;
        return q.Options.Count() - 1;
    }

    /// Un choix d'événement : la somme de ce que vaut chaque effet pour ce joueur, pondérée par sa chance.
    int IA::ChoixEvenement(Partie& p, const Ptr<Joueur>& j, SQuestion& q)
    {
        int best = -1;
        double bv = -DBL_MAX;
        for (int i = 0; i < q.Options.Count(); i++)
        {
            if (!q.Actives[i]) continue;
            double v = r.NextDouble() * 0.3;
            if (i < q.EffetsChoix.Count())
                for (auto& e : q.EffetsChoix[i]) v += ValeurEffet(p, j, *e) * std::max(0, std::min(100, e->Chance)) / 100.0;
            if (v > bv)
            {
                bv = v;
                best = i;
            }
        }
        return best;
    }

    double IA::ValeurEffet(const Partie& p, const Ptr<Joueur>& j, const EffetEvenement& e)
    {
        double pourMoi;
        double vie = j->PV / static_cast<double>(std::max(1, j->PVMax()));
        switch (e.Type)
        {
            case TypeEffet::Or: pourMoi = e.Valeur / 8.0; break;
            case TypeEffet::PV: pourMoi = e.Valeur * (e.Valeur > 0 ? (1.2 - vie) : (j->PV + e.Valeur <= 0 ? 4 : 0.8)) * 0.6; break;
            case TypeEffet::SoinTotal: pourMoi = (j->PVMax() - j->PV) * 0.4; break;
            case TypeEffet::XP: pourMoi = e.Valeur * 2.0; break;
            case TypeEffet::Niveau: pourMoi = e.Valeur * 4.5; break;
            case TypeEffet::Avancer: pourMoi = 0.6; break;
            case TypeEffet::Reculer: pourMoi = -0.6 * std::max(1, e.Valeur) / 2; break;
            case TypeEffet::Combat:
            {
                auto m = Donnees::Monstre(e.Parametre);
                pourMoi = m == nullptr ? 0.5 : (ChanceMonstre(j->Attaque() + BonusMonstre(*j), m->Force) - 0.5) * 6;
                break;
            }
            case TypeEffet::ObjetHasard:
            case TypeEffet::ObjetDonne: pourMoi = 1.5; break;
            case TypeEffet::Faiblesse: pourMoi = -0.8 * std::max(1, e.Valeur); break;
            case TypeEffet::Immobiliser: pourMoi = -2.5 * std::max(1, e.Valeur); break;
            case TypeEffet::Lenteur: pourMoi = -1; break;
            case TypeEffet::Benediction: pourMoi = 0.7 * std::max(1, e.Valeur); break;
            case TypeEffet::JetMalediction: pourMoi = j->Classe == ClasseHeros::Clerc ? -1.5 : -3; break;
            case TypeEffet::LeverMalediction: pourMoi = (j->ToursMaudit > 0 || j->Lenteur ? 2 : 0.2); break;
            case TypeEffet::Boutique: pourMoi = j->Or >= 25 ? 1 : 0.1; break;
            case TypeEffet::Quetes: pourMoi = j->Quete == nullptr ? 1 : 0.1; break;
            default: pourMoi = 0; break;
        }
        switch (e.Cible)
        {
            case CibleEffet::Autres: return -pourMoi * 0.6;
            case CibleEffet::Tous: return pourMoi * 0.4;
            case CibleEffet::Meneur: return MeneurAdverse(p, j) == nullptr ? pourMoi : -pourMoi * 0.7;
            case CibleEffet::Dernier: return -pourMoi * 0.2;
            default: return pourMoi;
        }
    }

    int IA::ChoixCible(Partie& p, const Ptr<Joueur>& j, SQuestion& q)
    {
        (void)j;
        int best = -1;
        double bv = -DBL_MAX;
        bool foudre = q.Contexte == "sort:foudre";
        for (int i = 0; i < q.Options.Count(); i++)
        {
            if (q.Cibles[i] < 0) continue;
            auto o = p.Joueurs[q.Cibles[i]];
            double v = o->Niveau * 10 + o->XP;
            if (foudre && o->PV <= std::max(1, 5 - o->Defense())) v += 100;
            if (q.Contexte == "sort:malediction" && o->A(EffetObjet::Amulette)) v -= 15;
            if (v > bv)
            {
                bv = v;
                best = i;
            }
        }
        return best >= 0 ? best : q.Options.Count() - 1;
    }
}
