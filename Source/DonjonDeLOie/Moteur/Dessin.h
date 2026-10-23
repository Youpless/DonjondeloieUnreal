// Donjon de l'Oie — ce que la couche d'affichage lit pour peindre l'interface : une liste ordonnée de commandes
// (des quadrilatères texturés et des lignes de texte), en pixels d'écran, origine en bas à gauche. Le moteur y fait
// tout le travail d'uGUI : ancres et pivots, échelles, images découpées en neuf, jauges, transparences de groupe,
// teintes des boutons, mise en page des textes riches (gras, italique, taille, couleur), retours à la ligne,
// alignements, ombres et contours. La couche Unreal n'a plus qu'à dessiner.
#pragma once

#include "UI.h"

namespace Moteur
{
    namespace Dessin
    {
        /// La mesure des textes, fournie par la couche d'affichage (les polices sont les siennes).
        class MesureurTexte
        {
        public:
            virtual ~MesureurTexte() = default;
            /// Largeur en pixels d'un morceau de texte d'une ligne (UTF-8), à cette taille (hauteur de corps en pixels).
            virtual float Largeur(const string& texte, float taillePx, bool gras, bool italique) = 0;
            /// Hauteur d'une ligne (interligne simple) pour cette taille.
            virtual float HauteurLigne(float taillePx) = 0;
        };

        /// Une estimation sans police (essais) : largeurs moyennes des caractères d'une police sans empattement.
        class MesureurApproximatif : public MesureurTexte
        {
        public:
            float Largeur(const string& texte, float taillePx, bool gras, bool italique) override;
            float HauteurLigne(float taillePx) override { return taillePx * 1.15f; }
        };

        /// La mesure utilisée par Text.preferredWidth / preferredHeight (la couche d'affichage la remplace par la vraie).
        MesureurTexte& MesureCourante();
        void ChoisirMesure(MesureurTexte* m);

        struct Sommet
        {
            Vector2 position;       // pixels, origine en bas à gauche
            Vector2 uv;             // 0..1, v = 0 en bas de la texture
            Color couleur;          // couleur finale (teinte × alpha des groupes), espace sRGB comme dans Unity
        };

        /// Un morceau de texte d'une seule ligne, d'un seul style.
        struct Morceau
        {
            string texte;
            Vector2 position;       // coin haut-gauche de la boîte de ligne, en pixels (origine en bas à gauche)
            float taillePx = 14.0f;
            bool gras = false, italique = false;
            Color couleur;
        };

        struct Commande
        {
            enum class Sorte { Quads, Texte };
            Sorte sorte = Sorte::Quads;
            // Quads : des quadrilatères (4 sommets chacun, dans l'ordre bas-gauche, haut-gauche, haut-droit, bas-droit)
            Ref<Texture2D> texture;          // null : blanc uni
            Liste<Sommet> sommets;
            // Texte : des morceaux (les ombres et contours sont des morceaux de plus, dessinés avant)
            Liste<Morceau> morceaux;
        };

        /// La liste de dessin de toutes les toiles actives, pour un écran de cette taille.
        void Interface(int largeurPx, int hauteurPx, MesureurTexte& mesure, Liste<Commande>& sortie);

        /// L'élément d'interface le plus haut sous ce point (pixels, origine en bas à gauche) qui reçoit les clics
        /// (raycastTarget, groupes qui bloquent), ou null.
        Ref<Graphic> ElementSous(Vector2 pointPx);

        /// Le rectangle d'un élément en pixels d'écran (après ancres, pivots et échelles), calculé lors du dernier
        /// passage de la mise en page.
        Rect RectEcran(const Ref<RectTransform>& r);

        /// Refait la mise en page (rectangles et matrices) pour un écran de cette taille ; Interface() et ElementSous()
        /// l'appellent au besoin.
        void MettreEnPage(int largeurPx, int hauteurPx);
    }
}
