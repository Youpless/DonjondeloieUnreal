// Donjon de l'Oie — le module d'édition : il crée, à la première ouverture du projet dans l'éditeur, les matériaux qui
// rendent les shaders du jeu (dossier /Game/DonjonDeLOie/Materiaux), et les recrée quand leur version change
// (DDMateriauxUnreal::Version) ou sur demande (commande de console « dd.RecreerMateriaux »).
//
// Chaque matériau reproduit un shader d'Unity du jeu (Resources/DDVoxel.shader, DDLueur.shader, DDFx.shader) :
//  - M_DD_Voxel : éclairé (soleil, lampes, ombres d'Unreal) ; couleur des sommets passée de sRGB en linéaire, × _BaseColor ;
//    lumière ambiante en trois tons (ciel, horizon, sol) selon la normale, + _EmissionColor ; reflet doux (SimpleLit) ;
//  - M_DD_Lueur : sans éclairage ; couleur des sommets × _BaseColor × _Intensite, battement si _Pulsation > 0 ;
//  - M_DD_FxAdd, M_DD_FxDessus (toujours visible), M_DD_FxAlpha, M_DD_Eau (ondulation _Vague) : transparents, sans
//    éclairage, les deux faces ;
//  - M_DD_Brouillard : post-traitement, la brume linéaire d'Unity (RenderSettings.fog) et la couleur de fond de la caméra.
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformFileManager.h"
#include "MaterialDomain.h"
#include "MaterialEditingLibrary.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionCustom.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionSceneDepth.h"
#include "Materials/MaterialExpressionSceneTexture.h"
#include "Materials/MaterialExpressionTime.h"
#include "Materials/MaterialExpressionTruncateLWC.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionVertexColor.h"
#include "Materials/MaterialExpressionVertexNormalWS.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "Misc/CoreDelegates.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

#include "Unreal/DDMateriauxUnreal.h"

DEFINE_LOG_CATEGORY_STATIC(LogDonjonEditeur, Log, All);

using DDMateriauxUnreal::EGenre;

namespace
{
    // ------------------------------------------------------------------ morceaux de code HLSL (nœuds « Custom »)

    /// sRGB → linéaire, comme SRGBToLinear d'URP (les couleurs des sommets du jeu sont en sRGB).
    const TCHAR* LineaireHlsl = TEXT("float3 c = saturate(VC);\n")
                                TEXT("float3 lin = lerp(pow((c + 0.055) / 1.055, 2.4), c / 12.92, step(c, 0.04045));\n");

    // ------------------------------------------------------------------ outils de construction

    struct FGraphe
    {
        UMaterial* M = nullptr;
        int32 Y = 0;

        UMaterialExpression* Ajouter(TSubclassOf<UMaterialExpression> Classe, int32 X = -800)
        {
            UMaterialExpression* E = UMaterialEditingLibrary::CreateMaterialExpression(M, Classe, X, Y);
            Y += 140;
            return E;
        }

        UMaterialExpression* Vecteur(const TCHAR* Nom, const FLinearColor& Defaut)
        {
            auto* P = Cast<UMaterialExpressionVectorParameter>(Ajouter(UMaterialExpressionVectorParameter::StaticClass()));
            P->ParameterName = FName(Nom);
            P->DefaultValue = Defaut;
            return P;
        }

        UMaterialExpression* Scalaire(const TCHAR* Nom, float Defaut)
        {
            auto* P = Cast<UMaterialExpressionScalarParameter>(Ajouter(UMaterialExpressionScalarParameter::StaticClass()));
            P->ParameterName = FName(Nom);
            P->DefaultValue = Defaut;
            return P;
        }

        UMaterialExpression* Constante(float Valeur)
        {
            auto* C = Cast<UMaterialExpressionConstant>(Ajouter(UMaterialExpressionConstant::StaticClass()));
            C->R = Valeur;
            return C;
        }

        /// Un nœud « Custom » : son code HLSL et ses entrées (nom, nœud source, sortie du nœud source).
        struct FEntree
        {
            const TCHAR* Nom;
            UMaterialExpression* Source;
            const TCHAR* Sortie;
        };

