// Donjon de l'Oie — lumière et atmosphère : soleil d'après-midi, ciel en dégradé, brume légère, post-traitement d'URP
// (halo, tons ACES, couleurs, vignette) posé par réflexion pour que le jeu compile aussi sans URP.
//
// Portage : le C# règle les volumes d'URP par réflexion ; dans le moteur des essais C# ces types n'existent pas, si
// bien que son post-traitement ne crée aucun objet et ne règle rien. Ici, les mêmes réglages sont gardés dans
// Ambiance::PostTraitement (des valeurs simples), que la couche Unreal applique à chaque image ; aucun GameObject
// n'est créé pour eux, comme dans les essais C#.
#pragma once

#include "../Moteur/Moteur.h"

namespace DonjonDeLOie::Monde
{
    struct Ambiance
    {
        static constexpr Color Horizon = Color(0.80f, 0.86f, 0.89f);
        static constexpr Color Zenith = Color(0.36f, 0.56f, 0.80f);
        static inline Ref<Light> Soleil;
        static float NuitActuelle() { return nuitActuelle; }           // 0 : plein jour … 1 : nuit noire
        static inline Color CielHaut = Zenith, CielBas = Horizon;        // couleurs actuelles du fond (captures hors Unity)

        /// Le flou de profondeur en cours (force 0 : éteint) ; les captures hors Unity l'imitent.
        static float ForceFlou() { return forceFlou; }
        static float DebutFlou() { return debutFlou; }
        static float FinFlou() { return finFlou; }

        /// Les réglages du post-traitement (les volumes d'URP du C#), que la couche Unreal applique à chaque image.
        /// Valeurs de départ : celles que le C# pose à l'installation ; Nuit() et Flou() les font varier comme le C#
        /// règle ses volumes. Tons : ACES, toujours.
        struct ReglagesPostTraitement
        {
            float Halo = 0.55f, SeuilHalo = 1.05f, DiffusionHalo = 0.62f;          // Bloom : intensity, threshold, scatter
            float Exposition = 0.35f;                                              // ColorAdjustments : postExposure
            float Contraste = 12.0f;                                               //                    contrast
            float Saturation = 10.0f;                                              //                    saturation
            Color Filtre = Color::white;                                           //                    colorFilter
            float Temperature = 6.0f;                                              // WhiteBalance : temperature
            float Vignette = 0.22f, DouceurVignette = 0.5f;                        // Vignette : intensity, smoothness
            // DepthOfField gaussien : FlouForce 0 = éteint (mode Off), sinon la force (0..1 ; rayon maximal
            // Lerp(0.5, 1.5, force)) ; net jusqu'à FlouDebut mètres de la caméra, flou complet à FlouFin.
            float FlouForce = 0.0f, FlouDebut = 0.0f, FlouFin = 0.0f;
        };
        /// (défini juste après la classe : une structure imbriquée à valeurs par défaut ne peut pas servir de
        /// variable statique avant la fin de la classe qui l'entoure.)
        static ReglagesPostTraitement PostTraitement;

        /// Flou de profondeur gaussien : net jusqu'à « debut » mètres de la caméra, flou complet à « fin ». force 0 : éteint.
        /// La vignette se resserre un peu avec lui.
        static void Flou(float force, float debut, float fin);

        static void Installer(const Ref<Transform>& parent, const Ref<Camera>& cam);

        /// Du jour (0) à la nuit (1) : le soleil devient lune, la lumière ambiante bleuit, la brume s'assombrit, les lampes
        /// brillent davantage, et le post-traitement d'URP refroidit l'image.
        static void Nuit(float k, const Ref<Camera>& cam);

    private:
        static inline float nuitActuelle = 0.0f;
        static inline Ref<Material> cielMat;
        static inline Ref<GameObject> lune;
        // (static object ajustements, balance, profondeur, vignette : les volumes d'URP du C#, remplacés par PostTraitement)
        static inline float giFait = -1.0f;
        static inline float forceFlou = 0.0f, debutFlou = 0.0f, finFlou = 0.0f;

        /// Dôme du ciel : dégradé du zénith à l'horizon, sans éclairage.
        static void Ciel(const Ref<Transform>& parent);

        // ------------------------------------------------------------------ post-traitement (URP, par réflexion)
        // (T et Regler, les outils de réflexion du C#, disparaissent : chaque Regler(…) écrit le champ de PostTraitement.)

        /// Le PostTraitement(parent, cam) du C#, renommé (PostTraitement est ici la variable des réglages) : remet les
        /// réglages à leurs valeurs de départ ; ne crée aucun objet (comme le C# sans URP).
        static void InstallerPostTraitement(const Ref<Transform>& parent, const Ref<Camera>& cam);

        /// Remise des variables statiques à leur valeur de départ (Boucle::Reinitialiser) : inscrite dans le .cpp.
        static const bool remiseAZero;
    };

    inline Ambiance::ReglagesPostTraitement Ambiance::PostTraitement;

    class SuivreCamera : public MonoBehaviour
    {
        DD_TYPE_OBJET(SuivreCamera, MonoBehaviour)

        void LateUpdate() override;
    };
}
