// Donjon de l'Oie — un décor ou un bâtiment posé à la main dans l'éditeur de cartes (tour, ruine, rocher, arbre…).
#include "DDDecorMarqueur.h"

namespace DonjonDeLOie::Cartes
{
    // Les gizmos (Gizmos.DrawWireSphere du rayon du lieu, IleModele.RayonLieu, la ligne de son orientation et, sous
    // UNITY_EDITOR, l'étiquette Handles.Label) ne se dessinent que dans la vue de scène de l'éditeur d'Unity : il n'y a
    // rien de tel dans le jeu ni dans Unreal. La méthode est gardée pour suivre le C#, mais ne fait rien (et le moteur
    // ne l'appelle jamais).
    void DDDecorMarqueur::OnDrawGizmos() {}
}
