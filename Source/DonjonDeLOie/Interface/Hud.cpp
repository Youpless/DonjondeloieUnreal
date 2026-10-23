// Donjon de l'Oie — l'affichage en jeu : fiches des joueurs, bannière du tour, journal, grandes annonces,
// textes flottants au-dessus des pions et étiquettes de noms.
//
// (Dans Hud, « Canvas » et « Camera » désignent les champs : leurs types s'écrivent Moteur::Canvas, Moteur::Camera.)
#include "Hud.h"

#include "UIK.h"
#include "../Monde/Ambiance.h"
#include "../Monde/CameraRig.h"
#include "../Voxel/Pal.h"

namespace DonjonDeLOie::Interface
{
    using namespace DonjonDeLOie::Regles;
    using namespace DonjonDeLOie::Voxel;

    Ref<Hud> Hud::Creer(const Ref<RectTransform>& racine, const Ref<Moteur::Canvas>& canvas)
    {
        auto go = Neuf<GameObject>("HUD", TypeDe<RectTransform>());
        go->transform()->SetParent(racine, false);
        auto h = go->AddComponent<Hud>();
        h->Racine = UIK::Remplir(Vers<RectTransform>(go->transform()));
        h->Canvas = canvas;
        h->Batir();
        return h;
    }

    void Hud::Batir()
    {
        calqueMonde = UIK::Remplir(UIK::Rect(Racine, "Calque monde"));
        // Bannière du tour
        banniere = UIK::Placer(UIK::Panneau(Racine, "Bannière", Style::Bois), 0.5f, 1.0f, 0, -14, 640, 62, 0.5f, 1.0f);
        banniereCouleur = UIK::Image(banniere, "Couleur", UIK::Plein(), Color::white);
        UIK::Placer(banniereCouleur->rectTransform(), 0.0f, 0.5f, 10, 0, 14, 42, 0.0f, 0.5f);
        banniereCouleur->type = Image::Type::Sliced;
        banniereTexte = UIK::Texte(banniere, "Texte", "", 28, UIK::Creme, TextAnchor::MiddleCenter, true);
        UIK::Remplir(banniereTexte->rectTransform(), 30, 0, 12, 0);
        // Le Dragonnoie, sous la chronique : sa force, sa case, son prochain envol, son nid (clic : la caméra le montre).
        // Même colonne et même largeur que la chronique : rien ne se chevauche, quel que soit le format de l'écran.
        oie = UIK::Bouton(Racine, "Dragonnoie", "", Style::Verre, [this] { if (MontrerOie) MontrerOie(); }, 17);
        UIK::Placer(Vers<RectTransform>(oie->transform()), 1.0f, 1.0f, -16, -310, 470, 62, 1.0f, 1.0f);
        oieTexte = UIK::Libelle(oie);
        oieTexte->color = UIK::Creme;
        oieTexte->alignment = TextAnchor::MiddleLeft;
        oieTexte->horizontalOverflow = HorizontalWrapMode::Wrap;
        oieTexte->resizeTextForBestFit = true; oieTexte->resizeTextMinSize = 12; oieTexte->resizeTextMaxSize = 17;
        UIK::Remplir(oieTexte->rectTransform(), 52, 4, 10, 4);
        auto icOie = UIK::IconeImage(oie->transform(), "oie", 32);
        UIK::Placer(icOie->rectTransform(), 0, 0.5f, 12, 0, 32, 32, 0, 0.5f);
        oie->gameObject()->SetActive(false);
        // La prochaine nuit : même colonne, juste en dessous (ou à la place de l'encart du Dragonnoie s'il n'y est pas)
        nuit = UIK::Placer(UIK::Panneau(Racine, "Nuit", Style::Verre), 1.0f, 1.0f, -16, -310, 470, 40, 1.0f, 1.0f);
        nuit->GetComponent<Image>()->raycastTarget = false;
        auto icNuit = UIK::IconeImage(nuit, "lune", 26);
        UIK::Placer(icNuit->rectTransform(), 0, 0.5f, 14, 0, 26, 26, 0, 0.5f);
        nuitTexte = UIK::Texte(nuit, "Texte", "", 17, UIK::Creme, TextAnchor::MiddleLeft, false);
        UIK::Remplir(nuitTexte->rectTransform(), 52, 2, 10, 2);
        nuit->gameObject()->SetActive(false);
        // Journal
        journal = UIK::Placer(UIK::Panneau(Racine, "Journal", Style::Verre), 1.0f, 1.0f, -16, -16, 470, 286, 1.0f, 1.0f);
        auto titre = UIK::Texte(journal, "Titre", "Chronique", 20, UIK::Or, TextAnchor::UpperLeft, true);
        UIK::Placer(titre->rectTransform(), 0, 1, 16, -8, 300, 26, 0, 1);
        for (int i = 0; i < 9; i++)
        {
            auto t = UIK::Texte(journal, "Ligne" + Texte::De(i), "", 17, UIK::Creme, TextAnchor::UpperLeft, false);
            UIK::Placer(t->rectTransform(), 0, 1, 16, static_cast<float>(-38 - i * 27), 440, 26, 0, 1);
            t->horizontalOverflow = HorizontalWrapMode::Overflow;
            lignes.Add(t);
        }
        // Grande annonce
        annonce = UIK::Placer(UIK::Rect(Racine, "Annonce"), 0.5f, 0.62f, 0, 0, 1400, 120);
        annonceGroupe = annonce->gameObject()->AddComponent<CanvasGroup>();
        annonceGroupe->alpha = 0; annonceGroupe->blocksRaycasts = false;
        auto lueur = UIK::Image(annonce, "Lueur", UIK::Lueur(), Color(0, 0, 0, 0.55f));
        UIK::Remplir(lueur->rectTransform(), -60, -40, -60, -40);
        annonceTexte = UIK::Texte(annonce, "Texte", "", 46, UIK::Or, TextAnchor::MiddleCenter, true);
        UIK::Remplir(annonceTexte->rectTransform());
        auto ol = annonceTexte->gameObject()->AddComponent<Outline>();
        ol->effectColor = Color(0.15f, 0.08f, 0.02f, 0.9f); ol->effectDistance = Vector2(2, -2);
    }

