// Donjon de l'Oie — grandes fenêtres : boutique (fiches d'objets), panneau des quêtes, rencontre d'un monstre ou du Dragonnoie.
#include "FenetreQuestion.h"

#include "Survol.h"
#include "UIK.h"
#include "../Regles/IA.h"
#include "../Voxel/Pal.h"

namespace DonjonDeLOie::Interface
{
    using namespace DonjonDeLOie::Regles;
    using namespace DonjonDeLOie::Voxel;

    Ref<RectTransform> FenetreQuestion::Fenetre(const string& titre, float w, float h, const Ptr<Joueur>& j, bool enBas)
    {
        auto voile = UIK::Image(racine, "Voile", UIK::Blanc(), Color(0, 0, 0, 0.35f));
        UIK::Remplir(voile->rectTransform());
        voile->raycastTarget = true;
        fenetre = voile->rectTransform();
        if (enBas) voile->color = Color(0, 0, 0, 0.12f);
        auto f = enBas ? UIK::Placer(UIK::Panneau(fenetre, "Fenêtre", Style::Parchemin), 0.5f, 0.0f, 0, 26, w, h, 0.5f, 0.0f)
                       : UIK::Placer(UIK::Panneau(fenetre, "Fenêtre", Style::Parchemin), 0.5f, 0.5f, 0, 0, w, h);
        // (« bandeau » en C# : renommé, il masquerait le champ du même nom)
        auto bandeauTitre = UIK::Placer(UIK::Panneau(f, "Titre", Style::Bois), 0.5f, 1.0f, 0, 26, Mathf::Min(w - 120, 620.0f), 62, 0.5f, 1.0f);
        auto t = UIK::Texte(bandeauTitre, "Texte", titre, 30, UIK::Or, TextAnchor::MiddleCenter, true);
        UIK::Remplir(t->rectTransform(), 10, 0, 10, 0);
        auto bande = UIK::Image(bandeauTitre, "Couleur", UIK::Plein(), Pal::Joueurs[j->Couleur]);
        bande->type = Image::Type::Sliced;
        UIK::Placer(bande->rectTransform(), 0.5f, 0.0f, 0, 6, Mathf::Min(w - 160, 560.0f), 5, 0.5f, 0.0f);
        return f;
    }

    // ------------------------------------------------------------------ boutique

