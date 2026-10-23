// Donjon de l'Oie — les quêtes : une seule à la fois. Atteindre un lieu, chasser un monstre précis, vaincre des
// ennemis, tendre une embuscade qui prend un joueur, défier un joueur précis, retrouver un évadé caché dans la ville
// (un mini-jeu : la question « Evade », à laquelle répond l'interface après la partie de cache-cache, ou un bot).
#include "Partie.h"

namespace DonjonDeLOie::Regles
{
    using Moteur::Texte::De;

    namespace
    {
        const char* const NomsEvades[] = {
            "Gaspard le Borgne", "Babette la Fouine", "Mortimer Deux-Doigts", "Lucien Main-Leste", "Ernestine la Rousse",
            "Oscar Sans-Sou", "Firmin la Belette", "Hortense Pied-Léger", "Anatole le Balafré", "Clémence la Taupe",
        };
        const bool EvadeesFeminins[] = {false, true, false, false, true, false, false, true, false, true};
        const int NombreEvades = 10;
    }

    /// Quatre quêtes au plus : un lieu, un combat (une chasse ou des ennemis à vaincre), et deux parmi l'embuscade, le
    /// défi et l'évadé.
    Liste<Ptr<Quete>> Partie::GenererQuetes(const Ptr<Joueur>& j)
    {
        Liste<Ptr<Quete>> l;
        auto lieu = QueteLieu(j);
        if (lieu != nullptr) l.Add(lieu);
        auto combat = Des.Reel() < 0.5 ? QueteChasse(j) : nullptr;
        l.Add(combat != nullptr ? combat : QueteVaincre(j));
        Liste<Ptr<Quete>> autres = {QueteEmbuscade()};
        auto defi = QueteDefi(j);
        if (defi != nullptr) autres.Add(defi);
        auto evade = QueteEvade(j);
        if (evade != nullptr) autres.Add(evade);
        for (int k = 0; k < 2 && autres.Count() > 0; k++)
        {
            auto q = Des.Choisir(autres);
            autres.Remove(q);
            l.Add(q);
        }
        return l;
    }

    /// Un lieu nommé à une distance raisonnable devant soi.
    Ptr<Quete> Partie::QueteLieu(const Ptr<Joueur>& j)
    {
        Liste<int> lieux;
        for (int i = 0; i < Plateau.Nombre(); i++)
        {
            if (Plateau[i]->Nom.empty() || i == j->Case) continue;
            int d = Plateau.DistanceEnAvant(j->Case, i);
            if (d >= 5 && d <= 24) lieux.Add(i);
        }
        if (lieux.Count() == 0) return nullptr;
        int c = Des.Choisir(lieux);
        int dist = Plateau.DistanceEnAvant(j->Case, c);
        auto q = Nouveau<Quete>();
        q->Type = Quete::Genre::Atteindre;
        q->CaseCible = c;
        q->RecompenseXP = 2 + dist / 8;
        q->RecompenseOr = 5 + dist;
        q->Titre = "Message pour " + Plateau[c]->Nom;
        q->Texte = "Portez un pli scellé jusqu'à « " + Plateau[c]->Nom + " » (passer dessus suffit).";
        return q;
    }

    /// Une chasse à la mesure du héros (nullptr s'il n'y a pas de proie).
    Ptr<Quete> Partie::QueteChasse(const Ptr<Joueur>& j)
    {
        Liste<Ptr<DefMonstre>> proies;
        for (auto& m : Donnees::Monstres)
            if (Chassable(m) && m->Force <= j->Attaque() + 5 && m->Force >= j->Attaque() - 3) proies.Add(m);
        if (proies.Count() == 0)
            for (auto& m : Donnees::Monstres)
                if (Chassable(m))
                {
                    proies.Add(m);
                    break;
                }
        if (proies.Count() == 0) return nullptr;
        auto proie = Des.Choisir(proies);
        int n = proie->Force <= 4 ? 2 : 1;
        string ou = Donnees::NomsZones[static_cast<int>(proie->Zones[0])];
        auto q = Nouveau<Quete>();
        q->Type = Quete::Genre::Chasser;
        q->MonstreCible = proie->Id;
        q->Nombre = n;
        q->RecompenseXP = 2 + (proie->Force * n) / 4;
        q->RecompenseOr = proie->Force * 2 * n;
        q->Titre = "Chasse : " + (n > 1 ? De(n) + " × " : string()) + proie->Nom;
        q->Texte = "Abattez " + (n > 1 ? De(n) + " " + Moteur::Texte::Minuscules(proie->Nom) + "s" : "un " + Moteur::Texte::Minuscules(proie->Nom)) +
                   ". On en voit vers " + ou + ".";
        return q;
    }

