// Donjon de l'Oie — lumière et atmosphère : soleil d'après-midi, ciel en dégradé, brume légère, post-traitement d'URP
// (halo, tons ACES, couleurs, vignette) posé par réflexion pour que le jeu compile aussi sans URP.
//
// Portage : chaque « Regler(volume, champ, valeur) » du C# écrit le champ correspondant d'Ambiance::PostTraitement ;
// aucun objet n'est créé pour le post-traitement (le C# n'en crée pas non plus sans URP, dans le moteur des essais).
#include "Ambiance.h"

#include "../Voxel/VoxelMesher.h"
#include "DDMateriaux.h"
#include "Formes.h"
#include "IleVue.h"

namespace DonjonDeLOie::Monde
{
    using namespace DonjonDeLOie::Voxel;

    /// Flou de profondeur gaussien : net jusqu'à « debut » mètres de la caméra, flou complet à « fin ». force 0 : éteint.
    /// La vignette se resserre un peu avec lui.
    void Ambiance::Flou(float force, float debut, float fin)
    {
        forceFlou = Mathf::Clamp01(force); debutFlou = debut; finFlou = fin;
        PostTraitement.Vignette = Mathf::Lerp(0.22f, 0.38f, forceFlou);                 // Regler(vignette, "intensity", …)
        // (le C# s'arrête ici quand il n'a pas de volume de flou ; les réglages, eux, sont toujours gardés)
        if (force <= 0.001f) { PostTraitement.FlouForce = 0.0f; return; }              // Regler(profondeur, "mode", "Off")
        // mode Gaussian ; gaussianStart, gaussianEnd ; gaussianMaxRadius = Lerp(0.5, 1.5, Clamp01(force)) ; haute qualité
        PostTraitement.FlouForce = Mathf::Clamp01(force);
        PostTraitement.FlouDebut = debut;
        PostTraitement.FlouFin = fin;
    }

    void Ambiance::Installer(const Ref<Transform>& parent, const Ref<Camera>& cam)
    {
        // Soleil
        auto sg = Neuf<GameObject>("Soleil");
        sg->transform()->SetParent(parent, false);
        sg->transform()->rotation(Quaternion::Euler(48.0f, 38.0f, 0.0f));
        Soleil = sg->AddComponent<Light>();
        Soleil->type = LightType::Directional;
        Soleil->color = Color(1.0f, 0.93f, 0.82f);
        Soleil->intensity = 1.35f;
        Soleil->shadows = LightShadows::Soft;
        Soleil->shadowStrength = 0.78f;
        RenderSettings::sun = Soleil;

        // Lumière ambiante en trois tons et brume
        RenderSettings::skybox = nullptr;
        RenderSettings::ambientMode = AmbientMode::Trilight;
        RenderSettings::ambientSkyColor = Color(0.58f, 0.67f, 0.80f);
        RenderSettings::ambientEquatorColor = Color(0.62f, 0.60f, 0.52f);
        RenderSettings::ambientGroundColor = Color(0.30f, 0.27f, 0.23f);
        RenderSettings::ambientIntensity = 1.0f;
        RenderSettings::fog = true;
        RenderSettings::fogMode = FogMode::Linear;
        RenderSettings::fogColor = Horizon;
        RenderSettings::fogStartDistance = 85.0f;
        RenderSettings::fogEndDistance = 320.0f;
        DynamicGI::UpdateEnvironment();                 // (try … catch du C# : rien à attraper ici)

        cam->backgroundColor = Horizon;
        Ciel(parent);
        InstallerPostTraitement(parent, cam);
    }

