// Donjon de l'Oie — le mini-jeu de l'évadé à l'écran : la caméra zoome sur la ville en vue presque isométrique (le
// champ se resserre pendant qu'elle recule), la ville se floute (flou de profondeur d'URP), le marché surgit en
// diorama au-dessus des toits, et l'on a trente secondes pour cliquer sur le visage de l'évadé, caché dans la foule.
// Un bot cherche à sa place, loupe à la main : il réussit ou non selon la réponse de l'IA, tirée d'avance.
#include "DDJeu.h"

#include "../Interface/EcranEvade.h"
#include "../Interface/EcranTitre.h"
#include "../Interface/Hud.h"
#include "../Interface/UIK.h"
#include "../Modeles/HerosModeles.h"
#include "../Monde/Ambiance.h"
#include "../Monde/Animateur.h"
#include "../Monde/CameraRig.h"
#include "../Monde/Diorama.h"
#include "../Monde/Etincelles.h"
#include "../Monde/IleModele.h"
#include "../Monde/IleVue.h"
#include "../Monde/PionVue.h"
#include "../Monde/RigVue.h"
#include "../Monde/Sons.h"
#include "../Voxel/Pal.h"
#include "DDInput.h"

namespace DonjonDeLOie::Pilote
{
    using namespace DonjonDeLOie::Regles;
    using namespace DonjonDeLOie::Voxel;
    using namespace DonjonDeLOie::Monde;
    using namespace DonjonDeLOie::Modeles;
    using namespace DonjonDeLOie::Interface;