    void Hud::Visible(bool v) { gameObject()->SetActive(v); }

    /// Le mini-jeu de l'évadé occupe l'écran : fiches, bannière, chronique et encarts s'effacent le temps de la recherche.
    void Hud::ModeMiniJeu(bool oui)
    {
        miniJeu = oui;
        for (auto& f : fiches) f->R->gameObject()->SetActive(!oui);
        banniere->gameObject()->SetActive(!oui);
        journal->gameObject()->SetActive(!oui);
        Rafraichir();
    }

    // ------------------------------------------------------------------ fiches des joueurs

    void Hud::Installer(const Ptr<Partie>& p)
    {
        partie = p;
        for (auto& f : fiches) Destroy(f->R->gameObject());
        fiches.Clear();
        for (auto& e : etiquettes.Values()) if (e != nullptr) Destroy(e->gameObject());
        etiquettes.Clear();
        messages.Clear();
        for (int i = 0; i < p->Joueurs.Count(); i++) fiches.Add(CreerFiche(p->Joueurs[i], i));
        for (auto& j : p->Joueurs)
        {
            auto t = UIK::Texte(calqueMonde, "Nom_" + j->Nom, j->Nom, 17, Pal::Joueurs[j->Couleur], TextAnchor::MiddleCenter, true);
            t->rectTransform()->anchorMin = t->rectTransform()->anchorMax = Vector2::zero;
            t->rectTransform()->sizeDelta = Vector2(220, 24);
            auto ol = t->gameObject()->AddComponent<Outline>(); ol->effectColor = Color(0, 0, 0, 0.85f); ol->effectDistance = Vector2(1.2f, -1.2f);
            etiquettes[j->Id] = t;
        }
        Rafraichir();
    }

