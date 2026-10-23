// Donjon de l'Oie — l'écran titre (l'île tourne lentement derrière) et le menu pause.
#pragma once

#include "../Moteur/Moteur.h"

namespace DonjonDeLOie::Interface
{
    class EcranTitre
    {
    public:
        Ref<RectTransform> R;

        EcranTitre(const Ref<RectTransform>& racine, std::function<void()> nouvelle, std::function<void()> regles, std::function<void()> quitter);

        void Visible(bool v);

    private:
        Ref<Text> but;

        /// Le but du jeu, selon le catalogue (avec ou sans Dragonnoie).
        static string TexteBut();
    };

    class MenuPause
    {
    public:
        Ref<RectTransform> R;

        MenuPause(const Ref<RectTransform>& racine, std::function<void()> reprendre, std::function<void()> regles, std::function<void()> abandonner);

        bool Ouvert() const { return R->gameObject()->activeSelf(); }

        void Visible(bool v);

    private:
        Ref<Text> musique, effets;

        Ref<Text> Reglage(const Ref<RectTransform>& p, float y, const string& nom, std::function<float()> lire, std::function<void(float)> ecrire);
    };
}
