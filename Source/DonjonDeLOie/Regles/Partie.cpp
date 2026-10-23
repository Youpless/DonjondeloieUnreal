// Donjon de l'Oie — la partie : l'état et le déroulé d'un tour.
#include "Partie.h"

namespace DonjonDeLOie::Regles
{
    using Moteur::Texte::De;

    Partie::Partie(const Liste<Ptr<ReglageJoueur>>& reglages, int graine, int niveauCible, Ptr<Regles::Plateau> plateau)
        : plateauTenu(plateau != nullptr ? plateau : Regles::Plateau::Standard()), Plateau(*plateauTenu), Des(graine)
    {
        NiveauCible = std::max(2, niveauCible);
        // L'ordre de jeu est tiré au sort.
        auto ordre = reglages.ToArray();
        for (int i = ordre.Count() - 1; i > 0; i--)
        {
            int k = Des.Entre(0, i);
            auto t = ordre[i];
            ordre[i] = ordre[k];
            ordre[k] = t;
        }
        for (int i = 0; i < ordre.Count(); i++)
        {
            auto& r = ordre[i];
            auto j = Nouveau<Joueur>();
            j->Id = i;
            j->Nom = r->Nom;
            j->Classe = r->Classe;
            j->Humain = r->Humain;
            j->Couleur = r->Couleur;
            j->Or = Donnees::OrDepart();
            j->Case = Plateau.Depart;
            j->PV = j->PVMax();
            // (objets d'origine : absents si on les a retirés du catalogue)
            if (j->Classe == ClasseHeros::Mage && Donnees::Objet("foudre") != nullptr) j->Sac.Add(Donnees::Objet("foudre"));
            if (j->Classe == ClasseHeros::Voleur && Donnees::Objet("fosse") != nullptr) j->Sac.Add(Donnees::Objet("fosse"));
            if (Donnees::Objet("potion") != nullptr) j->Sac.Add(Donnees::Objet("potion"));
            Joueurs.Add(j);
        }
    }

    Ptr<Joueur> Partie::Meneur() const
    {
        Ptr<Joueur> m;
        for (auto& j : Joueurs)
            if (m == nullptr || j->Niveau > m->Niveau || (j->Niveau == m->Niveau && j->XP > m->XP)) m = j;
        return m;
    }

    Liste<Ptr<Joueur>> Partie::JoueursSur(int caseId, const Ptr<Joueur>& sauf) const
    {
        Liste<Ptr<Joueur>> l;
        for (auto& j : Joueurs)
            if (j->Case == caseId && j != sauf) l.Add(j);
        return l;
    }

    Ptr<Piege> Partie::PiegeSur(int caseId) const
    {
        for (int i = 0; i < Pieges.Count(); i++)
            if (Pieges[i]->Case == caseId) return Pieges[i];
        return nullptr;
    }

    Ptr<Joueur> Partie::EmbusqueSur(int caseId, const Ptr<Joueur>& sauf) const
    {
        for (auto& j : Joueurs)
            if (j != sauf && j->Embuscade != nullptr && j->Embuscade->Couvre(caseId)) return j;
        return nullptr;
    }

    Signaux Partie::Debut()
    {
        Liste<string> noms;
        for (auto& j : Joueurs) noms.Add(j->Nom);
        co_yield Nouveau<SAnnonce>("L'ordre de jeu est tiré : " + Moteur::Texte::Joindre(", ", noms) + ".", -1, Ton::Systeme);
        co_yield Nouveau<SAnnonce>("Premier au niveau " + De(NiveauCible) + (Donnees::Dragonnoie() != nullptr ? ", ou vainqueur du Dragonnoie" : "") +
                                       " : la victoire !",
                                   -1, Ton::Systeme, true);
        for (auto& s : ArriveeDragonnoie()) co_yield s;
    }

    // ================================================================== un tour

