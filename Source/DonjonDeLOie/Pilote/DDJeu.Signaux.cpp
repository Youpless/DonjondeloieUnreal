// Donjon de l'Oie — chaque signal des règles devient ce que voient et entendent les joueurs.
#include "DDJeu.h"

#include "../Interface/EcranRegles.h"
#include "../Interface/FenetreQuestion.h"
#include "../Interface/Hud.h"
#include "../Interface/UIK.h"
#include "../Monde/Animateur.h"
#include "../Monde/CameraRig.h"
#include "../Monde/DeVue.h"
#include "../Monde/Etincelles.h"
#include "../Monde/IleModele.h"
#include "../Monde/IleVue.h"
#include "../Monde/PionVue.h"
#include "../Monde/RigVue.h"
#include "../Monde/Sons.h"
#include "../Voxel/Pal.h"

namespace DonjonDeLOie::Pilote
{
    using namespace DonjonDeLOie::Regles;
    using namespace DonjonDeLOie::Voxel;
    using namespace DonjonDeLOie::Monde;
    using namespace DonjonDeLOie::Interface;

    Tache DDJeu::Boucle()
    {
        co_yield WaitForSeconds(0.6f);
        for (auto& s : partie->Debut()) co_yield StartCoroutine(Presenter(s));
        while (!partie->Fini)
        {
            for (auto& s : partie->JouerTour())
            {
                co_yield StartCoroutine(Presenter(s));
                if (partie == nullptr) co_return;
            }
            hud->Rafraichir();
            co_yield Attendre(0.2f);
        }
        co_yield StartCoroutine(Victoire());
    }

    Color DDJeu::CouleurTon(Ton t)
    {
        switch (t)
        {
            case Ton::Bon: return UIK::Vert;
            case Ton::Mauvais: return UIK::Rouge;
            case Ton::Combat: return Color(1.0f, 0.62f, 0.35f);
            case Ton::Quete: return UIK::Or;
            case Ton::Systeme: return UIK::Bleu;
            default: return UIK::Creme;
        }
    }

    Tache DDJeu::Presenter(Ptr<Signal> s)
    {
        if (s->Sorte == SorteSignal::TourDebut) co_yield StartCoroutine(TourDebut(Regles::Comme<STourDebut>(s)));
        else if (s->Sorte == SorteSignal::Annonce)
        {
            auto a = Regles::Comme<SAnnonce>(s);
            Color c = a->Joueur >= 0 ? Color(Pal::Joueurs[partie->Joueurs[a->Joueur]->Couleur]) : CouleurTon(a->Ton);
            hud->Journal(a->Texte, c);
            if (a->Grande)
            {
                hud->Annonce(a->Texte, CouleurTon(a->Ton) == UIK::Creme ? UIK::Or : CouleurTon(a->Ton));
                co_yield Attendre(1.1f);
            }
            else co_yield Attendre(0.2f);
        }
        else if (s->Sorte == SorteSignal::De) co_yield StartCoroutine(De(Regles::Comme<SDe>(s)));
        else if (s->Sorte == SorteSignal::Pas) co_yield StartCoroutine(Pas(Regles::Comme<SPas>(s)));
        else if (s->Sorte == SorteSignal::Teleport) co_yield StartCoroutine(Teleport(Regles::Comme<STeleport>(s)));
        else if (s->Sorte == SorteSignal::Envol) co_yield StartCoroutine(Envol(Regles::Comme<SEnvol>(s)));
        else if (s->Sorte == SorteSignal::Stat) co_yield StartCoroutine(Stat(Regles::Comme<SStat>(s)));
        else if (s->Sorte == SorteSignal::Combat) co_yield StartCoroutine(Combat(Regles::Comme<SCombat>(s)));
        else if (s->Sorte == SorteSignal::Effet) co_yield StartCoroutine(Effet(Regles::Comme<SEffet>(s)));
        else if (s->Sorte == SorteSignal::Question) co_yield StartCoroutine(Question(Regles::Comme<SQuestion>(s)));
        else if (s->Sorte == SorteSignal::Nuit) co_yield StartCoroutine(Nuit(Regles::Comme<SNuit>(s)));
        VerifierOie();
        hud->Rafraichir();
    }