    Tache DDJeu::ChercherEvade(Ptr<SQuestion> q)
    {
        auto j = partie->Joueurs[q->Joueur];
        auto qu = q->Quete;
        int iTrouve = q->Codes.IndexOf("trouve"), iRate = q->Codes.IndexOf("rate");
        q->Reponse = iRate;
        if (qu == nullptr || iTrouve < 0 || iRate < 0) co_return;
        bool humain = j->Humain;
        bool reussiteBot = !humain && ia->Repondre(*partie, *q) == iTrouve;
        if (ecranEvade == nullptr) ecranEvade = Nouveau<EcranEvade>(ui, canvas);
        evadeEnCours = true;
        hud->ModeMiniJeu(true);
        boutonsJeu->gameObject()->SetActive(false);
        cam->CommandesActives = false;
        if (cam->VueEnsemble()) cam->BasculerEnsemble();
        hud->Annonce("Avis de recherche : " + qu->NomEvade, UIK::Or);
        Sons::Jouer(Son::Page);

        // le diorama flotte au-dessus de la ville, assez haut pour que toute la ville soit derrière lui (et floue)
        auto ville = modele->Cases[j->Case];
        float sommet = ile->SommetAutour(ville, 8.0f);
        auto centre = Vector3(ville.x, Mathf::Max(ville.y + 13.0f, sommet + 4.0f), ville.z);
        float lacetCam = LacetDiagonal(cam->Lacet);
        auto allure = HerosModeles::AllureEvade(qu->Graine, qu->NomEvade, qu->Feminin);
        diorama = Diorama::Creer(transitoires, centre, lacetCam + 180.0f - Diorama::LacetVue, qu->Graine, allure);
        float s = canvas->scaleFactor() > 0.0f ? canvas->scaleFactor() : 1.0f, w = Mathf::Max(1.0f, static_cast<float>(Screen::width)),
              hEcran = Mathf::Max(1.0f, static_cast<float>(Screen::height));
        Vector3 cible;
        float distance, lacet;
        Diorama::Cadrage(diorama->transform(), ChampDiorama, cam->Cam->aspect(), Mathf::Clamp(EcranEvade::LargeurAvis * s / w, 0.1f, 0.4f),
                         EcranEvade::HauteurSablier * s / hEcran, EcranEvade::HauteurConsigne * s / hEcran, cible, distance, lacet);
        StartCoroutine(diorama->Batir(true));
        co_yield StartCoroutine(Zoomer(cible, distance, lacet, Diorama::TangageVue, ChampDiorama, 1.7f / V(), 1.0f));
        while (diorama != nullptr && !diorama->Pret()) co_yield nullptr;
        if (diorama == nullptr)
        {
            FinirEvade();
            co_return;
        }

        // il surgit, pièce après pièce
        ecranEvade->Montrer(qu, HerosModeles::IconePortrait(qu->Graine, qu->NomEvade, qu->Feminin), j->Nom, humain, Pal::Joueurs[j->Couleur]);
        ecranEvade->Temps(static_cast<float>(Donnees::Reglages->DureeEvade), static_cast<float>(Donnees::Reglages->DureeEvade));
        // (« dernierPop » : la variable locale que la lambda de C# modifie ; partagée ici avec la lambda)
        auto dernierPop = Nouveau<float>(-1.0f);
        co_yield StartCoroutine(diorama->Apparaitre([this, dernierPop](Vector3 pos) {
            fumees->Fumee(pos, Color(0.85f, 0.8f, 0.7f, 0.5f), 3, 0.2f);
            if (Time::time - *dernierPop > 0.08f)
            {
                *dernierPop = Time::time;
                Sons::Jouer(Son::Pas, 0.3f);
            }
        }, V()));

        // la recherche
        diorama->EnJeu = true;
        auto trouve = Nouveau<Liste<bool>>(1);
        if (humain) co_yield StartCoroutine(RechercheHumaine(qu, trouve));
        else
        {
            (*trouve)[0] = reussiteBot;
            co_yield StartCoroutine(RechercheBot(qu, reussiteBot));
        }
        diorama->EnJeu = false;
        q->Reponse = (*trouve)[0] ? iTrouve : iRate;
        // la molette avait zoomé ? on revoit tout le marché pour le dénouement
        if (cam->Cam->fieldOfView < ChampDiorama - 0.1f) co_yield StartCoroutine(Zoomer(cible, distance, lacet, Diorama::TangageVue, ChampDiorama, 0.45f, 1.0f));

        // le dénouement
        auto ev = diorama->Evade();
        if ((*trouve)[0])
        {
            ecranEvade->Message(qu->Feminin ? "La voilà !" : "Le voilà !", UIK::Or);
            Sons::Jouer(Son::Niveau, 0.8f);
            ev->Rig->Eclair(Color(1.0f, 0.85f, 0.4f), 1.2f);
            etincelles->Spirale(ev->Rig->transform()->position(), Color(1.0f, 0.85f, 0.4f), 40, 1.4f);
            co_yield StartCoroutine(diorama->Attrape());
            etincelles->Explosion(ev->Tete() + Vector3::up * 0.3f, Color(1.0f, 0.85f, 0.4f), 30, 3.0f, 0.06f, 1.0f);
            co_yield Attendre(1.0f);
        }
        else
        {
            ecranEvade->Temps(0.0f, static_cast<float>(Donnees::Reglages->DureeEvade));
            ecranEvade->Message(humain ? "Temps écoulé !" : "Raté !", UIK::Rouge);
            Sons::Jouer(Son::Malus, 0.8f);
            ev->Rig->Eclair(Color(1.0f, 0.35f, 0.25f), 1.2f);
            co_yield Attendre(0.7f);
            ecranEvade->Message(qu->Feminin ? "Elle s'enfuit !" : "Il s'enfuit !", UIK::Rouge);
            co_yield StartCoroutine(diorama->Fuite());
            co_yield Attendre(0.3f);
        }

        // le diorama se replie, la caméra revient au héros
        ecranEvade->Cacher();
        co_yield StartCoroutine(diorama->Disparaitre(0.55f / V()));
        Destroy(diorama->gameObject());
        diorama = nullptr;
        auto heros = pions.At(j->Id)->transform()->position() + Vector3::up * 0.6f;
        co_yield StartCoroutine(Zoomer(heros, 12.5f, cam->Lacet, 50.0f, CameraRig::ChampNormal, 1.1f / V(), 0.0f));
        FinirEvade();
        cam->Suivre(pions.At(j->Id)->transform(), 12.5f);
    }

    /// Trente secondes (réglables) pour cliquer sur le bon visage ; une erreur en coûte trois. La molette zoome vers
    /// la souris (sans sortir du cadre de départ).
    Tache DDJeu::RechercheHumaine(Ptr<Quete> qu, Ptr<Liste<bool>> trouve)
    {
        float total = Mathf::Max(5.0f, static_cast<float>(Donnees::Reglages->DureeEvade)), reste = total;
        int dernierTic = -1;
        Vector3 cible0 = cam->Cible;
        float zoom = 1.0f;
        while (reste > 0.0f && diorama != nullptr)
        {
            if (!pause->Ouvert())
            {
                reste -= Time::deltaTime;
                bool surUI = EventSystem::current() != nullptr && EventSystem::current()->IsPointerOverGameObject();
                float molette = DDInput::Molette();
                if (Mathf::Abs(molette) > 0.01f && !surUI) zoom = ZoomerVers(cible0, zoom, Mathf::Clamp(zoom * (1.0f + molette * 0.18f), 1.0f, ZoomMax));
                if (DDInput::ClicGauche() && !surUI)
                {
                    auto h = diorama->Toucher(DDInput::PositionSouris(), cam->Cam);
                    if (h != nullptr && h->Evade)
                    {
                        (*trouve)[0] = true;
                        break;
                    }
                    if (h != nullptr)
                    {
                        reste -= 3.0f;
                        Sons::Jouer(Son::Malus, 0.6f);
                        h->Rig->Eclair(Color(1.0f, 0.25f, 0.2f), 0.5f);
                        h->Anim->Jouer(Geste::Touche);
                        ecranEvade->Message(qu->Feminin ? "Ce n'est pas elle !  −3 s" : "Ce n'est pas lui !  −3 s", UIK::Rouge);
                    }
                }
                int s = Mathf::CeilToInt(reste);
                if (s <= 5 && s > 0 && s != dernierTic)
                {
                    dernierTic = s;
                    Sons::Jouer(Son::Clic, 0.6f);
                }
            }
            ecranEvade->Temps(reste, total);
            co_yield nullptr;
        }
    }