    Signaux Partie::JouerTour()
    {
        if (Fini) co_return;
        tourFini = false;
        // Après chaque cinquième tour de table (réglable), la nuit tombe, avec son événement.
        if (Courant == 0 && NuitAvantLeTour(Tour) && tourDerniereNuit != Tour)
        {
            for (auto& s : Nuit()) co_yield s;
            if (Fini) co_return;
        }
        // Au début d'un tour de table sur N, le Dragonnoie change de perchoir.
        if (Courant == 0 && Dragonnoie() != nullptr && Tour >= TourEnvol)
        {
            for (auto& s : EnvolDragonnoie()) co_yield s;
            if (Fini) co_return;
        }
        auto j = JoueurCourant();
        j->Embuscade = nullptr;  // une embuscade dure jusqu'au tour suivant de celui qui l'a tendue
        {
            auto td = Nouveau<STourDebut>();
            td->Joueur = j->Id;
            td->Tour = Tour;
            co_yield td;
        }

        for (auto& s : ChasseEvade(j)) co_yield s;      // en ville avec l'avis de recherche : la chasse commence
        if (Fini) co_return;

        if (j->Classe == ClasseHeros::Clerc && j->PV < j->PVMax())
            for (auto& s : Soigner(j, 2, "Foi")) co_yield s;

        if (j->ToursImmobilise > 0)
        {
            j->ToursImmobilise--;
            co_yield Nouveau<SAnnonce>(j->Nom + " est immobilisé et passe son tour.", j->Id, Ton::Mauvais);
        }
        else
        {
            for (auto& s : PhaseAction(j)) co_yield s;
        }
        if (Fini) co_return;
        FinDeTour(j);
    }

    void Partie::FinDeTour(const Ptr<Joueur>& j)
    {
        if (j->ToursMaudit > 0) j->ToursMaudit--;
        if (j->ProtegeDuel && !protectionNeeCeTour.Contains(j->Id)) j->ProtegeDuel = false;
        protectionNeeCeTour.Clear();
        TourJoueurs++;
        Courant = (Courant + 1) % Joueurs.Count();
        if (Courant == 0) Tour++;
    }

    Signaux Partie::PhaseAction(Ptr<Joueur> j)
    {
        while (true)
        {
            auto q = Nouveau<SQuestion>(j->Id, GenreQuestion::Action, "À vous, " + j->Nom, "Lancez le dé, tendez une embuscade ou utilisez un objet.");
            q->Ajouter("Lancer le dé", "de", "Avancer de 1 à 6 cases.");
            q->Ajouter("Tendre une embuscade", "embuscade",
                       "Restez sur place : votre case et une case voisine deviennent un piège vivant jusqu'à votre prochain tour (+2 en duel).");
            Ensemble<string> vus;
            for (int i = 0; i < j->Sac.Count(); i++)
            {
                auto o = j->Sac[i];
                if (vus.Contains(o->Id)) continue;
                vus.Add(o->Id);
                bool utile = ObjetUtilisable(j, o);
                q->Ajouter((o->Type == TypeObjet::Piege ? "Poser : " : "Utiliser : ") + o->Nom, "objet:" + o->Id, o->Description, utile, -1, -1, o);
            }
            co_yield q;
            string code = Moteur::Texte::Ou(q->CodeChoisi(), "de");
            if (code == "de")
            {
                for (auto& s : Avancer(j)) co_yield s;
                co_return;
            }
            if (code == "embuscade")
            {
                bool tendue = false;
                for (auto& s : TendreEmbuscade(j, [&tendue](bool r) { tendue = r; })) co_yield s;
                if (tendue) co_return;
                continue;
            }
            if (Moteur::Texte::CommencePar(code, "objet:"))
            {
                auto o = Donnees::Objet(Moteur::Texte::Sous(code, 6));
                for (auto& s : UtiliserObjet(j, o)) co_yield s;
                if (Arret()) co_return;
            }
        }
    }

    Signaux Partie::TendreEmbuscade(Ptr<Joueur> j, std::function<void(bool)> resultat)
    {
        auto q = Nouveau<SQuestion>(j->Id, GenreQuestion::ChoixCase, "Embuscade", "Choisissez la case voisine que vous couvrez en plus de la vôtre.");
        q->Contexte = "embuscade";
        for (int v : Plateau.Voisins(j->Case))
            q->Ajouter(Plateau[v]->Libelle() + (Plateau[j->Case]->Suivants.Contains(v) ? " (devant)" : " (derrière)"), "case", "", true, v);
        q->Ajouter("Annuler", "annuler");
        co_yield q;
        if (q->CodeChoisi() != "case")
        {
            resultat(false);
            co_return;
        }
        int c = q->Cases[q->Reponse];
        j->Embuscade = Nouveau<Regles::Embuscade>();
        j->Embuscade->Case = j->Case;
        j->Embuscade->CaseVoisine = c;
        resultat(true);
        {
            auto e = Nouveau<SEffet>(Effet::EmbuscadeTendue);
            e->Joueur = j->Id;
            e->Case = c;
            co_yield e;
        }
        co_yield Nouveau<SAnnonce>(j->Nom + " se tapit en embuscade.", j->Id, Ton::Combat);
    }

