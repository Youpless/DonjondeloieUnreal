// Donjon de l'Oie — le chef d'orchestre : il bâtit le monde et l'interface, mène les écrans (titre, salon, partie,
// victoire) et fait jouer la partie en traduisant chaque signal des règles en animation, son et affichage.
#include "DDJeu.h"

#include "../Cartes/DDCatalogue.h"
#include "../Interface/EcranEvade.h"
#include "../Interface/EcranRegles.h"
#include "../Interface/EcranSalon.h"
#include "../Interface/EcranTitre.h"
#include "../Interface/FenetreCombat.h"
#include "../Interface/FenetreQuestion.h"
#include "../Interface/Hud.h"
#include "../Interface/UIK.h"
#include "../Monde/Ambiance.h"
#include "../Monde/CameraRig.h"
#include "../Monde/Etincelles.h"
#include "../Monde/IleModele.h"
#include "../Monde/IleVue.h"
#include "../Monde/PionVue.h"
#include "../Monde/Sons.h"
#include "DDInput.h"

namespace DonjonDeLOie::Pilote
{
    using namespace DonjonDeLOie::Regles;
    using namespace DonjonDeLOie::Voxel;
    using namespace DonjonDeLOie::Monde;
    using namespace DonjonDeLOie::Interface;

    void DDJeu::Awake()
    {
        Application::targetFrameRate = 60;
        Cartes::DDCatalogue::Charger();   // le catalogue de l'éditeur, s'il existe
        UIK::Initialiser();
        monde = Neuf<GameObject>("Monde")->transform();
        cam = CameraRig::Creer(transform());
        Ambiance::Installer(monde, cam->Cam);
        Sons::Installer(cam->gameObject());
        if (FindAnyObjectByType<EventSystem>() == nullptr)
        {
            auto es = Neuf<GameObject>("EventSystem");
            es->AddComponent<EventSystem>();
            // (#if ENABLE_INPUT_SYSTEM : la branche du nouveau système d'entrées, que le moteur imite)
            auto mod = es->AddComponent<InputSystemUIInputModule>();
            mod->AssignDefaultActions();
        }
        auto cg = Neuf<GameObject>("Interface", TypeDe<RectTransform>());
        canvas = cg->AddComponent<Canvas>();
        canvas->renderMode = RenderMode::ScreenSpaceOverlay;
        canvas->sortingOrder = 10;
        auto sc = cg->AddComponent<CanvasScaler>();
        sc->uiScaleMode = CanvasScaler::ScaleMode::ScaleWithScreenSize;
        sc->referenceResolution = Vector2(1920.0f, 1080.0f);
        sc->matchWidthOrHeight = 0.5f;
        cg->AddComponent<GraphicRaycaster>();
        ui = Vers<RectTransform>(cg->transform());
        chargement = UIK::Texte(ui, "Chargement", "Donjon de l'Oie\n<size=26>L'île se dessine…</size>", 56, UIK::Or, TextAnchor::MiddleCenter, true);
        UIK::Remplir(chargement->rectTransform());
        cam->Cam->backgroundColor = Color(0.12f, 0.09f, 0.07f);
    }

    Tache DDJeu::StartCoroutineDeDemarrage()
    {
        co_yield nullptr;
        co_yield nullptr;
        PreparerCartes();
        ConstruireIle(carteParDefaut);
        transitoires = Neuf<GameObject>("Transitoires")->transform();
        transitoires->SetParent(monde, false);
        PreparerArene();
        etincelles = Etincelles::Creer(monde, true);
        fumees = Etincelles::Creer(monde, false);
        cam->Cam->backgroundColor = Ambiance::Horizon;
        hud = Hud::Creer(ui, canvas);
        hud->Camera = cam;
        hud->MontrerOie = [this] { MontrerOie(); };
        hud->Visible(false);
        fc = Nouveau<FenetreCombat>(ui);
        fq = Nouveau<FenetreQuestion>(ui);
        fq->SurvolCase = [this](int c) { ile->Survoler(c); };
        BoutonsJeu();
        titre = Nouveau<EcranTitre>(ui, [this] { AllerSalon(); }, [this] { regles->Visible(true); }, [this] { Quitter(); });
        salon = Nouveau<EcranSalon>(ui, [this] { AllerTitre(); }, [this](const Liste<Ptr<ReglageJoueur>>& l) { Lancer(l); });
        salon->DefinirCartes(NomsCartes(), carteParDefaut, NiveauxConseilles());
        salon->Visible(false);
        regles = Nouveau<EcranRegles>(ui, [this] { regles->Visible(false); });
        pause = Nouveau<MenuPause>(ui, [this] { Pause(false); }, [this] { regles->Visible(true); }, [this] {
            Pause(false);
            AllerTitre();
        });
        Destroy(chargement->gameObject());
        Sons::Musique(true);
        AllerTitre();
    }

