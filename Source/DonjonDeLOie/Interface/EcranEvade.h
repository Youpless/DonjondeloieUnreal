// Donjon de l'Oie — l'écran du mini-jeu de l'évadé : l'avis de recherche à gauche (portrait, nom, récompense), le
// sablier en haut, la consigne en bas, les messages (« Ce n'est pas lui ! », « Le voilà ! ») et la loupe d'un bot
// qui cherche à sa place.
#pragma once

#include "../Moteur/Moteur.h"
#include "../Regles/Joueur.h"

namespace DonjonDeLOie::Interface
{
    class EcranEvade
    {
    public:
        /// La largeur prise à gauche par l'avis (unités de l'interface, référence 1920 × 1080) : la caméra décale le
        /// diorama d'autant vers la droite.
        static constexpr float LargeurAvis = 440.0f;
        /// Les bandes prises en haut (le sablier) et en bas (la consigne), mêmes unités.
        static constexpr float HauteurSablier = 116.0f, HauteurConsigne = 76.0f;

        EcranEvade(const Ref<RectTransform>& racine_, const Ref<Canvas>& canvas_);

        bool Ouvert() const { return p != nullptr; }

        void Cacher();

        /// « icone » : l'icône du portrait ; « chercheur » : le nom du héros (pour un bot) ; humain : consigne et clics.
        void Montrer(const Ptr<Regles::Quete>& qu, const string& icone, const string& chercheur, bool humain, Color couleur);

        void Temps(float reste, float total);

        void Message(const string& s, Color c);

        /// À chaque image : le message apparaît, reste, s'efface.
        void Animer(float dt);

        /// La loupe du bot sur ce point de l'écran (en pixels) ; « montrer » faux : cachée.
        void Loupe(Vector2 ecran, bool montrer);

    private:
        const Ref<RectTransform> racine;
        const Ref<Canvas> canvas;
        Ref<RectTransform> p, loupe;
        Ref<Image> barre;
        Ref<Text> temps, message;
        Ref<CanvasGroup> messageGroupe;
        float messageT = 99.0f;
    };
}