    Tache DDJeu::TourDebut(Ptr<STourDebut> s)
    {
        auto j = partie->Joueurs[s->Joueur];
        for (auto& p : pions.Values()) p->Actif(p->Joueur == j->Id);
        hud->Rafraichir();
        Color c = Pal::Joueurs[j->Couleur];
        hud->Banniere("Tour " + Texte::De(s->Tour) + "  ·  " + j->Nom + (j->Humain ? "" : "  (bot)"), c);
        hud->Journal("— " + j->Nom + " joue —", c);
        cam->Suivre(pions.At(j->Id)->transform(), 12.5f);
        MajMarqueurs(j);
        if (j->Humain && Humains() > 1) hud->Annonce("À vous, " + j->Nom + " !", c);
        co_yield Attendre(j->Humain ? 0.5f : 0.45f);
    }

    Tache DDJeu::De(Ptr<SDe> s)
    {
        auto p = pions.At(s->Joueur);
        auto droite = cam->transform()->right();
        droite.y = 0.0f;
        droite.Normalize();
        auto avant = cam->transform()->forward();
        avant.y = 0.0f;
        avant.Normalize();
        auto arrivee = p->transform()->position() + droite * 0.95f - avant * 0.35f;
        arrivee.y = p->transform()->position().y;
        auto depart = arrivee - droite * 1.6f - avant * 1.2f + Vector3::up * 1.6f;
        auto de = DeVue::Creer(transitoires, s->Faces);
        Sons::Jouer(Son::De);
        float duree = 1.0f / V();
        co_yield StartCoroutine(de->Lancer(depart, arrivee, s->Valeur, duree));
        string txt = s->Bonus > 0 ? Texte::De(s->Valeur) + " + " + Texte::De(s->Bonus) : Texte::De(s->Valeur);
        hud->Flottant(de->transform()->position() + Vector3::up * 0.5f, txt, UIK::Or, 44);
        // (Raison n'est jamais null en C# : les règles la remplissent toujours ; le vide tient lieu de null)
        if (!Texte::Vide(s->Raison) && s->Raison != "Déplacement")
            hud->Journal(s->Raison + " : " + Texte::De(s->Valeur) + (s->Bonus > 0 ? " + " + Texte::De(s->Bonus) : string()), UIK::Creme);
        co_yield Attendre(0.55f);
        StartCoroutine(Disparaitre(de->gameObject(), 0.4f));
    }

    Tache DDJeu::Disparaitre(Ref<GameObject> go, float duree)
    {
        if (go == nullptr) co_return;
        auto e0 = go->transform()->localScale;
        float t = 0.0f;
        while (t < duree && go != nullptr)
        {
            t += Time::deltaTime;
            go->transform()->localScale = e0 * (1.0f - t / duree);
            co_yield nullptr;
        }
        if (go != nullptr) Destroy(go);
    }

    Tache DDJeu::Pas(Ptr<SPas> s)
    {
        auto p = pions.At(s->Joueur);
        auto vers = Place(s->Joueur);
        // la caméra regardait ailleurs (un combat, le Dragonnoie montré d'un clic) : elle reprend le héros qui marche
        if (partie->JoueurCourant()->Id == s->Joueur && !cam->Suit() && !cam->VueEnsemble()) cam->Suivre(p->transform(), 12.5f);
        Sons::Jouer(Son::Pas, 0.7f);
        co_yield StartCoroutine(p->Sauter(vers, 0.3f / V(), 0.5f));
        fumees->Fumee(vers, Color(0.75f, 0.68f, 0.55f, 0.55f), 4, 0.25f);
        RangerCase(s->De, true);
        RangerCase(s->Vers, true, s->Joueur);
    }

