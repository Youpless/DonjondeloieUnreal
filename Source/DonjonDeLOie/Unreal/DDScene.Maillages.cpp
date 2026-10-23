// Donjon de l'Oie — la scène : les maillages du jeu (MeshFilter + MeshRenderer) deviennent des composants de maillage
// procédural, avec leurs matériaux.
//
// Un maillage n'est recopié que s'il a changé (Mesh::Version), un matériau n'est réglé que si ses propriétés ont
// changé (Material::Version, MaterialPropertyBlock). Chaque matériau du jeu a son instance de matériau Unreal,
// partagée par tous les objets qui l'utilisent ; un objet qui a ses propres couleurs (MaterialPropertyBlock : l'éclair
// d'un coup reçu) a la sienne.
#include "DDScene.h"

#include "MaterialDomain.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ProceduralMeshComponent.h"
#include "HAL/IConsoleManager.h"

#include "DDConversions.h"
#include "DDJournal.h"

using DDMateriauxUnreal::EGenre;

namespace
{
    TAutoConsoleVariable<int32> CVarOrdreUnity(TEXT("dd.Faces.OrdreUnity"), 0,
                                               TEXT("Ordre des sommets des triangles. 0 : retourné pour Unreal (par défaut). 1 : celui d'Unity, ")
                                                   TEXT("si les faces des objets semblaient vues de l'intérieur."),
                                               ECVF_Default);

    template <class V>
    V Lire(const Moteur::Dico<std::string, V>& D, const char* Cle, V Defaut)
    {
        V v = Defaut;
        return D.TryGetValue(Cle, v) ? v : Defaut;
    }

    bool Nul(const Moteur::Vector3& s) { return FMath::Abs(s.x) < 1e-5f || FMath::Abs(s.y) < 1e-5f || FMath::Abs(s.z) < 1e-5f; }
}

void ADDScene::ChargerMateriaux()
{
    Maitres.Reset();
    for (int32 g = 0; g < static_cast<int32>(EGenre::Nombre); g++)
    {
        const TCHAR* Nom = DDMateriauxUnreal::Nom(static_cast<EGenre>(g));
        UMaterialInterface* M = LoadObject<UMaterialInterface>(nullptr, *DDMateriauxUnreal::Chemin(Nom));
        if (M == nullptr)
        {
            UE_LOG(LogDonjon, Error, TEXT("Matériau introuvable : %s. Ouvrez le projet dans l'éditeur : le module DonjonDeLOieEditor crée les matériaux au démarrage."),
                   *DDMateriauxUnreal::Chemin(Nom));
            M = UMaterial::GetDefaultMaterial(MD_Surface);
        }
        Maitres.Add(M);
    }
    Brume = nullptr;
    if (UMaterialInterface* B = LoadObject<UMaterialInterface>(nullptr, *DDMateriauxUnreal::Chemin(DDMateriauxUnreal::NomBrouillard)))
        Brume = UMaterialInstanceDynamic::Create(B, this);
    else
        UE_LOG(LogDonjon, Warning, TEXT("Matériau de brume introuvable : %s (la brume ne sera pas affichée)."), *DDMateriauxUnreal::Chemin(DDMateriauxUnreal::NomBrouillard));
}

EGenre ADDScene::Genre(const Moteur::Material& Materiau)
{
    const std::string Shader = Materiau.shader != nullptr ? Materiau.shader->name() : std::string();
    if (Shader == "DonjonDeLOie/Lueur" || Shader == "Universal Render Pipeline/Particles/Unlit") return EGenre::Lueur;
    if (Shader == "DonjonDeLOie/Fx" || Shader == "Sprites/Default")
    {
        const auto& N = Materiau.Nombres();
        const bool Additif = Lire(N, "_DstBlend", 1.0f) == static_cast<float>(Moteur::BlendMode::One);
        const bool Dessus = Lire(N, "_ZTest", 4.0f) == static_cast<float>(Moteur::CompareFunction::Always);
        if (Additif) return Dessus ? EGenre::FxDessus : EGenre::FxAdd;
        return Lire(N, "_Vague", 0.0f) > 0.0f ? EGenre::Eau : EGenre::FxAlpha;
    }
    return EGenre::Voxel;
}

