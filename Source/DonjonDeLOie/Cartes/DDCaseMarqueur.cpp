// Donjon de l'Oie — une case de la carte, posée dans la scène de l'éditeur de cartes. On la déplace comme n'importe quel
// objet (outil Déplacer, touche W) ; son inspecteur règle son type, sa zone, ses événements, son bâtiment et ses liens.
// En jeu, ces marqueurs ne s'affichent pas : la carte est lue une fois au lancement.
#include "DDCaseMarqueur.h"

#include "DDEditeurCarte.h"

namespace DonjonDeLOie::Cartes
{
    namespace
    {
        // HauteurSol est une variable statique publique : remise à null quand le moteur se réinitialise.
        const bool remiseAZeroDDCaseMarqueur = Boucle::AuReinitialiser([] { DDCaseMarqueur::HauteurSol = nullptr; });
    }

    Color DDCaseMarqueur::CouleurType(Regles::TypeCase t)
    {
        switch (t)
        {
            case Regles::TypeCase::Ville: return Color(0.24f, 0.39f, 0.69f);
            case Regles::TypeCase::Monstre: return Color(0.72f, 0.22f, 0.18f);
            case Regles::TypeCase::Tresor: return Color(0.89f, 0.68f, 0.24f);
            case Regles::TypeCase::Evenement: return Color(0.25f, 0.53f, 0.79f);
            case Regles::TypeCase::Malediction: return Color(0.48f, 0.25f, 0.66f);
            case Regles::TypeCase::Sanctuaire: return Color(0.9f, 0.93f, 0.95f);
            case Regles::TypeCase::Marchand: return Color(0.88f, 0.48f, 0.18f);
            case Regles::TypeCase::Taverne: return Color(0.55f, 0.37f, 0.24f);
            case Regles::TypeCase::Antre: return Color(0.48f, 0.12f, 0.1f);
            default: return Color(0.55f, 0.54f, 0.52f);
        }
    }

    Ref<DDEditeurCarte> DDCaseMarqueur::Racine() const { return GetComponentInParent<DDEditeurCarte>(); }

    Vector3 DDCaseMarqueur::PositionDessin(Vector3 p)
    {
        if (HauteurSol) p.y = Mathf::Max(p.y, HauteurSol(p.x, p.z));
        return p;
    }

    // Les gizmos (Gizmos.DrawCube, DrawWireCube, les flèches vers les cases suivantes et, sous UNITY_EDITOR, l'étiquette
    // Handles.Label) ne se dessinent que dans la vue de scène de l'éditeur d'Unity : il n'y a rien de tel dans le jeu ni
    // dans Unreal. La méthode est gardée pour suivre le C#, mais ne fait rien (et le moteur ne l'appelle jamais).
    void DDCaseMarqueur::OnDrawGizmos() {}

    // Une flèche de gizmo entre deux cases (Gizmos.DrawLine) : ne fait rien non plus.
    void DDCaseMarqueur::Fleche(Vector3, Vector3) {}
}