    /// Un monstre qu'une quête de chasse peut viser : ni le boss, ni le Dragonnoie, et il vit quelque part.
    bool Partie::Chassable(const Ptr<DefMonstre>& m) { return !m->Boss && !m->Errant && m->Zones.Count() > 0; }

    Ptr<Quete> Partie::QueteVaincre(const Ptr<Joueur>& j)
    {
        int n = j->Niveau >= 5 ? 3 : 2;
        auto q = Nouveau<Quete>();
        q->Type = Quete::Genre::Vaincre;
        q->Nombre = n;
        q->RecompenseXP = 2 + n;
        q->RecompenseOr = 6 * n;
        q->Titre = "Vaincre " + De(n) + " ennemis";
        q->Texte = "Terrassez " + De(n) + " monstres, n'importe lesquels, où que ce soit.";
        return q;
    }

    Ptr<Quete> Partie::QueteEmbuscade()
    {
        auto q = Nouveau<Quete>();
        q->Type = Quete::Genre::Embuscade;
        q->RecompenseXP = 3;
        q->RecompenseOr = 15;
        q->Titre = "Le guet-apens";
        q->Texte = "Tendez une embuscade et surprenez un aventurier qui s'y engage. Gagner le duel n'est pas exigé.";
        return q;
    }

    /// Défier un autre aventurier précis (nullptr s'il est seul).
    Ptr<Quete> Partie::QueteDefi(const Ptr<Joueur>& j)
    {
        auto autres = Joueurs.FindAll([&j](const Ptr<Joueur>& o) { return o != j; });
        if (autres.Count() == 0) return nullptr;
        auto cible = Des.Choisir(autres);
        auto q = Nouveau<Quete>();
        q->Type = Quete::Genre::Defier;
        q->JoueurCible = cible->Id;
        q->RecompenseXP = 4;
        q->RecompenseOr = 10 + 2 * cible->Niveau;
        q->Titre = "Défier " + cible->Nom;
        q->Texte = "Provoquez " + cible->Nom + " en duel (rejoignez sa case). Gagner ou perdre, l'honneur est sauf.";
        return q;
    }

    /// Retrouver un évadé caché au marché de la ville (nullptr si la carte n'a pas de ville).
    Ptr<Quete> Partie::QueteEvade(const Ptr<Joueur>& j)
    {
        int ville = VillePourEvade(j);
        if (ville < 0) return nullptr;
        int k = Des.Entre(0, NombreEvades - 1);
        string nom = NomsEvades[k];
        bool f = EvadeesFeminins[k];
        auto q = Nouveau<Quete>();
        q->Type = Quete::Genre::Evade;
        q->CaseCible = ville;
        q->Ville = Plateau[ville]->Libelle();
        q->Graine = Des.Entre(1, 1000000);
        q->NomEvade = nom;
        q->Feminin = f;
        q->RecompenseXP = 4;
        q->RecompenseOr = 25;
        q->Titre = "Avis de recherche : " + nom;
        q->Texte = nom + (f ? " s'est évadée" : " s'est évadé") + " et se cache au marché de " + Plateau[ville]->Libelle() + ". Allez-y : vous aurez " +
                   De(Donnees::Reglages->DureeEvade) + " secondes pour " + (f ? "la" : "le") + " repérer dans la foule.";
        return q;
    }

    /// La ville la plus proche devant soi (celle où l'on se trouve compte).
    int Partie::VillePourEvade(const Ptr<Joueur>& j) const
    {
        int best = -1, bd = 2147483647;
        for (int i = 0; i < Plateau.Nombre(); i++)
        {
            if (Plateau[i]->Type != TypeCase::Ville) continue;
            int d = i == j->Case ? 0 : Plateau.DistanceEnAvant(j->Case, i);
            if (d < 0) d = 1000 + i;
            if (d < bd)
            {
                bd = d;
                best = i;
            }
        }
        return best;
    }