void ADDScene::LireAmbiance()
{
    using Moteur::RenderSettings;
    FLinearColor Ciel, Equateur, Sol;
    if (RenderSettings::ambientMode == Moteur::AmbientMode::Flat)
        Ciel = Equateur = Sol = DDUnreal::CouleurLineaire(RenderSettings::ambientLight);
    else
    {
        Ciel = DDUnreal::CouleurLineaire(RenderSettings::ambientSkyColor);
        Equateur = DDUnreal::CouleurLineaire(RenderSettings::ambientEquatorColor);
        Sol = DDUnreal::CouleurLineaire(RenderSettings::ambientGroundColor);
    }
    if (!Ciel.Equals(AmbianceCiel, 1e-5f) || !Equateur.Equals(AmbianceEquateur, 1e-5f) || !Sol.Equals(AmbianceSol, 1e-5f))
    {
        AmbianceCiel = Ciel;
        AmbianceEquateur = Equateur;
        AmbianceSol = Sol;
        AmbianceChangee = true;
    }
    if (!AmbianceChangee) return;
    AmbianceChangee = false;
    for (auto& P : MateriauxPartages)
        if (P.Value != nullptr) ReglerAmbiance(P.Value);
    for (auto& P : MateriauxPropres)
        if (P.Value != nullptr) ReglerAmbiance(P.Value);
}

void ADDScene::ReglerAmbiance(UMaterialInstanceDynamic* Instance)
{
    Instance->SetVectorParameterValue(FName(DDMateriauxUnreal::AmbiantCiel), AmbianceCiel);
    Instance->SetVectorParameterValue(FName(DDMateriauxUnreal::AmbiantEquateur), AmbianceEquateur);
    Instance->SetVectorParameterValue(FName(DDMateriauxUnreal::AmbiantSol), AmbianceSol);
}

void ADDScene::ReglerMateriau(UMaterialInstanceDynamic* Instance, const Moteur::Material& Materiau, const Moteur::MaterialPropertyBlock* Bloc, EGenre GenreRendu)
{
    // les valeurs par défaut sont celles des Properties des shaders d'Unity
    Moteur::Color Teinte = Lire(Materiau.Couleurs(), "_BaseColor", Moteur::Color::white);
    Moteur::Color Emission = Lire(Materiau.Couleurs(), "_EmissionColor", Moteur::Color(0.0f, 0.0f, 0.0f, 0.0f));
    float Intensite = Lire(Materiau.Nombres(), "_Intensite", 2.2f);
    float Pulsation = Lire(Materiau.Nombres(), "_Pulsation", 0.0f);
    float Vague = Lire(Materiau.Nombres(), "_Vague", 0.0f);
    if (Bloc != nullptr)
    {
        Teinte = Lire(Bloc->Couleurs(), "_BaseColor", Teinte);
        Emission = Lire(Bloc->Couleurs(), "_EmissionColor", Emission);
        Intensite = Lire(Bloc->Nombres(), "_Intensite", Intensite);
        Pulsation = Lire(Bloc->Nombres(), "_Pulsation", Pulsation);
        Vague = Lire(Bloc->Nombres(), "_Vague", Vague);
    }
    Instance->SetVectorParameterValue(FName(DDMateriauxUnreal::Teinte), DDUnreal::CouleurLineaire(Teinte));
    switch (GenreRendu)
    {
        case EGenre::Voxel:
            Instance->SetVectorParameterValue(FName(DDMateriauxUnreal::Emission), DDUnreal::CouleurLineaire(Emission));
            break;
        case EGenre::Lueur:
            Instance->SetScalarParameterValue(FName(DDMateriauxUnreal::Intensite), Intensite);
            Instance->SetScalarParameterValue(FName(DDMateriauxUnreal::Pulsation), Pulsation);
            break;
        case EGenre::Eau:
            Instance->SetScalarParameterValue(FName(DDMateriauxUnreal::Vague), Vague);
            break;
        default:
            break;
    }
}

UMaterialInterface* ADDScene::MateriauPour(int32 IdRendu, const Moteur::Renderer& Rendu, const Moteur::Material& Materiau, EGenre GenreRendu)
{
    UMaterialInterface* Maitre = Maitres.IsValidIndex(static_cast<int32>(GenreRendu)) ? Maitres[static_cast<int32>(GenreRendu)].Get() : nullptr;
    if (Maitre == nullptr) return nullptr;
    const int32 IdMateriau = Materiau.GetInstanceID();
    const Moteur::MaterialPropertyBlock* Bloc = Rendu.Bloc();
    if (Bloc != nullptr && !Bloc->Vide())
    {
        // ses propres couleurs : une instance pour lui seul
        TObjectPtr<UMaterialInstanceDynamic>& Propre = MateriauxPropres.FindOrAdd(IdRendu);
        FIntVector& Versions = VersionsMateriauxPropres.FindOrAdd(IdRendu, FIntVector(-1, -1, -1));
        if (Propre == nullptr || Propre->Parent != Maitre)
        {
            Propre = UMaterialInstanceDynamic::Create(Maitre, this);
            ReglerAmbiance(Propre);
            Versions = FIntVector(-1, -1, -1);
        }
        const FIntVector Voulues(IdMateriau, Materiau.Version(), Rendu.VersionBloc());
        if (Versions != Voulues)
        {
            ReglerMateriau(Propre, Materiau, Bloc, GenreRendu);
            Versions = Voulues;
        }
        return Propre;
    }
    TObjectPtr<UMaterialInstanceDynamic>& Partage = MateriauxPartages.FindOrAdd(IdMateriau);
    int32& Version = VersionsMateriauxPartages.FindOrAdd(IdMateriau, -1);
    if (Partage == nullptr || Partage->Parent != Maitre)
    {
        Partage = UMaterialInstanceDynamic::Create(Maitre, this);
        ReglerAmbiance(Partage);
        Version = -1;
    }
    if (Version != Materiau.Version())
    {
        ReglerMateriau(Partage, Materiau, nullptr, GenreRendu);
        Version = Materiau.Version();
    }
    return Partage;
}

