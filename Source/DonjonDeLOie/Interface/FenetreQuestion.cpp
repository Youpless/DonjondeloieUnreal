// Donjon de l'Oie — les choix proposés au joueur humain : le bandeau du bas (action, embranchement, cible, case…)
// et les grandes fenêtres (boutique, quêtes, rencontre d'un monstre).
#include "FenetreQuestion.h"

#include "Survol.h"
#include "UIK.h"
#include "../Voxel/Pal.h"

namespace DonjonDeLOie::Interface
{
    using namespace DonjonDeLOie::Regles;
    using namespace DonjonDeLOie::Voxel;

    FenetreQuestion::FenetreQuestion(const Ref<RectTransform>& racine_) : racine(racine_) {}

    void FenetreQuestion::Cacher()
    {
        if (bandeau != nullptr) Object::Destroy(bandeau->gameObject());
        if (fenetre != nullptr) Object::Destroy(fenetre->gameObject());
        bandeau = fenetre = nullptr;
        ouverte = false;
        question = nullptr;
        CacherReflexion();
    }

    void FenetreQuestion::Choisir(int i)
    {
        if (!ouverte || !repondre) return;
        auto r = repondre;
        Cacher();
        r(i);
    }

    // ------------------------------------------------------------------ un bot réfléchit

    void FenetreQuestion::MontrerReflexion(const string& texte, Color c)
    {
        if (reflexion == nullptr)
        {
            reflexion = UIK::Placer(UIK::Panneau(racine, "Réflexion", Style::Verre), 0.5f, 0.0f, 0, 24, 520, 54, 0.5f, 0.0f);
            reflexionTexte = UIK::Texte(reflexion, "Texte", "", 21, UIK::Creme, TextAnchor::MiddleCenter, false);
            UIK::Remplir(reflexionTexte->rectTransform(), 10, 0, 10, 0);
        }
        reflexionTexte->text(UIK::Colorer("● ", c) + texte);
    }

    void FenetreQuestion::CacherReflexion()
    {
        if (reflexion != nullptr) Object::Destroy(reflexion->gameObject());
        reflexion = nullptr;
    }

    // ------------------------------------------------------------------ aiguillage

    void FenetreQuestion::Montrer(const Ptr<SQuestion>& q, const Ptr<Partie>& p, std::function<void(int)> rep)
    {
        Cacher();
        question = q;
        repondre = rep;
        ouverte = true;
        auto j = p->Joueurs[q->Joueur];
        switch (q->Genre)
        {
            case GenreQuestion::Boutique: Boutique(q, p, j); break;
            case GenreQuestion::Quetes: Quetes(q, p, j); break;
            case GenreQuestion::Monstre: Monstre(q, p, j); break;
            case GenreQuestion::Dragonnoie: Dragonnoie(q, p, j); break;
            default: Bandeau(q, p, j); break;
        }
    }

    // ------------------------------------------------------------------ bandeau du bas