    /// Dôme du ciel : dégradé du zénith à l'horizon, sans éclairage.
    void Ambiance::Ciel(const Ref<Transform>& parent)
    {
        auto md = Nouveau<MeshData>();
        const int seg = 48, anneaux = 16;
        const float r = 170.0f;
        Color32 zen = Zenith, hor = Horizon, bas = Color(0.62f, 0.75f, 0.84f);
        for (int j = 0; j <= anneaux; j++)
        {
            float v = static_cast<float>(j) / static_cast<float>(anneaux);            // 0 : bas (−10°), 1 : zénith
            float lat = Mathf::Lerp(-12.0f, 90.0f, v) * Mathf::Deg2Rad;
            Color32 c = v < 0.12f ? Color32::Lerp(bas, hor, v / 0.12f) : Color32::Lerp(hor, zen, Mathf::Pow((v - 0.12f) / 0.88f, 0.7f));
            for (int i = 0; i <= seg; i++)
            {
                float lon = static_cast<float>(i) / static_cast<float>(seg) * Mathf::Pi * 2.0f;
                md->V.Add(Vector3(Mathf::Cos(lat) * Mathf::Cos(lon), Mathf::Sin(lat), Mathf::Cos(lat) * Mathf::Sin(lon)) * r);
                md->N.Add(Vector3::down);
                md->C.Add(c);
            }
        }
        for (int j = 0; j < anneaux; j++)
            for (int i = 0; i < seg; i++)
            {
                int a = j * (seg + 1) + i, b = a + 1, c = a + seg + 1, d = c + 1;
                // vu de l'intérieur
                md->T.Add(a); md->T.Add(b); md->T.Add(d);
                md->T.Add(a); md->T.Add(d); md->T.Add(c);
            }
        auto go = Neuf<GameObject>("Ciel");
        go->transform()->SetParent(parent, false);
        // C# : go.AddComponent<MeshFilter>().sharedMesh = DDMateriaux.Maillage(md, "ciel") — C# évalue la gauche
        // d'abord (le MeshFilter, puis le maillage) ; en C++ une affectation évalue la droite d'abord : deux lignes.
        auto mf = go->AddComponent<MeshFilter>();
        mf->sharedMesh = DDMateriaux::Maillage(md, "ciel");
        auto mr = go->AddComponent<MeshRenderer>();
        auto mat = Neuf<Material>(DDMateriaux::Lueur(1.0f, 0.0f));
        mat->renderQueue = 1990;
        mr->sharedMaterial = mat;
        mr->shadowCastingMode = ShadowCastingMode::Off;
        mr->receiveShadows = false;
        go->AddComponent<SuivreCamera>();
        cielMat = mat;
        // la lune, à l'opposé du soleil, visible seulement la nuit
        Vector3 versLune = Vector3(-0.51f, 0.56f, -0.65f).normalized();
        lune = Formes::Disque(go->transform(), 7.0f, Color(1.0f, 0.97f, 0.88f), Color(0.9f, 0.9f, 0.86f), DDMateriaux::Lueur(1.8f));
        lune->name("Lune");
        lune->transform()->localPosition = versLune * 150.0f;
        lune->transform()->localRotation = Quaternion::FromToRotation(Vector3::up, -versLune);
        lune->SetActive(false);
    }