    Tache DDJeu::Teleport(Ptr<STeleport> s)
    {
        auto p = pions.At(s->Joueur);
        etincelles->Spirale(p->transform()->position(), Color(0.6f, 0.75f, 1.0f), 30, 2.5f);
        Sons::Jouer(Son::Sort, 0.7f);
        auto vers = Place(s->Joueur);
        co_yield StartCoroutine(p->Teleporter(vers));
        etincelles->Spirale(vers, Color(0.6f, 0.75f, 1.0f), 30, 2.5f);
        RangerCase(s->Vers, true, s->Joueur);
        for (auto& j : partie->Joueurs) RangerCase(j->Case, true);
        if (partie->JoueurCourant()->Id == s->Joueur) cam->Suivre(p->transform(), 12.5f);
        co_yield Attendre(0.2f);
    }

    Tache DDJeu::Stat(Ptr<SStat> s)
    {
        auto j = partie->Joueurs[s->Joueur];
        auto tete = Tete(s->Joueur);
        string signe = s->Delta > 0 ? "+" : "−";
        int v = Mathf::Abs(s->Delta);
        switch (s->Stat)
        {
            case Regles::Stat::PV:
                hud->Flottant(tete, signe + Texte::De(v) + " PV", s->Delta > 0 ? UIK::Vert : UIK::Rouge);
                if (s->Delta < 0)
                {
                    pions.At(j->Id)->Rig->Eclair(Color(1.0f, 0.15f, 0.1f), 0.35f);
                    etincelles->Explosion(tete - Vector3::up * 0.5f, Color(1.0f, 0.3f, 0.2f), 10, 2.5f);
                }
                break;
            case Regles::Stat::Or:
                hud->Flottant(tete, signe + Texte::De(v) + " or", s->Delta > 0 ? UIK::Or : Color(0.9f, 0.55f, 0.3f));
                if (s->Delta > 0)
                {
                    etincelles->Pieces(pions.At(j->Id)->transform()->position(), Mathf::Clamp(v / 2, 4, 24));
                    Sons::Jouer(Son::Or, 0.8f);
                }
                break;
            case Regles::Stat::XP:
                hud->Flottant(tete, signe + Texte::De(v) + " XP", s->Delta > 0 ? UIK::Bleu : UIK::Violet);
                break;
            case Regles::Stat::Niveau:
                hud->Flottant(tete + Vector3::up * 0.3f, s->Delta > 0 ? "NIVEAU " + Texte::De(j->Niveau) + " !" : "Niveau " + Texte::De(j->Niveau), s->Delta > 0 ? UIK::Or : UIK::Violet, 38);
                break;
            default:
                break;
        }
        hud->Rafraichir();
        co_yield Attendre(0.15f);
    }

