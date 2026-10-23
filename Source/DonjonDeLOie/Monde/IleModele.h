// Donjon de l'Oie — l'île, calculée sans Unity : relief en colonnes (cellules de 50 cm, marches de 25 cm), zones,
// chemin entre les cases, dalles des cases, emplacements du décor et des lieux, lampes.
// La vue Unity (IleVue) et le rendu hors Unity (outils d'aperçu) lisent le même modèle.
//
// Une seule classe pour les trois fichiers C# (classe partielle), chacun donnant son .cpp : IleModele.cs (relief,
// zones, rivage), IleModele.Maillages.cs (terrain, mer, dalles), IleModele.Placements.cs (lieux, décor, lampes).
// Les membres sont déclarés dans l'ordre des fichiers C#.
#pragma once

#include "../Moteur/Moteur.h"
#include "../Regles/Plateau.h"
#include "../Voxel/Pal.h"
#include "../Voxel/VoxelMesher.h"

namespace DonjonDeLOie::Monde
{
    struct Placement
    {
        string Id;
        int Graine = 0;
        Vector3 Pos;
        float RotY = 0.0f;
        float Echelle = 0.0f;
        bool Lieu = false;
    };

    struct Lampe
    {
        Vector3 Pos;
        Color32 Couleur;
        float Portee = 0.0f, Intensite = 0.0f;
    };

    class IleModele
    {
        // ================================================================== IleModele.cs
    public:
        static constexpr float Cell = 0.5f, Marche = 0.25f, NiveauEau = 0.0f;
        static constexpr float EpaisseurDalle = 0.3f;
        static constexpr int TailleMax = 900;                   // cellules par côté au plus (450 m)

        // La grille couvre les cases, plus la marge de terre et un peu de mer.
        int NX = 0, NZ = 0;
        float X0 = 0.0f, Z0 = 0.0f;
        Vector3 Centre;                                         // milieu des cases

        Ptr<Regles::Plateau> Plateau;
        Ptr<Regles::CarteDef> Options;
        Liste<float> H;
        Liste<uint8_t> Mat;
        Liste<Regles::Zone> ZoneCell;
        Liste<float> Dist;                                      // distance au chemin
        Liste<float> Cote;                                      // < 1 : terre (0 au cœur de l'île), 1 : le rivage, > 1 : la mer
        Liste<Vector3> Cases;                                   // centre du dessus de chaque dalle
        Liste<Placement> Decors;
        Liste<Placement> Lieux;
        Liste<Lampe> Lampes;
        Vector3 Montagne;                                       // centre de la montagne de l'antre

    private:
        int graine = 0;

    public:
        /// options : la carte (marge, décor, graine, décors posés à la main) ; null pour les réglages de la carte standard.
        explicit IleModele(const Ptr<Regles::Plateau>& p, const Ptr<Regles::CarteDef>& options = nullptr);

    private:
        int I(int x, int z) const { return z * NX + x; }

    public:
        float CX(int x) const;
        float CZ(int z) const;
        float LargeurM() const;
        float ProfondeurM() const;

        float HauteurEn(float x, float z) const;

        Regles::Zone ZoneEn(float x, float z) const;

        float DistanceChemin(float x, float z) const;

        // ------------------------------------------------------------------ bruit

    private:
        float Bruit(float x, float z, int o) const;

        float Fbm(float x, float z, int o) const;

        static float Lisse(float a, float b, float v);

    public:
        /// < 1 sur terre (0 au cœur de l'île), 1 au rivage, > 1 en mer — calculé depuis le tracé du chemin (voir CalculerCote).
        float Ellipse(float x, float z) const;

    private:
        /// La terre = une bande de « Marge » mètres autour du chemin (bord bruité) + tout ce que les boucles entourent.
        /// Puis, pour chaque cellule, la distance au rivage : Cote = 1 − d/30 sur terre, 1 + d/30 en mer.
        void CalculerCote();

        // (Queue<int> file : une Liste<int> où l'on ajoute au bout ; la lecture se fait dans l'ordre, par un indice de tête.)
        void Semer(int x, int z, const Liste<bool>& terre, Liste<bool>& mer, Liste<int>& file) const;

        /// Distance (m) de chaque cellule « dedans » à la cellule la plus proche « dehors ».
        Liste<float> Chanfrein(const Liste<bool>& terre, bool dedansEstTerre) const;

        // ------------------------------------------------------------------ construction

        void Construire();

        bool aMontagne = false;

        /// Derrière la case Antre (s'il y en a une), du côté opposé au chemin, là où la montagne ne recouvre aucune route.
        Vector3 PlacerMontagne();

        static float Quant(float h);

        float Echantillon(const Liste<float>& t, float x, float z) const;

        static float DistSeg(float px, float pz, float ax, float az, float bx, float bz);

        float HauteurChemin(float px, float pz, const Liste<float>& hc) const;

        float Relief(float x, float z, Regles::Zone zo, float d) const;

        // Matières : 0 herbe claire, 1 herbe de forêt, 2 mousse olive, 3 boue, 4 herbe morte, 5 terre sombre, 6 cendre, 7 roche,
        // 8 roche sombre, 9 sable, 10 fond mouillé, 11 chemin de terre, 12 pavés, 13 chemin de cendre, 14 chemin de boue, 15 fleurs
        uint8_t Matiere(float x, float z, Regles::Zone zo, float h, float d) const;

