// Donjon de l'Oie — les règles du jeu, en six pages, et l'écran de victoire.
#pragma once

#include "../Moteur/Moteur.h"
#include "../Regles/Partie.h"

namespace DonjonDeLOie::Interface
{
    class EcranRegles
    {
    public:
        Ref<RectTransform> R;

        EcranRegles(const Ref<RectTransform>& racine, std::function<void()> fermer);

        bool Ouvert() const { return R->gameObject()->activeSelf(); }

        void Visible(bool v);

    private:
        Ref<Text> titre, corps;
        Liste<Ref<Button>> onglets;

        /// (un « static readonly string[] » en C#)
        static const Liste<string>& Titres();

        static Liste<string> Pages();

        /// Le Dragonnoie, avec les valeurs du catalogue (vide s'il est désactivé).
        static string TexteDragonnoie();

        /// La nuit et ses événements, selon les réglages du catalogue.
        static string TexteNuit();

        static string TexteQuetes();

        /// (« sb » : le StringBuilder de C#.)
        static void Lister(string& sb, Regles::TypeObjet t);

        void Page(int i, const Liste<string>& pages);
    };

    class EcranVictoire
    {
    public:
        Ref<RectTransform> R;

        EcranVictoire(const Ref<RectTransform>& racine, const Ptr<Regles::Partie>& p, std::function<void()> rejouer, std::function<void()> menu);

        void Detruire();
    };
}
