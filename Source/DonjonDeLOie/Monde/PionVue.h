// Donjon de l'Oie — le pion d'un joueur : son héros animé, un socle à sa couleur, ses déplacements de case en case.
#pragma once

#include "../Moteur/Moteur.h"
#include "../Regles/Joueur.h"
#include "Animateur.h"
#include "RigVue.h"

namespace DonjonDeLOie::Monde
{
    class PionVue : public MonoBehaviour
    {
        DD_TYPE_OBJET(PionVue, MonoBehaviour)
    public:
        int Joueur = 0;
        Ref<RigVue> Rig;
        Ref<Animateur> Anim;
        Color Couleur;

        static Ref<PionVue> Creer(const Ptr<Regles::Joueur>& j, const Ref<Transform>& parent);

        /// Le joueur dont c'est le tour a une colonne de lumière à sa couleur.
        void Actif(bool oui);

        void Regarder(Vector3 point);

        /// Saut d'une case à l'autre (arc), en durée fixe.
        Tache Sauter(Vector3 vers, float duree, float hauteur = 0.55f);

        /// Glisser doucement vers une place (quand plusieurs pions se partagent une case).
        Tache Glisser(Vector3 vers, float duree);

        /// Disparaître dans un éclair puis réapparaître ailleurs.
        Tache Teleporter(Vector3 vers);

    private:
        Ref<GameObject> socle, halo;
        Vector3 regard = Vector3::back;

        void Update() override;
    };
}