    /// Du jour (0) à la nuit (1) : le soleil devient lune, la lumière ambiante bleuit, la brume s'assombrit, les lampes
    /// brillent davantage, et le post-traitement d'URP refroidit l'image.
    void Ambiance::Nuit(float k, const Ref<Camera>& cam)
    {
        k = Mathf::Clamp01(k);
        nuitActuelle = k;
        if (Soleil != nullptr)
        {
            Soleil->color = Color::Lerp(Color(1.0f, 0.93f, 0.82f), Color(0.55f, 0.66f, 1.0f), k);
            Soleil->intensity = Mathf::Lerp(1.35f, 0.4f, k);
            Soleil->shadowStrength = Mathf::Lerp(0.78f, 0.55f, k);
        }
        RenderSettings::ambientSkyColor = Color::Lerp(Color(0.58f, 0.67f, 0.80f), Color(0.12f, 0.15f, 0.28f), k);
        RenderSettings::ambientEquatorColor = Color::Lerp(Color(0.62f, 0.60f, 0.52f), Color(0.10f, 0.11f, 0.18f), k);
        RenderSettings::ambientGroundColor = Color::Lerp(Color(0.30f, 0.27f, 0.23f), Color(0.05f, 0.05f, 0.08f), k);
        Color brume = Color::Lerp(Horizon, Color(0.07f, 0.09f, 0.18f), k);
        RenderSettings::fogColor = brume;
        if (cam != nullptr) cam->backgroundColor = brume;
        CielHaut = Color::Lerp(Zenith, Color(0.03f, 0.05f, 0.14f), k);
        CielBas = brume;
        if (cielMat != nullptr) cielMat->SetFloat("_Intensite", Mathf::Lerp(1.0f, 0.16f, k));
        Vacillement::Facteur = 1.0f + 1.4f * k;
        if (lune != nullptr)
        {
            lune->SetActive(k > 0.02f);
            lune->transform()->localScale = Vector3::one * Mathf::Lerp(0.4f, 1.0f, k);
        }
        PostTraitement.Exposition = Mathf::Lerp(0.35f, 0.05f, k);                                   // Regler(ajustements, "postExposure", …)
        PostTraitement.Saturation = Mathf::Lerp(10.0f, -22.0f, k);                                  // Regler(ajustements, "saturation", …)
        PostTraitement.Filtre = Color::Lerp(Color::white, Color(0.74f, 0.82f, 1.0f), k);            // Regler(ajustements, "colorFilter", …)
        PostTraitement.Temperature = Mathf::Lerp(6.0f, -14.0f, k);                                  // Regler(balance, "temperature", …)
        // la lumière ambiante d'URP est recalculée de temps en temps pendant la transition, et à ses deux bouts
        if (Mathf::Abs(k - giFait) > 0.08f || k == 0.0f || k == 1.0f)
        {
            giFait = k;
            DynamicGI::UpdateEnvironment();
        }
    }

    // ------------------------------------------------------------------ post-traitement (URP, par réflexion)

    /// Le PostTraitement(parent, cam) du C#. Le C# cherche par réflexion les types d'URP (Volume, VolumeProfile,
    /// UniversalAdditionalCameraData) ; sans eux il s'arrête aussitôt, sans rien créer. Ici non plus aucun objet : les
    /// réglages que le C# pose sur les volumes neufs vont dans PostTraitement (la couche Unreal les applique).
    void Ambiance::InstallerPostTraitement(const Ref<Transform>& /*parent*/, const Ref<Camera>& /*cam*/)
    {
        PostTraitement = ReglagesPostTraitement{};
        PostTraitement.Halo = 0.55f; PostTraitement.SeuilHalo = 1.05f; PostTraitement.DiffusionHalo = 0.62f;     // Bloom
        // Tonemapping : ACES (fixe)
        PostTraitement.Exposition = 0.35f; PostTraitement.Contraste = 12.0f; PostTraitement.Saturation = 10.0f;    // ColorAdjustments
        PostTraitement.Filtre = Color::white;
        PostTraitement.Temperature = 6.0f;                                                                         // WhiteBalance
        // flou de profondeur (gaussien), éteint : le mini-jeu de l'évadé l'allume pour flouter la ville
        PostTraitement.FlouForce = 0.0f;                                                                           // DepthOfField : mode Off
        PostTraitement.Vignette = 0.22f; PostTraitement.DouceurVignette = 0.5f;                                    // Vignette
    }

    void SuivreCamera::LateUpdate()
    {
        auto c = Camera::main();
        if (c != nullptr) transform()->position(Vector3(c->transform()->position().x, 0.0f, c->transform()->position().z));
    }

    // Remise à zéro des variables statiques (une partie relancée dans Unreal repart comme un lancement d'Unity).
    const bool Ambiance::remiseAZero = Boucle::AuReinitialiser([] {
        Ambiance::Soleil = nullptr;
        Ambiance::nuitActuelle = 0.0f;
        Ambiance::CielHaut = Ambiance::Zenith;
        Ambiance::CielBas = Ambiance::Horizon;
        Ambiance::cielMat = nullptr;
        Ambiance::lune = nullptr;
        Ambiance::giFait = -1.0f;
        Ambiance::forceFlou = 0.0f;
        Ambiance::debutFlou = 0.0f;
        Ambiance::finFlou = 0.0f;
        Ambiance::PostTraitement = Ambiance::ReglagesPostTraitement{};
    });
}
