// Donjon de l'Oie — un décor ou un bâtiment posé à la main dans l'éditeur de cartes (tour, ruine, rocher, arbre…).
#pragma once

#include "../Moteur/Moteur.h"

namespace DonjonDeLOie::Cartes
{
    /// [DisallowMultipleComponent]
    class DDDecorMarqueur : public MonoBehaviour
    {
        DD_TYPE_OBJET(DDDecorMarqueur, MonoBehaviour)
    public:
        /// Identifiant du décor (ex. tour_garde, maison, ruine, chene, rocher). Voir la liste dans l'inspecteur.
        string Id = "maison";
        /// Fait varier la forme d'un exemplaire à l'autre.
        int Graine = 1;

    private:
        /// Les gizmos (Gizmos.DrawWireSphere…, Handles.Label sous UNITY_EDITOR) ne se dessinent que dans l'éditeur
        /// d'Unity : rien de tel ici, la méthode est gardée mais ne fait rien (le moteur ne l'appelle d'ailleurs jamais).
        void OnDrawGizmos();
    };
}