    public:
        static constexpr Color32 CouleursMat[16] = {
            Voxel::Pal::C("86ad4c"), Voxel::Pal::C("557d33"), Voxel::Pal::C("75773b"), Voxel::Pal::C("5c4a31"), Voxel::Pal::C("8c8a62"), Voxel::Pal::C("5b4f45"), Voxel::Pal::C("6e6058"), Voxel::Pal::C("7c7068"),
            Voxel::Pal::C("4a4240"), Voxel::Pal::C("d8c48f"), Voxel::Pal::C("9c8a64"), Voxel::Pal::C("a07f56"), Voxel::Pal::C("9a958b"), Voxel::Pal::C("6b5e57"), Voxel::Pal::C("6e5a3e"), Voxel::Pal::C("8fb24e"),
        };

        static constexpr Color32 CouleursFlanc[16] = {
            Voxel::Pal::C("7b5c3e"), Voxel::Pal::C("6a4d33"), Voxel::Pal::C("5d4a33"), Voxel::Pal::C("4c3d2a"), Voxel::Pal::C("6a5e4c"), Voxel::Pal::C("4c433c"), Voxel::Pal::C("564c48"), Voxel::Pal::C("615851"),
            Voxel::Pal::C("3c3534"), Voxel::Pal::C("c2ad78"), Voxel::Pal::C("85755a"), Voxel::Pal::C("85694a"), Voxel::Pal::C("7e7a72"), Voxel::Pal::C("564b46"), Voxel::Pal::C("594833"), Voxel::Pal::C("7b5c3e"),
        };

        // ================================================================== IleModele.Maillages.cs
        // maillages de l'île : terrain en colonnes (par morceaux de 32 × 32 cellules), mer, dalles des cases.
    public:
        static constexpr int Morceau = 32;

    private:
        float HC(int x, int z) const;

        static void Quad(Voxel::MeshData& md, Vector3 a, Vector3 b, Vector3 c, Vector3 d, Vector3 n, Color32 ca, Color32 cb, Color32 cc, Color32 cd);

        Color32 CouleurDessus(int x, int z) const;

    public:
        /// Maillage d'un morceau de terrain (mx, mz : indices du morceau).
        Ptr<Voxel::MeshData> MaillageTerrain(int mx, int mz) const;

    private:
        float Occ(int x, int z, int dx, int dz, float h) const;

        void Flanc(Voxel::MeshData& md, int x, int z, float h, int dx, int dz, Vector3 p, Vector3 q, Vector3 n) const;

    public:
        /// La mer : une grande grille ondulée, plus claire et transparente près des côtes.
        Ptr<Voxel::MeshData> MaillageMer() const;

        // ------------------------------------------------------------------ dalles

        static Color32 CouleurType(Regles::TypeCase t);

        static string IconeType(Regles::TypeCase t);

    private:
        static inline Dico<Regles::TypeCase, Ptr<Voxel::MeshData>> dalles;

    public:
        /// Dalle d'une case : pierre taillée chanfreinée, liseré de la couleur du type, icône gravée au centre. Dessus à y = 0.
        static Ptr<Voxel::MeshData> MaillageDalle(Regles::TypeCase t);

        // ================================================================== IleModele.Placements.cs
        // où poser les lieux remarquables, le décor semé et les lampes.
    private:
        // (List<KeyValuePair<Vector3, float>> : first = la position, second = le rayon)
        Liste<std::pair<Vector3, float>> zonesInterdites;

        Vector3 PosCase(int c) const;

        /// Direction de la route au passage de la case c (moyenne entrée/sortie).
        Vector3 Tangente(int c) const;

        /// Point au bord de la route : côté extérieur de l'île (exterieur = true) ou intérieur.
        Vector3 Bord(int c, float dist, bool exterieur, float glisse = 0.0f) const;

        static float CapVers(Vector3 de, Vector3 vers);

        void Lieu(const string& id, Vector3 pos, float rotY, float rayon, int g = 1, float echelle = 1.0f);

        void LieuFace(const string& id, int c, float dist, bool exterieur, float rayon, int g = 1, float glisse = 0.0f);

        /// Met le terrain d'un disque à la hauteur de son centre (sans toucher au chemin).
        void Aplanir(Vector3 centre, float rayon);

        /// Case à k pas sur la grande route (k négatif : en arrière).
        int Pas(int c, int k) const;

    public:
        /// Rayon occupé par un bâtiment ou un décor posé (pour aplanir le sol et écarter le reste).
        static float RayonLieu(const string& id);

    private:
        /// Pose un lieu au bord de la case : à l'extérieur d'abord, à l'intérieur si la place est prise.
        bool LieuPres(const string& id, int c, int g = 1);

        /// Place libre pour un bâtiment de la ville : seules comptent les zones posées avant la ville, et le chemin.
        bool LibrePourVille(Vector3 p, float r, int avantVille) const;

        bool LibrePourLieu(Vector3 p, float r) const;

        /// Le bâtiment qui va avec une case quand l'auteur de la carte n'a rien choisi.
        static string LieuAuto(const Ptr<Regles::Case>& c, const Ptr<Regles::Plateau>& p);

        void PlacerLieux();

        // ------------------------------------------------------------------ décor semé

        static float Poids(const string& id);

        static float Densite(Regles::Zone z);

        bool Libre(Vector3 p, float marge) const;

        // (static readonly HashSet<string> Grands : un ensemble constant, rendu par une fonction)
        static const Ensemble<string>& Grands();

        void Semer();

        /// Un grand élément juste au sud d'une case la cacherait à la caméra.
        bool CacheUneCase(float x, float z) const;

        static string Tirer(const Liste<string>& ids, AleaNet& r);

        void PlacerLampes();

        // ==================================================================

        /// Remise des variables statiques à leur valeur de départ (Boucle::Reinitialiser) : inscrite dans le .cpp.
        static const bool remiseAZero;
    };
}