    void FenetreQuestion::Boutique(const Ptr<SQuestion>& q, const Ptr<Partie>& p, const Ptr<Joueur>& j)
    {
        int n = 0;
        for (int i = 0; i < q->Options.Count(); i++) if (q->Objets[i] != nullptr) n++;
        int cols = n > 12 ? 4 : 3;
        int rangs = Mathf::CeilToInt(static_cast<float>(n) / static_cast<float>(cols));
        float cw = cols == 4 ? 286.0f : 330.0f, ch = 104;
        float w = static_cast<float>(cols) * (cw + 10) + 50, h = 196 + static_cast<float>(rangs) * (ch + 10);
        auto f = Fenetre(q->Titre, w, h, j);
        // en-tête : or et équipement
        auto entete = UIK::Texte(f, "Équipement", Equipement(j), 18, UIK::Encre, TextAnchor::UpperLeft, false, false);
        UIK::Placer(entete->rectTransform(), 0, 1, 30, -52, w - 330, 50, 0, 1);
        auto bourse = UIK::Texte(f, "Bourse", "", 30, Color(0.55f, 0.36f, 0.06f), TextAnchor::MiddleRight, true, false);
        UIK::Placer(bourse->rectTransform(), 1, 1, -76, -50, 220, 40, 1, 1);
        bourse->text(Texte::De(j->Or) + " or");
        auto piece = UIK::IconeImage(f, "piece", 34);
        UIK::Placer(piece->rectTransform(), 1, 1, -30, -52, 34, 34, 1, 1);
        int k = 0;
        for (int i = 0; i < q->Options.Count(); i++)
        {
            auto o = q->Objets[i];
            if (o == nullptr) continue;
            int col = k % cols, rang = k / cols;
            k++;
            auto carte = UIK::Placer(UIK::Panneau(f, "Objet_" + o->Id, Style::Carte), 0, 1, 25 + static_cast<float>(col) * (cw + 10), -112 - static_cast<float>(rang) * (ch + 10), cw, ch, 0, 1);
            auto ic = UIK::IconeImage(carte, UIK::IconeObjet(o), 44);
            UIK::Placer(ic->rectTransform(), 0, 1, 12, -12, 44, 44, 0, 1);
            auto nom = UIK::Texte(carte, "Nom", o->Nom, 19, UIK::Encre, TextAnchor::UpperLeft, true, false);
            UIK::Placer(nom->rectTransform(), 0, 1, 64, -8, cw - 74, 24, 0, 1);
            nom->horizontalOverflow = HorizontalWrapMode::Overflow;
            auto desc = UIK::Texte(carte, "Desc", q->Actives[i] ? o->Description : UIK::Colorer(q->Aides[i], Color(0.62f, 0.15f, 0.1f)), 14, Color(0.3f, 0.22f, 0.15f), TextAnchor::UpperLeft, false, false);
            UIK::Placer(desc->rectTransform(), 0, 1, 64, -32, cw - 172, 60, 0, 1);
            int choix = i;
            auto b = UIK::Bouton(carte, "Acheter", Texte::De(q->Prix[i]) + " or", q->Actives[i] ? Style::BoutonOr : Style::Bouton, [this, choix] { Choisir(choix); }, 19);
            UIK::Placer(Vers<RectTransform>(b->transform()), 1, 0, -10, 10, 88, 40, 1, 0);
            b->interactable = q->Actives[i];
            if (o->Prix != q->Prix[i] && q->Actives[i])
            {
                auto barre = UIK::Texte(carte, "Ancien prix", "<size=13>" + Texte::De(o->Prix) + "</size>", 13, Color(0.45f, 0.35f, 0.25f), TextAnchor::LowerRight, false, false);
                UIK::Placer(barre->rectTransform(), 1, 0, -102, 12, 60, 20, 1, 0);
            }
        }
        int partir = q->Codes.IndexOf("partir");
        auto bp = UIK::Bouton(f, "Partir", "Partir", Style::Bouton, [this, partir] { Choisir(partir); }, 24);
        UIK::Placer(Vers<RectTransform>(bp->transform()), 0.5f, 0, 0, 22, 260, 56, 0.5f, 0);
    }

    string FenetreQuestion::Equipement(const Ptr<Joueur>& j)
    {
        string arme = j->Arme != nullptr ? j->Arme->Nom + " (+" + Texte::De(j->Arme->Bonus) + ")" : string("aucune");
        string armure = j->Armure != nullptr ? j->Armure->Nom + " (−" + Texte::De(j->Armure->Bonus) + ")" : string("aucune");
        Liste<string> acc; for (auto& a : j->Accessoires) acc.Add(a->Nom);
        Liste<string> sac; for (auto& o : j->Sac) sac.Add(o->Nom);
        return "<b>Arme</b> " + arme + "   <b>Armure</b> " + armure + "   <b>Accessoires</b> " + (acc.Count() > 0 ? Texte::Joindre(", ", acc) : string("aucun"))
            + "\n<b>Sac</b> (" + Texte::De(j->Sac.Count()) + "/" + Texte::De(Donnees::TailleSac()) + ") " + (sac.Count() > 0 ? Texte::Joindre(", ", sac) : string("vide"));
    }

    // ------------------------------------------------------------------ quêtes

