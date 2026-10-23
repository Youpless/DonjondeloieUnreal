// Donjon de l'Oie — survol d'un élément d'interface : appelle une action à l'entrée et à la sortie du pointeur.
#pragma once

#include "../Moteur/Moteur.h"

namespace DonjonDeLOie::Interface
{
    class Survol : public MonoBehaviour, public IPointerEnterHandler, public IPointerExitHandler
    {
        DD_TYPE_OBJET(Survol, MonoBehaviour)
    public:
        std::function<void()> Entree, Sortie;

        void OnPointerEnter(PointerEventData& e) override;
        void OnPointerExit(PointerEventData& e) override;

        // les interfaces de pointeur, pour le système d'événements du moteur
        IPointerEnterHandler* GestionnaireEntree() override { return this; }
        IPointerExitHandler* GestionnaireSortie() override { return this; }
    };
}
