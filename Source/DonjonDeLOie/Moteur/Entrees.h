// Donjon de l'Oie — la souris et le clavier, avec les noms du « nouveau » système d'entrées d'Unity (Mouse.current,
// Keyboard.current…). La couche Unreal remplit leur état au début de chaque image (Entrees::DebutImage puis les
// Changer…) ; le jeu le lit comme dans Unity. Coordonnées de la souris en pixels, origine en bas à gauche.
#pragma once

#include "Objets.h"

namespace Moteur
{
    class ButtonControl
    {
    public:
        bool isPressed = false, wasPressedThisFrame = false, wasReleasedThisFrame = false;
        /// Nouvel état (appelé par la couche d'affichage).
        void Changer(bool enfonce)
        {
            if (enfonce && !isPressed) wasPressedThisFrame = true;
            if (!enfonce && isPressed) wasReleasedThisFrame = true;
            isPressed = enfonce;
        }
    };
    using KeyControl = ButtonControl;

    class Vector2Control
    {
    public:
        Vector2 valeur;
        Vector2 ReadValue() const { return valeur; }
    };

    class Mouse
    {
    public:
        static Mouse* current();
        Vector2Control position, scroll, delta;
        ButtonControl leftButton, rightButton, middleButton;
        /// Vrai quand une vraie souris est branchée sur le moteur (faux pendant les essais) : sans elle, le système
        /// d'événements de l'interface ne fait rien.
        bool Presente = false;
    };

    class Keyboard
    {
    public:
        static Keyboard* current();
        KeyControl leftArrowKey, rightArrowKey, upArrowKey, downArrowKey, spaceKey, escapeKey, enterKey, f1Key, tabKey, qKey, eKey, backspaceKey;
        /// Les caractères tapés pendant l'image (UTF-8), pour le champ de saisie.
        string CaracteresTapes;
    };

    /// Ce que la couche d'affichage appelle pour faire avancer les entrées.
    struct Entrees
    {
        /// Au début de chaque image : efface les « appuyé / relâché cette image », la molette, le déplacement et les
        /// caractères de l'image précédente.
        static void DebutImage();
        /// Position de la souris en pixels (origine en bas à gauche) ; le déplacement depuis la dernière position est
        /// calculé.
        static void PositionSouris(Vector2 p);
        static void Molette(float crans);
    };
}
