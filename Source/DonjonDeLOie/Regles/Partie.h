// Donjon de l'Oie — la partie : l'état et le déroulé d'un tour. Le moteur de règles est une suite de fonctions qui
// produisent des signaux (co_yield) : la présentation les montre un à un, et répond aux questions avant de reprendre.
// Les définitions sont réparties en plusieurs fichiers, comme dans la version C# : Partie.cpp (le tour), Partie.Etat.cpp
// (PV, or, niveaux, dés), Partie.Combats.cpp, Partie.Cases.cpp, Partie.Objets.cpp, Partie.Evenements.cpp,
// Partie.Dragonnoie.cpp, Partie.Nuit.cpp, Partie.Quetes.cpp.
#pragma once

#include "Evenements.h"
#include "Joueur.h"
#include "Plateau.h"
#include "Signaux.h"

namespace DonjonDeLOie::Regles
{
    struct ReglageJoueur
    {
        string Nom;
        ClasseHeros Classe = ClasseHeros::Guerrier;
        bool Humain = false;
        int Couleur = 0;
        ReglageJoueur() = default;
        ReglageJoueur(string nom, ClasseHeros c, bool humain, int couleur) : Nom(std::move(nom)), Classe(c), Humain(humain), Couleur(couleur) {}
    };

    /// Les dés de la partie (le générateur de .NET : même graine, mêmes tirages).
    class Des
    {
    public:
        explicit Des(int graine) : r(graine) {}
        int D6() { return r.Next(1, 7); }
        int D3() { return r.Next(1, 4); }
        /// Entier entre a et b inclus.
        int Entre(int a, int b) { return r.Next(a, b + 1); }
        double Reel() { return r.NextDouble(); }
        template <class T>
        T Choisir(const Liste<T>& l) { return l[r.Next(l.Count())]; }

    private:
        AleaNet r;
    };

    enum class EvenementNuit { Voleur, TrouDeVer, BonneNuit, Arene };
    string NomEnum(EvenementNuit v);

    class Partie
    {
    public:
        Partie(const Liste<Ptr<ReglageJoueur>>& reglages, int graine, int niveauCible = 10, Ptr<Regles::Plateau> plateau = nullptr);

    private:
        Ptr<Regles::Plateau> plateauTenu;      // (garde le plateau en vie ; on s'en sert par « Plateau »)

    public:
        Regles::Plateau& Plateau;
        Liste<Ptr<Joueur>> Joueurs;
        Liste<Ptr<Piege>> Pieges;
        Regles::Des Des;
        int NiveauCible = 10;
        int Courant = 0;                 // index dans Joueurs
        int Tour = 1;                    // un tour = chaque joueur a joué
        int TourJoueurs = 0;             // nombre total de tours individuels joués
        bool Fini = false;
        int Gagnant = -1;

        Ptr<Joueur> JoueurCourant() const { return Joueurs[Courant]; }
        Ptr<Joueur> Meneur() const;
        Liste<Ptr<Joueur>> JoueursSur(int caseId, const Ptr<Joueur>& sauf = nullptr) const;
        Ptr<Piege> PiegeSur(int caseId) const;
        Ptr<Joueur> EmbusqueSur(int caseId, const Ptr<Joueur>& sauf) const;

        Signaux Debut();
        Signaux JouerTour();
        /// Case atteinte en suivant toujours la grande route (pour l'aperçu du Rôdeur).
        int CaseApres(int depart, int pas) const;
        void TeleporterSansEffet(const Ptr<Joueur>& j, int c) { j->Case = c; }

        // Partie.Cases
        string RaisonRefus(const Ptr<Joueur>& j, const Ptr<DefObjet>& o, int prix) const;
        int PrixEnBoutique(const Ptr<Joueur>& j, const Ptr<DefObjet>& o, float remise) const;

        // Partie.Objets
        Liste<int> CasesPourPiege(const Ptr<Joueur>& j) const;
        int PiegesPosesPar(const Ptr<Joueur>& j) const;
        bool ObjetUtilisable(const Ptr<Joueur>& j, const Ptr<DefObjet>& o) const;

