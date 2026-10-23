// Donjon de l'Oie — un modèle en voxels, construit en code (boîtes, ellipsoïdes, cylindres, cônes, traits).
// Les coordonnées sont des entiers ; un voxel occupe le cube [x, x+1] × [y, y+1] × [z, z+1].
// Convention : +y vers le haut, +z vers l'avant du personnage (il regarde vers +z), +x vers sa gauche… vue de face :
// +x à droite de l'écran.
#pragma once

#include "../Moteur/Maths.h"
#include "../Noyau/Jeu.h"

namespace DonjonDeLOie::Voxel
{
    /// Un voxel lu dans un modèle : sa position et sa couleur.
    struct VoxelLu
    {
        Vector3Int Key;
        Color32 Value;
    };

    class VoxelModel
    {
    public:
        int Count() const { return vox.Count(); }

        void Set(int x, int y, int z, Color32 c) { vox[Cle(x, y, z)] = c; }
        void Remove(int x, int y, int z) { vox.Remove(Cle(x, y, z)); }
        bool Has(int x, int y, int z) const { return vox.ContainsKey(Cle(x, y, z)); }
        bool TryGet(int x, int y, int z, Color32& c) const { return vox.TryGetValue(Cle(x, y, z), c); }
        void Clear() { vox.Clear(); }

        /// Tous les voxels (x, y, z, couleur), dans l'ordre où le jeu C# les parcourait.
        Liste<VoxelLu> Tous() const;

        /// Boîte pleine, bornes incluses, dans n'importe quel ordre.
        void Box(int x0, int y0, int z0, int x1, int y1, int z1, Color32 c);
        /// Boîte creuse (seulement la coque d'épaisseur 1).
        void Shell(int x0, int y0, int z0, int x1, int y1, int z1, Color32 c);
        /// Ellipsoïde plein centré en (cx, cy, cz) — les centres de voxels sont en (x + 0.5, …).
        void Ellipsoid(float cx, float cy, float cz, float rx, float ry, float rz, Color32 c);
        void Sphere(float cx, float cy, float cz, float r, Color32 c) { Ellipsoid(cx, cy, cz, r, r, r, c); }
        /// Cylindre vertical de y0 à y1 inclus.
        void Cylinder(float cx, float cz, int y0, int y1, float r, Color32 c) { Cone(cx, cz, y0, y1, r, r, c); }
        /// Tronc de cône vertical : rayon r0 en bas (y0), r1 en haut (y1).
        void Cone(float cx, float cz, int y0, int y1, float r0, float r1, Color32 c);
        /// Trait épais (capsule) entre deux points, rayon r (0.5 donne un trait d'un voxel).
        void Line(float x0, float y0, float z0, float x1, float y1, float z1, float r, Color32 c) { Line(x0, y0, z0, x1, y1, z1, r, r, c); }
        /// Trait dont le rayon passe de r0 à r1 (cornes, queues, griffes).
        void Line(float x0, float y0, float z0, float x1, float y1, float z1, float r0, float r1, Color32 c);
        /// Copie en miroir : chaque voxel en x va en (2·axe − 1 − x). Avec axe = 0, x = 0 ↔ x = −1.
        void MirrorX(int axe = 0);
        /// Repeint les voxels existants. La fonction reçoit (x, y, z, couleur) et rend la nouvelle couleur.
        void Paint(const std::function<Color32(int, int, int, Color32)>& f);
        /// Retire les voxels pour lesquels le prédicat est vrai.
        void Carve(const std::function<bool(int, int, int)>& f);
        /// Ajoute les voxels d'un autre modèle, décalés.
        void Merge(const VoxelModel& o, int dx, int dy, int dz);
        Ptr<VoxelModel> Clone() const;
        bool Bounds(Vector3Int& min, Vector3Int& max) const;

    private:
        // Clé : 11 bits par axe, coordonnées de −1024 à 1023.
        static constexpr int Decal = 1024;
        static long long Cle(int x, int y, int z)
        {
            return (static_cast<long long>(x + Decal) << 22) + (static_cast<long long>(y + Decal) << 11) + static_cast<long long>(z + Decal);
        }
        static void Decle(long long k, int& x, int& y, int& z)
        {
            x = static_cast<int>((k >> 22) & 2047) - Decal;
            y = static_cast<int>((k >> 11) & 2047) - Decal;
            z = static_cast<int>(k & 2047) - Decal;
        }

        Dico<long long, Color32> vox;
    };

    /// Une pièce animable d'un personnage : un modèle, un pivot (dans le même repère que les voxels), un parent.
    struct VoxelPart
    {
        string Name;
        Ptr<VoxelModel> Model;
        Vector3 Pivot;              // en voxels, repère commun à toutes les pièces
        string Parent;              // vide : racine
    };

    /// Un personnage ou un objet découpé en pièces. Toutes les pièces sont modelées dans le même repère.
    /// Noms de pièces reconnus par l'animation : corps, tete, brasG, brasD, jambeG, jambeD, arme, bouclier, aileG, aileD,
    /// queue, machoire, pattesG, pattesD, cape. Les pièces manquantes sont simplement ignorées.
    class VoxelRig
    {
    public:
        float VoxelSize = 0.05f;          // taille d'un voxel en mètres
        Vector3 Origin;                   // point du repère (en voxels) posé au sol, au centre de la case
        Liste<Ptr<VoxelPart>> Parts;
        string Name;

        Ptr<VoxelPart> Add(const string& name, const Ptr<VoxelModel>& m, Vector3 pivot, const string& parent = "")
        {
            auto p = Nouveau<VoxelPart>();
            p->Name = name;
            p->Model = m;
            p->Pivot = pivot;
            p->Parent = parent;
            Parts.Add(p);
            return p;
        }

        Ptr<VoxelPart> Get(const string& name) const
        {
            for (int i = 0; i < Parts.Count(); i++)
                if (Parts[i]->Name == name) return Parts[i];
            return nullptr;
        }

        /// Un rig d'une seule pièce.
        static Ptr<VoxelRig> Single(const string& name, const Ptr<VoxelModel>& m, float voxelSize, Vector3 origin)
        {
            auto r = Nouveau<VoxelRig>();
            r->Name = name;
            r->VoxelSize = voxelSize;
            r->Origin = origin;
            r->Add("corps", m, origin);
            return r;
        }
    };
}
