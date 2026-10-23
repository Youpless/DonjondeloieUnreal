// Donjon de l'Oie — le plateau : une grande boucle de 40 cases et trois raccourcis, ou une carte faite dans l'éditeur.
#pragma once

#include "Donnees.h"

namespace DonjonDeLOie::Regles
{
    /// Les événements propres d'une case : Standard = l'effet du type (une case Événement tire dans sa liste si elle en a
    /// une), Remplacer = un événement de la liste au lieu de l'effet du type, Ajouter = l'effet du type puis un événement.
    enum class ModeEvenements { Standard, Remplacer, Ajouter };
    string NomEnum(ModeEvenements v);

    struct Case
    {
        int Id = 0;
        TypeCase Type = TypeCase::Chemin;
        Regles::Zone Zone = Regles::Zone::Village;
        string Nom;                 // lieu nommé (cible possible des quêtes), sinon vide
        string Voie;                // nom du chemin quand cette case ouvre un raccourci ou la grande route
        float X = 0, Z = 0;         // position sur l'île, en mètres
        Liste<int> Suivants;
        Liste<int> Precedents;
        bool Raccourci = false;     // case d'un raccourci (hors de la grande boucle)
        ModeEvenements Mode = ModeEvenements::Standard;     // comment ses événements propres s'ajoutent à son type
        Liste<string> Evenements;   // ids d'événements propres à la case
        string Lieu;                // bâtiment posé à côté : "" automatique, "-" aucun, sinon l'id d'un décor

        string Libelle() const { return Nom.empty() ? Donnees::NomsTypesCase[static_cast<int>(Type)] : Nom; }
    };

    // ------------------------------------------------------------------ la carte en données (éditeur de cartes)

    struct CaseCarte
    {
        string Nom;                             // lieu nommé (cible des quêtes de voyage) ; vide : une case ordinaire
        TypeCase Type = TypeCase::Chemin;
        Regles::Zone Zone = Regles::Zone::Village;
        float X = 0, Z = 0;                     // position en mètres
        Liste<int> Suivants;                    // cases suivantes ; la première est la « grande route »
        string Voie;                            // nom du chemin quand cette case ouvre un embranchement
        ModeEvenements Mode = ModeEvenements::Standard;
        Liste<string> Evenements;
        string Lieu;                            // bâtiment : "" automatique, "-" aucun, sinon l'id d'un décor
    };

    struct DecorPose
    {
        string Id = "rocher";
        int Graine = 1;
        float X = 0, Z = 0, RotY = 0, Echelle = 1.0f;
    };

    struct CarteDef
    {
        string Nom = "Nouvelle carte";
        int Depart = 0;                         // index de la case de départ (soins, boutique complète, solde)
        Liste<Ptr<CaseCarte>> Cases;
        Liste<Ptr<DecorPose>> Decors;           // décors et bâtiments posés à la main
        bool DecorAutomatique = true;           // arbres, rochers… semés selon les zones
        float DensiteDecor = 1.0f;              // 0 : aucun, 1 : normal, 2 : très dense
        bool LieuxAutomatiques = true;          // bâtiments posés selon le type des cases (sauf choix de la case)
        float Marge = 12.0f;                    // largeur de terre autour du chemin, en mètres
        int Graine = 7;                         // relief et décor
    };

    class Plateau
    {
    public:
        Liste<Ptr<Case>> Cases;
        int Depart = 0;             // Valcreux

        const Ptr<Case>& operator[](int i) const { return Cases[i]; }
        int Nombre() const { return Cases.Count(); }

        /// Les cases voisines (suivantes puis précédentes).
        Liste<int> Voisins(int id) const;
        bool SontVoisines(int a, int b) const;
        /// Distance minimale en avançant de a vers b (les raccourcis comptent). -1 si plus de max pas.
        int DistanceEnAvant(int a, int b, int max = 60) const;
        Liste<int> CasesDeType(TypeCase t) const;

        static Ptr<Plateau> Standard();

        /// Fabrique un plateau à partir d'une carte (sans vérifier : appelez Verifier avant).
        static Ptr<Plateau> Depuis(const CarteDef& carte);
        /// La grande boucle part du départ en suivant toujours la première sortie ; tout le reste est un raccourci.
        void MarquerRaccourcis();
        /// La carte d'un plateau (pour partir de la carte standard dans l'éditeur).
        Ptr<CarteDef> VersCarte(const string& nom) const;
        /// Les problèmes d'une carte. Les « erreurs » empêchent de jouer ; les « conseils » non.
        static void Verifier(const CarteDef& carte, Liste<string>& erreurs, Liste<string>& conseils);
        /// Une petite carte de départ pour l'éditeur : un cercle de cases autour d'une ville.
        static Ptr<CarteDef> CarteVide(const string& nom, int nombre = 16, float rayon = 14.0f);

    private:
        static void Lier(Plateau& p, int a, int b);
    };
}