    void FenetreQuestion::Bandeau(const Ptr<SQuestion>& q, const Ptr<Partie>& p, const Ptr<Joueur>& j)
    {
        Color cj = Pal::Joueurs[j->Couleur];
        // largeur des boutons selon leur texte, rangés en lignes
        const float largeurMax = 1240.0f;
        Liste<float> tailles;
        for (int i = 0; i < q->Options.Count(); i++)   // (+34 : l'icône du dé à gauche de « Lancer le dé »)
            tailles.Add(Mathf::Clamp(46 + Texte::Longueur(q->Options[i]) * 11.5f + (q->Codes[i] == "de" ? 34 : 0), 190.0f, 440.0f));
        Liste<Liste<int>> lignes;
        Liste<int> cur; float w = 0;
        for (int i = 0; i < q->Options.Count(); i++)
        {
            if (cur.Count() > 0 && w + tailles[i] + 12 > largeurMax - 60) { lignes.Add(cur); cur = Liste<int>(); w = 0; }
            cur.Add(i); w += tailles[i] + 12;
        }
        if (cur.Count() > 0) lignes.Add(cur);
        float largeur = 0;
        for (auto& l : lignes) { float s = 0; for (int i : l) s += tailles[i] + 12; largeur = Mathf::Max(largeur, s); }
        largeur = Mathf::Clamp(largeur + 60, 640.0f, largeurMax);
        float hauteur = static_cast<float>(128 + lignes.Count() * 66);
        bandeau = UIK::Placer(UIK::Panneau(racine, "Bandeau", Style::Bois), 0.5f, 0.0f, 0, 22, largeur, hauteur, 0.5f, 0.0f);
        auto bande = UIK::Image(bandeau, "Couleur", UIK::Plein(), cj);
        bande->type = Image::Type::Sliced;
        UIK::Placer(bande->rectTransform(), 0.5f, 1.0f, 0, -6, largeur - 40, 6, 0.5f, 1.0f);
        auto titre = UIK::Texte(bandeau, "Titre", q->Titre, 27, UIK::Or, TextAnchor::UpperCenter, true);
        UIK::Placer(titre->rectTransform(), 0.5f, 1.0f, 0, -16, largeur - 40, 34, 0.5f, 1.0f);
        auto texte = UIK::Texte(bandeau, "Texte", q->Texte, 19, UIK::Creme, TextAnchor::UpperCenter, false);
        UIK::Placer(texte->rectTransform(), 0.5f, 1.0f, 0, -52, largeur - 60, 30, 0.5f, 1.0f);
        auto aide = UIK::Texte(bandeau, "Aide", "", 17, Color(0.86f, 0.8f, 0.68f), TextAnchor::LowerCenter, false);
        UIK::Placer(aide->rectTransform(), 0.5f, 0.0f, 0, 10, largeur - 60, 24, 0.5f, 0.0f);
        for (int li = 0; li < lignes.Count(); li++)
        {
            float s = 0; for (int i : lignes[li]) s += tailles[i] + 12;
            float x = -s / 2.0f + 6;
            for (int i : lignes[li])
            {
                int k = i;
                bool principal = q->Codes[i] == "de" || q->Codes[i] == "combat" || q->Codes[i] == "defi" || q->Codes[i] == "voie" || (q->Codes[i] == "case" && q->Genre == GenreQuestion::Rodeur);
                Style st = q->Codes[i] == "de" ? Style::BoutonOr : (q->Codes[i] == "annuler" || q->Codes[i] == "non") ? Style::Bouton : Style::Bouton;
                if (q->Genre == GenreQuestion::ChoixCible && q->Cibles[i] >= 0) st = Style::Parchemin;
                auto b = UIK::Bouton(bandeau, "Option" + Texte::De(i), q->Options[i], st, [this, k] { Choisir(k); }, 21);
                UIK::Placer(Vers<RectTransform>(b->transform()), 0.5f, 1.0f, x + tailles[i] / 2.0f, static_cast<float>(-96 - li * 66), tailles[i], 54, 0.5f, 1.0f);
                b->interactable = q->Actives[i];
                if (q->Genre == GenreQuestion::ChoixCible && q->Cibles[i] >= 0)
                {
                    auto c = Pal::Joueurs[p->Joueurs[q->Cibles[i]]->Couleur];
                    auto pastille = UIK::Image(b->transform(), "Pastille", UIK::Plein(), c);
                    pastille->type = Image::Type::Sliced;
                    UIK::Placer(pastille->rectTransform(), 0, 0.5f, 6, 0, 8, 40, 0, 0.5f);
                }
                if (q->Objets[i] != nullptr)
                {
                    auto ic = UIK::IconeImage(b->transform(), UIK::IconeObjet(q->Objets[i]), 30);
                    UIK::Placer(ic->rectTransform(), 0, 0.5f, 10, 0, 30, 30, 0, 0.5f);
                    UIK::Remplir(UIK::Libelle(b)->rectTransform(), 44, 2, 8, 2);
                }
                if (principal && q->Codes[i] == "de")
                {
                    auto ic = UIK::IconeImage(b->transform(), "de", 30);
                    UIK::Placer(ic->rectTransform(), 0, 0.5f, 12, 0, 30, 30, 0, 0.5f);
                    UIK::Remplir(UIK::Libelle(b)->rectTransform(), 46, 2, 8, 2);   // le texte à droite de l'icône
                }
                string a = q->Aides[i];
                auto sv = b->gameObject()->AddComponent<Survol>();
                sv->Entree = [this, aide, a, k, q] { aide->text(a); if (SurvolCase && k < q->Cases.Count()) SurvolCase(q->Cases[k]); };
                sv->Sortie = [this, aide] { aide->text(""); if (SurvolCase) SurvolCase(-1); };
                x += tailles[i] + 12;
            }
        }
        if (q->Cases.Exists([](int c) { return c >= 0; }) && q->Genre != GenreQuestion::Quetes)
            aide->text("Vous pouvez aussi cliquer sur une case lumineuse du plateau.");
    }
}