    Tache DDJeu::Effet(Ptr<SEffet> e)
    {
        Ref<PionVue> p = e->Joueur >= 0 && pions.ContainsKey(e->Joueur) ? pions.At(e->Joueur) : Ref<PionVue>();
        Ref<PionVue> cible = e->Cible >= 0 && pions.ContainsKey(e->Cible) ? pions.At(e->Cible) : Ref<PionVue>();
        switch (e->Effet)
        {
            case Regles::Effet::PiegePose:
                Sons::Jouer(Son::Piege, 0.5f);
                if (p != nullptr) p->Anim->Jouer(Geste::Lancer);
                fumees->Fumee(modele->Cases[e->Case], Color(0.55f, 0.45f, 0.32f, 0.7f), 10, 0.5f);
                MajMarqueurs(partie->JoueurCourant());
                co_yield Attendre(0.5f);
                break;
            case Regles::Effet::PiegeDeclenche:
                Sons::Jouer(Son::Piege);
                if (cible != nullptr)
                {
                    cible->Anim->Jouer(Geste::Touche);
                    cible->Rig->Eclair(Color(1.0f, 0.2f, 0.1f), 0.5f);
                }
                etincelles->Explosion(modele->Cases[e->Case] + Vector3::up * 0.3f, Color(1.0f, 0.55f, 0.25f), 28, 4.0f);
                fumees->Fumee(modele->Cases[e->Case], Color(0.4f, 0.35f, 0.3f, 0.7f), 14, 0.6f);
                MajMarqueurs(partie->JoueurCourant());
                co_yield Attendre(0.6f);
                break;
            case Regles::Effet::EmbuscadeTendue:
                Sons::Jouer(Son::Page);
                if (p != nullptr) p->Anim->Jouer(Geste::Sort, 0.6f);
                MajMarqueurs(partie->JoueurCourant());
                co_yield Attendre(0.5f);
                break;
            case Regles::Effet::EmbuscadeDeclenchee:
                Sons::Jouer(Son::Monstre, 0.6f);
                MajMarqueurs(partie->JoueurCourant());
                if (p != nullptr && cible != nullptr)
                {
                    p->Regarder(cible->transform()->position());
                    cible->Regarder(p->transform()->position());
                    p->Anim->Jouer(Geste::Joie);
                }
                co_yield Attendre(0.5f);
                break;
            case Regles::Effet::SortLance:
                Sons::Jouer(Son::Sort);
                if (p != nullptr)
                {
                    p->Anim->Jouer(Geste::Sort);
                    if (cible != nullptr) p->Regarder(cible->transform()->position());
                }
                if (p != nullptr && cible != nullptr && cible != p)
                {
                    auto col = e->Objet == "foudre" ? Color(0.65f, 0.85f, 1.0f) : e->Objet == "lenteur" ? Color(0.5f, 0.9f, 1.0f) : Color(0.75f, 0.45f, 1.0f);
                    etincelles->Trait(Tete(p->Joueur), Tete(cible->Joueur), col, 40);
                    co_yield Attendre(0.4f);
                    etincelles->Explosion(Tete(cible->Joueur), col, 24, 3.0f, 0.08f, 1.0f);
                    if (e->Objet == "foudre")
                    {
                        cible->Anim->Jouer(Geste::Touche);
                        cible->Rig->Eclair(Color(0.6f, 0.8f, 1.0f), 0.5f);
                    }
                }
                co_yield Attendre(0.4f);
                break;
            case Regles::Effet::SortBloque:
                if (p != nullptr) etincelles->Explosion(Tete(p->Joueur), Color(0.95f, 0.9f, 0.6f), 30, 2.5f, 0.06f, 0.0f);
                Sons::Jouer(Son::Soin);
                co_yield Attendre(0.4f);
                break;
            case Regles::Effet::Malediction:
                if (p != nullptr)
                {
                    etincelles->Spirale(p->transform()->position(), Color(0.62f, 0.3f, 0.95f), 36, 1.6f);
                    fumees->Fumee(p->transform()->position(), Color(0.25f, 0.1f, 0.35f, 0.6f), 16, 0.5f);
                    p->Rig->Eclair(Color(0.5f, 0.15f, 0.8f), 0.6f);
                }
                Sons::Jouer(Son::Malus, 0.8f);
                co_yield Attendre(0.5f);
                break;
            case Regles::Effet::NiveauGagne:
                if (p != nullptr)
                {
                    etincelles->Spirale(p->transform()->position(), Color(1.0f, 0.82f, 0.35f), 60, 3.2f);
                    p->Anim->Jouer(Geste::Joie);
                    p->Rig->Eclair(Color(1.0f, 0.85f, 0.4f), 0.8f);
                }
                Sons::Jouer(Son::Niveau);
                co_yield Attendre(0.6f);
                break;
            case Regles::Effet::NiveauPerdu:
                if (p != nullptr)
                {
                    fumees->Fumee(p->transform()->position() + Vector3::up * 0.5f, Color(0.3f, 0.2f, 0.35f, 0.7f), 18, 0.5f);
                    p->Anim->Jouer(Geste::Touche);
                }
                Sons::Jouer(Son::Malus);
                co_yield Attendre(0.6f);
                break;
            case Regles::Effet::KO:
                if (p != nullptr)
                {
                    p->Anim->Jouer(Geste::Ko);
                    fumees->Fumee(p->transform()->position(), Color(0.5f, 0.5f, 0.5f, 0.7f), 24, 0.6f);
                }
                Sons::Jouer(Son::Malus);
                co_yield Attendre(1.1f);
                if (p != nullptr) p->Anim->Relever();
                break;
            case Regles::Effet::Soin:
                if (p != nullptr) etincelles->Spirale(p->transform()->position(), Color(0.45f, 1.0f, 0.55f), 20, 1.8f);
                Sons::Jouer(Son::Soin, 0.6f);
                co_yield Attendre(0.3f);
                break;
            case Regles::Effet::Tresor:
                etincelles->Explosion(modele->Cases[e->Case] + Vector3::up * 0.4f, Color(1.0f, 0.85f, 0.4f), 26, 3.0f, 0.07f, 2.0f);
                if (p != nullptr) p->Anim->Jouer(Geste::Joie);
                co_yield Attendre(0.3f);
                break;
            case Regles::Effet::Achat:
                Sons::Jouer(Son::Or, 0.6f);
                co_yield Attendre(0.1f);
                break;
            case Regles::Effet::QueteAcceptee:
                Sons::Jouer(Son::Page);
                MajMarqueurs(partie->JoueurCourant());
                co_yield Attendre(0.3f);
                break;
            case Regles::Effet::QueteFinie:
                if (p != nullptr)
                {
                    etincelles->Spirale(p->transform()->position(), Color(1.0f, 0.9f, 0.5f), 40, 2.6f);
                    p->Anim->Jouer(Geste::Victoire, 1.0f);
                }
                Sons::Jouer(Son::Niveau, 0.8f);
                MajMarqueurs(partie->JoueurCourant());
                co_yield Attendre(0.5f);
                break;
            case Regles::Effet::Evenement:
                Sons::Jouer(Son::Page);
                if (p != nullptr) etincelles->Explosion(Tete(p->Joueur) + Vector3::up * 0.4f, Color(0.45f, 0.7f, 1.0f), 18, 2.0f, 0.06f, 0.0f);
                co_yield Attendre(0.2f);
                break;
            case Regles::Effet::Victoire:
                co_yield Attendre(0.1f);
                break;
            case Regles::Effet::OrDragonnoie:
                co_yield StartCoroutine(OrArrache(e));
                break;
            case Regles::Effet::VolNocturne: co_yield StartCoroutine(VolNocturne(e)); break;
            case Regles::Effet::TrouDeVer: co_yield StartCoroutine(TrouDeVer(e)); break;
            case Regles::Effet::BonneNuit: co_yield StartCoroutine(BonneNuit(e)); break;
            case Regles::Effet::AreneDebut: co_yield StartCoroutine(EntrerArene(e)); break;
            case Regles::Effet::AreneFin: co_yield StartCoroutine(ChampionArene(e)); break;
            case Regles::Effet::QueteRatee: co_yield StartCoroutine(QueteRatee(e)); break;
        }
    }