void ADDScene::CopierMaillage(UProceduralMeshComponent* Composant, const Moteur::Mesh& Maillage, FDDEtatMaillage& Etat)
{
    const int32 N = Maillage.V.Count();
    const bool AvecNormales = Maillage.N.Count() == N;
    const bool AvecCouleurs = Maillage.C.Count() == N;
    TArray<FVector> Sommets;
    TArray<FVector> Normales;
    TArray<FColor> Couleurs;
    TArray<FProcMeshTangent> Tangentes;
    Sommets.SetNumUninitialized(N);
    Normales.SetNumUninitialized(N);
    Couleurs.SetNumUninitialized(N);
    Tangentes.SetNumUninitialized(N);
    for (int32 i = 0; i < N; i++)
    {
        Sommets[i] = DDUnreal::Position(Maillage.V[i]);
        const FVector Normale = AvecNormales ? DDUnreal::Direction(Maillage.N[i]) : FVector::UpVector;
        Normales[i] = Normale;
        // une tangente quelconque perpendiculaire à la normale (aucun matériau du jeu n'a de carte de normales)
        Tangentes[i] = FProcMeshTangent(FMath::Abs(Normale.X) > 0.9 ? FVector(0.0, 1.0, 0.0) : FVector(1.0, 0.0, 0.0), false);
        const Moteur::Color32 c = AvecCouleurs ? Maillage.C[i] : Moteur::Color32(255, 255, 255, 255);
        Couleurs[i] = FColor(c.r, c.g, c.b, c.a);
    }
    // Unity : faces avant dans le sens des aiguilles d'une montre ; Unreal : dans l'autre sens
    const bool Retourner = FacesInversees == 0;
    const int32 NT = Maillage.T.Count() / 3 * 3;
    TArray<int32> Triangles;
    Triangles.SetNumUninitialized(NT);
    for (int32 t = 0; t < NT; t += 3)
    {
        Triangles[t] = Maillage.T[t];
        Triangles[t + 1] = Retourner ? Maillage.T[t + 2] : Maillage.T[t + 1];
        Triangles[t + 2] = Retourner ? Maillage.T[t + 1] : Maillage.T[t + 2];
    }
    Composant->CreateMeshSection(0, Sommets, Triangles, Normales, TArray<FVector2D>(), Couleurs, Tangentes, false);
    Etat.SectionVide = NT == 0;
}

