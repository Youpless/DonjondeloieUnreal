// Donjon de l'Oie — le catalogue modifiable : équipements, monstres, classes, événements et réglages, dans un fichier
// Assets/DonjonDeLOie/Resources/DDCatalogue.asset. On le modifie avec la fenêtre « Donjon de l'Oie › Catalogue ».
// Au lancement, le jeu le lit et remplace les valeurs écrites dans Regles/Donnees.cpp. Sans ce fichier, rien ne change.
#pragma once

#include "../Moteur/Moteur.h"
#include "../Regles/Donnees.h"
#include "../Regles/Evenements.h"

namespace DonjonDeLOie::Cartes
{
    // Pas d'entrée dans le menu Assets › Create : le jeu ne lit que Resources/DDCatalogue.asset, que la fenêtre du
    // catalogue crée (rempli des valeurs d'origine) d'un clic.
    class DDCatalogue : public ScriptableObject
    {
        DD_TYPE_OBJET(DDCatalogue, ScriptableObject)
    public:
        static constexpr const char* CheminResources = "DDCatalogue";

        Liste<Ptr<Regles::DefObjet>> Objets;
        Liste<Ptr<Regles::DefMonstre>> Monstres;
        Liste<Ptr<Regles::DefClasse>> Classes;
        Liste<Ptr<Regles::DefEvenement>> Evenements;
        Ptr<Regles::DefReglages> Reglages = Nouveau<Regles::DefReglages>();      // (la classe Reglages du C#)

        /// Remplit le catalogue avec les valeurs écrites dans le code.
        void RemplirParDefaut();

        /// Met ce catalogue en service (les règles lisent ensuite ces valeurs).
        void Appliquer();

        /// Lit le catalogue du dossier Resources et le met en service. Rend false s'il n'existe pas (valeurs du code).
        static bool Charger();

    private:
        /// Appelé par Unity quand on choisit « Reset » dans l'inspecteur du fichier : les valeurs d'origine.
        /// (Un geste de l'éditeur d'Unity : le moteur ne l'appelle jamais.)
        void Reset();
    };
}
