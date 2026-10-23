// Donjon de l'Oie — la scène : démarrage, image, entrées, caméra.
#include "DDScene.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

#include "DDConversions.h"
#include "DDJournal.h"
#include "DDViewportClient.h"

#include "Pilote/DDJeu.h"
#include "Regles/Donnees.h"
#include "Voxel/PixelIcones.h"

namespace
{
    /// Le journal du jeu (Debug.Log, LogWarning, LogError) va dans celui d'Unreal (Output Log).
    void JournalDuJeu(int niveau, const std::string& texte)
    {
        const FString T = DDUnreal::Texte(texte);
        // UE_LOG est un bloc entre accolades (pas une expression) : chaque branche a les siennes
        if (niveau >= 2)
        {
            UE_LOG(LogDonjon, Error, TEXT("%s"), *T);
        }
        else if (niveau == 1)
        {
            UE_LOG(LogDonjon, Warning, TEXT("%s"), *T);
        }
        else
        {
            UE_LOG(LogDonjon, Log, TEXT("%s"), *T);
        }
    }

    /// Une erreur de programmation dans le jeu (indice hors limites…) : le message dans le journal, puis l'arrêt.
    void ErreurDuJeu(const char* message)
    {
        UE_LOG(LogDonjon, Fatal, TEXT("Donjon de l'Oie : %s"), UTF8_TO_TCHAR(message));
    }

    float ImagesMaxAvant = 0.0f;
}

ADDScene::ADDScene()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    Racine = CreateDefaultSubobject<USceneComponent>(TEXT("Racine"));
    SetRootComponent(Racine);

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(Racine);
    Camera->bConstrainAspectRatio = false;
    Camera->bUsePawnControlRotation = false;
    Camera->SetFieldOfView(60.0f);
    Camera->PostProcessBlendWeight = 1.0f;
}

void ADDScene::BeginPlay()
{
    Super::BeginPlay();
    Demarrer();
}

void ADDScene::EndPlay(const EEndPlayReason::Type Raison)
{
    Arreter();
    Super::EndPlay(Raison);
}

void ADDScene::Demarrer()
{
    // une partie relancée dans l'éditeur réutilise le même programme : tout repart de zéro, comme au lancement d'une
    // partie dans Unity (qui recharge les scripts) : le petit moteur, les données des règles, les icônes faites en jeu
    Moteur::Boucle::Reinitialiser();
    DonjonDeLOie::Regles::Donnees::Reinitialiser();
    DonjonDeLOie::Voxel::PixelIcones::Dynamiques().Clear();
    Moteur::Debug::Sortie = &JournalDuJeu;
    Moteur::RapportErreurFatale = &ErreurDuJeu;
    Moteur::Application::isEditor = GIsEditor;
    Moteur::Application::isPlaying = true;
    Moteur::Application::QuitterDemande = false;
    Moteur::Mouse::current()->Presente = true;
    Moteur::Dessin::ChoisirMesure(&Mesureur);
    if (GEngine != nullptr) ImagesMaxAvant = GEngine->GetMaxFPS();

    if (UWorld* Monde = GetWorld())
        if (UGameViewportClient* Fenetre = Monde->GetGameViewport())
        {
            FVector2D Taille(1920.0, 1080.0);
            Fenetre->GetViewportSize(Taille);
            if (Taille.X >= 1.0 && Taille.Y >= 1.0)
            {
                Moteur::Screen::width = FMath::RoundToInt(Taille.X);
                Moteur::Screen::height = FMath::RoundToInt(Taille.Y);
            }
        }

    ChargerMateriaux();
    InstallerInterface();
    PrendreLaVue();

    // le jeu : comme dans la scène d'Unity, un objet qui porte DDJeu (Awake construit l'interface et la caméra ;
    // Start, à l'image suivante, bâtit l'île)
    auto Jeu = Moteur::Neuf<Moteur::GameObject>("Donjon de l'Oie");
    Jeu->AddComponent<DonjonDeLOie::Pilote::DDJeu>();
    Lance = true;
    UE_LOG(LogDonjon, Log, TEXT("Donjon de l'Oie : partie lancée (%d × %d)."), Moteur::Screen::width, Moteur::Screen::height);
}

void ADDScene::Arreter()
{
    if (!Lance) return;
    Lance = false;
    ArreterSons();
    RetirerInterface();
    Moteur::Boucle::Reinitialiser();
    Moteur::Dessin::ChoisirMesure(nullptr);
    Moteur::Debug::Sortie = nullptr;
    Moteur::RapportErreurFatale = nullptr;
    Etats.Empty();
    MateriauxPartages.Empty();
    MateriauxPropres.Empty();
    VersionsMateriauxPartages.Empty();
    VersionsMateriauxPropres.Empty();
    if (GEngine != nullptr && ImagesParSeconde > 0) GEngine->SetMaxFPS(ImagesMaxAvant);
}