    void DDJeu::BoutonsJeu()
    {
        boutonsJeu = UIK::Placer(UIK::Rect(ui, "Boutons du jeu"), 1.0f, 0.0f, -16.0f, 16.0f, 210.0f, 196.0f, 1.0f, 0.0f);
        auto b1 = UIK::Bouton(boutonsJeu, "Plateau", "Tout le plateau", Style::Bouton, [this] { cam->BasculerEnsemble(); }, 19);
        UIK::Placer(Vers<RectTransform>(b1->transform()), 1.0f, 0.0f, 0.0f, 132.0f, 210.0f, 54.0f, 1.0f, 0.0f);
        auto b2 = UIK::Bouton(boutonsJeu, "Règles", "Règles", Style::Bouton, [this] { regles->Visible(true); }, 19);
        UIK::Placer(Vers<RectTransform>(b2->transform()), 1.0f, 0.0f, 0.0f, 66.0f, 210.0f, 54.0f, 1.0f, 0.0f);
        auto b3 = UIK::Bouton(boutonsJeu, "Menu", "Menu (Échap)", Style::Bouton, [this] { Pause(!pause->Ouvert()); }, 19);
        UIK::Placer(Vers<RectTransform>(b3->transform()), 1.0f, 0.0f, 0.0f, 0.0f, 210.0f, 54.0f, 1.0f, 0.0f);
        boutonsJeu->gameObject()->SetActive(false);
    }

    // ------------------------------------------------------------------ écrans

    void DDJeu::AllerTitre()
    {
        Nettoyer();
        etat = Etat::Titre;
        titre->Visible(true);
        salon->Visible(false);
        hud->Visible(false);
        boutonsJeu->gameObject()->SetActive(false);
        cam->CommandesActives = false;
        cam->Regarder(cam->CentreCarte, cam->TailleCarte * 0.8f, 42.0f);
        cam->Placer();
    }

    void DDJeu::AllerSalon()
    {
        etat = Etat::Salon;
        titre->Visible(false);
        salon->Visible(true);
    }

    void DDJeu::Quitter()
    {
        // (#if UNITY_EDITOR : la branche du jeu, hors de l'éditeur)
        Application::Quit();
    }

    void DDJeu::Lancer(const Liste<Ptr<ReglageJoueur>>& reglages)
    {
        Nettoyer();
        if (salon->Carte != carteConstruite) ConstruireIle(salon->Carte);
        derniersReglages = reglages;
        botsRapides = salon->BotsRapides;
        int graine = Environnement::TickCount() & 0x7fffffff;
        partie = Nouveau<Partie>(reglages, graine, salon->NiveauCible, modele->Plateau);
        ia = Nouveau<IA>(graine);
        for (auto& j : partie->Joueurs)
        {
            auto pv = PionVue::Creer(j, monde);
            pions[j->Id] = pv;
        }
        RangerCase(partie->Plateau.Depart, false);
        hud->Installer(partie);
        hud->PionDe = [this](int id) { return pions.ContainsKey(id) ? pions.At(id)->transform() : Ref<Transform>(); };
        hud->HauteurPion = [this](int id) { return pions.ContainsKey(id) ? pions.At(id)->Rig->Hauteur : 1.5f; };
        hud->Visible(true);
        titre->Visible(false);
        salon->Visible(false);
        boutonsJeu->gameObject()->SetActive(true);
        cam->CommandesActives = true;
        etat = Etat::Partie;
        cam->Regarder(modele->Cases[partie->Plateau.Depart], 30.0f, 48.0f);
        boucle = StartCoroutine(Boucle());
    }

    void DDJeu::Nettoyer()
    {
        StopAllCoroutines();     // la boucle de jeu et toutes les présentations en cours
        boucle = nullptr;
        for (auto& p : pions.Values())
            if (p != nullptr) Destroy(p->gameObject());
        pions.Clear();
        EnleverMonstre(true);
        FinirEvade();
        JourImmediat();
        if (transitoires != nullptr)
            for (auto& t : transitoires->Enfants()) Destroy(t->gameObject());
        oieVue = nullptr;
        oieAnim = nullptr;           // il vivait sous « transitoires »
        if (fq != nullptr) fq->Cacher();
        UIK::OublierIcones("portrait_");          // les avis de recherche de la partie finie
        if (fc != nullptr) fc->Cacher();
        if (victoire != nullptr)
        {
            victoire->Detruire();
            victoire = nullptr;
        }
        if (ile != nullptr)
        {
            ile->EffacerChoix();
            ile->Survoler(-1);
            ile->MontrerPieges(Liste<Ptr<Piege>>(), -1, false);
            ile->MontrerEmbuscades(Liste<Ptr<Joueur>>());
            ile->MontrerQuete(-1);
        }
        Time::timeScale = 1.0f;
        partie = nullptr;
    }

