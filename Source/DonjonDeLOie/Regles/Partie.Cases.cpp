// Donjon de l'Oie — ce qui arrive sur chaque type de case : trésors, événements, malédictions, boutiques, quêtes.
#include "Partie.h"

#include <cmath>

namespace DonjonDeLOie::Regles
{
    using Moteur::Texte::De;

    Signaux Partie::EffetCase(Ptr<Joueur> j)
    {
        auto c = Plateau[j->Case];
        bool aSaListe = c->Evenements.Count() > 0;
        // Mode « Remplacer » : la case joue un de ses événements au lieu de l'effet de son type.
        if (c->Mode == ModeEvenements::Remplacer && aSaListe)
        {
            for (auto& s : JouerEvenement(j, TirerEvenement(c->Evenements, c->Zone))) co_yield s;
            co_return;
        }
        for (auto& s : EffetType(j, c)) co_yield s;
        // Mode « Ajouter » : un de ses événements après l'effet du type (si le joueur n'a pas été mis K.O. ou déplacé).
        if (c->Mode == ModeEvenements::Ajouter && aSaListe && !Arret() && j->Case == c->Id && c->Type != TypeCase::Evenement)
            for (auto& s : JouerEvenement(j, TirerEvenement(c->Evenements, c->Zone))) co_yield s;
    }

    Signaux Partie::EffetType(Ptr<Joueur> j, Ptr<Case> c)
    {
        switch (c->Type)
        {
            case TypeCase::Ville:
            {
                string ville = c->Libelle();
                for (auto& s : Soigner(j, j->PVMax(), "Soins de " + ville)) co_yield s;
                if (j->ToursMaudit > 0 || j->Lenteur)
                {
                    j->ToursMaudit = 0;
                    j->Lenteur = false;
                    co_yield Nouveau<SAnnonce>("Les prêtres de " + ville + " lèvent les malédictions de " + j->Nom + ".", j->Id, Ton::Bon);
                }
                for (auto& s : ProposerQuetes(j, "Le panneau des quêtes de " + ville)) co_yield s;
                if (Arret()) break;               // l'évadé retrouvé sur-le-champ a pu donner le niveau de la victoire
                for (auto& s : Boutique(j, OffreVille(), "Bazar de " + ville, 1.0f)) co_yield s;
                break;
            }
            case TypeCase::Chemin:
                co_yield Nouveau<SAnnonce>(j->Nom + " fait halte : " + c->Libelle() + ".", j->Id);
                break;
            case TypeCase::Monstre:
                for (auto& s : CombatMonstre(j, TirerMonstre(j, c->Zone), true)) co_yield s;
                break;
            case TypeCase::Antre:
            {
                auto boss = Donnees::Boss();
                if (boss == nullptr) break;
                co_yield Nouveau<SAnnonce>(boss->Nom + " s'éveille sur son tas d'or…", j->Id, Ton::Combat, true);
                for (auto& s : CombatMonstre(j, boss, true)) co_yield s;
                break;
            }
            case TypeCase::Tresor:
                for (auto& s : Tresor(j, c)) co_yield s;
                break;
            case TypeCase::Evenement:
            {
                // la liste propre à la case, sinon la pioche commune
                auto ev = c->Evenements.Count() > 0 ? TirerEvenement(c->Evenements, c->Zone) : TirerEvenementPioche(c->Zone);
                for (auto& s : JouerEvenement(j, ev)) co_yield s;
                break;
            }
            case TypeCase::Malediction:
                for (auto& s : Malediction(j)) co_yield s;
                break;
            case TypeCase::Sanctuaire:
                co_yield Nouveau<SAnnonce>(j->Nom + " se recueille : " + c->Libelle() + ".", j->Id, Ton::Bon);
                for (auto& s : Soigner(j, j->PVMax(), "Sanctuaire")) co_yield s;
                if (j->ToursMaudit > 0 || j->Lenteur)
                {
                    j->ToursMaudit = 0;
                    j->Lenteur = false;
                    co_yield Nouveau<SAnnonce>("Les malédictions de " + j->Nom + " sont levées.", j->Id, Ton::Bon);
                }
                break;
            case TypeCase::Marchand:
                for (auto& s : Boutique(j, OffreMarchand(c->Id), c->Libelle(), 1.0f)) co_yield s;
                break;
            case TypeCase::Taverne:
                for (auto& s : ProposerQuetes(j, c->Libelle())) co_yield s;
                if (Arret()) break;
                for (auto& s : Boutique(j, OffreTaverne(), c->Libelle() + " — le comptoir", 1.0f)) co_yield s;
                break;
        }
    }

    // ------------------------------------------------------------------ trésors