    // ================================================================== déplacement

    Signaux Partie::Avancer(Ptr<Joueur> j)
    {
        int de = Des.D6();
        int bonus = 0;
        for (auto& acc : j->Accessoires)
            if (acc->Effet == EffetObjet::Bottes) bonus += std::max(1, acc->Bonus);
        {
            auto lancer = Nouveau<SDe>();
            lancer->Joueur = j->Id;
            lancer->Faces = 6;
            lancer->Valeur = de;
            lancer->Bonus = bonus;
            lancer->Raison = "Déplacement";
            co_yield lancer;
        }
        int pas = de + bonus;
        if (j->Lenteur)
        {
            j->Lenteur = false;
            pas = std::max(1, (pas + 1) / 2);
            co_yield Nouveau<SAnnonce>("Ralenti par un sort, " + j->Nom + " n'avance que de " + De(pas) + ".", j->Id, Ton::Mauvais);
        }
        if (j->Classe == ClasseHeros::Rodeur)
        {
            auto q = Nouveau<SQuestion>(j->Id, GenreQuestion::Rodeur, "Pisteur", "Combien de cases parcourir ?");
            if (pas > 1) q->Ajouter(De(pas - 1) + " cases", "-1", "", true, CaseApres(j->Case, pas - 1));
            q->Ajouter(De(pas) + " cases", "0", "", true, CaseApres(j->Case, pas));
            q->Ajouter(De(pas + 1) + " cases", "+1", "", true, CaseApres(j->Case, pas + 1));
            q->ParDefaut = pas > 1 ? 1 : 0;
            co_yield q;
            if (q->CodeChoisi() == "-1") pas--;
            else if (q->CodeChoisi() == "+1") pas++;
        }
        for (auto& s : Marcher(j, pas, true)) co_yield s;
    }

    int Partie::CaseApres(int depart, int pas) const
    {
        int c = depart;
        for (int i = 0; i < pas; i++) c = Plateau[c]->Suivants[0];
        return c;
    }

    Signaux Partie::Marcher(Ptr<Joueur> j, int pas, bool effetArrivee)
    {
        bool arrete = false;
        while (pas > 0 && !Arret())
        {
            auto c = Plateau[j->Case];
            int suivante = c->Suivants[0];
            if (c->Suivants.Count() > 1)
            {
                auto q = Nouveau<SQuestion>(j->Id, GenreQuestion::Embranchement, "Embranchement",
                                            "Il vous reste " + De(pas) + " case" + (pas > 1 ? "s" : "") + ". Quel chemin prendre ?");
                for (int s : c->Suivants) q->Ajouter(Moteur::Texte::Ou(Plateau[s]->Voie, Plateau[s]->Libelle()), "voie", AideVoie(s), true, s);
                co_yield q;
                suivante = q->Reponse >= 0 ? q->Cases[q->Reponse] : c->Suivants[0];
            }
            int de = j->Case;
            j->Case = suivante;
            pas--;
            co_yield Nouveau<SPas>(j->Id, de, suivante);

            if (suivante == Plateau.Depart)
            {
                for (auto& s : GagnerOr(j, Donnees::SoldeVille(), "solde de " + Plateau[Plateau.Depart]->Libelle())) co_yield s;
                for (auto& s : GagnerXP(j, Donnees::XPVille())) co_yield s;
                if (Arret()) co_return;          // le niveau visé atteint en passant par la ville : la partie est finie
            }
            for (auto& s : VerifierQueteLieu(j)) co_yield s;
            for (auto& s : ChasseEvade(j)) co_yield s;
            if (Arret()) co_return;

            // Le Dragonnoie : en passant, on choisit ; en s'arrêtant sur sa case, on l'affronte.
            if (suivante == CaseDragonnoie && Dragonnoie() != nullptr)
            {
                bool attaquer = pas == 0;
                if (attaquer) co_yield Nouveau<SAnnonce>(j->Nom + " s'arrête devant le Dragonnoie : pas moyen de l'éviter !", j->Id, Ton::Combat, true);
                else
                    for (auto& s : PasserDragonnoie(j, pas, [&attaquer](bool r) { attaquer = r; })) co_yield s;
                if (attaquer && Dragonnoie() != nullptr)     // (il a pu être retiré par le catalogue pendant la question)
                {
                    for (auto& s : CombatDragonnoie(j)) co_yield s;
                    arrete = true;
                    break;
                }
            }

            auto embusque = EmbusqueSur(suivante, j);
            if (embusque != nullptr)
            {
                embusque->Embuscade = nullptr;
                {
                    auto e = Nouveau<SEffet>(Effet::EmbuscadeDeclenchee);
                    e->Joueur = embusque->Id;
                    e->Cible = j->Id;
                    e->Case = suivante;
                    co_yield e;
                }
                co_yield Nouveau<SAnnonce>("Embuscade ! " + embusque->Nom + " surgit devant " + j->Nom + ".", embusque->Id, Ton::Combat, true);
                for (auto& s : Duel(embusque, j, true)) co_yield s;
                for (auto& s : ProgresQuete(embusque, Quete::Genre::Embuscade)) co_yield s;
                for (auto& s : ArretDragonnoie(j, nullptr)) co_yield s;     // arrêté sur la case du Dragonnoie : il faut l'affronter
                arrete = true;
                break;
            }
            auto piege = PiegeSur(suivante);
            if (piege != nullptr && piege->Proprietaire != j->Id)
            {
                if (j->Classe == ClasseHeros::Voleur)
                {
                    if (pas == 0) co_yield Nouveau<SAnnonce>(j->Nom + " enjambe un piège sans le déclencher.", j->Id, Ton::Bon);
                }
                else
                {
                    for (auto& s : DeclencherPiege(j, piege)) co_yield s;
                    for (auto& s : ArretDragonnoie(j, nullptr)) co_yield s;
                    arrete = true;
                    break;
                }
            }
        }
        if (Arret() || arrete || !effetArrivee) co_return;
        for (auto& s : Arrivee(j)) co_yield s;
    }

