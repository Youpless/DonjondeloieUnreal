// Donjon de l'Oie — les choix proposés au joueur humain : le bandeau du bas (action, embranchement, cible, case…)
// et les grandes fenêtres (boutique, quêtes, rencontre d'un monstre).
//
// Une seule classe, partielle en C# : cet en-tête déclare les membres des deux fichiers, et chaque fichier C# donne
// son .cpp : FenetreQuestion.cpp (aiguillage, bandeau du bas, réflexion d'un bot) et FenetreQuestion.Fenetres.cpp
// (grandes fenêtres : boutique (fiches d'objets), panneau des quêtes, rencontre d'un monstre ou du Dragonnoie).
#pragma once

#include "../Moteur/Moteur.h"
#include "../Regles/Partie.h"

namespace DonjonDeLOie::Interface
{
    class FenetreQuestion
    {
    public:
        explicit FenetreQuestion(const Ref<RectTransform>& racine_);

        bool Ouverte() const { return ouverte; }
        Ptr<Regles::SQuestion> Question() const { return question; }

        void Cacher();

        void Choisir(int i);

        // ------------------------------------------------------------------ un bot réfléchit

        void MontrerReflexion(const string& texte, Color c);

        void CacherReflexion();

        // ------------------------------------------------------------------ aiguillage

        void Montrer(const Ptr<Regles::SQuestion>& q, const Ptr<Regles::Partie>& p, std::function<void(int)> rep);

        /// Appelé quand le pointeur survole une option liée à une case (pour la mettre en valeur sur le plateau).
        std::function<void(int)> SurvolCase;

    private:
        const Ref<RectTransform> racine;
        Ref<RectTransform> bandeau, fenetre, reflexion;
        Ref<Text> reflexionTexte;
        std::function<void(int)> repondre;
        bool ouverte = false;                        // Ouverte { get; private set; }
        Ptr<Regles::SQuestion> question;             // Question { get; private set; }

        // ------------------------------------------------------------------ bandeau du bas

        void Bandeau(const Ptr<Regles::SQuestion>& q, const Ptr<Regles::Partie>& p, const Ptr<Regles::Joueur>& j);

        // ================================================================== FenetreQuestion.Fenetres.cpp

        Ref<RectTransform> Fenetre(const string& titre, float w, float h, const Ptr<Regles::Joueur>& j, bool enBas = false);

        // ------------------------------------------------------------------ boutique

        void Boutique(const Ptr<Regles::SQuestion>& q, const Ptr<Regles::Partie>& p, const Ptr<Regles::Joueur>& j);

        static string Equipement(const Ptr<Regles::Joueur>& j);

        // ------------------------------------------------------------------ quêtes

        void Quetes(const Ptr<Regles::SQuestion>& q, const Ptr<Regles::Partie>& p, const Ptr<Regles::Joueur>& j);

        // ------------------------------------------------------------------ monstre

        void Monstre(const Ptr<Regles::SQuestion>& q, const Ptr<Regles::Partie>& p, const Ptr<Regles::Joueur>& j);

        /// En passant sur la case du Dragonnoie : l'attaquer (victoire = la partie) ou continuer sa route.
        void Dragonnoie(const Ptr<Regles::SQuestion>& q, const Ptr<Regles::Partie>& p, const Ptr<Regles::Joueur>& j);
    };
}