    Signaux Partie::Tresor(Ptr<Joueur> j, Ptr<Case> c)
    {
        int butin = 5 + 3 * Des.D6();
        if (c->Zone == Zone::Antre) butin += 25;
        else if (c->Raccourci) butin += 10;
        if (j->Classe == ClasseHeros::Voleur) butin += 5;
        {
            auto e = Nouveau<SEffet>(Effet::Tresor);
            e->Joueur = j->Id;
            e->Case = c->Id;
            co_yield e;
        }
        for (auto& s : GagnerOr(j, butin, "trésor")) co_yield s;
        if (Des.Reel() < (c->Raccourci ? 0.5 : 0.25) && j->Sac.Count() < Donnees::TailleSac())
        {
            Liste<Ptr<DefObjet>> pool;
            for (auto& o : Donnees::Objets)
                if (o->Consommable()) pool.Add(o);
            auto trouve = Des.Choisir(pool);
            j->Sac.Add(trouve);
            {
                auto e = Nouveau<SEffet>(Effet::Achat);
                e->Joueur = j->Id;
                e->Objet = trouve->Id;
                co_yield e;
            }
            co_yield Nouveau<SAnnonce>("Au fond du coffre : " + trouve->Nom + " !", j->Id, Ton::Bon);
        }
    }

    // ------------------------------------------------------------------ malédictions

    Signaux Partie::Malediction(Ptr<Joueur> j)
    {
        co_yield Nouveau<SAnnonce>("Une ombre maudite fond sur " + j->Nom + "…", j->Id, Ton::Mauvais, true);
        bool bloque = false;
        for (auto& s : Amulette(j, [&bloque](bool r) { bloque = r; })) co_yield s;
        if (bloque) co_return;
        int bonus = j->Classe == ClasseHeros::Clerc ? 2 : 0;
        int de = Des.D6();
        {
            auto d = Nouveau<SDe>();
            d->Joueur = j->Id;
            d->Faces = 6;
            d->Valeur = de;
            d->Bonus = bonus;
            d->Raison = "Résister à la malédiction";
            co_yield d;
        }
        {
            auto e = Nouveau<SEffet>(Effet::Malediction);
            e->Joueur = j->Id;
            co_yield e;
        }
        switch (std::min(6, de + bonus))
        {
            case 1:
                for (auto& s : PerdreNiveau(j, 1, "malédiction de l'oubli")) co_yield s;
                break;
            case 2:
                co_yield Nouveau<SAnnonce>("Malédiction d'avarice : l'or de " + j->Nom + " fond.", j->Id, Ton::Mauvais);
                for (auto& s : PerdreOr(j, 15, "")) co_yield s;
                break;
            case 3:
                j->ToursMaudit = std::max(j->ToursMaudit, 3);
                co_yield Nouveau<SAnnonce>("Malédiction de faiblesse : −2 en attaque pendant 3 tours.", j->Id, Ton::Mauvais);
                break;
            case 4:
                j->ToursImmobilise = std::max(j->ToursImmobilise, 1);
                co_yield Nouveau<SAnnonce>("Pétrification : " + j->Nom + " passera son prochain tour.", j->Id, Ton::Mauvais);
                break;
            case 5:
                co_yield Nouveau<SAnnonce>("Brûlure noire : −3 PV.", j->Id, Ton::Mauvais);
                for (auto& s : Blesser(j, 3, "brûlure noire")) co_yield s;
                break;
            default:
                co_yield Nouveau<SAnnonce>(j->Nom + " résiste à la malédiction !", j->Id, Ton::Bon);
                break;
        }
    }

    // ------------------------------------------------------------------ boutiques

    Liste<Ptr<DefObjet>> Partie::OffreVille() const { return Donnees::Objets.ToArray(); }

    Liste<Ptr<DefObjet>> Partie::OffreTaverne() const
    {
        Liste<Ptr<DefObjet>> l;
        for (const char* id : {"potion", "elixir", "lenteur", "foudre"})
            if (Donnees::Objet(id) != nullptr) l.Add(Donnees::Objet(id));     // (absent si on l'a retiré du catalogue)
        return l;
    }

    namespace
    {
        int ParTypeEtPrix(const Ptr<DefObjet>& x, const Ptr<DefObjet>& y)
        {
            return x->Type != y->Type ? Comparer(static_cast<int>(x->Type), static_cast<int>(y->Type)) : Comparer(x->Prix, y->Prix);
        }
    }