        UMaterialExpression* Code(const FString& Hlsl, ECustomMaterialOutputType Type, std::initializer_list<FEntree> Entrees)
        {
            auto* C = Cast<UMaterialExpressionCustom>(Ajouter(UMaterialExpressionCustom::StaticClass(), -400));
            C->Code = Hlsl;
            C->OutputType = Type;
            C->Description = TEXT("Donjon de l'Oie");
            C->Inputs.Reset();
            for (const FEntree& E : Entrees)
            {
                FCustomInput Entree;
                Entree.InputName = FName(E.Nom);
                C->Inputs.Add(Entree);
            }
            for (const FEntree& E : Entrees) UMaterialEditingLibrary::ConnectMaterialExpressions(E.Source, E.Sortie, C, E.Nom);
            return C;
        }

        /// La position dans le monde en réels simples (les nœuds « Custom » ne prennent pas les coordonnées étendues).
        UMaterialExpression* PositionMonde()
        {
            UMaterialExpression* P = Ajouter(UMaterialExpressionWorldPosition::StaticClass(), -1100);
            auto* T = Cast<UMaterialExpressionTruncateLWC>(Ajouter(UMaterialExpressionTruncateLWC::StaticClass()));
            T->Input.Connect(0, P);
            return T;
        }

        void Sortie(UMaterialExpression* E, EMaterialProperty Propriete) { UMaterialEditingLibrary::ConnectMaterialProperty(E, TEXT(""), Propriete); }
    };

    /// La lumière ambiante de départ (Ambiance.Installer, en linéaire), pour l'aperçu dans l'éditeur.
    FLinearColor Lin(float r, float g, float b) { return FLinearColor(FColor(FMath::RoundToInt(r * 255.0f), FMath::RoundToInt(g * 255.0f), FMath::RoundToInt(b * 255.0f))); }