    void DDJeu::Pause(bool oui)
    {
        if (etat != Etat::Partie && etat != Etat::Victoire)
        {
            pause->Visible(false);
            return;
        }
        pause->Visible(oui);
        Time::timeScale = oui ? 0.0f : 1.0f;
        cam->CommandesActives = !oui && !evadeEnCours;
    }

    void DDJeu::Update()
    {
        if (etat == Etat::Titre || etat == Etat::Salon) cam->Orbiter(3.5f);
        if (ecranEvade != nullptr) ecranEvade->Animer(Time::unscaledDeltaTime);
        if (DDInput::ToucheAppuyee(KeyCodeDD::Echap))
        {
            if (regles != nullptr && regles->Ouvert()) regles->Visible(false);
            else if (etat == Etat::Partie) Pause(!pause->Ouvert());
            else if (etat == Etat::Salon) AllerTitre();
        }
        // cliquer une case lumineuse répond à la question
        if (etat == Etat::Partie && fq != nullptr && fq->Ouverte() && fq->Question() != nullptr && DDInput::ClicGauche() && !pause->Ouvert())
        {
            bool surUI = EventSystem::current() != nullptr && EventSystem::current()->IsPointerOverGameObject();
            if (!surUI) CliquerCase(fq->Question());
        }
    }

    void DDJeu::CliquerCase(const Ptr<SQuestion>& q)
    {
        if (q->Genre != GenreQuestion::Embranchement && q->Genre != GenreQuestion::Rodeur && q->Genre != GenreQuestion::ChoixCase) return;
        auto souris = DDInput::PositionSouris();
        int best = -1;
        float bd = 75.0f * 75.0f;
        for (int i = 0; i < q->Cases.Count(); i++)
        {
            if (q->Cases[i] < 0 || !q->Actives[i]) continue;
            auto e = cam->Ecran(modele->Cases[q->Cases[i]]);
            if (e.z <= 0.0f) continue;
            float d = (Vector2(e.x, e.y) - souris).sqrMagnitude();
            if (d < bd)
            {
                bd = d;
                best = i;
            }
        }
        if (best >= 0) fq->Choisir(best);
    }

    // ------------------------------------------------------------------ outils

    float DDJeu::V() const { return botsRapides && partie != nullptr && !partie->JoueurCourant()->Humain && !nuitEnCours ? 2.4f : 1.0f; }

    Tache DDJeu::Attendre(float s) { co_yield WaitForSeconds(s / V()); }

    /// Range les pions d'une case sur leurs places (anime ceux qui bougent).
    void DDJeu::RangerCase(int c, bool animer, int sauf)
    {
        if (partie == nullptr) return;
        auto l = partie->JoueursSur(c);
        for (int i = 0; i < l.Count(); i++)
        {
            Ref<PionVue> p;
            if (!pions.TryGetValue(l[i]->Id, p) || l[i]->Id == sauf) continue;
            auto pos = ile->Emplacement(c, i, l.Count());
            if (animer) StartCoroutine(p->Glisser(pos, 0.25f));
            else p->transform()->position(pos);
        }
    }

    Vector3 DDJeu::Place(int joueur) const
    {
        auto j = partie->Joueurs[joueur];
        auto l = partie->JoueursSur(j->Case);
        return ile->Emplacement(j->Case, Mathf::Max(0, l.IndexOf(j)), l.Count());
    }

    Vector3 DDJeu::Tete(int joueur) const
    {
        auto p = pions.At(joueur);
        return p->transform()->position() + Vector3::up * (p->Rig->Hauteur + 0.2f);
    }

    void DDJeu::MajMarqueurs(const Ptr<Joueur>& j)
    {
        bool humain = j != nullptr && j->Humain;
        ile->MontrerPieges(partie->Pieges, humain ? j->Id : -1, humain && j->Classe == ClasseHeros::Voleur);
        ile->MontrerEmbuscades(partie->Joueurs);
        bool lieu = j != nullptr && j->Quete != nullptr && (j->Quete->Type == Quete::Genre::Atteindre || j->Quete->Type == Quete::Genre::Evade);
        ile->MontrerQuete(humain && lieu ? j->Quete->CaseCible : -1);
    }

    int DDJeu::Humains() const
    {
        int n = 0;
        for (auto& j : partie->Joueurs)
            if (j->Humain) n++;
        return n;
    }
}
