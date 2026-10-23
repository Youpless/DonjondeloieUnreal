// Donjon de l'Oie — outils communs des bâtiments (Batiments/*.cpp) : la façade (FaceBat) et le chantier (ChantierBat)
// de DecorsBatiments.Outils.cs, les couleurs de Valcreux (DecorsBatiments.cs) et les bâtiments que l'aiguillage
// appelle. En C#, ce sont des membres privés de la classe partielle DecorsModeles (et deux types internes) ; ici ils
// sont rangés dans DecorsModeles::Batiments, pour qu'aucun nom ne se heurte à ceux de la nature (Nature/).
#pragma once

#include "../../Voxel/Pal.h"
#include "../DecorsModeles.h"

#include <array>
#include <iterator>

namespace DonjonDeLOie::Modeles::DecorsModeles::Batiments
{
    using namespace DonjonDeLOie::Voxel;

    /// Une façade : origine sur la surface extérieure, axe u le long du mur, normale n vers l'extérieur (dans le plan x/z).
    struct FaceBat
    {
        int Ox = 0, Oz = 0, Ux = 0, Uz = 0, Nx = 0, Nz = 0;
        static FaceBat Avant(int zSurface) { return FaceBat{.Ox = 0, .Oz = zSurface, .Ux = 1, .Uz = 0, .Nx = 0, .Nz = 1}; }
        static FaceBat Arriere(int zSurface) { return FaceBat{.Ox = 0, .Oz = zSurface, .Ux = -1, .Uz = 0, .Nx = 0, .Nz = -1}; }
        static FaceBat Droite(int xSurface) { return FaceBat{.Ox = xSurface, .Oz = 0, .Ux = 0, .Uz = -1, .Nx = 1, .Nz = 0}; }
        static FaceBat Gauche(int xSurface) { return FaceBat{.Ox = xSurface, .Oz = 0, .Ux = 0, .Uz = 1, .Nx = -1, .Nz = 0}; }
        int X(int u, int d) const { return Ox + u * Ux + d * Nx; }
        int Z(int u, int d) const { return Oz + u * Uz + d * Nz; }
    };

    /// Le chantier d'un bâtiment (DecorsBatiments.Outils.cpp) : maçonnerie appareillée (joints en creux), planches à
    /// deux tons, toitures en rangs (tuiles, chaume, ardoise), pignons, fenêtres à volets, portes ferrées. Il tient son
    /// générateur (un AleaNet, non copiable) : on le garde sur la pile et on le passe par référence.
    class ChantierBat
    {
    public:
        const Ptr<VoxelModel> M = Nouveau<VoxelModel>();   // détails : bois, fer, tuiles… (prioritaires)
        const Ptr<VoxelModel> P = Nouveau<VoxelModel>();   // maçonnerie brute, appareillée par Finir()

    private:
        /// Une pièce lumineuse et son nom (la KeyValuePair<string, VoxelModel> du C#).
        struct LueurNommee
        {
            string Key;
            Ptr<VoxelModel> Value;
        };
        Liste<LueurNommee> lueurs;

    public:
        AleaNet R;
        int Assise = 3, Longueur = 5;               // hauteur et longueur des pierres (voxels)
        float Mousse = 0.0f;                        // probabilité de mousse au pied des murs
        Color32 CouleurMousse = Pal::C("5f7a3a");

    private:
        Liste<std::array<float, 5>> ronds;

    public:
        /// Déclare une zone ronde (tour, puits) : les pierres y sont appareillées le long de la circonférence.
        void Rond(float cx, float cz, int y0, int y1, float r);

        explicit ChantierBat(int graine);

        Ptr<VoxelModel> Lueur(const string& nom);

        static int Div(int a, int b);
        static int Mod(int a, int b);
        static float H(int a, int b, int c);

        // ------------------------------------------------------------------ maçonnerie
        void Pierre(int x0, int y0, int z0, int x1, int y1, int z1, Color32 c);
        void PierreCyl(float cx, float cz, int y0, int y1, float r, Color32 c);
        void PierreCone(float cx, float cz, int y0, int y1, float r0, float r1, Color32 c);

        /// Mur creux (coque d'épaisseur ep) de maçonnerie.
        void MursPierre(int x0, int y0, int z0, int x1, int y1, int z1, int ep, Color32 c);

        /// Retire partout (maçonnerie et détails).
        void Vide(int x0, int y0, int z0, int x1, int y1, int z1);

    private:
        bool Plein(int x, int y, int z) const;

    public:
        /// Appareille la maçonnerie (pierres teintées une à une, joints sombres, en creux sur les parements) et rend le modèle final.
        Ptr<VoxelModel> Finir();

        bool Miroir = false;   // retourne le bâtiment gauche ↔ droite (variantes)

        static Ptr<VoxelModel> Retourner(const VoxelModel& m);

        int Evidement = 0;   // > 0 : retire les voxels enfouis à plus de cette profondeur (budget des gros bâtiments)

        /// Retire les voxels invisibles : ceux dont tout le cube de côté 2·p + 1 qui les entoure est plein.
        static void Evider(VoxelModel& m, int p);

