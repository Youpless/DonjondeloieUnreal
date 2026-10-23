// Donjon de l'Oie — une carte enregistrée dans un fichier. Celles du dossier Resources/Cartes sont proposées dans le salon.
#pragma once

#include "../Moteur/Moteur.h"
#include "../Regles/Plateau.h"

namespace DonjonDeLOie::Cartes
{
    class DDCarte : public ScriptableObject
    {
        DD_TYPE_OBJET(DDCarte, ScriptableObject)
    public:
        static constexpr const char* DossierResources = "Cartes";

        Ptr<Regles::CarteDef> Carte = Nouveau<Regles::CarteDef>();
        string Description;                  // [TextArea]
        int NiveauConseille = 10;

        static Liste<Ref<DDCarte>> Toutes();
    };
}