    /// Le zoom de la molette : le champ se resserre (ou s'élargit) et la cible glisse dans le plan de l'image pour que
    /// le point sous la souris reste sous la souris ; la vue ne sort jamais du cadre de départ (cible0, zoom 1).
    float DDJeu::ZoomerVers(Vector3 cible0, float zoom, float nouveau)
    {
        float t0 = Mathf::Tan(ChampDiorama * 0.5f * Mathf::Deg2Rad);
        float ty = t0 / zoom, ty1 = t0 / nouveau, asp = cam->Cam->aspect(), d = cam->Distance;
        auto souris = DDInput::PositionSouris();
        float sx = souris.x / Mathf::Max(1.0f, static_cast<float>(Screen::width)) * 2.0f - 1.0f, sy = souris.y / Mathf::Max(1.0f, static_cast<float>(Screen::height)) * 2.0f - 1.0f;
        auto dr = cam->transform()->right();
        auto ht = cam->transform()->up();
        auto c = cam->Cible + dr * (sx * d * (ty - ty1) * asp) + ht * (sy * d * (ty - ty1));
        // rester dans le cadre de départ
        auto ecart = c - cible0;
        float jx = d * (t0 - ty1) * asp, jy = d * (t0 - ty1);
        float ex = Mathf::Clamp(Vector3::Dot(ecart, dr), -jx, jx), ey = Mathf::Clamp(Vector3::Dot(ecart, ht), -jy, jy);
        c = cible0 + dr * ex + ht * ey;
        cam->Fixer(c, d, cam->Lacet, cam->Tangage, 2.0f * Mathf::Atan(ty1) * Mathf::Rad2Deg);
        return nouveau;
    }

    /// Un bot cherche, loupe à la main : quelques visages, puis l'évadé (réussite) ou un sosie (raté).
    Tache DDJeu::RechercheBot(Ptr<Quete> qu, bool reussite)
    {
        float total = Mathf::Max(5.0f, static_cast<float>(Donnees::Reglages->DureeEvade));
        Liste<Ptr<Diorama::Habitant>> cibles;
        int n = reussite ? 2 + Random::Range(0, 2) : 3;
        for (int i = 0; i < n; i++) cibles.Add(diorama->Badaud());
        cibles.Add(reussite ? diorama->Evade() : diorama->UnSosie());
        float finSablier = reussite ? total * Random::Range(0.3f, 0.7f) : total * 0.15f;
        float duree = (reussite ? 3.2f : 3.6f) / V(), ecoule = 0.0f;
        auto pos = Vector2(static_cast<float>(Screen::width) * 0.62f, static_cast<float>(Screen::height) * 0.35f);
        for (int i = 0; i < cibles.Count() && diorama != nullptr; i++)
        {
            auto h = cibles[i];
            float seg = duree / static_cast<float>(cibles.Count()), u = 0.0f;
            auto de = pos;
            while (u < seg)
            {
                float dt = Time::deltaTime;
                u += dt;
                ecoule += dt;
                auto e = cam->Ecran(h->Tete());
                pos = Vector2::Lerp(de, Vector2(e.x, e.y), Mathf::SmoothStep(0.0f, 1.0f, Mathf::Clamp01(u / (seg * 0.7f))));
                ecranEvade->Loupe(pos, true);
                ecranEvade->Temps(Mathf::Lerp(total, finSablier, ecoule / duree), total);
                co_yield nullptr;
            }
            if (i == cibles.Count() - 1 && !reussite && h != nullptr)
            {
                Sons::Jouer(Son::Malus, 0.5f);
                h->Rig->Eclair(Color(1.0f, 0.25f, 0.2f), 0.5f);
                h->Anim->Jouer(Geste::Touche);
                ecranEvade->Message(qu->Feminin ? "Ce n'est pas elle !" : "Ce n'est pas lui !", UIK::Rouge);
                // le temps file jusqu'au bout
                float t = 0.0f;
                while (t < 0.8f / V())
                {
                    t += Time::deltaTime;
                    ecranEvade->Temps(Mathf::Lerp(finSablier, 0.0f, t * V() / 0.8f), total);
                    co_yield nullptr;
                }
            }
        }
        ecranEvade->Loupe(pos, false);
    }

