// Donjon de l'Oie — ce que le moteur de règles raconte à la présentation (ou à une simulation).
// Le moteur avance tour par tour en produisant des signaux ; une Question attend une réponse (d'un humain par
// l'interface, ou d'un bot) avant que le moteur reprenne.
#pragma once

#include "Evenements.h"
#include "Joueur.h"

namespace DonjonDeLOie::Regles
{
    /// La sorte d'un signal (le « is » de C# : on teste la sorte au lieu du type, sans RTTI).
    enum class SorteSignal { Annonce, TourDebut, De, Pas, Teleport, Envol, Nuit, Stat, Combat, Effet, Question, FinPartie };

    struct Signal
    {
        const SorteSignal Sorte;
        explicit Signal(SorteSignal s) : Sorte(s) {}
        virtual ~Signal() = default;
    };

    /// Le signal en tant que T s'il est de cette sorte, sinon nullptr (le « as » de C#).
    template <class T>
    inline Ptr<T> Comme(const Ptr<Signal>& s)
    {
        return (s != nullptr && s->Sorte == T::SorteStatique) ? std::static_pointer_cast<T>(s) : nullptr;
    }

    /// Une suite de signaux produite pas à pas (l'IEnumerable<Signal> de C#).
    using Signaux = Gen<Ptr<Signal>>;

    struct SAnnonce : Signal
    {
        static constexpr SorteSignal SorteStatique = SorteSignal::Annonce;
        string Texte;
        int Joueur = -1;
        Regles::Ton Ton = Regles::Ton::Neutre;
        bool Grande = false;
        SAnnonce(string texte, int joueur = -1, Regles::Ton ton = Regles::Ton::Neutre, bool grande = false)
            : Signal(SorteStatique), Texte(std::move(texte)), Joueur(joueur), Ton(ton), Grande(grande) {}
    };

    struct STourDebut : Signal
    {
        static constexpr SorteSignal SorteStatique = SorteSignal::TourDebut;
        int Joueur = 0, Tour = 0;
        STourDebut() : Signal(SorteStatique) {}
    };

    struct SDe : Signal
    {
        static constexpr SorteSignal SorteStatique = SorteSignal::De;
        int Joueur = 0;
        int Faces = 6;
        int Valeur = 0;
        int Bonus = 0;
        string Raison;
        int Total() const { return Valeur + Bonus; }
        SDe() : Signal(SorteStatique) {}
    };

    struct SPas : Signal
    {
        static constexpr SorteSignal SorteStatique = SorteSignal::Pas;
        int Joueur = 0, De = 0, Vers = 0;
        SPas(int joueur, int de, int vers) : Signal(SorteStatique), Joueur(joueur), De(de), Vers(vers) {}
    };

    struct STeleport : Signal
    {
        static constexpr SorteSignal SorteStatique = SorteSignal::Teleport;
        int Joueur = 0, Vers = 0;
        string Raison;
        STeleport(int joueur, int vers, string raison) : Signal(SorteStatique), Joueur(joueur), Vers(vers), Raison(std::move(raison)) {}
    };

    /// Le Dragonnoie s'envole de « De » (-1 : il arrive sur l'île) et se pose sur « Vers ».
    struct SEnvol : Signal
    {
        static constexpr SorteSignal SorteStatique = SorteSignal::Envol;
        int De = -1, Vers = 0;
        SEnvol(int de, int vers) : Signal(SorteStatique), De(de), Vers(vers) {}
    };

    /// La nuit tombe (Tombe = vrai) après le tour de table « Tour », ou le jour se lève (Tombe = faux).
    struct SNuit : Signal
    {
        static constexpr SorteSignal SorteStatique = SorteSignal::Nuit;
        bool Tombe = false;
        int Tour = 0, Numero = 0;
        SNuit(bool tombe, int tour, int numero) : Signal(SorteStatique), Tombe(tombe), Tour(tour), Numero(numero) {}
    };

    enum class Stat { PV, Or, XP, Niveau, Attaque };

    struct SStat : Signal
    {
        static constexpr SorteSignal SorteStatique = SorteSignal::Stat;
        int Joueur = 0;
        Regles::Stat Stat = Regles::Stat::PV;
        int Delta = 0;
        SStat(int j, Regles::Stat s, int d) : Signal(SorteStatique), Joueur(j), Stat(s), Delta(d) {}
    };

