// Donjon de l'Oie — utiliser les objets du sac : potions, pièges, parchemins.
#include "Partie.h"

namespace DonjonDeLOie::Regles
{
    using Moteur::Texte::De;

    Liste<int> Partie::CasesPourPiege(const Ptr<Joueur>& j) const
    {
        Liste<int> l;
        Liste<int> candidates = {j->Case};
        for (int v : Plateau.Voisins(j->Case))
            if (!candidates.Contains(v)) candidates.Add(v);
        for (int c : candidates)
        {
            auto t = Plateau[c]->Type;
            if (t == TypeCase::Ville || t == TypeCase::Sanctuaire || t == TypeCase::Antre) continue;
            if (PiegeSur(c) != nullptr) continue;
            l.Add(c);
        }
        return l;
    }

    int Partie::PiegesPosesPar(const Ptr<Joueur>& j) const
    {
        int n = 0;
        for (auto& p : Pieges)
            if (p->Proprietaire == j->Id) n++;
        return n;
    }

    bool Partie::ObjetUtilisable(const Ptr<Joueur>& j, const Ptr<DefObjet>& o) const
    {
        switch (o->Type)
        {
            case TypeObjet::Potion: return j->PV < j->PVMax();
            case TypeObjet::Piege: return CasesPourPiege(j).Count() > 0 && PiegesPosesPar(j) < Donnees::PiegesMaxParJoueur();
            case TypeObjet::Parchemin: return o->Effet == EffetObjet::SortRappel || Joueurs.Count() > 1;
            default: return false;
        }
    }

    Signaux Partie::UtiliserObjet(Ptr<Joueur> j, Ptr<DefObjet> o)
    {
        if (o == nullptr || !j->Sac.Contains(o) || !ObjetUtilisable(j, o)) co_return;
        switch (o->Type)
        {
            case TypeObjet::Potion:
                j->Sac.Remove(o);
                co_yield Nouveau<SAnnonce>(j->Nom + " boit : " + o->Nom + ".", j->Id, Ton::Bon);
                for (auto& s : Soigner(j, o->Effet == EffetObjet::SoinTotal ? j->PVMax() : o->Bonus, o->Nom)) co_yield s;
                break;

            case TypeObjet::Piege:
            {
                auto q = Nouveau<SQuestion>(j->Id, GenreQuestion::ChoixCase, o->Nom, "Où poser le piège ? (votre case ou une case voisine — vous seul le verrez)");
                q->Contexte = "piege";
                for (int c : CasesPourPiege(j))
                    q->Ajouter(c == j->Case ? "Ma case : " + Plateau[c]->Libelle() : Plateau[c]->Libelle(), "case", "", true, c);
                q->Ajouter("Annuler", "annuler");
                co_yield q;
                if (q->CodeChoisi() != "case") co_return;
                int cible = q->Cases[q->Reponse];
                j->Sac.Remove(o);
                auto p = Nouveau<Piege>();
                p->Proprietaire = j->Id;
                p->Objet = o;
                p->Case = cible;
                Pieges.Add(p);
                {
                    auto e = Nouveau<SEffet>(Effet::PiegePose);
                    e->Joueur = j->Id;
                    e->Case = cible;
                    e->Objet = o->Id;
                    co_yield e;
                }
                co_yield Nouveau<SAnnonce>(j->Nom + " dissimule quelque chose dans les herbes…", j->Id, Ton::Neutre);
                break;
            }

            case TypeObjet::Parchemin:
                if (o->Effet == EffetObjet::SortRappel)
                {
                    auto q = Nouveau<SQuestion>(j->Id, GenreQuestion::ChoixCase, o->Nom, "Où voulez-vous apparaître ?");
                    q->Contexte = "rappel";
                    q->Ajouter(Plateau[Plateau.Depart]->Libelle(), "case", "La case de départ (soins, boutique et quêtes à votre prochain arrêt).", true, Plateau.Depart);
                    for (int c : Plateau.CasesDeType(TypeCase::Sanctuaire))
                        if (c != j->Case) q->Ajouter(Plateau[c]->Libelle(), "case", "", true, c);
                    q->Ajouter("Annuler", "annuler");
                    co_yield q;
                    if (q->CodeChoisi() != "case") co_return;
                    j->Sac.Remove(o);
                    j->Case = q->Cases[q->Reponse];
                    j->Embuscade = nullptr;
                    {
                        auto e = Nouveau<SEffet>(Effet::SortLance);
                        e->Joueur = j->Id;
                        e->Cible = j->Id;
                        e->Objet = o->Id;
                        co_yield e;
                    }
                    co_yield Nouveau<STeleport>(j->Id, j->Case, "Rappel");
                    for (auto& s : VerifierQueteLieu(j)) co_yield s;
                    for (auto& s : ArretDragonnoie(j, nullptr)) co_yield s;
                    if (!Arret())
                        for (auto& s : ChasseEvade(j)) co_yield s;     // rappelé dans la ville de l'avis
                    break;
                }
                else
                {
                    auto q = Nouveau<SQuestion>(j->Id, GenreQuestion::ChoixCible, o->Nom, o->Description + "\nSur qui ?");
                    q->Contexte = "sort:" + o->Id;
                    for (auto& cible : Joueurs)
                        if (cible != j)
                            q->Ajouter(cible->Nom + " — niveau " + De(cible->Niveau), "cible",
                                       cible->Def()->Nom + " · " + De(cible->PV) + "/" + De(cible->PVMax()) + " PV", true, cible->Case, cible->Id);
                    q->Ajouter("Annuler", "annuler");
                    co_yield q;
                    if (q->CodeChoisi() != "cible") co_return;
                    auto t = Joueurs[q->Cibles[q->Reponse]];
                    j->Sac.Remove(o);
                    for (auto& s : Sort(j, t, o)) co_yield s;
                }
                break;

            default: break;
        }
    }

