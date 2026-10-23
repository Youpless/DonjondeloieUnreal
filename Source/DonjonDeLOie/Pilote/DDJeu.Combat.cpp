// Donjon de l'Oie — mise en scène des combats : le monstre surgit à côté du héros, les dés roulent, les coups portent.
#include "DDJeu.h"

#include "../Interface/FenetreCombat.h"
#include "../Interface/Hud.h"
#include "../Interface/UIK.h"
#include "../Modeles/Catalogue.h"
#include "../Monde/Animateur.h"
#include "../Monde/CameraRig.h"
#include "../Monde/DeVue.h"
#include "../Monde/Etincelles.h"
#include "../Monde/IleModele.h"
#include "../Monde/IleVue.h"
#include "../Monde/PionVue.h"
#include "../Monde/RigVue.h"
#include "../Monde/Sons.h"

namespace DonjonDeLOie::Pilote
{
    using namespace DonjonDeLOie::Regles;
    using namespace DonjonDeLOie::Voxel;
    using namespace DonjonDeLOie::Monde;
    using namespace DonjonDeLOie::Modeles;
    using namespace DonjonDeLOie::Interface;

    /// Le monstre jaillit du sol à côté du héros (le dragon, lui, est déjà dans son antre, et le Dragonnoie sur sa case).
    Tache DDJeu::FaireSurgir(Ptr<DefMonstre> m, Ptr<Joueur> j)
    {
        EnleverMonstre(true);
        auto p = pions.At(j->Id);
        Ref<RigVue> present = m->Errant && oieVue != nullptr ? oieVue : (m->Boss ? ile->Dragon : Ref<RigVue>());
        if (present != nullptr)
        {
            monstreVue = present;
            monstreAnim = present->GetComponent<Animateur>();
            monstrePersistant = true;
            if (monstreAnim != nullptr)
            {
                monstreAnim->Vitesse = 1.0f;
                monstreAnim->Jouer(Geste::Victoire, 1.2f);
            }
            Sons::Jouer(Son::Monstre, 1.0f);
            cam->Cadrer(p->transform()->position(), monstreVue->transform()->position());
            p->Regarder(monstreVue->transform()->position());
            etincelles->Explosion(monstreVue->transform()->position() + Vector3::up * 2.0f, Color(1.0f, 0.45f, 0.15f), 40, 4.0f);
            co_yield Attendre(1.0f);
            co_return;
        }
        auto droite = cam->transform()->right();
        droite.y = 0.0f;
        droite.Normalize();
        auto avant = cam->transform()->forward();
        avant.y = 0.0f;
        avant.Normalize();
        auto pos = p->transform()->position() + droite * 2.2f + avant * 0.4f;
        pos.y = Mathf::Max(modele->HauteurEn(pos.x, pos.z), p->transform()->position().y - 0.3f);
        monstreVue = RigVue::Creer(Catalogue::Monstre(m->Modele()), transitoires, "monstre_" + m->Id, true, 2.6f);
        monstreVue->transform()->position(pos - Vector3::up * 1.5f);
        auto d = p->transform()->position() - pos;
        d.y = 0.0f;
        monstreVue->transform()->rotation(Quaternion::LookRotation(d.sqrMagnitude() > 0.01f ? d : Vector3::back));
        monstreAnim = Animateur::Sur(monstreVue);
        monstreAnim->Flotte = m->Modele() == "spectre";
        monstrePersistant = false;
        p->Regarder(pos);
        cam->Cadrer(p->transform()->position(), pos);
        fumees->Fumee(pos, Color(0.45f, 0.38f, 0.3f, 0.75f), 22, 0.8f);
        Sons::Jouer(Son::Monstre, 0.9f);
        float t = 0.0f;
        auto de = monstreVue->transform()->position();
        while (t < 0.45f)
        {
            t += Time::deltaTime * V();
            float k = Mathf::Clamp01(t / 0.45f);
            monstreVue->transform()->position(Vector3::Lerp(de, pos, 1.0f - (1.0f - k) * (1.0f - k)));
            co_yield nullptr;
        }
        monstreVue->transform()->position(pos);
        monstreAnim->Jouer(Geste::Attaque, 0.6f);
        co_yield Attendre(0.5f);
    }

