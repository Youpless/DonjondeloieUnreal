// Donjon de l'Oie — données du jeu (classes, monstres, objets, cases, zones).
// Ce fichier ne dépend pas d'Unreal : les règles tournent aussi hors du moteur (essais, simulations).
#pragma once

#include "../Noyau/Jeu.h"

namespace DonjonDeLOie::Regles
{
    enum class ClasseHeros { Guerrier, Mage, Voleur, Clerc, Rodeur, Barbare };

    enum class Zone { Village, Bois, Marais, Cryptes, Pics, Antre };

    enum class TypeCase { Ville, Chemin, Monstre, Tresor, Evenement, Malediction, Sanctuaire, Marchand, Taverne, Antre };

    enum class TypeObjet { Arme, Armure, Accessoire, Potion, Piege, Parchemin };

    enum class EffetObjet
    {
        Aucun,
        // accessoires
        Amulette, Bottes, Anneau,
        // potions
        Soin, SoinTotal,
        // pièges
        PiegeLoups, PiegeFosse, PiegeFilet, PiegeRune,
        // parchemins
        SortMalediction, SortFoudre, SortLenteur, SortRappel
    };

    /// Le ton d'une annonce (sa couleur dans la chronique).
    enum class Ton { Neutre, Bon, Mauvais, Combat, Quete, Systeme };

    struct DefClasse
    {
        ClasseHeros Classe = ClasseHeros::Guerrier;
        string Nom, Passif, Description;
        int Attaque = 0, Defense = 0, Vie = 0;
        DefClasse() = default;
        DefClasse(ClasseHeros c, string nom, int atk, int def, int vie, string passif, string desc)
            : Classe(c), Nom(std::move(nom)), Passif(std::move(passif)), Description(std::move(desc)), Attaque(atk), Defense(def), Vie(vie) {}
        Ptr<DefClasse> Copie() const { return Nouveau<DefClasse>(*this); }
    };

    struct DefMonstre
    {
        string Id, Nom, Description;
        int Force = 0, Degats = 0, Or = 0, XP = 0;
        Liste<Regles::Zone> Zones;
        bool Boss = false;
        bool Errant = false;                 // le Dragonnoie : il erre sur la carte au lieu de surgir des cases (Partie.Dragonnoie)
        string Apparence;                    // modèle 3D d'un autre monstre (vide : celui de son identifiant)
        string Modele() const { return Apparence.empty() ? Id : Apparence; }
        DefMonstre() = default;
        DefMonstre(string id, string nom, int force, int degats, int or_, int xp, Liste<Regles::Zone> zones, string desc, bool boss = false)
            : Id(std::move(id)), Nom(std::move(nom)), Description(std::move(desc)), Force(force), Degats(degats), Or(or_), XP(xp),
              Zones(std::move(zones)), Boss(boss) {}
        Ptr<DefMonstre> Copie() const { return Nouveau<DefMonstre>(*this); }
    };

    struct DefObjet
    {
        string Id, Nom, Description;
        TypeObjet Type = TypeObjet::Arme;
        EffetObjet Effet = EffetObjet::Aucun;
        int Prix = 0, Bonus = 0;
        bool SeulementEnVille = false;
        DefObjet() = default;
        DefObjet(string id, string nom, TypeObjet type, int prix, int bonus, EffetObjet effet, string desc, bool ville = false)
            : Id(std::move(id)), Nom(std::move(nom)), Description(std::move(desc)), Type(type), Effet(effet), Prix(prix), Bonus(bonus),
              SeulementEnVille(ville) {}
        Ptr<DefObjet> Copie() const { return Nouveau<DefObjet>(*this); }
        bool Consommable() const { return Type == TypeObjet::Potion || Type == TypeObjet::Piege || Type == TypeObjet::Parchemin; }
    };

