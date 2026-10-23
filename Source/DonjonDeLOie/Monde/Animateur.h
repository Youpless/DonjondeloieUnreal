// Donjon de l'Oie — animation procédurale des rigs voxel : respiration, marche, battements d'ailes, gestes (attaque,
// coup reçu, victoire, K.O.). Chaque pièce est retrouvée par son nom ; celles qui manquent sont ignorées.
#pragma once

#include "../Moteur/Moteur.h"
#include "RigVue.h"

namespace DonjonDeLOie::Monde
{
    enum class Geste { Aucun, Attaque, Touche, Victoire, Ko, Joie, Sort, Lancer };

    class Animateur : public MonoBehaviour
    {
        DD_TYPE_OBJET(Animateur, MonoBehaviour)
    public:
        Ref<RigVue> Rig;
        bool EnMarche = false;           // pendant un saut d'une case à l'autre
        bool Flotte = false;             // spectre, feu follet : ondule au lieu de respirer
        bool AilesActives = false;       // battement permanent (wyverne, dragon)
        float Vitesse = 1.0f;
        float Graine = 0.0f;

        static Ref<Animateur> Sur(const Ref<RigVue>& rv);

        void Jouer(Geste g, float duree = -1.0f);

        void Relever();

        bool EnGeste() const { return geste != Geste::Aucun && tGeste < dureeGeste; }

    private:
        Geste geste = Geste::Aucun; float tGeste = 0.0f, dureeGeste = 0.0f; bool koTenu = false;
        float t = 0.0f, marche = 0.0f;
        float echelle = 1.0f;

        Ref<Transform> corps, tete, brasG, brasD, jambeG, jambeD, arme, cape, machoire, aileG, aileD, queue, avG, avD, arG, arD, feuillage, bouclier;

        void Prendre();

        static Quaternion E(float x, float y, float z);

        void Poser(const Ref<Transform>& p, Quaternion rot, Vector3 decalage);

        void LateUpdate() override;
    };
}
