// Donjon de l'Oie — survol d'un élément d'interface : appelle une action à l'entrée et à la sortie du pointeur.
#include "Survol.h"

namespace DonjonDeLOie::Interface
{
    void Survol::OnPointerEnter(PointerEventData& e) { if (Entree) Entree(); }
    void Survol::OnPointerExit(PointerEventData& e) { if (Sortie) Sortie(); }
}