    Signaux Partie::Sort(Ptr<Joueur> lanceur, Ptr<Joueur> cible, Ptr<DefObjet> o)
    {
        {
            auto e = Nouveau<SEffet>(Effet::SortLance);
            e->Joueur = lanceur->Id;
            e->Cible = cible->Id;
            e->Objet = o->Id;
            co_yield e;
        }
        co_yield Nouveau<SAnnonce>(lanceur->Nom + " lit un " + Moteur::Texte::Minuscules(o->Nom) + " contre " + cible->Nom + " !", lanceur->Id, Ton::Combat, true);
        bool bloque = false;
        for (auto& s : Amulette(cible, [&bloque](bool r) { bloque = r; })) co_yield s;
        if (bloque) co_return;
        switch (o->Effet)
        {
            case EffetObjet::SortMalediction:
            {
                // Résister : 5+ au dé ; 6 contre un mage ; le clerc résiste mieux.
                int seuil = 5;
                if (lanceur->Classe == ClasseHeros::Mage) seuil++;
                if (cible->Classe == ClasseHeros::Clerc) seuil--;
                int de = Des.D6();
                {
                    auto d = Nouveau<SDe>();
                    d->Joueur = cible->Id;
                    d->Faces = 6;
                    d->Valeur = de;
                    d->Raison = "Résister (" + De(seuil) + "+)";
                    co_yield d;
                }
                if (de >= seuil)
                {
                    co_yield Nouveau<SAnnonce>(cible->Nom + " résiste à la malédiction !", cible->Id, Ton::Bon);
                    co_return;
                }
                {
                    auto e = Nouveau<SEffet>(Effet::Malediction);
                    e->Joueur = cible->Id;
                    co_yield e;
                }
                for (auto& s : PerdreNiveau(cible, std::max(1, o->Bonus), "malédiction de " + lanceur->Nom)) co_yield s;
                break;
            }
            case EffetObjet::SortFoudre:
            {
                int degats = std::max(1, o->Bonus + (lanceur->Classe == ClasseHeros::Mage ? 1 : 0) - cible->Defense());
                co_yield Nouveau<SAnnonce>("La foudre frappe " + cible->Nom + " : −" + De(degats) + " PV.", cible->Id, Ton::Mauvais);
                for (auto& s : Blesser(cible, degats, "foudre de " + lanceur->Nom)) co_yield s;
                break;
            }
            case EffetObjet::SortLenteur:
                cible->Lenteur = true;
                co_yield Nouveau<SAnnonce>(cible->Nom + " est ralenti : son prochain déplacement sera divisé par deux.", cible->Id, Ton::Mauvais);
                break;
            default: break;
        }
    }
}