    Ptr<Hud::Fiche> Hud::CreerFiche(const Ptr<Joueur>& j, int rang)
    {
        auto f = Nouveau<Fiche>();
        f->Joueur = j->Id;
        f->R = UIK::Placer(UIK::Panneau(Racine, "Fiche_" + j->Nom, Style::Bois), 0, 1, 16, static_cast<float>(-16 - rang * 122), 352, 114, 0, 1);
        f->Halo = UIK::Image(f->R, "Halo", UIK::Lueur(), Color(1, 0.85f, 0.4f, 0));
        UIK::Remplir(f->Halo->rectTransform(), -26, -22, -26, -22);
        f->Halo->transform()->SetAsFirstSibling();
        Color c = Pal::Joueurs[j->Couleur];
        f->Bande = UIK::Image(f->R, "Bande", UIK::Plein(), c);
        f->Bande->type = Image::Type::Sliced;
        UIK::Placer(f->Bande->rectTransform(), 0, 0.5f, 7, 0, 12, 100, 0, 0.5f);
        // niveau dans une couronne
        auto cour = UIK::IconeImage(f->R, "couronne", 30);
        UIK::Placer(cour->rectTransform(), 0, 1, 26, -8, 34, 34, 0, 1);
        f->Niveau = UIK::Texte(f->R, "Niveau", "1", 34, UIK::Or, TextAnchor::MiddleCenter, true);
        UIK::Placer(f->Niveau->rectTransform(), 0, 1, 24, -40, 40, 40, 0, 1);
        f->XP = Liste<Ref<Image>>(Donnees::XPParNiveau());
        for (int k = 0; k < f->XP.Length(); k++)
        {
            auto pip = UIK::Image(f->R, "XP" + Texte::De(k), UIK::Plein(), Color(1, 1, 1, 0.2f));
            pip->type = Image::Type::Sliced;
            UIK::Placer(pip->rectTransform(), 0, 1, static_cast<float>(26 + k * 19), -86, 15, 10, 0, 1);
            f->XP[k] = pip;
        }
        f->Nom = UIK::Texte(f->R, "Nom", j->Nom, 22, UIK::Creme, TextAnchor::UpperLeft, true);
        UIK::Placer(f->Nom->rectTransform(), 0, 1, 74, -8, 180, 28, 0, 1);
        f->Nom->horizontalOverflow = HorizontalWrapMode::Overflow;
        f->Classe = UIK::Texte(f->R, "Classe", j->Def()->Nom + (j->Humain ? "" : " · bot"), 16, Color(0.85f, 0.78f, 0.65f), TextAnchor::UpperRight, false);
        UIK::Placer(f->Classe->rectTransform(), 1, 1, -12, -11, 120, 22, 1, 1);
        // PV
        Ref<RectTransform> cadre;
        f->BarrePV = UIK::Barre(f->R, "PV", Color(0.82f, 0.25f, 0.22f), cadre);
        UIK::Placer(cadre, 0, 1, 98, -42, 236, 20, 0, 1);
        auto coeur = UIK::IconeImage(f->R, "coeur", 22);
        UIK::Placer(coeur->rectTransform(), 0, 1, 72, -41, 22, 22, 0, 1);
        f->PV = UIK::Texte(cadre, "Texte", "", 15, Color::white, TextAnchor::MiddleCenter, true);
        UIK::Remplir(f->PV->rectTransform());
        // or, attaque, défense
        float y = -72;
        auto piece = UIK::IconeImage(f->R, "piece", 20); UIK::Placer(piece->rectTransform(), 0, 1, 72, y, 20, 20, 0, 1);
        f->Or = UIK::Texte(f->R, "Or", "", 19, UIK::Or, TextAnchor::MiddleLeft, true); UIK::Placer(f->Or->rectTransform(), 0, 1, 96, y + 1, 70, 22, 0, 1);
        auto epee = UIK::IconeImage(f->R, "epee", 20); UIK::Placer(epee->rectTransform(), 0, 1, 166, y, 20, 20, 0, 1);
        f->Atk = UIK::Texte(f->R, "Atk", "", 19, UIK::Creme, TextAnchor::MiddleLeft, true); UIK::Placer(f->Atk->rectTransform(), 0, 1, 190, y + 1, 50, 22, 0, 1);
        auto bouc = UIK::IconeImage(f->R, "bouclier", 20); UIK::Placer(bouc->rectTransform(), 0, 1, 236, y, 20, 20, 0, 1);
        f->Def = UIK::Texte(f->R, "Def", "", 19, UIK::Creme, TextAnchor::MiddleLeft, true); UIK::Placer(f->Def->rectTransform(), 0, 1, 260, y + 1, 40, 22, 0, 1);
        f->Etats = UIK::Texte(f->R, "États", "", 15, Color(0.9f, 0.82f, 0.7f), TextAnchor::MiddleLeft, false);
        UIK::Placer(f->Etats->rectTransform(), 0, 1, 98, -94, 246, 20, 0, 1);
        f->Etats->horizontalOverflow = HorizontalWrapMode::Overflow;
        return f;
    }

