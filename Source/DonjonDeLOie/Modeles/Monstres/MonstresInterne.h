// Donjon de l'Oie — les outils partagés par les fichiers des monstres (Modeles/Monstres/) : les primitives « taillées »,
// les retouches, les couleurs du dragon, et le constructeur de chaque monstre. En C#, ce sont les membres privés de la
// classe partielle MonstresModeles ; chacun est défini dans le fichier .cpp qui porte le nom du fichier C# d'origine
// (les commentaires de documentation sont avec les définitions, comme en C#).
// Interne à la famille : rien en dehors de Modeles/Monstres/ ne l'inclut.
#pragma once

#include "../../Voxel/Pal.h"
#include "../MonstresModeles.h"

#include <algorithm>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstdlib>

namespace DonjonDeLOie::Modeles::MonstresModeles
{
    // ------------------------------------------------------------------ couleurs (MonstresModeles.cs)
    // Ces petits outils sont définis ici, en ligne : C doit être constexpr pour les couleurs fixes du dragon et du
    // Dragonnoie (les « static readonly » de C#, calculées à la compilation).

    constexpr Color32 C(const char* h) { return Voxel::Pal::C(h); }

    inline float Lerp(float a, float b, float t) { return a + (b - a) * t; }

    inline Vector3 V(float x, float y, float z) { return Vector3(x, y, z); }

    inline bool Meme(Color32 a, Color32 b) { return a.r == b.r && a.g == b.g && a.b == b.b; }

    // ------------------------------------------------------------------ rigs (MonstresModeles.cpp)

    Ptr<Voxel::VoxelRig> NouveauRig(const string& nom, float taille);
    Ptr<Voxel::VoxelRig> Fin(const Ptr<Voxel::VoxelRig>& r);

    // ------------------------------------------------------------------ volumes taillés (MonstresModeles.cpp)

    void BoiteC(Voxel::VoxelModel& m, int x0, int y0, int z0, int x1, int y1, int z1, Color32 c, int ch);
    void Section(Voxel::VoxelModel& m, int axe, int k, float cu, float cv, float a, float b, float ch, Color32 c);
    void TronconZ(Voxel::VoxelModel& m, int z0, int z1, float cx, float cy0, float cy1, float w0, float w1, float h0, float h1, float ch, Color32 c);
    void TronconY(Voxel::VoxelModel& m, int y0, int y1, float cx, float cz0, float cz1, float w0, float w1, float d0, float d1, float ch, Color32 c);
    void ProfilZ(Voxel::VoxelModel& m, float cx, const Liste<float>& k, float ch, Color32 c);
    void ProfilY(Voxel::VoxelModel& m, float cx, const Liste<float>& k, float ch, Color32 c);
    void Chaine(Voxel::VoxelModel& m, const Liste<float>& p, float r0, float r1, Color32 c);
    void Triangle(Voxel::VoxelModel& m, Vector3 a, Vector3 b, Vector3 c, Color32 col);
    void DisqueZ(Voxel::VoxelModel& m, float cx, float cy, int z0, int z1, float r, Color32 c);

    // ------------------------------------------------------------------ retouches (MonstresModeles.cpp)

    int Avant(const Voxel::VoxelModel& m, int x, int y, int zmin, int zmax);
    int Haut(const Voxel::VoxelModel& m, int x, int z, int ymin, int ymax);
    void Soustraire(Voxel::VoxelModel& a, const Voxel::VoxelModel& b);
    void Oeil(Voxel::VoxelModel& porteur, Voxel::VoxelModel& lueur, int x, int y, int z, Color32 c);
    void Taches(Voxel::VoxelModel& m, Color32 depuis, Color32 b, float seuil, int graine);

    // ------------------------------------------------------------------ outils définis avec un monstre

    // MonstresModeles.Squelette.cpp (le troll s'en sert aussi)
    Color32 Couleur(const Voxel::VoxelModel& m, int x, int y, int z);
    // MonstresModeles.Wyverne.cpp (Ecailles sert aussi au Dragonnoie ; Aile, au dragon et au Dragonnoie)
    void Ecailles(Voxel::VoxelModel& m, Color32 base_, Color32 sombre, Color32 claire, int pas);
    Ptr<Voxel::VoxelModel> Aile(float sx, Vector3 epaule, Vector3 coude, Vector3 poignet, const Liste<Vector3>& doigts, const Liste<Vector3>& flanc,
                                float rOs, Color32 os, Color32 membrane, Color32 membraneS, Color32 griffe);

    // ------------------------------------------------------------------ le dragon (Dragon.cpp et DragonMembres.cpp)

    inline constexpr Color32 DrRouge = C("a8322a"), DrRougeS = C("6e1f1c"), DrRougeC = C("cf5236"), DrRougeT = C("4a1514");
    inline constexpr Color32 DrOr = C("d9a441"), DrOrS = C("a0722a"), DrOrC = C("f0c870"), DrCorne = C("2e2622"), DrCorneC = C("6d5a48");
    inline constexpr Color32 DrMembrane = C("9a4630"), DrMembraneS = C("63231d"), DrDent = C("efe4c6"), DrFeu = C("ffb03a"), DrOeil = C("ffe066");

    // MonstresModeles.Dragon.cpp
    void PlaquesDragon(Voxel::VoxelModel& m);
    // MonstresModeles.DragonMembres.cpp
    void AjouterPattesDragon(const Ptr<Voxel::VoxelRig>& r);
    void AjouterAilesDragon(const Ptr<Voxel::VoxelRig>& r);
    void AjouterQueueDragon(const Ptr<Voxel::VoxelRig>& r);

    // ------------------------------------------------------------------ les monstres, chacun dans son fichier

    Ptr<Voxel::VoxelRig> Rat();            // MonstresModeles.Rat.cpp
    Ptr<Voxel::VoxelRig> Loup();
    Ptr<Voxel::VoxelRig> Gobelin();        // MonstresModeles.Gobelin.cpp
    Ptr<Voxel::VoxelRig> Orc();
    Ptr<Voxel::VoxelRig> Araignee();       // MonstresModeles.Araignee.cpp
    Ptr<Voxel::VoxelRig> Gelee();
    Ptr<Voxel::VoxelRig> Squelette();      // MonstresModeles.Squelette.cpp
    Ptr<Voxel::VoxelRig> Spectre();
    Ptr<Voxel::VoxelRig> Troll();          // MonstresModeles.Troll.cpp
    Ptr<Voxel::VoxelRig> Wyverne();        // MonstresModeles.Wyverne.cpp
    Ptr<Voxel::VoxelRig> Dragon();         // MonstresModeles.Dragon.cpp
    Ptr<Voxel::VoxelRig> Dragonnoie();     // MonstresModeles.Dragonnoie.cpp
}