    string Partie::AideVoie(int premiere) const
    {
        auto& c = Plateau[premiere];
        if (!c->Raccourci) return "La route principale, plus longue mais plus sûre.";
        switch (c->Zone)
        {
            case Zone::Marais: return "Raccourci à travers le marais : monstres et trésor.";
            case Zone::Cryptes: return "Raccourci sous terre : trésor, monstres et malédiction.";
            case Zone::Antre: return "Le chemin le plus court… par l'antre d'Ignathor, le dragon.";
            default: break;
        }
        return "Un raccourci.";
    }

    namespace
    {
        /// Remet un compteur à sa valeur en sortant d'une fonction, même abandonnée en route (le « finally » de C#).
        struct Decompte
        {
            int& n;
            explicit Decompte(int& c) : n(c) { n++; }
            ~Decompte() { n--; }
            Decompte(const Decompte&) = delete;
            Decompte& operator=(const Decompte&) = delete;
        };
    }

    Signaux Partie::Arrivee(Ptr<Joueur> j)
    {
        if (profondeurEffets > 1) co_return;
        Decompte garde(profondeurEffets);
        // D'abord le défi aux joueurs présents sur la case.
        Liste<Ptr<Joueur>> presents;
        for (auto& o : JoueursSur(j->Case, j))
            if (!o->ProtegeDuel) presents.Add(o);
        if (presents.Count() > 0 && Plateau[j->Case]->Type != TypeCase::Ville)
        {
            auto q = Nouveau<SQuestion>(j->Id, GenreQuestion::Defi, "Duel ?",
                                        "Vous croisez d'autres aventuriers. Lancer un défi ? Chacun lance son attaque + 1D3 ; le perdant perd un niveau.");
            for (auto& o : presents) q->Ajouter("Défier " + o->Nom, "defi", "Niveau " + De(o->Niveau) + " · attaque " + De(o->Attaque()), true, -1, o->Id);
            q->Ajouter("Passer son chemin", "non");
            q->ParDefaut = q->Options.Count() - 1;
            co_yield q;
            if (q->CodeChoisi() == "defi")
            {
                auto cible = Joueurs[q->Cibles[q->Reponse]];
                for (auto& s : Duel(j, cible, false)) co_yield s;
                for (auto& s : ProgresQuete(j, Quete::Genre::Defier, cible->Id)) co_yield s;
                if (Arret()) co_return;
            }
        }
        for (auto& s : EffetCase(j)) co_yield s;
    }
}
