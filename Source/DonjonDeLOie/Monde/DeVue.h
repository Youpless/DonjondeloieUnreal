// Donjon de l'Oie — un dé en voxels lancé sur le plateau : il roule, rebondit et s'arrête sur la valeur tirée par les règles.
// Le D6 est ivoire à points rouges ; le D3 des duels est doré, ses faces portent 1, 2 et 3 points (deux fois chacune).
#pragma once

#include "../Moteur/Moteur.h"
#include "../Voxel/VoxelModel.h"

namespace DonjonDeLOie::Monde
{
    class DeVue : public MonoBehaviour
    {
        DD_TYPE_OBJET(DeVue, MonoBehaviour)

        static constexpr int N = 10;                    // voxels par arête
        static constexpr float Taille = 0.05f;          // mètres par voxel : dé de 50 cm
        static inline Ref<Mesh> d6, d3;

        // Faces : normale → valeur. D6 : faces opposées = 7.
        // (Vector3.up, Vector3.forward, Vector3.right, Vector3.left, Vector3.back, Vector3.down)
        static constexpr Vector3 Normales[6] = {Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector3(1.0f, 0.0f, 0.0f),
                                                Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, -1.0f, 0.0f)};
        static constexpr int ValeursD6[6] = {1, 2, 3, 4, 5, 6};
        static constexpr int ValeursD3[6] = {1, 2, 3, 3, 2, 1};

    public:
        // (nbFaces : le paramètre « faces » du C#, renommé ici et dans NormalePour et Creer pour ne pas masquer le champ.)
        static Ref<Mesh> Maillage(int nbFaces);

    private:
        /// Creuse les points d'une face (points de 2 × 2 voxels, sur une grille 3 × 3).
        static void Points(Voxel::VoxelModel& m, Vector3 n, int v, Color32 c);

    public:
        static Vector3 NormalePour(int nbFaces, int valeur);

        static Ref<DeVue> Creer(const Ref<Transform>& parent, int nbFaces);

    private:
        int faces = 0;
        bool fini = false;

    public:
        bool Fini() const { return fini; }

        /// Lance le dé depuis « depart » jusqu'à « arrivee » (sol) et le fait s'arrêter sur « valeur ».
        Tache Lancer(Vector3 depart, Vector3 arrivee, int valeur, float duree = 1.1f);

    private:
        /// Remise des variables statiques à leur valeur de départ (Boucle::Reinitialiser) : inscrite dans le .cpp.
        static const bool remiseAZero;
    };
}
