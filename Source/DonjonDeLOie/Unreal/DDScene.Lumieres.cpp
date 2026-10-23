// Donjon de l'Oie — la scène : le soleil, les lampes, la brume et le post-traitement.
//
// Les intensités : Unity (URP) éclaire une surface blanche de « couleur × intensité » ; Unreal compte en unités
// physiques (lux pour le soleil, candelas pour les lampes) et divise par π (diffusion de Lambert). Avec une exposition
// fixe de 1 (exposition manuelle, sans caméra physique), multiplier les intensités d'Unity par π donne la même
// lumière. La lumière ambiante (trois tons du ciel, de l'horizon et du sol) est ajoutée par le matériau des voxels.
//
// L'ombre du soleil : dans Unity, shadowStrength laisse passer une part de la lumière dans l'ombre. Unreal n'a pas ce
// réglage : le soleil est fait de deux lampes de même direction, l'une ombrée (shadowStrength), l'autre non (le reste).
#include "DDScene.h"

#include "Camera/CameraComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/Scene.h"
#include "HAL/IConsoleManager.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "DDConversions.h"
#include "Monde/Ambiance.h"

namespace
{
    TAutoConsoleVariable<float> CVarExposition(TEXT("dd.Exposition"), 0.0f,
                                               TEXT("Correction d'exposition (en diaphragmes) ajoutée à celle du jeu, si l'image paraît trop sombre ou trop claire."),
                                               ECVF_Default);

    TAutoConsoleVariable<float> CVarOmbre(TEXT("dd.DistanceOmbres"), 80.0f, TEXT("Distance (m) jusqu'à laquelle le soleil porte des ombres."), ECVF_Default);

    /// La « température » de la balance des blancs d'URP (−100…100) en kelvins pour Unreal : URP déplace le point blanc
    /// le long du lieu des illuminants D (x = 0,31271 − t/65 × 0,05 ou 0,1) ; la formule de McCamy donne sa température.
    float Kelvins(float Temperature)
    {
        const float t1 = Temperature / 65.0f;
        const float x = 0.31271f - t1 * (t1 < 0.0f ? 0.1f : 0.05f);
        const float y = 2.87f * x - 3.0f * x * x - 0.27509507f;
        const float n = (x - 0.3320f) / (0.1858f - y);
        return 449.0f * n * n * n + 3525.0f * n * n + 6823.3f * n + 5520.33f;
    }

    UDirectionalLightComponent* NouveauSoleil(AActor* Proprietaire, USceneComponent* Parent, const TCHAR* Nom, bool Ombre)
    {
        UDirectionalLightComponent* L = NewObject<UDirectionalLightComponent>(Proprietaire, Nom);
        L->SetupAttachment(Parent);
        L->SetMobility(EComponentMobility::Movable);
        L->DynamicShadowDistanceMovableLight = CVarOmbre.GetValueOnGameThread() * 100.0f;
        L->DynamicShadowCascades = 3;
        L->SetCastShadows(Ombre);
        L->SetIntensity(0.0f);
        L->RegisterComponent();
        return L;
    }
}