        // Partie.Evenements
        Signaux JouerEvenement(Ptr<Joueur> j, Ptr<DefEvenement> ev);

        // Partie.Dragonnoie
        int CaseDragonnoie = -1;         // sa case ; -1 : pas de Dragonnoie dans cette partie
        int OrDragonnoie = 0;            // l'or arraché aux vaincus, entassé dans son nid
        int TourEnvol = 0;               // le tour de table au début duquel il s'envolera
        bool VaincuDragonnoie = false;   // la partie s'est gagnée en le terrassant
        /// Sa fiche (catalogue), ou nullptr s'il n'est pas sur la carte.
        Ptr<DefMonstre> Dragonnoie() const { return CaseDragonnoie >= 0 ? Donnees::Dragonnoie() : nullptr; }
        /// Tours de table entiers avant son prochain envol (0 : il s'envole au début du prochain tour de table).
        int ToursAvantEnvol() const { return std::max(0, TourEnvol - Tour - (Courant == 0 ? 0 : 1)); }

        // Partie.Nuit
        int Nuits = 0;                                  // nuits déjà tombées
        EvenementNuit DerniereNuit = EvenementNuit::Voleur;   // l'événement de la dernière nuit (si Nuits > 0)
        /// La nuit tombe au début du tour de table qui suit chaque N-ième : tours 6, 11, 16… pour N = 5.
        static bool NuitAvantLeTour(int tour);
        /// Le tour de table après lequel tombera la prochaine nuit (-1 : jamais).
        int ProchaineNuit() const;

    private:
        bool tourFini = false;           // le joueur courant ne peut plus rien faire ce tour (K.O., piège…)
        bool Arret() const { return Fini || tourFini; }
        Ensemble<int> protectionNeeCeTour;
        int profondeurEffets = 0;
        int tourDerniereNuit = 0;

        void FinDeTour(const Ptr<Joueur>& j);
        Signaux PhaseAction(Ptr<Joueur> j);
        Signaux TendreEmbuscade(Ptr<Joueur> j, std::function<void(bool)> resultat);
        Signaux Avancer(Ptr<Joueur> j);
        Signaux Marcher(Ptr<Joueur> j, int pas, bool effetArrivee);
        string AideVoie(int premiere) const;
        Signaux Arrivee(Ptr<Joueur> j);

        // Partie.Etat
        Signaux Soigner(Ptr<Joueur> j, int n, string raison);
        Signaux Blesser(Ptr<Joueur> j, int n, string raison);
        Signaux Ko(Ptr<Joueur> j, string raison);
        Signaux GagnerOr(Ptr<Joueur> j, int n, string raison);
        Signaux PerdreOr(Ptr<Joueur> j, int n, string raison);
        Signaux GagnerXP(Ptr<Joueur> j, int n);
        Signaux PerdreNiveau(Ptr<Joueur> j, int n, string raison);
        /// Une protection (amulette) bloque l'effet néfaste ? Elle se brise alors.
        Signaux Amulette(Ptr<Joueur> j, std::function<void(bool)> bloque);

        // Partie.Combats
        Signaux Duel(Ptr<Joueur> a, Ptr<Joueur> b, bool embuscade);
        Ptr<DefMonstre> TirerMonstre(const Ptr<Joueur>& j, Zone z);
        Signaux CombatMonstre(Ptr<Joueur> j, Ptr<DefMonstre> m, bool fuitePossible);
        Ptr<SCombat> CombatContreMonstre(const Ptr<Joueur>& j, const Ptr<DefMonstre>& m);
        Signaux DeclencherPiege(Ptr<Joueur> j, Ptr<Piege> p);

