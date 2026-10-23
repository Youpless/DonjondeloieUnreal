// Donjon de l'Oie — particules en petits cubes, toutes dans un seul maillage rebâti à chaque image (peu d'objets).
// Deux systèmes : un additif (étincelles, magie, or) et un en transparence (fumée, poussière).
#pragma once

#include "../Moteur/Moteur.h"

namespace DonjonDeLOie::Monde
{
    class Etincelles : public MonoBehaviour
    {
        DD_TYPE_OBJET(Etincelles, MonoBehaviour)
    public:
        // (estAdditif : le paramètre « additif » du C#, renommé pour ne pas masquer le champ.)
        static Ref<Etincelles> Creer(const Ref<Transform>& parent, bool estAdditif);

        void Emettre(Vector3 pos, Vector3 vel, Color col, float duree, float taille, float gravite = 0.0f, float tournoie = 0.0f);

        // ------------------------------------------------------------------ recettes

        void Explosion(Vector3 p, Color col, int nombre, float vitesse, float taille = 0.07f, float gravite = 4.0f);

        void Pieces(Vector3 p, int nombre);

        /// Spirale montante (niveau gagné, soin, bénédiction).
        void Spirale(Vector3 p, Color col, int nombre, float hauteur = 2.4f);

        void Fumee(Vector3 p, Color col, int nombre, float rayon = 0.5f);

        /// Un jet d'un point à un autre (sorts lancés).
        void Trait(Vector3 de, Vector3 vers, Color col, int nombre);

    private:
        struct P
        {
            Vector3 pos, vel;
            float vie = 0.0f, duree = 0.0f, taille = 0.0f, gravite = 0.0f, tournoie = 0.0f;
            Color32 c;
            Vector3 centre;
        };
        Liste<P> ps;
        Ref<Mesh> mesh;
        Liste<Vector3> v;
        Liste<Vector3> n;
        Liste<Color32> c;
        Liste<int> t;
        bool additif = false;

        // (couleur : le paramètre « c » du C#, renommé pour ne pas masquer le champ c.)
        static Color Teinte(Color couleur);

        // ------------------------------------------------------------------ animation

        void Update() override;

        static constexpr Vector3 Coins[8] = {
            Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, -1.0f), Vector3(-1.0f, 1.0f, -1.0f),
            Vector3(-1.0f, -1.0f, 1.0f), Vector3(1.0f, -1.0f, 1.0f), Vector3(1.0f, 1.0f, 1.0f), Vector3(-1.0f, 1.0f, 1.0f)};
        static constexpr int Faces[36] = {0, 2, 1, 0, 3, 2, 4, 5, 6, 4, 6, 7, 0, 1, 5, 0, 5, 4, 3, 7, 6, 3, 6, 2, 0, 4, 7, 0, 7, 3, 1, 2, 6, 1, 6, 5};

        void Cube(Vector3 p, float s, Color32 col);
    };
}
