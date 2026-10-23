// Donjon de l'Oie — le catalogue modifiable : équipements, monstres, classes, événements et réglages, dans un fichier
// Assets/DonjonDeLOie/Resources/DDCatalogue.asset. On le modifie avec la fenêtre « Donjon de l'Oie › Catalogue ».
// Au lancement, le jeu le lit et remplace les valeurs écrites dans Regles/Donnees.cpp. Sans ce fichier, rien ne change.
#include "DDCatalogue.h"

namespace DonjonDeLOie::Cartes
{
    /// Appelé par Unity quand on choisit « Reset » dans l'inspecteur du fichier : les valeurs d'origine.
    void DDCatalogue::Reset() { RemplirParDefaut(); }

    /// Remplit le catalogue avec les valeurs écrites dans le code.
    void DDCatalogue::RemplirParDefaut()
    {
        Objets = Regles::Donnees::CopieObjetsParDefaut();
        Monstres = Regles::Donnees::CopieMonstresParDefaut();
        Classes = Regles::Donnees::CopieClassesParDefaut();
        Evenements = Regles::EvenementsParDefaut::Creer();
        Reglages = Nouveau<Regles::DefReglages>();
    }

    /// Met ce catalogue en service (les règles lisent ensuite ces valeurs).
    void DDCatalogue::Appliquer()
    {
        Regles::Donnees::Reinitialiser();
        Regles::Donnees::Appliquer(&Classes, &Monstres, &Objets, &Evenements, Reglages);
    }

    /// Lit le catalogue du dossier Resources et le met en service. Rend false s'il n'existe pas (valeurs du code).
    bool DDCatalogue::Charger()
    {
        auto c = Resources::Load<DDCatalogue>(CheminResources);
        if (c == nullptr)
        {
            Regles::Donnees::Reinitialiser();
            return false;
        }
        c->Appliquer();
        return true;
    }
}