    void FenetreQuestion::Quetes(const Ptr<SQuestion>& q, const Ptr<Partie>& p, const Ptr<Joueur>& j)
    {
        int n = 0;
        for (int i = 0; i < q->Options.Count(); i++) if (q->Quetes[i] != nullptr) n++;
        float cw = 340, chh = 300;
        float w = Mathf::Max(760.0f, static_cast<float>(n) * (cw + 16) + 60), h = 520;
        auto f = Fenetre(q->Titre, w, h, j);
        auto t = UIK::Texte(f, "Texte", q->Texte, 19, UIK::Encre, TextAnchor::UpperCenter, false, false);
        UIK::Placer(t->rectTransform(), 0.5f, 1, 0, -54, w - 80, 40, 0.5f, 1);
        int k = 0;
        float x0 = -(static_cast<float>(n) * (cw + 16) - 16) / 2.0f;
        for (int i = 0; i < q->Options.Count(); i++)
        {
            auto qu = q->Quetes[i];
            if (qu == nullptr) continue;
            auto carte = UIK::Placer(UIK::Panneau(f, "Quête" + Texte::De(i), Style::Carte), 0.5f, 1, x0 + static_cast<float>(k) * (cw + 16) + cw / 2.0f, -104, cw, chh, 0.5f, 1);
            k++;
            auto ic = UIK::IconeImage(carte, UIK::IconeQuete(qu), 52);
            UIK::Placer(ic->rectTransform(), 0.5f, 1, 0, -14, 52, 52, 0.5f, 1);
            auto titre = UIK::Texte(carte, "Titre", qu->Titre, 21, UIK::Encre, TextAnchor::UpperCenter, true, false);
            UIK::Placer(titre->rectTransform(), 0.5f, 1, 0, -72, cw - 30, 52, 0.5f, 1);
            auto texte = UIK::Texte(carte, "Texte", qu->Texte, 16, Color(0.3f, 0.22f, 0.15f), TextAnchor::UpperCenter, false, false);
            UIK::Placer(texte->rectTransform(), 0.5f, 1, 0, -128, cw - 36, 80, 0.5f, 1);
            auto rec = UIK::Texte(carte, "Récompense", "<b>" + Texte::De(qu->RecompenseXP) + " XP</b> · <b>" + Texte::De(qu->RecompenseOr) + " or</b>", 19, Color(0.5f, 0.32f, 0.05f), TextAnchor::MiddleCenter, false, false);
            UIK::Placer(rec->rectTransform(), 0.5f, 0, 0, 72, cw - 30, 28, 0.5f, 0);
            int choix = i;
            auto b = UIK::Bouton(carte, "Accepter", "Accepter", Style::BoutonOr, [this, choix] { Choisir(choix); }, 21);
            UIK::Placer(Vers<RectTransform>(b->transform()), 0.5f, 0, 0, 16, 200, 48, 0.5f, 0);
            int caseCible = qu->CaseCible;
            auto sv = carte->gameObject()->AddComponent<Survol>();
            sv->Entree = [this, caseCible] { if (SurvolCase) SurvolCase(caseCible); };
            sv->Sortie = [this] { if (SurvolCase) SurvolCase(-1); };
        }
        int non = q->Codes.IndexOf("non");
        auto bn = UIK::Bouton(f, "Aucune", q->Options[non], Style::Bouton, [this, non] { Choisir(non); }, 22);
        UIK::Placer(Vers<RectTransform>(bn->transform()), 0.5f, 0, 0, 22, 280, 54, 0.5f, 0);
    }

    // ------------------------------------------------------------------ monstre

    void FenetreQuestion::Monstre(const Ptr<SQuestion>& q, const Ptr<Partie>& p, const Ptr<Joueur>& j)
    {
        auto m = q->Monstre;
        auto f = Fenetre(q->Titre, 780, 400, j, true);
        auto ic = UIK::IconeImage(f, m->Boss ? "dragon" : "crane", 64);
        UIK::Placer(ic->rectTransform(), 0, 1, 40, -60, 64, 64, 0, 1);
        auto desc = UIK::Texte(f, "Description", "<i>" + m->Description + "</i>", 20, UIK::Encre, TextAnchor::UpperLeft, false, false);
        UIK::Placer(desc->rectTransform(), 0, 1, 124, -62, 610, 60, 0, 1);
        int bonus = (j->Classe == ClasseHeros::Guerrier || j->Classe == ClasseHeros::Mage ? 1 : 0) + j->Benediction;
        double chance = IA::ChanceMonstre(j->Attaque() + bonus, m->Force);
        int degats = std::max(1, m->Degats - j->Defense());
        auto stats = UIK::Texte(f, "Stats",
            "<b>Force</b> " + Texte::De(m->Force) + "      <b>Dégâts</b> " + Texte::De(m->Degats) + " (vous en subiriez " + Texte::De(degats) + ")      <b>Butin</b> " + Texte::De(m->Or) + " or · " + Texte::De(m->XP) + " XP\n"
            + "<b>Votre attaque</b> " + Texte::De(j->Attaque()) + (bonus > 0 ? " +" + Texte::De(bonus) : string()) + " + 1D6   contre   <b>sa force</b> " + Texte::De(m->Force) + " + 1D6\n"
            + "Vos chances : <b>" + Texte::De(Mathf::RoundToInt(static_cast<float>(chance) * 100)) + " %</b>" + (j->PV <= degats ? UIK::Colorer("   — une défaite vous mettrait K.O. !", Color(0.7f, 0.1f, 0.05f)) : string()),
            20, UIK::Encre, TextAnchor::UpperLeft, false, false);
        UIK::Placer(stats->rectTransform(), 0, 1, 40, -150, 700, 110, 0, 1);
        float x = q->Options.Count() > 1 ? -150.0f : 0.0f;
        for (int i = 0; i < q->Options.Count(); i++)
        {
            int k = i;
            auto b = UIK::Bouton(f, q->Codes[i], q->Options[i], q->Codes[i] == "combat" ? Style::BoutonRouge : Style::Bouton, [this, k] { Choisir(k); }, 26);
            UIK::Placer(Vers<RectTransform>(b->transform()), 0.5f, 0, x + static_cast<float>(i * 300), 30, 260, 62, 0.5f, 0);
            auto aide = UIK::Texte(f, "Aide" + Texte::De(i), q->Aides[i], 15, Color(0.35f, 0.25f, 0.15f), TextAnchor::UpperCenter, false, false);
            UIK::Placer(aide->rectTransform(), 0.5f, 0, x + static_cast<float>(i * 300), 98, 290, 40, 0.5f, 0);
        }
    }