    void DDJeu::EnleverMonstre(bool immediat)
    {
        if (monstreVue == nullptr) return;
        if (monstrePersistant)
        {
            if (monstreAnim != nullptr)
            {
                monstreAnim->Relever();
                monstreAnim->Vitesse = monstreVue == oieVue ? 1.0f : 0.45f;   // le dragon se rendort
            }
        }
        else if (immediat) Destroy(monstreVue->gameObject());
        else StartCoroutine(Enfouir(monstreVue->gameObject()));
        monstreVue = nullptr;
        monstreAnim = nullptr;
        monstrePersistant = false;
    }

    Tache DDJeu::Enfouir(Ref<GameObject> go)
    {
        if (go == nullptr) co_return;
        fumees->Fumee(go->transform()->position(), Color(0.45f, 0.4f, 0.35f, 0.7f), 16, 0.7f);
        auto de = go->transform()->position();
        float t = 0.0f;
        while (t < 0.5f && go != nullptr)
        {
            t += Time::deltaTime;
            go->transform()->position(de - Vector3::up * (t / 0.5f) * 1.8f);
            co_yield nullptr;
        }
        if (go != nullptr) Destroy(go);
    }

    Tache DDJeu::Combat(Ptr<SCombat> c)
    {
        auto a = pions.At(c->Attaquant);
        Ref<Transform> tb;
        Ref<Animateur> animB;
        Ref<RigVue> rigB;
        if (c->Duel)
        {
            auto b = pions.At(c->Defenseur);
            tb = b->transform();
            animB = b->Anim;
            rigB = b->Rig;
            if (c->Arene) co_yield StartCoroutine(AvancerDuelArene(c));
            // deux pions sur la même place : on les écarte un peu
            else if ((a->transform()->position() - b->transform()->position()).sqrMagnitude() < 0.3f)
            {
                auto dec = cam->transform()->right() * 0.5f;
                dec.y = 0.0f;
                StartCoroutine(a->Glisser(a->transform()->position() - dec, 0.2f));
                StartCoroutine(b->Glisser(b->transform()->position() + dec, 0.2f));
                co_yield WaitForSeconds(0.22f);
            }
            a->Regarder(b->transform()->position());
            b->Regarder(a->transform()->position());
            if (!c->Arene) cam->Cadrer(a->transform()->position(), b->transform()->position());
        }
        else
        {
            if (monstreVue == nullptr) co_yield StartCoroutine(FaireSurgir(c->Monstre, partie->Joueurs[c->Attaquant]));
            tb = monstreVue->transform();
            animB = monstreAnim;
            rigB = monstreVue;
            a->Regarder(tb->position());
        }
        fc->Montrer(c, partie);
        co_yield Attendre(0.35f);

        // les deux dés roulent entre les adversaires
        auto milieu = (a->transform()->position() + tb->position()) * 0.5f;
        auto avant = cam->transform()->forward();
        avant.y = 0.0f;
        avant.Normalize();
        auto droite = cam->transform()->right();
        droite.y = 0.0f;
        droite.Normalize();
        auto posA = milieu - droite * 0.5f - avant * 1.35f;
        posA.y = a->transform()->position().y;
        auto posB = milieu + droite * 0.5f - avant * 1.35f;
        posB.y = a->transform()->position().y;
        auto deA = DeVue::Creer(transitoires, c->FacesDe);
        auto deB = DeVue::Creer(transitoires, c->FacesDe);
        Sons::Jouer(Son::De);
        StartCoroutine(deA->Lancer(posA - droite * 1.2f + Vector3::up * 1.4f - avant * 0.8f, posA, c->DeA, 0.95f / V()));
        co_yield StartCoroutine(deB->Lancer(posB + droite * 1.2f + Vector3::up * 1.4f - avant * 0.8f, posB, c->DeB, 1.0f / V()));
        fc->Des(c);
        hud->Flottant(deA->transform()->position() + Vector3::up * 0.45f, Texte::De(c->DeA), UIK::Or, 40);
        hud->Flottant(deB->transform()->position() + Vector3::up * 0.45f, Texte::De(c->DeB), UIK::Or, 40);
        co_yield Attendre(0.6f);
        fc->Verdict(c);

        if (c->Egalite)
        {
            a->Anim->Jouer(Geste::Attaque);
            if (animB != nullptr) animB->Jouer(Geste::Attaque);
            Sons::Jouer(Son::Coup, 0.6f);
            etincelles->Explosion(milieu + Vector3::up * 0.9f, Color(1.0f, 0.9f, 0.6f), 20, 3.0f);
            co_yield Attendre(0.9f);
        }
        else
        {
            Ref<Animateur> animGagnant = c->AttaquantGagne ? a->Anim : animB;
            Ref<Animateur> animPerdant = c->AttaquantGagne ? animB : a->Anim;
            Ref<RigVue> rigPerdant = c->AttaquantGagne ? rigB : a->Rig;
            Ref<Transform> tPerdant = c->AttaquantGagne ? tb : a->transform();
            if (animGagnant != nullptr) animGagnant->Jouer(Geste::Attaque);
            co_yield Attendre(0.38f);
            Sons::Jouer(Son::Coup);
            if (animPerdant != nullptr) animPerdant->Jouer(Geste::Touche);
            if (rigPerdant != nullptr) rigPerdant->Eclair(Color(1.0f, 0.25f, 0.15f), 0.4f);
            etincelles->Explosion(tPerdant->position() + Vector3::up * Mathf::Max(0.6f, (rigPerdant != nullptr ? rigPerdant->Hauteur : 1.0f) * 0.55f), Color(1.0f, 0.7f, 0.35f), 30, 4.0f);
            co_yield Attendre(0.7f);
            if (!c->Duel)
            {
                if (c->AttaquantGagne)
                {
                    // le monstre s'effondre
                    if (monstreAnim != nullptr) monstreAnim->Jouer(Geste::Ko);
                    co_yield Attendre(0.6f);
                    if (monstrePersistant && c->Monstre->Errant)   // le Dragonnoie reste à terre
                    {
                        monstreVue = nullptr;
                        monstreAnim = nullptr;
                        monstrePersistant = false;
                    }
                    else if (monstrePersistant) EnleverMonstre(false);
                    else if (monstreVue != nullptr)
                    {
                        fumees->Fumee(monstreVue->transform()->position(), Color(0.35f, 0.32f, 0.3f, 0.8f), 30, 0.8f);
                        etincelles->Explosion(monstreVue->transform()->position() + Vector3::up * 0.6f, Color(1.0f, 0.85f, 0.45f), 24, 3.0f);
                        auto go = monstreVue->gameObject();
                        monstreVue = nullptr;
                        monstreAnim = nullptr;
                        StartCoroutine(Disparaitre(go, 0.35f));
                    }
                    a->Anim->Jouer(Geste::Joie);
                }
                else
                {
                    co_yield Attendre(0.2f);
                    EnleverMonstre(false);
                }
            }
            else
            {
                auto gagnant = c->AttaquantGagne ? a : pions.At(c->Defenseur);
                gagnant->Anim->Jouer(Geste::Victoire, 1.0f);
            }
        }
        StartCoroutine(Disparaitre(deA->gameObject(), 0.35f));
        StartCoroutine(Disparaitre(deB->gameObject(), 0.35f));
        co_yield Attendre(0.5f);
        fc->Cacher();
        if (c->Arene)
        {
            co_yield StartCoroutine(ApresDuelArene(c));
            co_return;
        }
        if (c->Duel && !c->Egalite)
        {
            // chacun reprend sa place
            RangerCase(partie->Joueurs[c->Attaquant]->Case, true);
            RangerCase(partie->Joueurs[c->Defenseur]->Case, true);
        }
        if (!c->Egalite) cam->Suivre(pions.At(partie->JoueurCourant()->Id)->transform(), 12.5f);
    }
}
