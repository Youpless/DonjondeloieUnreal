// Donjon de l'Oie — l'île dans la scène : terrain, mer, dalles, décor, lieux, lampes, et les marqueurs du plateau
// (cases à choisir, pièges, embuscades, cible de quête).
#pragma once

#include "../Moteur/Moteur.h"
#include "../Regles/Joueur.h"
#include "IleModele.h"

namespace DonjonDeLOie::Monde
{
    class RigVue;

    class IleVue : public MonoBehaviour
    {
        DD_TYPE_OBJET(IleVue, MonoBehaviour)
    public:
        Ptr<IleModele> Modele;
        Liste<Ref<Transform>> Dalles;
        Ref<RigVue> Dragon;

    private:
        Ref<Transform> racineDecor, racineMarqueurs;

    public:
        static Ref<IleVue> Construire(const Ptr<IleModele>& m, const Ref<Transform>& parent);

    private:
        static Ref<GameObject> Objet(const string& nom, const Ref<Transform>& parent, const Ref<Mesh>& mesh, const Ref<Material>& mat, bool ombres, bool recoit = true);

        /// Les maillages faits pour cette île seulement (terrain, mer, dalles) : détruits avec elle. Ceux des modèles
        /// voxel sont gardés en cache par DDMateriaux et servent à toutes les îles.
        Liste<Ref<Mesh>> maillagesPropres;

        Ref<Mesh> Propre(const Ref<Mesh>& m);

        void OnDestroy() override;

        void Batir();

    public:
        /// Le plus haut sommet du décor (ou du terrain) dans un rayon autour d'un point : le diorama du mini-jeu de
        /// l'évadé flotte au-dessus.
        float SommetAutour(Vector3 p, float rayon) const;

    private:
        static void Poser(const Ref<Transform>& t, const Placement& p);

        // ------------------------------------------------------------------ positions

    public:
        Vector3 PositionCase(int c) const;

        /// Place d'un pion sur une case partagée (jusqu'à six autour du centre).
        Vector3 Emplacement(int c, int rang, int total) const;

        // ------------------------------------------------------------------ marqueurs

    private:
        Liste<Ref<GameObject>> choix;
        Dico<int, Ref<GameObject>> pieges;
        Dico<int, Ref<GameObject>> embuscades;
        Ref<GameObject> quete;

    public:
        /// Allume des anneaux sur les cases proposées (choix d'embranchement, de piège, de case d'embuscade…).
        void MontrerChoix(const Liste<int>& cases, Color c);

        void EffacerChoix();

    private:
        Ref<GameObject> survol;
        int caseSurvolee = -1;

    public:
        /// Met en valeur une case (survol d'une option liée à une case) ; -1 pour effacer.
        void Survoler(int c);

        /// Pièges : visibles seulement pour leur poseur (et pour un voleur).
        void MontrerPieges(const Liste<Ptr<Regles::Piege>>& liste, int joueurQuiRegarde, bool voitTout);

        /// Embuscades : deux cases teintées de la couleur du joueur embusqué, visibles de tous.
        void MontrerEmbuscades(const Liste<Ptr<Regles::Joueur>>& joueurs);

        /// Le lieu à atteindre de la quête du joueur qui joue (ou rien).
        void MontrerQuete(int caseCible);
    };

    /// Vacillement doux d'une lampe (flammes, lave).
    class Vacillement : public MonoBehaviour
    {
        DD_TYPE_OBJET(Vacillement, MonoBehaviour)
    public:
        float Base = 1.0f;
        static inline float Facteur = 1.0f;                     // la nuit, les lampes brillent davantage (Ambiance.Nuit)

    private:
        Ref<Light> l;
        float graine = 0.0f;
        void Start() override;
        void Update() override;
    };

    class Pulsation : public MonoBehaviour
    {
        DD_TYPE_OBJET(Pulsation, MonoBehaviour)

        Vector3 echelle;
        float graine = 0.0f;
        void Start() override;
        void Update() override;
    };

    class Flotter : public MonoBehaviour
    {
        DD_TYPE_OBJET(Flotter, MonoBehaviour)

        Vector3 basePos;
        float graine = 0.0f;
        void Start() override;
        void Update() override;
    };
}
