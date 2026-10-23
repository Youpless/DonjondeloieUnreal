// Donjon de l'Oie — le salon : six places (humain, bot ou fermée), un nom, une classe et une couleur pour chacune,
// le niveau à atteindre, la vitesse des bots et la carte jouée.
#pragma once

#include "../Moteur/Moteur.h"
#include "../Regles/Partie.h"

namespace DonjonDeLOie::Interface
{
    enum class TypePlace { Humain, Bot, Fermee };

    /// (une classe en C# : tenue par Ptr, les boutons de sa ligne la modifient)
    struct Place
    {
        TypePlace Type = TypePlace::Humain;
        string Nom;
        Regles::ClasseHeros Classe = Regles::ClasseHeros::Guerrier;
        int Couleur = 0;
    };

    class EcranSalon
    {
    public:
        Ref<RectTransform> R;
        Liste<Ptr<Place>> Places;
        int NiveauCible = 10;
        bool BotsRapides = false;
        int Carte = 0;                         // index dans la liste donnée par DefinirCartes

        EcranSalon(const Ref<RectTransform>& racine, std::function<void()> retour, std::function<void(const Liste<Ptr<Regles::ReglageJoueur>>&)> lancer_);

        /// La liste des cartes jouables (noms et niveaux conseillés) et celle choisie au départ.
        void DefinirCartes(const Liste<string>& noms, int choisie, const Liste<int>& niveaux);

        void Visible(bool v);

    private:
        Liste<string> nomsCartes = {"Carte d'origine"};
        Liste<int> niveauxCartes = {10};
        Ref<Text> carteTexte;
        Ref<Button> bCarte;
        const std::function<void(const Liste<Ptr<Regles::ReglageJoueur>>&)> lancer;
        Liste<std::function<void()>> majLignes;
        Ref<Text> niveauTexte, vitesseTexte, erreur;
        Ref<Button> bLancer;

        static const Liste<string>& NomsBots();

        void Ligne(const Ref<RectTransform>& p, int i);

        int CouleurLibre(int actuelle, int place);

        void ChoisirCarte(int i);

        void Maj();

        void Lancer();
    };
}
