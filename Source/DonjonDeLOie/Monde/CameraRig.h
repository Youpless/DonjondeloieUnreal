// Donjon de l'Oie — la caméra : suit le joueur actif en vue plongeante, cadre les combats, montre tout le plateau.
// Molette : zoom · clic droit glissé (ou flèches gauche / droite) : tourner · clic milieu glissé : déplacer · Espace : vue d'ensemble.
#pragma once

#include "../Moteur/Moteur.h"

namespace DonjonDeLOie::Monde
{
    class CameraRig : public MonoBehaviour
    {
        DD_TYPE_OBJET(CameraRig, MonoBehaviour)
    public:
        Ref<Camera> Cam;
        Vector3 Cible;              // point regardé (lissé)
        float Distance = 13.0f, Lacet = 0.0f, Tangage = 50.0f;
        bool VueEnsemble() const { return vueEnsemble_; }
        bool Suit() const { return suivi != nullptr; }     // suit-elle un pion (ou le Dragonnoie en vol) ?
        bool CommandesActives = true;
        Vector3 CentreCarte = Vector3(0.0f, 0.0f, -2.0f);   // milieu de l'île jouée
        float TailleCarte = 80.0f;                          // sa plus grande dimension (terre), en mètres
        /// Le champ de vision habituel (degrés) ; le mini-jeu de l'évadé le resserre pour une vue presque isométrique.
        static constexpr float ChampNormal = 38.0f;

        static Ref<CameraRig> Creer(const Ref<Transform>& parent);

        void Suivre(const Ref<Transform>& t, float distance = 13.0f);

        void Regarder(Vector3 p, float distance, float tangage = 50.0f);

        /// Cadre deux points (duel, combat contre un monstre), vue un peu plus rasante.
        void Cadrer(Vector3 a, Vector3 b);

        /// Place la caméra exactement, champ compris (une mise en scène image par image : le zoom du mini-jeu de l'évadé).
        void Fixer(Vector3 cible, float distance, float lacet, float tangage, float champ);

        /// Le champ de vision redevient l'habituel, tout de suite.
        void RetablirChamp();

        /// Fait tourner lentement la caméra (écran titre).
        void Orbiter(float degresParSeconde);

        void BasculerEnsemble();

        void Placer();

        /// Position écran d'un point du monde (pour les bulles et les étiquettes). z < 0 : derrière la caméra.
        Vector3 Ecran(Vector3 p) const;

    private:
        Vector3 cibleVoulue; float distanceVoulue = 13.0f, lacetVoulu = 0.0f, tangageVoulu = 50.0f;
        Ref<Transform> suivi; Vector3 decalageLibre;
        bool vueEnsemble_ = false;            // la propriété VueEnsemble { get; private set; } du C#
        float distanceAvantEnsemble = 13.0f;
        float champVoulu = ChampNormal;

        void LateUpdate() override;

        void Appliquer();
    };
}