void ADDScene::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!Lance) return;
    ImageUnreal++;
    PrendreLaVue();

    // 1. la fenêtre, la souris, le clavier
    if (UWorld* Monde = GetWorld())
        if (UGameViewportClient* Fenetre = Monde->GetGameViewport())
        {
            FVector2D Taille(0.0, 0.0);
            Fenetre->GetViewportSize(Taille);
            if (Taille.X >= 1.0 && Taille.Y >= 1.0)
            {
                Moteur::Screen::width = FMath::RoundToInt(Taille.X);
                Moteur::Screen::height = FMath::RoundToInt(Taille.Y);
            }
        }
    LireEntrees();

    // 2. une image du jeu (le pas est borné comme Time.maximumDeltaTime d'Unity : un tiers de seconde)
    Moteur::Boucle::Image(FMath::Clamp(DeltaSeconds, 0.0f, 1.0f / 3.0f));
    if (Moteur::Application::targetFrameRate != ImagesParSeconde)
    {
        ImagesParSeconde = Moteur::Application::targetFrameRate;
        if (GEngine != nullptr) GEngine->SetMaxFPS(ImagesParSeconde > 0 ? static_cast<float>(ImagesParSeconde) : ImagesMaxAvant);
    }

    // 3. ce que le jeu montre
    ReproduireMaillages();
    ReproduireLumieres();
    ReproduireCamera();
    ReproduirePostTraitement();
    ReproduireSons();
    ReproduireInterface();

    if (Moteur::Application::QuitterDemande)
    {
        Moteur::Application::QuitterDemande = false;
        UKismetSystemLibrary::QuitGame(this, UGameplayStatics::GetPlayerController(this, 0), EQuitPreference::Quit, false);
    }
}

void ADDScene::PrendreLaVue()
{
    APlayerController* Joueur = UGameplayStatics::GetPlayerController(this, 0);
    if (Joueur == nullptr) return;
    if (Joueur->GetViewTarget() != this) Joueur->SetViewTarget(this);
    // la scène lit les entrées après que le contrôleur les a reçues
    static TWeakObjectPtr<APlayerController> Prerequis;
    if (Prerequis.Get() != Joueur)
    {
        AddTickPrerequisiteActor(Joueur);
        Prerequis = Joueur;
    }
}

void ADDScene::LireEntrees()
{
    Moteur::Entrees::DebutImage();
    Moteur::Mouse* Souris = Moteur::Mouse::current();
    Moteur::Keyboard* Clavier = Moteur::Keyboard::current();
    if (APlayerController* Joueur = UGameplayStatics::GetPlayerController(this, 0))
    {
        float X = 0.0f, Y = 0.0f;
        // Unreal compte depuis le haut de la fenêtre, Unity depuis le bas
        if (Joueur->GetMousePosition(X, Y)) Moteur::Entrees::PositionSouris(Moteur::Vector2(X, static_cast<float>(Moteur::Screen::height) - Y));
        Souris->leftButton.Changer(Joueur->IsInputKeyDown(EKeys::LeftMouseButton));
        Souris->rightButton.Changer(Joueur->IsInputKeyDown(EKeys::RightMouseButton));
        Souris->middleButton.Changer(Joueur->IsInputKeyDown(EKeys::MiddleMouseButton));
        Clavier->leftArrowKey.Changer(Joueur->IsInputKeyDown(EKeys::Left));
        Clavier->rightArrowKey.Changer(Joueur->IsInputKeyDown(EKeys::Right));
        Clavier->upArrowKey.Changer(Joueur->IsInputKeyDown(EKeys::Up));
        Clavier->downArrowKey.Changer(Joueur->IsInputKeyDown(EKeys::Down));
        Clavier->spaceKey.Changer(Joueur->IsInputKeyDown(EKeys::SpaceBar));
        Clavier->escapeKey.Changer(Joueur->IsInputKeyDown(EKeys::Escape));
        Clavier->enterKey.Changer(Joueur->IsInputKeyDown(EKeys::Enter));
        Clavier->f1Key.Changer(Joueur->IsInputKeyDown(EKeys::F1));
        Clavier->tabKey.Changer(Joueur->IsInputKeyDown(EKeys::Tab));
        Clavier->qKey.Changer(Joueur->IsInputKeyDown(EKeys::Q));
        Clavier->eKey.Changer(Joueur->IsInputKeyDown(EKeys::E));
        Clavier->backspaceKey.Changer(Joueur->IsInputKeyDown(EKeys::BackSpace));
    }
    Moteur::Entrees::Molette(UDDViewportClient::PrendreMolette());
    Clavier->CaracteresTapes = DDUnreal::Utf8(UDDViewportClient::PrendreCaracteres());
}

void ADDScene::ReproduireCamera()
{
    const Moteur::Ref<Moteur::Camera> Cam = Moteur::Camera::main();
    if (Cam == nullptr) return;
    const Moteur::Ref<Moteur::Transform> T = Cam->transform();
    Camera->SetWorldLocationAndRotation(DDUnreal::Position(T->position()), DDUnreal::Rotation(T->rotation()));
    // Unity donne le champ vertical, Unreal le champ horizontal
    const float Aspect = Moteur::Screen::height > 0 ? static_cast<float>(Moteur::Screen::width) / static_cast<float>(Moteur::Screen::height) : 16.0f / 9.0f;
    const float Vertical = FMath::DegreesToRadians(Cam->fieldOfView);
    const float Horizontal = 2.0f * FMath::Atan(FMath::Tan(Vertical * 0.5f) * Aspect);
    Camera->SetFieldOfView(FMath::RadiansToDegrees(Horizontal));
}
