// Donjon de l'Oie — AleaNet : le générateur System.Random de .NET, à l'identique (même graine, mêmes tirages). Les règles,
// les bots, l'île et les modèles voxel tirent leurs nombres ici : une partie lancée avec la même graine se déroule
// exactement comme dans la version C# du jeu.
#pragma once

namespace Moteur
{
    class AleaNet
    {
    public:
        explicit AleaNet(int graine);
        /// Sans graine : graine tirée de l'horloge (comme « new Random() »).
        AleaNet();
        AleaNet(const AleaNet&) = delete;               // comme une référence C# : on le passe par référence, jamais par copie
        AleaNet& operator=(const AleaNet&) = delete;
        AleaNet(AleaNet&&) = default;
        AleaNet& operator=(AleaNet&&) = default;

        /// Un entier dans [0, 2^31 − 1[.
        int Next();
        /// Un entier dans [0, max[ (0 si max vaut 0).
        int Next(int max);
        /// Un entier dans [min, max[.
        int Next(int min, int max);
        /// Un réel dans [0, 1[.
        double NextDouble();

    private:
        void Initialiser(int graine);
        int Tirage();
        double Echantillon();
        double EchantillonLarge();

        int t[56] = {};
        int suivant = 0, suivantP = 21;
    };
}
