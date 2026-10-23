// Donjon de l'Oie — le tableau d'un combat : les deux adversaires, leur attaque, leurs dés, les totaux et le verdict.
#pragma once

#include "../Moteur/Moteur.h"
#include "../Regles/Partie.h"

namespace DonjonDeLOie::Interface
{
    class FenetreCombat
    {
    public:
        explicit FenetreCombat(const Ref<RectTransform>& racine_);

        void Cacher();

        void Montrer(const Ptr<Regles::SCombat>& c, const Ptr<Regles::Partie>& partie);

        void Des(const Ptr<Regles::SCombat>& c);

        void Verdict(const Ptr<Regles::SCombat>& c);

    private:
        const Ref<RectTransform> racine;
        Ref<RectTransform> p;
        Ref<Text> deA, deB, totA, totB, verdict;
        Ref<Image> fondA, fondB;

        /// (« out » de C# : de, total et fond reçoivent les éléments créés.)
        Ref<Text> Cote(const string& nom, Color c, const string& attaque, float x, Ref<Text>& de, Ref<Text>& total, Ref<Image>& fond);
    };
}
