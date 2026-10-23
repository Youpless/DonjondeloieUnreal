// Donjon de l'Oie — la nuit à l'écran : la lumière tombe (lune, lampes plus vives), puis l'événement de la nuit — le
// voleur, le trou de ver, la bonne nuit ou l'arène du diable, qui flotte au-dessus de l'île et où les pions se battent
// l'un après l'autre — et le jour se lève.
#include "DDJeu.h"

#include "../Interface/Hud.h"
#include "../Interface/UIK.h"
#include "../Modeles/Catalogue.h"
#include "../Monde/Ambiance.h"
#include "../Monde/Animateur.h"
#include "../Monde/CameraRig.h"
#include "../Monde/Etincelles.h"
#include "../Monde/IleModele.h"
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

    // modeleArene change (construit au premier chargement) : il redevient nul quand le moteur se réinitialise, comme
    // au lancement d'une partie dans Unity.
    const bool DDJeu::statiquesInscrits = ::Moteur::Boucle::AuReinitialiser([] { DDJeu::modeleArene = nullptr; });

    Tache DDJeu::Nuit(Ptr<SNuit> s)
    {
        if (s->Tombe)
        {
            nuitEnCours = true;
            hud->Banniere("Fin du tour " + Texte::De(s->Tour) + "  ·  la nuit tombe", CouleurNuit);
            cam->Regarder(cam->CentreCarte, Mathf::Max(30.0f, cam->TailleCarte * 0.62f), 52.0f);
            Sons::Jouer(Son::Sort, 0.5f);
            co_yield StartCoroutine(Crepuscule(0.0f, 1.0f, 1.8f));
        }
        else
        {
            if (arene != nullptr) co_yield StartCoroutine(QuitterArene());
            hud->Banniere("Le jour se lève", UIK::Or);
            co_yield StartCoroutine(Crepuscule(1.0f, 0.0f, 1.4f));
            nuitEnCours = false;
        }
    }

    Tache DDJeu::Crepuscule(float de, float vers, float duree)
    {
        float t = 0.0f;
        duree /= V();
        while (t < duree)
        {
            t += Time::deltaTime;
            Ambiance::Nuit(Mathf::Lerp(de, vers, Mathf::SmoothStep(0.0f, 1.0f, t / duree)), cam->Cam);
            co_yield nullptr;
        }
        Ambiance::Nuit(vers, cam->Cam);
    }

    /// Une partie gagnée en pleine nuit (le champion de l'arène atteint le niveau visé, un trou de ver mène au
    /// Dragonnoie) : chacun regagne sa case, l'arène s'enfonce et le jour se lève avant la fête.
    Tache DDJeu::AubeVictoire()
    {
        if (arene != nullptr) co_yield StartCoroutine(QuitterArene());
        hud->Banniere("Le jour se lève", UIK::Or);
        co_yield StartCoroutine(Crepuscule(1.0f, 0.0f, 1.2f));
        nuitEnCours = false;
    }

    /// Au chargement : le modèle de l'arène et ses maillages sont prêts d'avance (pas d'à-coup à la première nuit
    /// d'arène ; les nuits suivantes réutilisent le même modèle).
    void DDJeu::PreparerArene()
    {
        if (modeleArene == nullptr) modeleArene = Catalogue::Decor("arene_diable", 7);
        auto essai = RigVue::Creer(modeleArene, transitoires, "Arène du diable", true, 2.6f);
        Destroy(essai->gameObject());
    }

    /// Une partie interrompue en pleine nuit (retour au menu, nouvelle partie) : le jour revient d'un coup.
    void DDJeu::JourImmediat()
    {
        if (Ambiance::NuitActuelle() > 0.0f) Ambiance::Nuit(0.0f, cam != nullptr ? cam->Cam : Ref<Camera>());
        if (arene != nullptr)
        {
            Destroy(arene->gameObject());
            arene = nullptr;
        }
        areneOrdre.Clear();
        nuitEnCours = false;
    }

    // ------------------------------------------------------------------ le voleur, le trou de ver, la bonne nuit

    Tache DDJeu::VolNocturne(Ptr<SEffet> e)
    {
        if (!pions.ContainsKey(e->Joueur)) co_return;
        auto p = pions.At(e->Joueur);
        cam->Regarder(p->transform()->position() + Vector3::up * 0.6f, 9.0f, 40.0f);
        co_yield Attendre(0.5f);
        auto loin = p->transform()->position() + (p->transform()->position() - cam->CentreCarte).normalized() * 7.0f + Vector3::up * 2.0f;
        fumees->Fumee(p->transform()->position(), Color(0.12f, 0.1f, 0.2f, 0.85f), 18, 0.6f);
        etincelles->Trait(Tete(e->Joueur), loin, Color(1.0f, 0.82f, 0.3f), 30);
        Sons::Jouer(Son::Or, 0.8f);
        p->Anim->Jouer(Geste::Touche);
        hud->Flottant(Tete(e->Joueur) + Vector3::up * 0.4f, "Au voleur !", UIK::Rouge, 30);
        co_yield Attendre(0.9f);
    }

    Tache DDJeu::TrouDeVer(Ptr<SEffet> e)
    {
        if (!pions.ContainsKey(e->Joueur) || !pions.ContainsKey(e->Cible)) co_return;
        auto a = pions.At(e->Joueur)->transform()->position();
        auto b = pions.At(e->Cible)->transform()->position();
        float d = Vector3::Distance(a, b);
        cam->Regarder((a + b) * 0.5f + Vector3::up * 0.6f, Mathf::Clamp(d * 1.1f + 8.0f, 10.0f, 60.0f), 48.0f);
        Sons::Jouer(Son::Sort, 1.0f);
        auto violet = Color(0.7f, 0.4f, 1.0f);
        for (int k = 0; k < 3; k++)
        {
            etincelles->Spirale(a, violet, 30, 2.2f);
            etincelles->Spirale(b, violet, 30, 2.2f);
            co_yield Attendre(0.25f);
        }
        etincelles->Trait(a + Vector3::up, b + Vector3::up, violet, 40);
        co_yield Attendre(0.3f);
    }

    Tache DDJeu::BonneNuit(Ptr<SEffet> e)
    {
        if (!pions.ContainsKey(e->Joueur)) co_return;
        auto p = pions.At(e->Joueur);
        cam->Regarder(p->transform()->position() + Vector3::up * 0.6f, 8.0f, 36.0f);
        co_yield Attendre(0.4f);
        for (int k = 0; k < 3; k++)
        {
            hud->Flottant(Tete(e->Joueur) + Vector3(0.25f * static_cast<float>(k), 0.3f * static_cast<float>(k), 0.0f), "Z", Color(0.75f, 0.85f, 1.0f), 26 + 8 * k);
            co_yield Attendre(0.35f);
        }
        etincelles->Spirale(p->transform()->position(), Color(0.7f, 0.8f, 1.0f), 30, 2.0f);
        co_yield Attendre(0.3f);
    }

    // ------------------------------------------------------------------ l'arène du diable

    /// L'arène flotte au-dessus de l'île ; les pions y sont transportés en cercle autour du centre.
    Tache DDJeu::EntrerArene(Ptr<SEffet> e)
    {
        areneOrdre.Clear();
        for (auto& morceau : Texte::Decouper(e->Texte, ','))
        {
            int id = 0;
            if (Texte::LireEntier(morceau, id) && pions.ContainsKey(id)) areneOrdre.Add(id);
        }
        float hmax = 0.0f;
        for (float h : modele->H) hmax = Mathf::Max(hmax, h);
        areneCentre = modele->Centre + Vector3::up * (hmax + 14.0f);
        if (modeleArene == nullptr) modeleArene = Catalogue::Decor("arene_diable", 7);
        if (arene == nullptr) arene = RigVue::Creer(modeleArene, transitoires, "Arène du diable", true, 2.6f);
        arene->transform()->position(areneCentre - Vector3::up * 6.0f);
        arene->transform()->rotation(Quaternion::Euler(0.0f, cam->Lacet, 0.0f));       // l'entrée, sans piliers, face à la caméra
        hud->Banniere("L'arène du diable", Color(1.0f, 0.35f, 0.2f));
        cam->Regarder(areneCentre + Vector3::up * 1.0f, 14.0f, 46.0f);
        Sons::Jouer(Son::Monstre, 0.9f);
        // elle monte des ténèbres
        float t = 0.0f;
        while (t < 1.2f / V())
        {
            t += Time::deltaTime;
            arene->transform()->position(Vector3::Lerp(areneCentre - Vector3::up * 6.0f, areneCentre, Mathf::SmoothStep(0.0f, 1.0f, t * V() / 1.2f)));
            co_yield nullptr;
        }
        arene->transform()->position(areneCentre);
        etincelles->Explosion(areneCentre + Vector3::up * 0.5f, Color(1.0f, 0.4f, 0.15f), 50, 5.0f);
        // chacun est happé vers sa place autour du cercle
        for (int i = 0; i < areneOrdre.Count(); i++)
        {
            auto p = pions.At(areneOrdre[i]);
            etincelles->Spirale(p->transform()->position(), Color(1.0f, 0.45f, 0.2f), 24, 1.6f);
            StartCoroutine(p->Teleporter(PlaceArene(i)));
            co_yield Attendre(0.18f);
        }
        co_yield Attendre(0.6f);
        for (int i = 0; i < areneOrdre.Count(); i++) pions.At(areneOrdre[i])->Regarder(areneCentre);
    }

    /// La place d'attente d'un combattant : en arc de cercle derrière le centre (vu de la caméra), en deçà de la lave.
    Vector3 DDJeu::PlaceArene(int rang) const
    {
        int n = Mathf::Max(1, areneOrdre.Count());
        float a = Mathf::Pi * (0.08f + 0.84f * (n == 1 ? 0.5f : static_cast<float>(rang) / static_cast<float>(n - 1)));
        auto avant = arene != nullptr ? arene->transform()->forward() : Vector3::forward;
        auto droite = arene != nullptr ? arene->transform()->right() : Vector3::right;
        return areneCentre + droite * (Mathf::Cos(a) * 2.0f) + avant * (0.6f + Mathf::Sin(a) * 1.0f);
    }

    /// Avant un duel de l'arène : les deux adversaires s'avancent au centre, la caméra les regarde d'assez haut pour
    /// passer au-dessus des cornes de l'entrée.
    Tache DDJeu::AvancerDuelArene(Ptr<SCombat> c)
    {
        if (arene == nullptr || !pions.ContainsKey(c->Attaquant) || !pions.ContainsKey(c->Defenseur)) co_return;
        auto droite = arene->transform()->right();
        auto a = pions.At(c->Attaquant);
        auto b = pions.At(c->Defenseur);
        cam->Regarder(areneCentre + Vector3::up * 0.7f, 9.5f, 46.0f);
        StartCoroutine(a->Glisser(areneCentre - droite * 0.8f, 0.35f / V()));
        co_yield StartCoroutine(b->Glisser(areneCentre + droite * 0.8f, 0.35f / V()));
        a->Regarder(b->transform()->position());
        b->Regarder(a->transform()->position());
    }

    /// Après un duel de l'arène : le vaincu regagne sa place, le champion attend le suivant.
    Tache DDJeu::ApresDuelArene(Ptr<SCombat> c)
    {
        if (arene == nullptr || c->Egalite) co_return;
        int perdant = c->AttaquantGagne ? c->Defenseur : c->Attaquant;
        int rang = areneOrdre.IndexOf(perdant);
        if (rang >= 0 && pions.ContainsKey(perdant)) StartCoroutine(pions.At(perdant)->Glisser(PlaceArene(rang), 0.4f / V()));
        cam->Regarder(areneCentre + Vector3::up * 1.0f, 14.0f, 46.0f);
        co_yield Attendre(0.3f);
    }

    Tache DDJeu::ChampionArene(Ptr<SEffet> e)
    {
        if (arene == nullptr || !pions.ContainsKey(e->Joueur)) co_return;
        auto p = pions.At(e->Joueur);
        co_yield StartCoroutine(p->Glisser(areneCentre, 0.3f / V()));
        cam->Regarder(areneCentre + Vector3::up * 0.8f, 9.0f, 40.0f);
        p->Anim->Jouer(Geste::Victoire, 1.2f);
        Sons::Jouer(Son::Victoire, 0.7f);
        for (int k = 0; k < 3; k++)
        {
            // (les trois tirages du Vector3, dans l'ordre de C#)
            float rx = Random::Range(-2.0f, 2.0f);
            float ry = Random::Range(2.0f, 3.5f);
            float rz = Random::Range(-1.0f, 1.0f);
            etincelles->Explosion(areneCentre + Vector3(rx, ry, rz),
                                  k % 2 == 0 ? Color(1.0f, 0.4f, 0.15f) : Color(1.0f, 0.85f, 0.4f), 40, 4.0f, 0.07f, 1.5f);
            co_yield Attendre(0.3f);
        }
    }

    /// Au lever du jour : chacun retrouve sa case, l'arène s'enfonce dans les ténèbres.
    Tache DDJeu::QuitterArene()
    {
        for (int id : areneOrdre)
        {
            if (!pions.ContainsKey(id)) continue;
            etincelles->Spirale(pions.At(id)->transform()->position(), Color(1.0f, 0.45f, 0.2f), 20, 1.4f);
            StartCoroutine(pions.At(id)->Teleporter(Place(id)));
        }
        co_yield Attendre(0.8f);
        for (auto& j : partie->Joueurs) RangerCase(j->Case, false);
        auto depart = arene->transform()->position();
        float t = 0.0f;
        while (t < 0.9f / V())
        {
            t += Time::deltaTime;
            arene->transform()->position(Vector3::Lerp(depart, depart - Vector3::up * 8.0f, t * V() / 0.9f));
            co_yield nullptr;
        }
        Destroy(arene->gameObject());
        arene = nullptr;
        areneOrdre.Clear();
        cam->Regarder(cam->CentreCarte, Mathf::Max(30.0f, cam->TailleCarte * 0.62f), 52.0f);
    }
}