    /// En passant sur la case du Dragonnoie : l'attaquer (victoire = la partie) ou continuer sa route.
    void FenetreQuestion::Dragonnoie(const Ptr<SQuestion>& q, const Ptr<Partie>& p, const Ptr<Joueur>& j)
    {
        auto m = q->Monstre;
        auto f = Fenetre(q->Titre, 820, 420, j, true);
        auto ic = UIK::IconeImage(f, "oie", 72);
        UIK::Placer(ic->rectTransform(), 0, 1, 36, -58, 72, 72, 0, 1);
        auto desc = UIK::Texte(f, "Description", "<i>" + m->Description + "</i>\n" + q->Texte, 20, UIK::Encre, TextAnchor::UpperLeft, false, false);
        UIK::Placer(desc->rectTransform(), 0, 1, 126, -58, 660, 84, 0, 1);
        int bonus = (j->Classe == ClasseHeros::Guerrier || j->Classe == ClasseHeros::Mage ? 1 : 0) + j->Benediction;
        double chance = IA::ChanceMonstre(j->Attaque() + bonus, m->Force);
        auto stats = UIK::Texte(f, "Stats",
            "<b>Votre attaque</b> " + Texte::De(j->Attaque()) + (bonus > 0 ? " +" + Texte::De(bonus) : string()) + " + 1D6   contre   <b>sa force</b> " + Texte::De(m->Force) + " + 1D6\n"
            + "Vos chances : <b>" + Texte::De(Mathf::RoundToInt(static_cast<float>(chance) * 100)) + " %</b>      "
            + UIK::Colorer("Victoire : la partie !", Color(0.15f, 0.45f, 0.1f)) + "      "
            + UIK::Colorer("Défaite : tout votre or (" + Texte::De(j->Or) + ")", Color(0.7f, 0.1f, 0.05f)),
            20, UIK::Encre, TextAnchor::UpperLeft, false, false);
        UIK::Placer(stats->rectTransform(), 0, 1, 36, -160, 760, 70, 0, 1);
        float x = q->Options.Count() > 1 ? -160.0f : 0.0f;
        for (int i = 0; i < q->Options.Count(); i++)
        {
            int k = i;
            auto b = UIK::Bouton(f, q->Codes[i], q->Options[i], q->Codes[i] == "combat" ? Style::BoutonRouge : Style::Bouton, [this, k] { Choisir(k); }, 24);
            UIK::Placer(Vers<RectTransform>(b->transform()), 0.5f, 0, x + static_cast<float>(i * 320), 30, 290, 62, 0.5f, 0);
            auto aide = UIK::Texte(f, "Aide" + Texte::De(i), q->Aides[i], 15, Color(0.35f, 0.25f, 0.15f), TextAnchor::UpperCenter, false, false);
            UIK::Placer(aide->rectTransform(), 0.5f, 0, x + static_cast<float>(i * 320), 98, 310, 40, 0.5f, 0);
        }
    }
}