void ADDScene::ReproduireLumieres()
{
    if (Soleil == nullptr) Soleil = NouveauSoleil(this, Racine, TEXT("Soleil"), true);
    if (SoleilDiffus == nullptr) SoleilDiffus = NouveauSoleil(this, Racine, TEXT("SoleilDiffus"), false);

    const Moteur::Light* Principal = nullptr;
    Moteur::Ref<Moteur::Transform> TransformPrincipal;
    const Moteur::Liste<Moteur::Ref<Moteur::GameObject>>& Objets = Moteur::Boucle::Objets();
    for (int32 i = 0; i < Objets.Count(); i++)
    {
        const Moteur::Ref<Moteur::GameObject>& Go = Objets[i];
        if (Go == nullptr) continue;
        const Moteur::Ref<Moteur::Light> L = Go->GetComponent<Moteur::Light>();
        if (L == nullptr) continue;
        const bool Allumee = Go->activeInHierarchy() && L->enabled;
        if (L->type == Moteur::LightType::Directional)
        {
            if (Allumee && Principal == nullptr)
            {
                Principal = L.get();
                TransformPrincipal = Go->transform();
            }
            continue;
        }
        if (L->type != Moteur::LightType::Point && L->type != Moteur::LightType::Spot) continue;
        const int32 Id = L->GetInstanceID();
        LampesVues.Add(Id, ImageUnreal);
        TObjectPtr<UPointLightComponent>& Lampe = Lampes.FindOrAdd(Id);
        if (Lampe == nullptr)
        {
            Lampe = NewObject<UPointLightComponent>(this, NAME_None, RF_Transient);
            Lampe->SetupAttachment(Racine);
            Lampe->SetMobility(EComponentMobility::Movable);
            Lampe->SetIntensityUnits(ELightUnits::Candelas);
            Lampe->SetCastShadows(false);
            Lampe->SetSourceRadius(0.0f);
            Lampe->SetIntensity(0.0f);
            Lampe->RegisterComponent();
        }
        Lampe->SetVisibility(Allumee);
        if (!Allumee) continue;
        Lampe->SetWorldLocation(DDUnreal::Position(Go->transform()->position()));
        Lampe->SetLightColor(DDUnreal::CouleurLineaire(L->color));
        Lampe->SetIntensity(UE_PI * L->intensity);
        // même atténuation qu'URP : en carré inverse de la distance, fondue jusqu'à zéro à la portée
        Lampe->SetAttenuationRadius(FMath::Max(1.0f, L->range * 100.0f));
    }
    for (auto It = LampesVues.CreateIterator(); It; ++It)
    {
        if (It.Value() == ImageUnreal) continue;
        if (UPointLightComponent* Lampe = Lampes.FindRef(It.Key()).Get()) Lampe->DestroyComponent();
        Lampes.Remove(It.Key());
        It.RemoveCurrent();
    }

    if (Principal == nullptr || TransformPrincipal == nullptr)
    {
        Soleil->SetVisibility(false);
        SoleilDiffus->SetVisibility(false);
        return;
    }
    // Unity : la lumière va dans la direction +z du soleil ; Unreal : dans la direction +X de la lampe
    const FQuat Rotation = DDUnreal::Rotation(TransformPrincipal->rotation());
    const FLinearColor Couleur = DDUnreal::CouleurLineaire(Principal->color);
    const float Lux = UE_PI * Principal->intensity;
    const float Ombree = Principal->shadows != Moteur::LightShadows::None ? FMath::Clamp(Principal->shadowStrength, 0.0f, 1.0f) : 0.0f;
    Soleil->SetWorldRotation(Rotation);
    SoleilDiffus->SetWorldRotation(Rotation);
    Soleil->SetLightColor(Couleur);
    SoleilDiffus->SetLightColor(Couleur);
    Soleil->SetIntensity(Lux * Ombree);
    SoleilDiffus->SetIntensity(Lux * (1.0f - Ombree));
    Soleil->SetVisibility(Ombree > 0.0f);
    SoleilDiffus->SetVisibility(Ombree < 1.0f);
}