    void Construire(FGraphe& G, EGenre Genre)
    {
        UMaterial* M = G.M;
        UMaterialExpression* VC = G.Ajouter(UMaterialExpressionVertexColor::StaticClass());
        UMaterialExpression* Teinte = G.Vecteur(DDMateriauxUnreal::Teinte, FLinearColor::White);
        switch (Genre)
        {
            case EGenre::Voxel:
            {
                M->MaterialDomain = MD_Surface;
                M->BlendMode = BLEND_Opaque;
                M->SetShadingModel(MSM_DefaultLit);
                UMaterialExpression* Emission = G.Vecteur(DDMateriauxUnreal::Emission, FLinearColor::Black);
                UMaterialExpression* Ciel = G.Vecteur(DDMateriauxUnreal::AmbiantCiel, Lin(0.58f, 0.67f, 0.80f));
                UMaterialExpression* Equateur = G.Vecteur(DDMateriauxUnreal::AmbiantEquateur, Lin(0.62f, 0.60f, 0.52f));
                UMaterialExpression* Sol = G.Vecteur(DDMateriauxUnreal::AmbiantSol, Lin(0.30f, 0.27f, 0.23f));
                UMaterialExpression* Normale = G.Ajouter(UMaterialExpressionVertexNormalWS::StaticClass());
                UMaterialExpression* Albedo = G.Code(FString(LineaireHlsl) + TEXT("return lin * Teinte;"), CMOT_Float3, {{TEXT("VC"), VC, TEXT("")}, {TEXT("Teinte"), Teinte, TEXT("")}});
                // la lumière ambiante en trois tons (AmbientMode.Trilight) : ciel au-dessus, horizon de côté, sol en dessous
                UMaterialExpression* Ambiance = G.Code(TEXT("float y = clamp(N.z, -1.0, 1.0);\n")
                                                           TEXT("float3 amb = y >= 0.0 ? lerp(Equateur, Ciel, y) : lerp(Equateur, Sol, -y);\n")
                                                           TEXT("return Albedo * amb + Emission;"),
                                                       CMOT_Float3,
                                                       {{TEXT("Albedo"), Albedo, TEXT("")},
                                                        {TEXT("N"), Normale, TEXT("")},
                                                        {TEXT("Ciel"), Ciel, TEXT("")},
                                                        {TEXT("Equateur"), Equateur, TEXT("")},
                                                        {TEXT("Sol"), Sol, TEXT("")},
                                                        {TEXT("Emission"), Emission, TEXT("")}});
                G.Sortie(Albedo, MP_BaseColor);
                G.Sortie(Ambiance, MP_EmissiveColor);
                G.Sortie(G.Constante(0.0f), MP_Metallic);
                G.Sortie(G.Constante(0.5f), MP_Specular);
                G.Sortie(G.Constante(0.65f), MP_Roughness);
                break;
            }
            case EGenre::Lueur:
            {
                M->MaterialDomain = MD_Surface;
                M->BlendMode = BLEND_Opaque;
                M->SetShadingModel(MSM_Unlit);
                UMaterialExpression* Intensite = G.Scalaire(DDMateriauxUnreal::Intensite, 2.2f);
                UMaterialExpression* Pulsation = G.Scalaire(DDMateriauxUnreal::Pulsation, 0.0f);
                UMaterialExpression* Position = G.PositionMonde();
                UMaterialExpression* Temps = G.Ajouter(UMaterialExpressionTime::StaticClass());
                // battement : 0,82 + 0,18 sin(t × pulsation + x × 1,7 + z × 1,3), x et z d'Unity en mètres (Y et X d'Unreal)
                UMaterialExpression* Couleur = G.Code(FString(LineaireHlsl) + TEXT("float p = 1.0;\n")
                                                          TEXT("if (Pulsation > 0.0) p = 0.82 + 0.18 * sin(T * Pulsation + WP.y * 0.017 + WP.x * 0.013);\n")
                                                          TEXT("return lin * Teinte * (Intensite * p);"),
                                                      CMOT_Float3,
                                                      {{TEXT("VC"), VC, TEXT("")},
                                                       {TEXT("Teinte"), Teinte, TEXT("")},
                                                       {TEXT("Intensite"), Intensite, TEXT("")},
                                                       {TEXT("Pulsation"), Pulsation, TEXT("")},
                                                       {TEXT("WP"), Position, TEXT("")},
                                                       {TEXT("T"), Temps, TEXT("")}});
                G.Sortie(Couleur, MP_EmissiveColor);
                break;
            }
            case EGenre::FxAdd:
            case EGenre::FxDessus:
            {
                M->MaterialDomain = MD_Surface;
                M->BlendMode = BLEND_Additive;
                M->SetShadingModel(MSM_Unlit);
                M->TwoSided = 1;
                M->TranslucencyPass = MTP_BeforeDOF;
                if (Genre == EGenre::FxDessus) M->bDisableDepthTest = 1;
                UMaterialExpression* Couleur = G.Code(FString(LineaireHlsl) + TEXT("return lin * Teinte;"), CMOT_Float3, {{TEXT("VC"), VC, TEXT("")}, {TEXT("Teinte"), Teinte, TEXT("")}});
                G.Sortie(Couleur, MP_EmissiveColor);
                break;
            }
            case EGenre::FxAlpha:
            case EGenre::Eau:
            {
                M->MaterialDomain = MD_Surface;
                M->BlendMode = BLEND_Translucent;
                M->SetShadingModel(MSM_Unlit);
                M->TwoSided = 1;
                M->TranslucencyPass = MTP_BeforeDOF;
                UMaterialExpression* Opacite = G.Code(TEXT("return saturate(VA * TA);"), CMOT_Float1, {{TEXT("VA"), VC, TEXT("A")}, {TEXT("TA"), Teinte, TEXT("A")}});
                G.Sortie(Opacite, MP_Opacity);
                if (Genre == EGenre::FxAlpha)
                {
                    UMaterialExpression* Couleur = G.Code(FString(LineaireHlsl) + TEXT("return lin * Teinte;"), CMOT_Float3, {{TEXT("VC"), VC, TEXT("")}, {TEXT("Teinte"), Teinte, TEXT("")}});
                    G.Sortie(Couleur, MP_EmissiveColor);
                    break;
                }
                // l'eau : les sommets ondulent (en mètres d'Unity : x = Y/100, z = X/100 d'Unreal), le reflet suit la vague
                UMaterialExpression* Vague = G.Scalaire(DDMateriauxUnreal::Vague, 0.16f);
                UMaterialExpression* Position = G.PositionMonde();
                UMaterialExpression* Temps = G.Ajouter(UMaterialExpressionTime::StaticClass());
                const FString Hauteur = TEXT("float3 w = WP * 0.01;\n")
                                        TEXT("float h = sin(w.y * 0.55 + T * 1.1) * 0.5 + sin(w.x * 0.7 - T * 0.9) * 0.35 + sin((w.y + w.x) * 1.3 + T * 2.0) * 0.15;\n");
                UMaterialExpression* Couleur = G.Code(FString(LineaireHlsl) + Hauteur + TEXT("float reflet = Vague > 0.0 ? 0.9 + 0.2 * h : 1.0;\n") TEXT("return lin * Teinte * reflet;"),
                                                      CMOT_Float3,
                                                      {{TEXT("VC"), VC, TEXT("")},
                                                       {TEXT("Teinte"), Teinte, TEXT("")},
                                                       {TEXT("Vague"), Vague, TEXT("")},
                                                       {TEXT("WP"), Position, TEXT("")},
                                                       {TEXT("T"), Temps, TEXT("")}});
                UMaterialExpression* Decalage = G.Code(Hauteur + TEXT("return float3(0.0, 0.0, Vague > 0.0 ? h * Vague * 100.0 : 0.0);"), CMOT_Float3,
                                                       {{TEXT("Vague"), Vague, TEXT("")}, {TEXT("WP"), Position, TEXT("")}, {TEXT("T"), Temps, TEXT("")}});
                G.Sortie(Couleur, MP_EmissiveColor);
                G.Sortie(Decalage, MP_WorldPositionOffset);
                break;
            }
            default:
                break;
        }
    }

