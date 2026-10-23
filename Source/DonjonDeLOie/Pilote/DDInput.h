// Donjon de l'Oie — lecture de la souris et du clavier, avec le nouveau système d'entrées d'Unity 6 (par défaut)
// ou l'ancien gestionnaire, selon le réglage « Active Input Handling » du projet.
//
// Portage : seule la branche ENABLE_INPUT_SYSTEM est gardée (le moteur imite Mouse.current et Keyboard.current, voir
// Moteur/Entrees.h) ; la branche de l'ancien gestionnaire (Input.GetKey, Legacy(KeyCodeDD)…) n'a pas d'équivalent ici.
#pragma once

#include "../Moteur/Moteur.h"

namespace DonjonDeLOie::Pilote
{
    enum class KeyCodeDD { Gauche, Droite, Espace, Echap, Entree, F1, Tab };

    struct DDInput
    {
        static Vector2 PositionSouris();

        static Vector2 DeplacementSouris();

        /// Crans de molette depuis l'image précédente (positif : vers l'avant).
        static float Molette();

        static bool BoutonDroitMaintenu();

        static bool BoutonMilieuMaintenu();

        static bool ClicGauche();

        static bool Touche(KeyCodeDD k);

        static bool ToucheAppuyee(KeyCodeDD k);

    private:
        static inline int image = -1;
        static inline Vector2 dernierePos, delta;

        static void MettreAJour();

        static KeyControl* Controle(Keyboard* kb, KeyCodeDD k);

        /// Remise des variables statiques à leur valeur de départ (Boucle::Reinitialiser) : inscrite dans le .cpp
        /// (image, dernierePos, delta).
        static const bool remiseAZero;
    };
}