void ADDScene::ReproduirePostTraitement()
{
    using DonjonDeLOie::Monde::Ambiance;
    const Ambiance::ReglagesPostTraitement& P = Ambiance::PostTraitement;
    FPostProcessSettings& S = Camera->PostProcessSettings;

    // exposition fixe : 1, plus la « post-exposition » d'URP (en diaphragmes)
    S.bOverride_AutoExposureMethod = true;
    S.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
    S.bOverride_AutoExposureApplyPhysicalCameraExposure = true;
    S.AutoExposureApplyPhysicalCameraExposure = false;
    S.bOverride_AutoExposureBias = true;
    S.AutoExposureBias = P.Exposition + CVarExposition.GetValueOnGameThread();

    // halo (bloom d'URP : intensité, seuil, diffusion)
    S.bOverride_BloomIntensity = true;
    S.BloomIntensity = P.Halo;
    S.bOverride_BloomThreshold = true;
    S.BloomThreshold = P.SeuilHalo;
    S.bOverride_BloomSizeScale = true;
    S.BloomSizeScale = 4.0f * (0.5f + P.DiffusionHalo);

    // couleurs (ColorAdjustments et WhiteBalance d'URP ; les tons ACES : le tone mapper d'Unreal par défaut)
    const float Contraste = 1.0f + P.Contraste / 100.0f;
    const float Saturation = 1.0f + P.Saturation / 100.0f;
    S.bOverride_ColorContrast = true;
    S.ColorContrast = FVector4(Contraste, Contraste, Contraste, 1.0f);
    S.bOverride_ColorSaturation = true;
    S.ColorSaturation = FVector4(Saturation, Saturation, Saturation, 1.0f);
    S.bOverride_SceneColorTint = true;
    S.SceneColorTint = DDUnreal::CouleurLineaire(P.Filtre);
    S.bOverride_WhiteTemp = true;
    S.WhiteTemp = Kelvins(P.Temperature);
    S.bOverride_VignetteIntensity = true;
    S.VignetteIntensity = P.Vignette;

    // ce qu'URP n'avait pas
    S.bOverride_MotionBlurAmount = true;
    S.MotionBlurAmount = 0.0f;
    S.bOverride_LensFlareIntensity = true;
    S.LensFlareIntensity = 0.0f;
    S.bOverride_AmbientOcclusionIntensity = true;
    S.AmbientOcclusionIntensity = 0.0f;

    // flou de profondeur (le mini-jeu de l'évadé) : net jusqu'à « début », de plus en plus flou au-delà
    S.bOverride_DepthOfFieldFocalDistance = true;
    if (P.FlouForce > 0.001f)
    {
        S.DepthOfFieldFocalDistance = FMath::Max(1.0f, P.FlouDebut * 100.0f);
        S.bOverride_DepthOfFieldFstop = true;
        S.DepthOfFieldFstop = FMath::Lerp(4.0f, 1.2f, P.FlouForce);
        S.bOverride_DepthOfFieldDepthBlurAmount = true;
        S.DepthOfFieldDepthBlurAmount = FMath::Max(0.001f, (P.FlouFin - P.FlouDebut) / 1000.0f);
        S.bOverride_DepthOfFieldDepthBlurRadius = true;
        S.DepthOfFieldDepthBlurRadius = FMath::Lerp(2.0f, 8.0f, P.FlouForce);
    }
    else
        S.DepthOfFieldFocalDistance = 0.0f;           // 0 : pas de flou de profondeur

    // la brume linéaire et la couleur de fond, par le matériau de post-traitement
    if (Brume != nullptr)
    {
        using Moteur::RenderSettings;
        const bool Active = RenderSettings::fog && RenderSettings::fogMode == Moteur::FogMode::Linear;
        Brume->SetScalarParameterValue(FName(DDMateriauxUnreal::BrumeActive), Active ? 1.0f : 0.0f);
        Brume->SetScalarParameterValue(FName(DDMateriauxUnreal::BrumeDebut), RenderSettings::fogStartDistance * 100.0f);
        Brume->SetScalarParameterValue(FName(DDMateriauxUnreal::BrumeFin), RenderSettings::fogEndDistance * 100.0f);
        Brume->SetVectorParameterValue(FName(DDMateriauxUnreal::BrumeCouleur), DDUnreal::CouleurLineaire(RenderSettings::fogColor));
        const Moteur::Ref<Moteur::Camera> Cam = Moteur::Camera::main();
        const Moteur::Color Fond = Cam != nullptr ? Cam->backgroundColor : RenderSettings::fogColor;
        Brume->SetVectorParameterValue(FName(DDMateriauxUnreal::BrumeFond), DDUnreal::CouleurLineaire(Fond));
        if (S.WeightedBlendables.Array.Num() == 0) S.WeightedBlendables.Array.Add(FWeightedBlendable(1.0f, Brume));
    }
}
