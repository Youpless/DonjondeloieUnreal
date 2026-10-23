// Donjon de l'Oie — une case de la carte, posée dans la scène de l'éditeur de cartes. On la déplace comme n'importe quel
// objet (outil Déplacer, touche W) ; son inspecteur règle son type, sa zone, ses événements, son bâtiment et ses liens.
// En jeu, ces marqueurs ne s'affichent pas : la carte est lue une fois au lancement.
#pragma once

#include "../Moteur/Moteur.h"
#include "../Regles/Donnees.h"
#include "../Regles/Plateau.h"

namespace DonjonDeLOie::Cartes
{
    class DDEditeurCarte;

    /// [DisallowMultipleComponent]
    class DDCaseMarqueur : public MonoBehaviour
    {
        DD_TYPE_OBJET(DDCaseMarqueur, MonoBehaviour)
    public:
        /// Ce qui se passe quand on s'arrête sur la case.
        Regles::TypeCase Type = Regles::TypeCase::Chemin;
        /// La région : décor, monstres tirés, couleurs du sol.
        Regles::Zone Zone = Regles::Zone::Village;
        /// Nom du lieu (cible des quêtes de voyage). Vide : une case ordinaire.
        string NomLieu;
        /// Nom du chemin affiché quand cette case ouvre un embranchement.
        string Voie;
        /// Cases suivantes. La première est la grande route ; les autres sont des embranchements.
        Liste<Ref<DDCaseMarqueur>> Suivants;
        /// Standard : l'effet du type. Remplacer : un de ses événements à la place. Ajouter : l'effet du type, puis un de ses événements.
        Regles::ModeEvenements Mode = Regles::ModeEvenements::Standard;
        /// Identifiants des événements propres à la case (voir la fenêtre du catalogue).
        Liste<string> Evenements;
        /// Bâtiment posé à côté : vide = automatique selon le type, « - » = aucun, sinon l'id d'un décor.
        string Lieu;

        static Color CouleurType(Regles::TypeCase t);

        /// Hauteur du sol en (x, z) quand l'aperçu de l'île est affiché dans l'éditeur (sinon null) : les marqueurs se
        /// dessinent alors sur le relief au lieu d'être cachés dessous.
        static inline std::function<float(float, float)> HauteurSol;

        static Vector3 PositionDessin(Vector3 p);

    private:
        Ref<DDEditeurCarte> Racine() const;

        /// Les gizmos (Gizmos.DrawCube…, Handles.Label sous UNITY_EDITOR) ne se dessinent que dans l'éditeur d'Unity :
        /// rien de tel ici, la méthode est gardée mais ne fait rien (le moteur ne l'appelle d'ailleurs jamais).
        void OnDrawGizmos();

        /// Une flèche de gizmo entre deux cases : ne fait rien non plus (voir OnDrawGizmos).
        static void Fleche(Vector3 a, Vector3 b);
    };
}
