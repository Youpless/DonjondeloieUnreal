// Donjon de l'Oie — les noms des genres de questions et de quêtes (comme ToString() en C#).
#include "Signaux.h"

namespace DonjonDeLOie::Regles
{
    string NomEnum(GenreQuestion v)
    {
        static const char* n[] = {"Action", "Embranchement", "Rodeur", "Defi", "Monstre", "Boutique", "Quetes", "ChoixCase", "ChoixCible", "Confirmer",
                                  "ChoixEvenement", "Dragonnoie", "Evade"};
        return n[static_cast<int>(v)];
    }

    string NomEnum(Quete::Genre v)
    {
        static const char* n[] = {"Atteindre", "Chasser", "Vaincre", "Embuscade", "Defier", "Evade"};
        return n[static_cast<int>(v)];
    }
}