    // ------------------------------------------------------------------ questions

    Tache DDJeu::Question(Ptr<SQuestion> q)
    {
        auto j = partie->Joueurs[q->Joueur];
        Color c = Pal::Joueurs[j->Couleur];
        bool montrerCases = q->Genre == GenreQuestion::Embranchement || q->Genre == GenreQuestion::Rodeur || q->Genre == GenreQuestion::ChoixCase;
        if (q->Genre == GenreQuestion::Evade)
        {
            co_yield StartCoroutine(ChercherEvade(q));
            co_return;
        }
        if (q->Genre == GenreQuestion::Monstre || q->Genre == GenreQuestion::Dragonnoie) co_yield StartCoroutine(FaireSurgir(q->Monstre, j));
        if (j->Humain)
        {
            if (montrerCases) ile->MontrerChoix(q->Cases, Color::Lerp(c, Color::white, 0.35f));
            // (« fini » : la variable locale que la lambda de C# modifie ; partagée ici avec la lambda)
            auto fini = Nouveau<bool>(false);
            fq->Montrer(q, partie, [q, fini](int i) {
                q->Reponse = i;
                *fini = true;
            });
            while (!*fini) co_yield nullptr;
            ile->EffacerChoix();
            ile->Survoler(-1);
        }
        else
        {
            fq->MontrerReflexion(j->Nom + " réfléchit…", c);
            if (montrerCases) ile->MontrerChoix(q->Cases, Color::Lerp(c, Color::white, 0.35f));
            co_yield Attendre(q->Genre == GenreQuestion::Action ? 0.55f : 0.75f);
            q->Reponse = ia->Repondre(*partie, *q);
            if (montrerCases && q->Reponse >= 0 && q->Reponse < q->Cases.Count() && q->Cases[q->Reponse] >= 0)
            {
                ile->Survoler(q->Cases[q->Reponse]);
                co_yield Attendre(0.4f);
                ile->Survoler(-1);
            }
            ile->EffacerChoix();
            fq->CacherReflexion();
            // un bot qui ne se contente pas de lancer le dé l'annonce
            string code = q->CodeChoisi();
            if (q->Genre == GenreQuestion::Action && !Texte::Vide(code) && code != "de" && q->Reponse >= 0)
                hud->Journal(j->Nom + " : " + Texte::Minuscules(q->Options[q->Reponse]) + ".", c);
        }
        if ((q->Genre == GenreQuestion::Monstre && q->CodeChoisi() == "fuir") || (q->Genre == GenreQuestion::Dragonnoie && q->CodeChoisi() == "passer"))
        {
            EnleverMonstre(false);
            cam->Suivre(pions.At(j->Id)->transform(), 12.5f);       // la caméra reprend le héros qui repart
        }
    }