    Signaux Partie::ProposerQuetes(Ptr<Joueur> j, string lieu)
    {
        auto offre = GenererQuetes(j);
        auto q = Nouveau<SQuestion>(j->Id, GenreQuestion::Quetes, lieu,
                                    j->Quete == nullptr ? string("Choisissez une quête (une seule à la fois).")
                                                        : "Vous avez déjà : « " + j->Quete->Titre + " ». En prendre une autre l'abandonne.");
        for (auto& qu : offre)
            q->Ajouter(qu->Titre, "quete", qu->Texte + "\nRécompense : " + De(qu->RecompenseXP) + " XP, " + De(qu->RecompenseOr) + " or", true, qu->CaseCible, -1,
                       nullptr, 0, qu);
        q->Ajouter(j->Quete == nullptr ? "Aucune" : "Garder ma quête", "non");
        q->ParDefaut = q->Options.Count() - 1;
        co_yield q;
        if (q->CodeChoisi() != "quete") co_return;
        j->Quete = q->Quetes[q->Reponse];
        {
            auto e = Nouveau<SEffet>(Effet::QueteAcceptee);
            e->Joueur = j->Id;
            e->Case = j->Quete->CaseCible;
            co_yield e;
        }
        co_yield Nouveau<SAnnonce>(j->Nom + " accepte : " + j->Quete->Titre + ".", j->Id, Ton::Quete);
        for (auto& s : ChasseEvade(j)) co_yield s;          // déjà en ville : la chasse commence
    }

    Signaux Partie::VerifierQueteLieu(Ptr<Joueur> j)
    {
        if (j->Quete == nullptr || j->Quete->Type != Quete::Genre::Atteindre || j->Quete->CaseCible != j->Case) co_return;
        j->Quete->Fait = 1;
        for (auto& s : FinirQuete(j)) co_yield s;
    }

    /// Un pas de plus vers une quête du genre donné (pour un défi : seulement contre son adversaire désigné).
    Signaux Partie::ProgresQuete(Ptr<Joueur> j, Quete::Genre genre, int cible)
    {
        auto q = j->Quete;
        if (q == nullptr || q->Type != genre || Fini) co_return;
        if (genre == Quete::Genre::Defier && q->JoueurCible != cible) co_return;
        q->Fait++;
        if (q->Fait >= q->Nombre)
            for (auto& s : FinirQuete(j)) co_yield s;
    }

    /// La chasse à l'évadé : elle se joue dès que le héros est dans la ville de l'avis avec cette quête (en y passant,
    /// en s'y arrêtant, en y arrivant par un parchemin de rappel, au début de son tour s'il s'y trouve, ou aussitôt la
    /// quête acceptée sur place). Une autre ville ne compte pas : l'évadé n'y est pas.
    Signaux Partie::ChasseEvade(Ptr<Joueur> j)
    {
        auto qu = j->Quete;
        if (qu == nullptr || qu->Type != Quete::Genre::Evade || Fini || j->Case != qu->CaseCible) co_return;
        co_yield Nouveau<SAnnonce>(j->Nom + " fouille le marché de " + Plateau[j->Case]->Libelle() + " à la recherche de " + qu->NomEvade + "…", j->Id, Ton::Quete);
        auto q = Nouveau<SQuestion>(j->Id, GenreQuestion::Evade, "Avis de recherche : " + qu->NomEvade,
                                    "Retrouvez " + qu->NomEvade + " dans la foule du marché : " + De(Donnees::Reglages->DureeEvade) + " secondes pour cliquer sur " +
                                        (qu->Feminin ? "elle." : "lui."));
        q->Quete = qu;
        q->Ajouter(qu->Feminin ? "Je l'ai trouvée" : "Je l'ai trouvé", "trouve");
        q->Ajouter(qu->Feminin ? "Elle m'a échappé" : "Il m'a échappé", "rate");
        q->ParDefaut = 1;
        co_yield q;
        if (j->Quete != qu) co_return;
        if (q->CodeChoisi() == "trouve")
        {
            qu->Fait = 1;
            co_yield Nouveau<SAnnonce>(j->Nom + " démasque " + qu->NomEvade + " dans la foule !", j->Id, Ton::Quete, true);
            for (auto& s : FinirQuete(j)) co_yield s;
        }
        else
        {
            j->Quete = nullptr;
            {
                auto e = Nouveau<SEffet>(Effet::QueteRatee);
                e->Joueur = j->Id;
                e->Texte = qu->Titre;
                co_yield e;
            }
            co_yield Nouveau<SAnnonce>(qu->NomEvade + " se fond dans la foule et disparaît : la quête est perdue.", j->Id, Ton::Mauvais, true);
        }
    }

    Signaux Partie::FinirQuete(Ptr<Joueur> j)
    {
        auto q = j->Quete;
        j->Quete = nullptr;
        j->QuetesFinies++;
        {
            auto e = Nouveau<SEffet>(Effet::QueteFinie);
            e->Joueur = j->Id;
            e->Texte = q->Titre;
            co_yield e;
        }
        co_yield Nouveau<SAnnonce>("Quête accomplie : " + q->Titre + " !", j->Id, Ton::Quete, true);
        for (auto& s : GagnerOr(j, q->RecompenseOr, "")) co_yield s;
        for (auto& s : GagnerXP(j, q->RecompenseXP)) co_yield s;
    }
}