    /// Un marchand change d'étal tous les trois tours.
    Liste<Ptr<DefObjet>> Partie::OffreMarchand(int caseId) const
    {
        AleaNet r(caseId * 7919 + (Tour / 3) * 104729);
        Liste<Ptr<DefObjet>> pool;
        for (auto& o : Donnees::Objets)
            if (!o->SeulementEnVille) pool.Add(o);
        Liste<Ptr<DefObjet>> l;
        // Toujours au moins une arme et une armure.
        auto armes = pool.FindAll([](const Ptr<DefObjet>& o) { return o->Type == TypeObjet::Arme; });
        auto armures = pool.FindAll([](const Ptr<DefObjet>& o) { return o->Type == TypeObjet::Armure; });
        l.Add(armes[r.Next(armes.Count())]);
        l.Add(armures[r.Next(armures.Count())]);
        while (l.Count() < 7)
        {
            auto o = pool[r.Next(pool.Count())];
            if (!l.Contains(o)) l.Add(o);
        }
        l.Sort(ParTypeEtPrix);
        return l;
    }

    Liste<Ptr<DefObjet>> Partie::OffreColporteur()
    {
        Liste<Ptr<DefObjet>> pool;
        for (auto& o : Donnees::Objets)
            if (!o->SeulementEnVille) pool.Add(o);
        Liste<Ptr<DefObjet>> l;
        while (l.Count() < 5)
        {
            auto o = Des.Choisir(pool);
            if (!l.Contains(o)) l.Add(o);
        }
        l.Sort(ParTypeEtPrix);
        return l;
    }

    string Partie::RaisonRefus(const Ptr<Joueur>& j, const Ptr<DefObjet>& o, int prix) const
    {
        if (j->Or < prix) return "Pas assez d'or";
        switch (o->Type)
        {
            case TypeObjet::Arme:
                if (j->Arme != nullptr && j->Arme->Bonus >= o->Bonus) return "Votre arme est meilleure";
                break;
            case TypeObjet::Armure:
                if (j->Armure != nullptr && j->Armure->Bonus >= o->Bonus) return "Votre armure est meilleure";
                break;
            case TypeObjet::Accessoire:
                if (j->A(o->Effet)) return "Déjà porté";
                if (j->Accessoires.Count() >= Donnees::AccessoiresMax()) return "Deux accessoires au plus";
                break;
            default:
                if (j->Sac.Count() >= Donnees::TailleSac()) return "Sac plein (" + De(Donnees::TailleSac()) + " objets)";
                break;
        }
        return string();
    }

    int Partie::PrixEnBoutique(const Ptr<Joueur>& j, const Ptr<DefObjet>& o, float remise) const
    {
        float brut = static_cast<float>(j->PrixPour(o)) * remise;
        return std::max(1, static_cast<int>(std::nearbyint(static_cast<double>(brut))));
    }

    Signaux Partie::Boutique(Ptr<Joueur> j, Liste<Ptr<DefObjet>> offre, string nom, float remise)
    {
        while (true)
        {
            auto q = Nouveau<SQuestion>(j->Id, GenreQuestion::Boutique, nom, "Vous avez " + De(j->Or) + " or.");
            for (auto& o : offre)
            {
                int prix = PrixEnBoutique(j, o, remise);
                string refus = RaisonRefus(j, o, prix);
                q->Ajouter(o->Nom, "achat", refus.empty() ? o->Description : refus, refus.empty(), -1, -1, o, prix);
            }
            q->Ajouter("Partir", "partir");
            q->ParDefaut = q->Options.Count() - 1;
            co_yield q;
            if (q->CodeChoisi() != "achat") co_return;
            auto choisi = q->Objets[q->Reponse];
            int p = q->Prix[q->Reponse];
            if (!RaisonRefus(j, choisi, p).empty()) continue;
            j->Or -= p;
            co_yield Nouveau<SStat>(j->Id, Stat::Or, -p);
            switch (choisi->Type)
            {
                case TypeObjet::Arme: j->Arme = choisi; break;
                case TypeObjet::Armure: j->Armure = choisi; break;
                case TypeObjet::Accessoire:
                    j->Accessoires.Add(choisi);
                    if (choisi->Effet == EffetObjet::Anneau)
                    {
                        j->PV += choisi->Bonus;
                        co_yield Nouveau<SStat>(j->Id, Stat::PV, choisi->Bonus);
                    }
                    break;
                default: j->Sac.Add(choisi); break;
            }
            {
                auto e = Nouveau<SEffet>(Effet::Achat);
                e->Joueur = j->Id;
                e->Objet = choisi->Id;
                co_yield e;
            }
            co_yield Nouveau<SAnnonce>(j->Nom + " achète : " + choisi->Nom + " (" + De(p) + " or).", j->Id, Ton::Neutre);
        }
    }
}