    // ------------------------------------------------------------------ fin

    Tache DDJeu::Victoire()
    {
        etat = Etat::Victoire;
        if (nuitEnCours) co_yield StartCoroutine(AubeVictoire());   // gagnée en pleine nuit : le jour se lève d'abord
        auto g = partie->Joueurs[partie->Gagnant];
        auto p = pions.At(g->Id);
        Color c = Pal::Joueurs[g->Couleur];
        hud->Banniere("Victoire de " + g->Nom + " !", c);
        hud->Annonce(partie->VaincuDragonnoie ? g->Nom + " a terrassé le Dragonnoie !" : g->Nom + " atteint le niveau " + Texte::De(g->Niveau) + " !", UIK::Or);
        if (partie->VaincuDragonnoie && oieVue != nullptr) cam->Cadrer(p->transform()->position(), oieVue->transform()->position());
        else cam->Regarder(p->transform()->position() + Vector3::up * 0.6f, 7.0f, 30.0f);
        Sons::Jouer(Son::Victoire);
        for (int k = 0; k < 6; k++)
        {
            p->Anim->Jouer(Geste::Victoire);
            // (les trois tirages du Vector3, dans l'ordre de C#)
            float rx = Random::Range(-2.5f, 2.5f);
            float ry = Random::Range(2.5f, 4.0f);
            float rz = Random::Range(-1.0f, 2.5f);
            auto pos = p->transform()->position() + Vector3(rx, ry, rz);
            etincelles->Explosion(pos, k % 2 == 0 ? c : Color(1.0f, 0.85f, 0.4f), 50, 4.5f, 0.07f, 1.5f);
            co_yield WaitForSeconds(0.45f);
        }
        co_yield WaitForSeconds(0.8f);
        victoire = Nouveau<EcranVictoire>(ui, partie, [this] { Lancer(derniersReglages); }, [this] { AllerTitre(); });
        boutonsJeu->gameObject()->SetActive(false);
    }
}