    void Hud::Rafraichir()
    {
        if (partie == nullptr) return;
        for (auto& f : fiches)
        {
            auto j = partie->Joueurs[f->Joueur];
            bool actif = partie->JoueurCourant() == j && !partie->Fini;
            f->Halo->color = Color(1.0f, 0.82f, 0.4f, actif ? 0.55f : 0.0f);
            f->R->anchoredPosition = Vector2(actif ? 30.0f : 16.0f, f->R->anchoredPosition.y);
            f->Niveau->text(Texte::De(j->Niveau));
            for (int k = 0; k < f->XP.Length(); k++) f->XP[k]->color = k < j->XP ? Color(0.98f, 0.82f, 0.36f) : Color(1, 1, 1, 0.18f);
            f->BarrePV->fillAmount(Mathf::Clamp01(static_cast<float>(j->PV) / static_cast<float>(j->PVMax())));
            f->PV->text(Texte::De(Mathf::Max(0, j->PV)) + " / " + Texte::De(j->PVMax()));
            f->Or->text(Texte::De(j->Or));
            f->Atk->text(Texte::De(j->Attaque()));
            f->Def->text(Texte::De(j->Defense()));
            Liste<string> e;
            if (j->Quete != nullptr) e.Add(UIK::Colorer("Quête " + j->Quete->Progression(), Color(1.0f, 0.85f, 0.45f)));
            if (j->ToursMaudit > 0) e.Add(UIK::Colorer("Maudit " + Texte::De(j->ToursMaudit), UIK::Violet));
            if (j->ToursImmobilise > 0) e.Add(UIK::Colorer("Immobilisé", UIK::Rouge));
            if (j->Lenteur) e.Add(UIK::Colorer("Ralenti", UIK::Bleu));
            if (j->Embuscade != nullptr) e.Add(UIK::Colorer("En embuscade", Pal::Joueurs[j->Couleur]));
            if (j->ProtegeDuel) e.Add(UIK::Colorer("Protégé", UIK::Vert));
            if (j->Benediction > 0) e.Add(UIK::Colorer("Béni +" + Texte::De(j->Benediction), UIK::Or));
            f->Etats->text(Texte::Joindre("  ", e));
        }
        auto m = partie->Dragonnoie();
        oie->gameObject()->SetActive(m != nullptr && !partie->Fini && !miniJeu);
        if (m != nullptr)
            oieTexte->text("<b>Dragonnoie</b> · force " + Texte::De(m->Force) + " · sur « " + partie->Plateau[partie->CaseDragonnoie]->Libelle() + " »\n"
                           + "Envol au tour " + Texte::De(partie->TourEnvol) + (partie->OrDragonnoie > 0 ? " · nid : " + Texte::De(partie->OrDragonnoie) + " or" : string()));
        int pn = partie->ProchaineNuit();
        bool montrerNuit = pn >= 0 && !partie->Fini && !miniJeu;
        nuit->gameObject()->SetActive(montrerNuit);
        if (montrerNuit)
        {
            nuit->anchoredPosition = Vector2(-16, oie->gameObject()->activeSelf() ? -380.0f : -310.0f);
            nuitTexte->text(Monde::Ambiance::NuitActuelle() > 0.5f ? string("<b>La nuit est tombée</b>…")
                            : pn <= partie->Tour ? string("<b>La nuit tombe</b> à la fin de ce tour") : "<b>Nuit</b> à la fin du tour " + Texte::De(pn));
        }
    }

    // ------------------------------------------------------------------ bannière, journal, annonces

    void Hud::Banniere(const string& texte, Color couleur)
    {
        banniereTexte->text(texte);
        banniereCouleur->color = couleur;
    }