        /// Rig final : pièce « corps » + pièces lumineuses.
        Ptr<VoxelRig> Rig(const string& id, float taille);

        // ------------------------------------------------------------------ bois
        /// Planches : la teinte change d'une planche à l'autre selon la coordonnée « axe » ('x', 'y' ou 'z').
        void Bois(int x0, int y0, int z0, int x1, int y1, int z1, Color32 c, char axe, int larg = 2);

        /// Poutre pleine, légère variation par voxel.
        void Poutre(int x0, int y0, int z0, int x1, int y1, int z1, Color32 c);

        void Trait(float x0, float y0, float z0, float x1, float y1, float z1, float r, Color32 c);

        // ------------------------------------------------------------------ toitures
    private:
        void SetUV(bool faitageX, int u, int y, int v, Color32 c);

    public:
        /// Toit à deux pans. Faîtage le long de x (faitageX) ou de z. montee : voxels de hauteur par pas. style 0 tuiles, 1 chaume, 2 ardoise, 3 bardeaux.
        void Toit(bool faitageX, int u0, int u1, int v0, int v1, int yb, int montee, int ep, Color32 c, int style);

    private:
        Color32 TuileCouleur(Color32 c, int style, int d, int u, int y, int yh, bool fait) const;

    public:
        /// Pignon triangulaire plein (plan u = uPlan) sous un toit de mêmes paramètres.
        void Pignon(bool faitageX, int uPlan, int v0, int v1, int yb, int montee, Color32 c, bool pierre);

        // ------------------------------------------------------------------ ouvertures
        void SetF(FaceBat f, int u, int y, int d, Color32 c);
        void VideF(FaceBat f, int u, int y, int d);

        /// Fenêtre : creusée d'un voxel, carreaux sombres à croisillon, appui, linteau, volets ouverts (si volet.a > 0).
        void Fenetre(FaceBat f, int u0, int y0, int w, int h, Color32 cadre, Color32 volet, bool volets, const Ptr<VoxelModel>& vitre = nullptr);

        /// Porte : embrasure creusée (en arc si arche), vantail en planches au fond, pentures de fer, marche de pierre.
        void Porte(FaceBat f, int u0, int y0, int w, int h, int prof, Color32 bois, bool arche, Color32 encadrement);
    };

    // ------------------------------------------------------------------ couleurs de Valcreux (DecorsBatiments.cs)
    inline constexpr Color32 VcPierre = Pal::C("a89c88"), VcPierreSombre = Pal::C("7d7366"), VcEnduit = Pal::C("e6d8b8");
    inline constexpr Color32 VcColombage = Pal::C("6a4530"), VcBois = Pal::C("8b5e3c");
    inline constexpr Color32 VcTuile = Pal::C("b35a3c"), VcArdoise = Pal::C("5b6372"), VcChaume = Pal::C("c4a35e"), VcBardeau = Pal::C("7d5a3e");
    inline constexpr Color32 VcVolets[] = {Pal::C("4f7092"), Pal::C("5f7f48"), Pal::C("8e4a3a"), Pal::C("7a6a9a")};
    inline constexpr Color32 VcFleurs = Pal::C("c94a4a"), VcFeuilles = Pal::C("5d8a3c");

    // ------------------------------------------------------------------ les bâtiments, fichier par fichier
    // DecorsBatiments.cpp (le reste : aiguillage, colombages, maisons, taverne)
    void Tonneau(ChantierBat& c, int x, int y, int z, int r, int h, bool couche);

    // DecorsBatiments.Valcreux.cpp
    Ptr<VoxelRig> TourGarde(int graine);
    Ptr<VoxelRig> PorteVille(int graine);
    Ptr<VoxelRig> Rempart(int graine);
    Ptr<VoxelRig> Puits(int graine);
    Ptr<VoxelRig> Banniere(int graine);
    Ptr<VoxelRig> Panneau(int graine);
    Ptr<VoxelRig> Etal(int graine);
    Ptr<VoxelRig> Pont(int graine);

    // DecorsBatiments.Ombres.cpp
    Ptr<VoxelRig> Chapelle(int graine);
    Ptr<VoxelRig> AutelOmbrelune(int graine);
    Ptr<VoxelRig> PortailCryptes(int graine);
    Ptr<VoxelRig> Mausolee(int graine);
    Ptr<VoxelRig> ComptoirFossoyeurs(int graine);
    Ptr<VoxelRig> Ruine(int graine);
    Ptr<VoxelRig> Ponton(int graine);

    // DecorsBatiments.Pics.cpp
    Ptr<VoxelRig> Forge(int graine);
    Ptr<VoxelRig> MontagneAntre(int graine);

    // DecorsBatiments.Marche.cpp
    Ptr<VoxelRig> EtalMarche(int graine);
    Ptr<VoxelRig> Caisses(int graine);
    Ptr<VoxelRig> Tonneaux(int graine);
    Ptr<VoxelRig> Sacs(int graine);
    Ptr<VoxelRig> CharretteFoin(int graine);

    // DecorsBatiments.Arene.cpp
    Ptr<VoxelRig> AreneDuDiable(int graine);
}