        // Partie.Cases
        Signaux EffetCase(Ptr<Joueur> j);
        Signaux EffetType(Ptr<Joueur> j, Ptr<Case> c);
        Signaux Tresor(Ptr<Joueur> j, Ptr<Case> c);
        Signaux Malediction(Ptr<Joueur> j);
        Liste<Ptr<DefObjet>> OffreVille() const;
        Liste<Ptr<DefObjet>> OffreTaverne() const;
        Liste<Ptr<DefObjet>> OffreMarchand(int caseId) const;
        Liste<Ptr<DefObjet>> OffreColporteur();
        Signaux Boutique(Ptr<Joueur> j, Liste<Ptr<DefObjet>> offre, string nom, float remise);

        // Partie.Objets
        Signaux UtiliserObjet(Ptr<Joueur> j, Ptr<DefObjet> o);
        Signaux Sort(Ptr<Joueur> lanceur, Ptr<Joueur> cible, Ptr<DefObjet> o);

        // Partie.Evenements
        Ptr<DefEvenement> TirerEvenement(const Liste<string>& ids, Zone zone);
        Ptr<DefEvenement> TirerEvenementPioche(Zone zone);
        Ptr<DefEvenement> Tirer(const Liste<Ptr<DefEvenement>>& l);
        static string TexteEvenement(const string& t, const Ptr<Joueur>& j);
        static bool PeutChoisir(const Ptr<Joueur>& j, const Ptr<ChoixEvenement>& c);
        Liste<Ptr<Joueur>> Cibles(const Ptr<Joueur>& j, CibleEffet c) const;
        Signaux Appliquer(Ptr<Joueur> j, Ptr<EffetEvenement> ef, string raison);
        Signaux AppliquerA(Ptr<Joueur> j, Ptr<Joueur> t, Ptr<EffetEvenement> ef, string raison);
        Signaux Teleporter(Ptr<Joueur> j, int vers, bool effetArrivee, string raison);
        static bool PeutEquiper(const Ptr<Joueur>& j, const Ptr<DefObjet>& o);
        static void Equiper(const Ptr<Joueur>& j, const Ptr<DefObjet>& o);

        // Partie.Dragonnoie
        static int Intervalle();
        Signaux ArriveeDragonnoie();
        Signaux EnvolDragonnoie();
        int CaseAtterrissage(int depuis);
        Signaux PasserDragonnoie(Ptr<Joueur> j, int pasRestants, std::function<void(bool)> attaque);
        /// « combat » (facultatif) reçoit vrai s'il y a eu combat.
        Signaux ArretDragonnoie(Ptr<Joueur> j, std::function<void(bool)> combat);
        Signaux CombatDragonnoie(Ptr<Joueur> j);

        // Partie.Nuit
        static int IntervalleNuits();
        Signaux Nuit();
        Signaux NuitVoleur();
        Liste<Ptr<Joueur>> PaireSurDeuxCases();
        Signaux NuitTrouDeVer();
        Signaux NuitBonneNuit();
        Signaux NuitArene();
        Signaux DuelArene(Ptr<Joueur> defi, Ptr<Joueur> champion, std::function<void(Ptr<SCombat>)> resultat);
        Signaux GagnerNiveau(Ptr<Joueur> j, bool peutGagner);

        // Partie.Quetes
        Liste<Ptr<Quete>> GenererQuetes(const Ptr<Joueur>& j);
        Ptr<Quete> QueteLieu(const Ptr<Joueur>& j);
        Ptr<Quete> QueteChasse(const Ptr<Joueur>& j);
        static bool Chassable(const Ptr<DefMonstre>& m);
        Ptr<Quete> QueteVaincre(const Ptr<Joueur>& j);
        Ptr<Quete> QueteEmbuscade();
        Ptr<Quete> QueteDefi(const Ptr<Joueur>& j);
        Ptr<Quete> QueteEvade(const Ptr<Joueur>& j);
        int VillePourEvade(const Ptr<Joueur>& j) const;
        Signaux ProposerQuetes(Ptr<Joueur> j, string lieu);
        Signaux VerifierQueteLieu(Ptr<Joueur> j);
        Signaux ProgresQuete(Ptr<Joueur> j, Quete::Genre genre, int cible = -1);
        Signaux ChasseEvade(Ptr<Joueur> j);
        Signaux FinirQuete(Ptr<Joueur> j);
    };
}
