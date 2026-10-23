// Donjon de l'Oie — décors de nature : ce que les fichiers de la famille (DecorsNature*.cpp) partagent.
// En C#, tout est dans la classe partielle DecorsModeles (avec les bâtiments) ; ici, les outils et les modèles de la
// nature sont dans l'espace de noms DecorsModeles::Nature, pour qu'aucun nom ne se heurte à ceux des bâtiments.
#pragma once

#include "../../Voxel/Pal.h"
#include "../DecorsModeles.h"

namespace DonjonDeLOie::Modeles::DecorsModeles::Nature
{
    // ------------------------------------------------------------------ hasard (DecorsNature.cpp)

    float NaF(AleaNet& r, float a, float b);
    int NaI(AleaNet& r, int a, int b);   // bornes incluses
    bool NaP(AleaNet& r, float p);

    /// Teinte légèrement décalée (luminosité ± amp, petite dérive vers le chaud ou le froid).
    Color32 NaTeinte(Color32 c, AleaNet& r, float amp);

    // ------------------------------------------------------------------ outils de modèle (DecorsNature.cpp)

    bool NaEgal(Color32 a, Color32 b);

    /// Éclaircit les voxels à ciel ouvert, assombrit ceux qui regardent le sol (lecture des volumes de loin).
    void NaDessusDessous(Voxel::VoxelModel& m, float dessus, float dessous);

    /// Retire tout ce qui passe sous le sol.
    void NaSol(Voxel::VoxelModel& m);

    Ptr<Voxel::VoxelRig> NaRig(const string& id, float taille);

    // ------------------------------------------------------------------ feuillage (DecorsNature.cpp)

    /// Masse de feuillage taillée : sections octogonales, étagée par paliers de « marche » voxels, dessous aplati.
    void NaMasse(Voxel::VoxelModel& m, float cx, float cy, float cz, float rx, float ry, float rz, int marche, float rot, Color32 c);

    /// Peint un feuillage : dessus clair, dessous sombre, quelques touffes d'un second ton.
    void NaPeindreFeuillage(Voxel::VoxelModel& m, Color32 clair, Color32 sombre, Color32 touffe, int graine);

    // ------------------------------------------------------------------ bois (DecorsNature.cpp)

    /// Tronc vertical taillé : centre qui ondule, évasement au pied, écorce à côtes (sillons creusés, deux tons).
    /// Rend le centre du tronc à la hauteur yHaut.
    Vector3 NaTronc(Voxel::VoxelModel& m, AleaNet& r, int yHaut, float r0, float r1, float penteX, float penteZ,
                    float ondule, int cotes, Color32 ecorce, Color32 sillon);

    /// Racines : traits effilés qui partent du tronc et plongent dans le sol.
    void NaRacines(Voxel::VoxelModel& m, AleaNet& r, int n, float rTronc, float longueur, Color32 c);

    /// Écorce striée sur des branches (traits) : sillons verticaux courts, deux tons.
    void NaStrier(Voxel::VoxelModel& m, Color32 ecorce, Color32 sillon, int graine);

    // ------------------------------------------------------------------ pierre (DecorsNature.cpp)

    /// Rocher à facettes : un ellipsoïde tranché par des plans au hasard ; chaque facette a son ton, strates inclinées.
    void NaRoche(Voxel::VoxelModel& m, AleaNet& r, Vector3 c, Vector3 rad, int nPlans, const Liste<Color32>& tons, float pasStrate, Color32 strate);

    /// Prisme taillé à K pans entre a et b (rayon r0 → r1), orienté librement. Chaque pan prend un ton de « tons ».
    void NaPrisme(Voxel::VoxelModel& m, Vector3 a, Vector3 b, float r0, float r1, int K, float rot, const Liste<Color32>& tons);

    // ------------------------------------------------------------------ définis dans un fichier, utilisés dans un autre

    // écorces (DecorsNature.Arbres.cpp ; le buisson et la souche de DecorsNature.Plantes.cpp s'en servent aussi)
    inline constexpr Color32 NaEcorce = Voxel::Pal::C("6e5038"), NaEcorceSombre = Voxel::Pal::C("4a3426");

    /// Jeux de tons de pierre : 0 gris chaud (prés, bois), 1 gris bleuté (cryptes), 2 volcanique (pics), 3 roche d'antre.
    /// (DecorsNature.Roches.cpp ; le cierge de DecorsNature.Ombres.cpp s'en sert aussi)
    Liste<Color32> NaTonsPierre(int famille, AleaNet& r);

    // ------------------------------------------------------------------ les modèles (aiguillés par ConstruireNature)

    // DecorsNature.Arbres.cpp
    Ptr<Voxel::VoxelRig> Chene(AleaNet& r);
    Ptr<Voxel::VoxelRig> Pin(AleaNet& r);
    Ptr<Voxel::VoxelRig> Saule(AleaNet& r);
    Ptr<Voxel::VoxelRig> ArbreMort(AleaNet& r);
    Ptr<Voxel::VoxelRig> PinBrule(AleaNet& r);

    // DecorsNature.Roches.cpp
    Ptr<Voxel::VoxelRig> Rocher(AleaNet& r, bool mousse);
    Ptr<Voxel::VoxelRig> PicRocheux(AleaNet& r);
    Ptr<Voxel::VoxelRig> Stalagmite(AleaNet& r);
    Ptr<Voxel::VoxelRig> Lave(AleaNet& r);
    Ptr<Voxel::VoxelRig> CristalBraise(AleaNet& r);

    // DecorsNature.Plantes.cpp
    Ptr<Voxel::VoxelRig> Buisson(AleaNet& r);
    Ptr<Voxel::VoxelRig> Souche(AleaNet& r);
    Ptr<Voxel::VoxelRig> Fleurs(AleaNet& r);
    Ptr<Voxel::VoxelRig> HerbesHautes(AleaNet& r);
    Ptr<Voxel::VoxelRig> Champignon(AleaNet& r);
    Ptr<Voxel::VoxelRig> ChampignonMarais(AleaNet& r);
    Ptr<Voxel::VoxelRig> Fougere(AleaNet& r);
    Ptr<Voxel::VoxelRig> Roseaux(AleaNet& r);

    // DecorsNature.Ombres.cpp
    Ptr<Voxel::VoxelRig> Tombe(AleaNet& r);
    Ptr<Voxel::VoxelRig> CroixPierre(AleaNet& r);
    Ptr<Voxel::VoxelRig> Os(AleaNet& r);
    Ptr<Voxel::VoxelRig> Cierge(AleaNet& r);
    Ptr<Voxel::VoxelRig> TasOr(AleaNet& r);
    Ptr<Voxel::VoxelRig> FeuFollet(AleaNet& r);
}