void ADDScene::ReproduireMaillages()
{
    const int32 Ordre = CVarOrdreUnity.GetValueOnGameThread();
    if (Ordre != FacesInversees)
    {
        FacesInversees = Ordre;
        for (auto& P : Etats) P.Value.VersionMaillage = -1;          // tout recopier dans le nouvel ordre
    }
    LireAmbiance();

    const Moteur::Liste<Moteur::Ref<Moteur::GameObject>>& Objets = Moteur::Boucle::Objets();
    for (int32 i = 0; i < Objets.Count(); i++)
    {
        const Moteur::Ref<Moteur::GameObject>& Go = Objets[i];
        if (Go == nullptr) continue;
        const Moteur::Ref<Moteur::MeshRenderer> Rendu = Go->GetComponent<Moteur::MeshRenderer>();
        if (Rendu == nullptr) continue;
        const int32 Id = Rendu->GetInstanceID();
        const Moteur::Ref<Moteur::MeshFilter> Filtre = Go->GetComponent<Moteur::MeshFilter>();
        const Moteur::Mesh* Maillage = (Filtre != nullptr && Filtre->sharedMesh != nullptr) ? Filtre->sharedMesh.get() : nullptr;
        const Moteur::Material* Materiau = Rendu->sharedMaterial != nullptr ? Rendu->sharedMaterial.get() : nullptr;
        const Moteur::Ref<Moteur::Transform> T = Go->transform();
        const Moteur::Vector3 Echelle = T->lossyScale();
        const bool Montre = Go->activeInHierarchy() && Rendu->enabled && Maillage != nullptr && Maillage->T.Count() >= 3 && Materiau != nullptr &&
                            Rendu->shadowCastingMode != Moteur::ShadowCastingMode::ShadowsOnly && !Nul(Echelle);

        FDDEtatMaillage* Etat = Etats.Find(Id);
        UProceduralMeshComponent* Composant = Etat != nullptr ? Maillages.FindRef(Id).Get() : nullptr;
        if (!Montre)
        {
            if (Etat != nullptr)
            {
                Etat->Vu = ImageUnreal;
                if (Etat->Visible && Composant != nullptr) Composant->SetVisibility(false);
                Etat->Visible = false;
            }
            continue;
        }
        if (Etat == nullptr || Composant == nullptr)
        {
            Composant = NewObject<UProceduralMeshComponent>(this, NAME_None, RF_Transient);
            Composant->SetupAttachment(Racine);
            Composant->SetMobility(EComponentMobility::Movable);
            Composant->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Composant->SetGenerateOverlapEvents(false);
            Composant->bUseAsyncCooking = false;
            Composant->bAffectDistanceFieldLighting = false;
            Composant->SetCastShadow(false);
            Composant->SetVisibility(false);
            Composant->RegisterComponent();
            Maillages.Add(Id, Composant);
            Etat = &Etats.Add(Id, FDDEtatMaillage());
        }
        Etat->Vu = ImageUnreal;

        // le maillage
        if (Etat->Maillage != Maillage->GetInstanceID() || Etat->VersionMaillage != Maillage->Version())
        {
            CopierMaillage(Composant, *Maillage, *Etat);
            Etat->Maillage = Maillage->GetInstanceID();
            Etat->VersionMaillage = Maillage->Version();
        }

        // le matériau
        const EGenre GenreRendu = Genre(*Materiau);
        Etat->Genre = GenreRendu;
        UMaterialInterface* M = MateriauPour(Id, *Rendu, *Materiau, GenreRendu);
        if (M != nullptr && Composant->GetMaterial(0) != M) Composant->SetMaterial(0, M);

        // les ombres : seuls les voxels éclairés en portent (les shaders Lueur et Fx d'Unity n'ont pas de passe d'ombre)
        const bool Ombre = GenreRendu == EGenre::Voxel && Rendu->shadowCastingMode != Moteur::ShadowCastingMode::Off;
        if (Ombre != Etat->Ombre)
        {
            Composant->SetCastShadow(Ombre);
            Etat->Ombre = Ombre;
        }

        // l'ordre des transparences (la file de rendu d'Unity : 2950 l'eau, 3000 les effets, 3100 les marqueurs)
        const int32 Priorite = GenreRendu == EGenre::Voxel || GenreRendu == EGenre::Lueur ? 0 : Materiau->renderQueue - 3000;
        if (Priorite != Etat->Priorite)
        {
            Composant->SetTranslucentSortPriority(Priorite);
            Etat->Priorite = Priorite;
        }

        // la place dans le monde
        const Moteur::Matrix4x4 Matrice = T->localToWorldMatrix();
        if (!Etat->MatriceConnue || FMemory::Memcmp(&Matrice, &Etat->Matrice, sizeof(Matrice)) != 0)
        {
            Etat->Matrice = Matrice;
            Etat->MatriceConnue = true;
            const FVector Position = DDUnreal::Position(Moteur::Vector3(Matrice.m03, Matrice.m13, Matrice.m23));
            Composant->SetWorldTransform(FTransform(DDUnreal::Rotation(T->rotation()), Position, DDUnreal::Echelle(Echelle)));
        }

        const bool Visible = !Etat->SectionVide;
        if (Visible != Etat->Visible)
        {
            Composant->SetVisibility(Visible);
            Etat->Visible = Visible;
        }
    }

    // ce que le jeu a détruit
    for (auto It = Etats.CreateIterator(); It; ++It)
    {
        if (It.Value().Vu == ImageUnreal) continue;
        const int32 Id = It.Key();
        if (UProceduralMeshComponent* C = Maillages.FindRef(Id).Get()) C->DestroyComponent();
        Maillages.Remove(Id);
        MateriauxPropres.Remove(Id);
        VersionsMateriauxPropres.Remove(Id);
        It.RemoveCurrent();
    }
}