    /// Mouvement de caméra image par image : la cible, la distance, le lacet, le tangage et le champ glissent ensemble ;
    /// la hauteur vue au point visé varie régulièrement (un « travelling compensé » : le décor se tasse quand le champ
    /// se resserre). Le flou de profondeur va de sa force actuelle à « flou », réglé sur la distance du moment.
    Tache DDJeu::Zoomer(Vector3 cible, float distance, float lacet, float tangage, float champ, float duree, float flou)
    {
        Vector3 c0 = cam->Cible;
        float d0 = cam->Distance, l0 = cam->Lacet, t0 = cam->Tangage, f0 = cam->Cam->fieldOfView;
        float h0 = 2.0f * d0 * Mathf::Tan(f0 * 0.5f * Mathf::Deg2Rad), h1 = 2.0f * distance * Mathf::Tan(champ * 0.5f * Mathf::Deg2Rad);
        float flou0 = Ambiance::ForceFlou();
        float t = 0.0f;
        while (t < duree)
        {
            t += Time::deltaTime;
            float k = Mathf::SmoothStep(0.0f, 1.0f, Mathf::Clamp01(t / duree));
            float f = Mathf::Lerp(f0, champ, k);
            float d = Mathf::Lerp(h0, h1, k) / (2.0f * Mathf::Tan(f * 0.5f * Mathf::Deg2Rad));
            cam->Fixer(Vector3::Lerp(c0, cible, k), d, Mathf::LerpAngle(l0, lacet, k), Mathf::Lerp(t0, tangage, k), f);
            Ambiance::Flou(Mathf::Lerp(flou0, flou, k), d + DebutFlou, d + FinFlou);
            co_yield nullptr;
        }
        cam->Fixer(cible, distance, lacet, tangage, champ);
        Ambiance::Flou(flou, distance + DebutFlou, distance + FinFlou);
    }

    /// Le lacet de la vue du diorama : la diagonale (45° + k·90°) la plus proche, sauf celle qui fait face au soleil
    /// (les visages seraient à contre-jour).
    float DDJeu::LacetDiagonal(float lacet)
    {
        float best = 45.0f, bd = 999.0f;
        for (float l : {45.0f, 135.0f, 315.0f})
        {
            float d = Mathf::Abs(Mathf::DeltaAngle(lacet, l));
            if (d < bd)
            {
                bd = d;
                best = l;
            }
        }
        return lacet + Mathf::DeltaAngle(lacet, best);
    }

    /// Une quête d'avis de recherche ratée : un nuage de dépit au-dessus du héros.
    Tache DDJeu::QueteRatee(Ptr<SEffet> e)
    {
        if (e->Joueur >= 0 && pions.ContainsKey(e->Joueur))
        {
            auto p = pions.At(e->Joueur);
            fumees->Fumee(p->transform()->position() + Vector3::up * 0.5f, Color(0.35f, 0.3f, 0.34f, 0.7f), 14, 0.5f);
            p->Anim->Jouer(Geste::Touche);
        }
        Sons::Jouer(Son::Malus, 0.7f);
        MajMarqueurs(partie->JoueurCourant());
        co_yield Attendre(0.4f);
    }

    /// Fin du mini-jeu (ou partie interrompue) : plus de diorama, plus de flou, l'écran de jeu revient.
    void DDJeu::FinirEvade()
    {
        if (diorama != nullptr)
        {
            Destroy(diorama->gameObject());
            diorama = nullptr;
        }
        if (ecranEvade != nullptr) ecranEvade->Cacher();
        if (Ambiance::ForceFlou() > 0.0f) Ambiance::Flou(0.0f, 0.0f, 0.0f);
        if (!evadeEnCours) return;
        evadeEnCours = false;
        if (cam != nullptr)
        {
            cam->RetablirChamp();
            cam->CommandesActives = etat == Etat::Partie && !pause->Ouvert();
        }
        if (hud != nullptr) hud->ModeMiniJeu(false);
        if (boutonsJeu != nullptr) boutonsJeu->gameObject()->SetActive(etat == Etat::Partie);
    }
}
