// Donjon de l'Oie — les matériaux, créés au lancement à partir des shaders du dossier Resources.
// Si un shader n'est pas pris en charge (autre pipeline, erreur de compilation), un shader d'Unity prend le relais.
#pragma once

#include "../Moteur/Moteur.h"
#include "../Voxel/VoxelMesher.h"

namespace DonjonDeLOie::Monde
{
    struct DDMateriaux
    {
        // (propriété C# Voxel)
        static Ref<Material> Voxel();

        /// Matériau lumineux. intensite : 1 = couleur telle quelle, 2–4 = rayonne avec le halo. pulsation : vitesse du battement (0 = fixe).
        static Ref<Material> Lueur(float intensite = 2.2f, float pulsation = 0.0f);

        /// Effets additifs (étincelles, halos).
        static Ref<Material> FxAdd();

        /// Effets en transparence (fumée, ombres, anneaux pleins).
        static Ref<Material> FxAlpha();

        /// Marqueurs toujours visibles, même derrière le décor (cases à choisir).
        static Ref<Material> FxDessus();

        // (propriété C# Eau)
        static Ref<Material> Eau();

        /// Maillage Unity à partir d'un MeshData (index 32 bits au-delà de 65 000 sommets).
        static Ref<Mesh> Maillage(const Ptr<DonjonDeLOie::Voxel::MeshData>& md, const string& nom);

        /// Maillage d'une pièce de rig, gardé en cache par clé (les mêmes modèles ne sont maillés qu'une fois).
        /// cle vide : pas de cache (le « null » de C#).
        static Ref<Mesh> MaillagePiece(const string& cle, const Ptr<DonjonDeLOie::Voxel::VoxelPart>& p, float taille);

    private:
        static inline Ref<Material> voxel, fxAdd, fxAlpha, eau, fxDessus;
        static inline Dico<int, Ref<Material>> lueurs;

        // (params string[] noms)
        static Ref<Shader> Trouver(const Liste<string>& noms);
        static Ref<Material> Fx(bool additif, bool pardessus);

        static inline Dico<string, Ref<Mesh>> cache;

        /// Remise des variables statiques à leur valeur de départ (Boucle::Reinitialiser) : inscrite dans le .cpp.
        static const bool remiseAZero;
    };
}