    struct SCombat : Signal
    {
        static constexpr SorteSignal SorteStatique = SorteSignal::Combat;
        bool Duel = false;                // sinon contre un monstre
        int Attaquant = 0, Defenseur = -1;
        Ptr<DefMonstre> Monstre;
        int FacesDe = 0;                  // 3 en duel, 6 contre un monstre
        int AtkA = 0, BonusA = 0, DeA = 0, AtkB = 0, BonusB = 0, DeB = 0;
        string RaisonBonusA, RaisonBonusB;
        bool Egalite = false, AttaquantGagne = false;
        bool Embuscade = false;
        bool Arene = false;               // duel du tournoi de l'arène du diable (la nuit)
        int TotalA() const { return AtkA + BonusA + DeA; }
        int TotalB() const { return AtkB + BonusB + DeB; }
        SCombat() : Signal(SorteStatique) {}
    };

    enum class Effet
    {
        PiegePose, PiegeDeclenche, EmbuscadeTendue, EmbuscadeDeclenchee, SortLance, SortBloque,
        Malediction, NiveauGagne, NiveauPerdu, KO, Soin, Tresor, Achat, QueteAcceptee, QueteFinie, Evenement, Victoire,
        OrDragonnoie,       // le Dragonnoie arrache la bourse d'un vaincu (Texte : la somme)
        VolNocturne,        // la nuit, un voleur prend la moitié de l'or d'un joueur (Texte : la somme)
        TrouDeVer,          // la nuit, Joueur et Cible échangent leur place
        BonneNuit,          // la nuit, un joueur dort bien et gagne un niveau
        AreneDebut,         // l'arène du diable surgit (Texte : l'ordre d'entrée, identifiants séparés par des virgules)
        AreneFin,           // Joueur remporte l'arène du diable
        QueteRatee          // une quête échoue (l'évadé s'échappe) ; Texte : son titre
    };

    struct SEffet : Signal
    {
        static constexpr SorteSignal SorteStatique = SorteSignal::Effet;
        Regles::Effet Effet = Regles::Effet::PiegePose;
        int Joueur = -1, Cible = -1, Case = -1;
        string Texte;
        string Objet;
        explicit SEffet(Regles::Effet e) : Signal(SorteStatique), Effet(e) {}
    };

    enum class GenreQuestion { Action, Embranchement, Rodeur, Defi, Monstre, Boutique, Quetes, ChoixCase, ChoixCible, Confirmer, ChoixEvenement, Dragonnoie, Evade };

    struct SQuestion : Signal
    {
        static constexpr SorteSignal SorteStatique = SorteSignal::Question;
        int Joueur = 0;
        GenreQuestion Genre = GenreQuestion::Action;
        string Contexte;          // précise la question : « embuscade », « piege », « rappel », « sort:<id> »
        string Titre, Texte;
        Liste<string> Options;
        Liste<string> Aides;
        Liste<string> Codes;
        Liste<bool> Actives;
        Liste<int> Cases;         // case associée à chaque option (-1 sinon)
        Liste<int> Cibles;        // joueur associé à chaque option (-1 sinon)
        Liste<Ptr<DefObjet>> Objets;
        Liste<int> Prix;
        Liste<Ptr<Regles::Quete>> Quetes;
        Liste<Liste<Ptr<EffetEvenement>>> EffetsChoix;   // ChoixEvenement : les effets de chaque option
        Ptr<DefMonstre> Monstre;
        Ptr<Regles::Quete> Quete;     // Evade : la quête en jeu (nom et allure de l'évadé)
        int ParDefaut = 0;
        int Reponse = -1;

        SQuestion(int joueur, GenreQuestion genre, string titre, string texte)
            : Signal(SorteStatique), Joueur(joueur), Genre(genre), Titre(std::move(titre)), Texte(std::move(texte)) {}

        SQuestion& Ajouter(const string& option, const string& code, const string& aide = "", bool active = true, int caseId = -1, int cible = -1,
                           const Ptr<DefObjet>& objet = nullptr, int prix = 0, const Ptr<Regles::Quete>& quete = nullptr)
        {
            Options.Add(option);
            Codes.Add(code);
            Aides.Add(aide);
            Actives.Add(active);
            Cases.Add(caseId);
            Cibles.Add(cible);
            Objets.Add(objet);
            Prix.Add(prix);
            Quetes.Add(quete);
            return *this;
        }

        /// Le code de l'option choisie (vide si aucune).
        string CodeChoisi() const { return Reponse >= 0 && Reponse < Codes.Count() ? Codes[Reponse] : string(); }
    };

    string NomEnum(GenreQuestion v);
    string NomEnum(Quete::Genre v);

    struct SFinPartie : Signal
    {
        static constexpr SorteSignal SorteStatique = SorteSignal::FinPartie;
        int Gagnant = 0;
        explicit SFinPartie(int gagnant) : Signal(SorteStatique), Gagnant(gagnant) {}
    };
}