    void ConstruireBrouillard(FGraphe& G)
    {
        UMaterial* M = G.M;
        M->MaterialDomain = MD_PostProcess;
        M->BlendableLocation = BL_SceneColorBeforeDOF;
        auto* Scene = Cast<UMaterialExpressionSceneTexture>(G.Ajouter(UMaterialExpressionSceneTexture::StaticClass()));
        Scene->SceneTextureId = PPI_PostProcessInput0;
        UMaterialExpression* Profondeur = G.Ajouter(UMaterialExpressionSceneDepth::StaticClass());
        UMaterialExpression* Couleur = G.Vecteur(DDMateriauxUnreal::BrumeCouleur, Lin(0.80f, 0.86f, 0.89f));
        UMaterialExpression* Fond = G.Vecteur(DDMateriauxUnreal::BrumeFond, Lin(0.80f, 0.86f, 0.89f));
        UMaterialExpression* Debut = G.Scalaire(DDMateriauxUnreal::BrumeDebut, 8500.0f);
        UMaterialExpression* Fin = G.Scalaire(DDMateriauxUnreal::BrumeFin, 32000.0f);
        UMaterialExpression* Active = G.Scalaire(DDMateriauxUnreal::BrumeActive, 1.0f);
        // Unity (brume linéaire) : f = (fin − d) / (fin − début), d la profondeur vue de la caméra ; la couleur de la
        // scène est « pré-exposée » dans Unreal, la couleur de la brume doit l'être aussi.
        UMaterialExpression* Resultat = G.Code(TEXT("float d = Profondeur;\n")
                                                   TEXT("if (d > 1.0e7) return Fond * View.PreExposure;\n")
                                                   TEXT("float f = saturate((Fin - d) / max(Fin - Debut, 1.0));\n")
                                                   TEXT("f = lerp(1.0, f, Active);\n")
                                                   TEXT("return lerp(Couleur * View.PreExposure, Scene.rgb, f);"),
                                               CMOT_Float3,
                                               {{TEXT("Scene"), Scene, TEXT("Color")},
                                                {TEXT("Profondeur"), Profondeur, TEXT("")},
                                                {TEXT("Couleur"), Couleur, TEXT("")},
                                                {TEXT("Fond"), Fond, TEXT("")},
                                                {TEXT("Debut"), Debut, TEXT("")},
                                                {TEXT("Fin"), Fin, TEXT("")},
                                                {TEXT("Active"), Active, TEXT("")}});
        G.Sortie(Resultat, MP_EmissiveColor);
    }