    void Hud::Journal(string s, Color c)
    {
        if (Texte::Longueur(s) > 58) s = Texte::RognerFin(Texte::Debut(s, 56)) + "…";
        messages.Add(UIK::Colorer("• ", c) + s);
        while (messages.Count() > 60) messages.RemoveAt(0);
        int n = lignes.Count();
        int debut = Mathf::Max(0, messages.Count() - n);
        for (int i = 0; i < n; i++)
        {
            int k = debut + i;
            lignes[i]->text(k < messages.Count() ? messages[k] : string());
        }
    }

    void Hud::Annonce(const string& texte, Color c)
    {
        annonceTexte->text(texte);
        annonceTexte->color = c;
        annonceT = 0;
    }

    // ------------------------------------------------------------------ textes flottants

    void Hud::Flottant(Vector3 monde, const string& texte, Color c, int taille)
    {
        auto t = UIK::Texte(calqueMonde, "Flottant", texte, taille, c, TextAnchor::MiddleCenter, true);
        t->rectTransform()->anchorMin = t->rectTransform()->anchorMax = Vector2::zero;
        t->rectTransform()->sizeDelta = Vector2(300, 50);
        auto ol = t->gameObject()->AddComponent<Outline>(); ol->effectColor = Color(0, 0, 0, 0.9f); ol->effectDistance = Vector2(1.6f, -1.6f);
        // les textes d'un même endroit s'empilent
        float empile = 0;
        for (auto& f : flottants) if ((f->Monde - monde).sqrMagnitude() < 0.5f && f->Vie < 0.6f) empile += 34;
        auto tf = Nouveau<TexteFlottant>();
        tf->T = t; tf->Monde = monde; tf->Vie = 0; tf->Duree = 1.6f; tf->Decalage = Vector2(0, empile);
        flottants.Add(tf);
    }

    Vector2 Hud::VersCanvas(Vector3 ecran)
    {
        float s = Canvas->scaleFactor() <= 0 ? 1.0f : Canvas->scaleFactor();
        return Vector2(ecran.x / s, ecran.y / s);
    }

    void Hud::Update()
    {
        float dt = Time::unscaledDeltaTime;
        // grande annonce : apparaît, reste, s'efface
        annonceT += dt;
        float a = annonceT < 0.15f ? annonceT / 0.15f : annonceT < 1.7f ? 1.0f : Mathf::Clamp01(1.0f - (annonceT - 1.7f) / 0.5f);
        annonceGroupe->alpha = a;
        annonce->localScale = Vector3::one * (annonceT < 0.15f ? Mathf::Lerp(1.25f, 1.0f, annonceT / 0.15f) : 1.0f);
        if (Camera == nullptr) return;
        for (int i = flottants.Count() - 1; i >= 0; i--)
        {
            auto f = flottants[i];
            f->Vie += dt;
            if (f->Vie >= f->Duree) { Destroy(f->T->gameObject()); flottants.RemoveAt(i); continue; }
            auto e = Camera->Ecran(f->Monde);
            f->T->enabled = e.z > 0;
            f->T->rectTransform()->anchoredPosition = VersCanvas(e) + f->Decalage + Vector2(0, f->Vie * 46.0f);
            auto c = f->T->color; c.a = f->Vie < f->Duree - 0.4f ? 1.0f : (f->Duree - f->Vie) / 0.4f; f->T->color = c;
            f->T->transform()->localScale = Vector3::one * (f->Vie < 0.12f ? Mathf::Lerp(1.5f, 1.0f, f->Vie / 0.12f) : 1.0f);
        }
        if (!PionDe) return;
        for (auto& kv : etiquettes)
        {
            auto tp = PionDe(kv.Key);
            if (tp == nullptr || kv.Value == nullptr) continue;
            auto e = Camera->Ecran(tp->position() + Vector3::up * (HauteurPion ? HauteurPion(kv.Key) + 0.3f : 1.8f));
            kv.Value->enabled = e.z > 0 && !Camera->VueEnsemble() && !miniJeu;
            kv.Value->rectTransform()->anchoredPosition = VersCanvas(e);
        }
    }
}
