// Donjon de l'Oie — une carte enregistrée dans un fichier. Celles du dossier Resources/Cartes sont proposées dans le salon.
#include "DDCarte.h"

namespace DonjonDeLOie::Cartes
{
    Liste<Ref<DDCarte>> DDCarte::Toutes() { return Resources::LoadAll<DDCarte>(DossierResources); }
}