    /// Les réglages globaux (modifiables dans le catalogue).
    struct DefReglages
    {
        int XPParNiveau = 2;          // XP pour un niveau
        int OrDepart = 20;            // or de départ
        int SoldeVille = 10;          // or à chaque passage par la case de départ
        int XPVille = 1;              // XP à chaque passage par la case de départ
        int TailleSac = 5;            // objets dans le sac
        int AccessoiresMax = 2;
        int PiegesMaxParJoueur = 3;
        bool DragonnoieActif = true;  // le Dragonnoie erre sur la carte
        int ToursEntreEnvols = 3;     // il s'envole au début d'un tour de table sur N
        bool NuitsActives = true;     // la nuit tombe tous les N tours de table, avec un événement
        int ToursEntreNuits = 5;      // N
        bool NuitVoleur = true, NuitTrouDeVer = true, NuitBonneNuit = true, NuitArene = true;   // les événements possibles
        int OrArene = 25;             // l'or du vainqueur de l'arène du diable
        int DureeEvade = 30;          // secondes pour retrouver l'évadé (mini-jeu)
        Ptr<DefReglages> Copie() const { return Nouveau<DefReglages>(*this); }
    };

    struct DefEvenement;

    struct Donnees
    {
        /// Les classes en cours (celles du catalogue si on l'a chargé). Rangées dans l'ordre de ClasseHeros.
        static Liste<Ptr<DefClasse>> Classes;
        static Ptr<DefClasse> Classe(ClasseHeros c) { return Classes[static_cast<int>(c)]; }

        static const Liste<string> NomsZones;

        static Liste<Ptr<DefMonstre>> Monstres;
        /// Le boss (le monstre de la case Antre) : le premier monstre marqué « Boss ».
        static Ptr<DefMonstre> Boss();
        /// Le monstre errant (le premier marqué « Errant »), ou nullptr s'il n'y en a pas ou si les réglages le désactivent.
        static Ptr<DefMonstre> Dragonnoie();
        static Ptr<DefMonstre> Monstre(const string& id);

        static Liste<Ptr<DefObjet>> Objets;
        static Ptr<DefObjet> Objet(const string& id);

        static const Liste<string> NomsTypesCase;
        static const Liste<string> AidesTypesCase;

        /// Réglages globaux (modifiables dans le catalogue).
        static Ptr<DefReglages> Reglages;
        static int XPParNiveau() { return std::max(1, Reglages->XPParNiveau); }
        static int OrDepart() { return Reglages->OrDepart; }
        static int SoldeVille() { return Reglages->SoldeVille; }
        static int XPVille() { return Reglages->XPVille; }
        static int TailleSac() { return std::max(1, Reglages->TailleSac); }
        static int AccessoiresMax() { return std::max(0, Reglages->AccessoiresMax); }
        static int PiegesMaxParJoueur() { return std::max(0, Reglages->PiegesMaxParJoueur); }

        /// Les événements (ceux d'origine + ceux créés dans l'éditeur). Voir Evenements.h.
        static Liste<Ptr<DefEvenement>> Evenements;
        static Ptr<DefEvenement> Evenement(const string& id);

        /// Remplace les données du jeu (catalogue de l'éditeur). Un pointeur nul garde ce qui est en place. Les classes
        /// manquantes sont reprises des valeurs d'origine ; une liste vide garde les valeurs en place.
        static void Appliquer(const Liste<Ptr<DefClasse>>* classes, const Liste<Ptr<DefMonstre>>* monstres,
                              const Liste<Ptr<DefObjet>>* objets, const Liste<Ptr<DefEvenement>>* evenements, const Ptr<DefReglages>& reglages);

        /// Revient aux valeurs écrites dans le code.
        static void Reinitialiser();

        static Liste<Ptr<DefClasse>> CopieClassesParDefaut();
        static Liste<Ptr<DefMonstre>> CopieMonstresParDefaut();
        static Liste<Ptr<DefObjet>> CopieObjetsParDefaut();
    };

    /// Le nom d'une valeur d'énumération, comme ToString() en C#.
    string NomEnum(ClasseHeros v);
    string NomEnum(Zone v);
    string NomEnum(TypeCase v);
    string NomEnum(TypeObjet v);
    string NomEnum(EffetObjet v);
    string NomEnum(Ton v);
}
