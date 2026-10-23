// Donjon de l'Oie — les bots : ils répondent aux mêmes questions que les joueurs humains (IA.cpp : la réponse, les
// actions, la boutique ; IA.Choix.cpp : cases, événements, cibles ; IA.Quetes.cpp : les quêtes).
#pragma once

#include "Partie.h"

namespace DonjonDeLOie::Regles
{
    class IA
    {
    public:
        explicit IA(int graine) : r(graine ^ 0x5bd1e995) {}

        /// Chance qu'un bot repère l'évadé dans la foule.
        static constexpr double ChanceEvade = 0.55;

        /// Chance que l'attaquant gagne un duel (attaque + D3 chacun, deux relances en cas d'égalité, le défenseur gagne la troisième).
        static double ChanceDuel(int atkA, int atkB);
        /// Chance de battre un monstre (attaque + D6 ≥ force + D6).
        static double ChanceMonstre(int atk, int force);

        int Repondre(Partie& p, SQuestion& q);

    private:
        AleaNet r;
        int actionsCeTour = 0, dernierTour = -1, dernierJoueur = -1;

        static int BonusMonstre(const Joueur& j) { return (j.Classe == ClasseHeros::Guerrier || j.Classe == ClasseHeros::Mage ? 1 : 0) + j.Benediction; }
        static int PremiereActive(const SQuestion& q, int prefere);
        static int Index(const SQuestion& q, const string& code);
        static Liste<Ptr<Joueur>> Poursuivants(const Partie& p, const Ptr<Joueur>& j, int portee);
        static Ptr<Joueur> MeneurAdverse(const Partie& p, const Ptr<Joueur>& j);
        static int ForceTypique(Zone z);
        static bool VeutAffronterDragonnoie(const Partie& p, const Ptr<Joueur>& j);
        static Ptr<DefObjet> ProchaineArme(const Joueur& j);

        int Action(Partie& p, const Ptr<Joueur>& j, SQuestion& q);
        int Embranchement(Partie& p, const Ptr<Joueur>& j, SQuestion& q);
        double ValeurCase(Partie& p, const Ptr<Joueur>& j, int c);
        int MeilleureCase(Partie& p, const Ptr<Joueur>& j, SQuestion& q);
        int Defi(Partie& p, const Ptr<Joueur>& j, SQuestion& q);
        int Monstre(const Ptr<Joueur>& j, SQuestion& q);
        int Boutique(Partie& p, const Ptr<Joueur>& j, SQuestion& q);

        // IA.Choix
        int ChoixCase(Partie& p, const Ptr<Joueur>& j, SQuestion& q);
        int ChoixEvenement(Partie& p, const Ptr<Joueur>& j, SQuestion& q);
        static double ValeurEffet(const Partie& p, const Ptr<Joueur>& j, const EffetEvenement& e);
        int ChoixCible(Partie& p, const Ptr<Joueur>& j, SQuestion& q);

        // IA.Quetes
        int Quetes(Partie& p, const Ptr<Joueur>& j, SQuestion& q);
        static bool QueteBloquee(const Partie& p, const Ptr<Joueur>& j);
        static double ValeurQuete(const Partie& p, const Ptr<Joueur>& j, const Quete& qu);
        bool EmbuscadePourQuete(Partie& p, const Ptr<Joueur>& j);
        static int DefiPourQuete(const Partie& p, const Ptr<Joueur>& j, const SQuestion& q);
        static double BonusCaseQuete(const Partie& p, const Ptr<Joueur>& j, int c);
    };
}