    /// Crée (ou recrée) un matériau, puis l'enregistre dans son paquet.
    bool CreerMateriau(const TCHAR* Nom, TFunctionRef<void(FGraphe&)> Remplir)
    {
        const FString NomPaquet = DDMateriauxUnreal::Paquet(Nom);
        UPackage* Paquet = CreatePackage(*NomPaquet);
        if (Paquet == nullptr) return false;
        Paquet->FullyLoad();
        UMaterial* M = FindObject<UMaterial>(Paquet, Nom);
        const bool Nouveau = M == nullptr;
        if (Nouveau) M = NewObject<UMaterial>(Paquet, FName(Nom), RF_Public | RF_Standalone | RF_Transactional);
        else UMaterialEditingLibrary::DeleteAllMaterialExpressions(M);
        FGraphe G;
        G.M = M;
        Remplir(G);
        UMaterialEditingLibrary::LayoutMaterialExpressions(M);
        UMaterialEditingLibrary::RecompileMaterial(M);
        if (Nouveau) FAssetRegistryModule::AssetCreated(M);
        Paquet->MarkPackageDirty();
        const FString Fichier = FPackageName::LongPackageNameToFilename(NomPaquet, FPackageName::GetAssetPackageExtension());
        FSavePackageArgs Arguments;
        Arguments.TopLevelFlags = RF_Public | RF_Standalone;
        Arguments.SaveFlags = SAVE_NoError;
        const bool Enregistre = UPackage::SavePackage(Paquet, M, *Fichier, Arguments);
        UE_LOG(LogDonjonEditeur, Log, TEXT("Matériau %s %s."), Nom, Enregistre ? TEXT("créé") : TEXT("créé mais non enregistré"));
        return Enregistre;
    }

    FString FichierVersion() { return FPaths::ProjectContentDir() / TEXT("DonjonDeLOie/Materiaux/version.txt"); }

    bool AJour()
    {
        FString Texte;
        if (!FFileHelper::LoadFileToString(Texte, *FichierVersion())) return false;
        if (FCString::Atoi(*Texte) != DDMateriauxUnreal::Version) return false;
        for (int32 g = 0; g < static_cast<int32>(EGenre::Nombre); g++)
            if (!FPackageName::DoesPackageExist(DDMateriauxUnreal::Paquet(DDMateriauxUnreal::Nom(static_cast<EGenre>(g))))) return false;
        return FPackageName::DoesPackageExist(DDMateriauxUnreal::Paquet(DDMateriauxUnreal::NomBrouillard));
    }

    void CreerTout()
    {
        UE_LOG(LogDonjonEditeur, Log, TEXT("Donjon de l'Oie : création des matériaux (version %d)…"), DDMateriauxUnreal::Version);
        bool Tout = true;
        for (int32 g = 0; g < static_cast<int32>(EGenre::Nombre); g++)
        {
            const EGenre Genre = static_cast<EGenre>(g);
            Tout &= CreerMateriau(DDMateriauxUnreal::Nom(Genre), [Genre](FGraphe& G) { Construire(G, Genre); });
        }
        Tout &= CreerMateriau(DDMateriauxUnreal::NomBrouillard, [](FGraphe& G) { ConstruireBrouillard(G); });
        if (Tout) FFileHelper::SaveStringToFile(FString::FromInt(DDMateriauxUnreal::Version), *FichierVersion());
    }

    FAutoConsoleCommand CommandeRecreer(TEXT("dd.RecreerMateriaux"), TEXT("Recrée les matériaux du Donjon de l'Oie (dossier /Game/DonjonDeLOie/Materiaux)."),
                                        FConsoleCommandDelegate::CreateLambda([]() { CreerTout(); }));
}

class FDonjonDeLOieEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        // une fois le moteur prêt (le module se charge après lui, voir le .uproject ; sinon on attend)
        if (GEngine != nullptr) Verifier();
        else Attente = FCoreDelegates::OnPostEngineInit.AddLambda([this]() { Verifier(); });
    }

    virtual void ShutdownModule() override
    {
        if (Attente.IsValid()) FCoreDelegates::OnPostEngineInit.Remove(Attente);
    }

private:
    void Verifier()
    {
        if (!AJour()) CreerTout();
    }

    FDelegateHandle Attente;
};

IMPLEMENT_MODULE(FDonjonDeLOieEditorModule, DonjonDeLOieEditor);
