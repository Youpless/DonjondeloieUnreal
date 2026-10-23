// Donjon de l'Oie — le salon : six places (humain, bot ou fermée), un nom, une classe et une couleur pour chacune,
// le niveau à atteindre, la vitesse des bots et la carte jouée.
#include "EcranSalon.h"

#include "UIK.h"
#include "../Voxel/Pal.h"

namespace DonjonDeLOie::Interface
{
    using namespace DonjonDeLOie::Regles;
    using namespace DonjonDeLOie::Voxel;

    const Liste<string>& EcranSalon::NomsBots()
    {
        static const Liste<string> noms = {"Aldric", "Brunehilde", "Cassian", "Doria", "Elric", "Fendrel", "Gwenaëlle", "Hrolf", "Isolde", "Jorund", "Kaelith", "Lysandre", "Morgane", "Nyx", "Osric", "Perrine", "Quillon", "Ragna"};
        return noms;
    }

    EcranSalon::EcranSalon(const Ref<RectTransform>& racine, std::function<void()> retour, std::function<void(const Liste<Ptr<ReglageJoueur>>&)> lancer_)
        : lancer(lancer_)
    {
        AleaNet r(Environnement::TickCount());
        Liste<string> noms = NomsBots();
        Liste<int> classes = {0, 1, 2, 3, 4, 5};
        for (int i = classes.Count() - 1; i > 0; i--) { int k = r.Next(i + 1); int tmp = classes[i]; classes[i] = classes[k]; classes[k] = tmp; }
        for (int i = 0; i < 6; i++)
        {
            int k = r.Next(noms.Count());
            auto pl = Nouveau<Place>();
            pl->Type = i == 0 ? TypePlace::Humain : i < 4 ? TypePlace::Bot : TypePlace::Fermee;
            pl->Nom = i == 0 ? string("Joueur 1") : noms[k];
            pl->Classe = static_cast<ClasseHeros>(classes[i]);
            pl->Couleur = i;
            Places.Add(pl);
            noms.RemoveAt(k);
        }
        R = UIK::Remplir(UIK::Rect(racine, "Salon"));
        auto voile = UIK::Image(R, "Voile", UIK::Blanc(), Color(0, 0, 0, 0.35f));
        UIK::Remplir(voile->rectTransform());
        auto p = UIK::Placer(UIK::Panneau(R, "Panneau", Style::Parchemin), 0.5f, 0.5f, 0, 0, 1320, 860);
        auto bandeau = UIK::Placer(UIK::Panneau(p, "Titre", Style::Bois), 0.5f, 1.0f, 0, 28, 640, 66, 0.5f, 1.0f);
        auto t = UIK::Texte(bandeau, "Texte", "Préparer l'aventure", 34, UIK::Or, TextAnchor::MiddleCenter, true);
        UIK::Remplir(t->rectTransform());
        auto aide = UIK::Texte(p, "Aide", "Chaque place peut être tenue par un humain (on se passe la souris à son tour) ou par un bot. Deux aventuriers au moins.", 18, Color(0.35f, 0.25f, 0.15f), TextAnchor::MiddleCenter, false, false);
        UIK::Placer(aide->rectTransform(), 0.5f, 1.0f, 0, -56, 1200, 28, 0.5f, 1.0f);
        for (int i = 0; i < 6; i++) Ligne(p, i);

        // Options
        float y = -660;
        niveauTexte = UIK::Texte(p, "Niveau", "", 22, UIK::Encre, TextAnchor::MiddleCenter, true, false);
        UIK::Placer(niveauTexte->rectTransform(), 0, 1, 260, y, 320, 44, 0.5f, 1.0f);
        auto nm = UIK::Bouton(p, "Niveau moins", "◄", Style::Parchemin, [this] { NiveauCible = Mathf::Max(3, NiveauCible - 1); Maj(); }, 22);
        UIK::Placer(Vers<RectTransform>(nm->transform()), 0, 1, 80, y, 50, 44, 0.5f, 1.0f);
        auto np = UIK::Bouton(p, "Niveau plus", "►", Style::Parchemin, [this] { NiveauCible = Mathf::Min(10, NiveauCible + 1); Maj(); }, 22);
        UIK::Placer(Vers<RectTransform>(np->transform()), 0, 1, 440, y, 50, 44, 0.5f, 1.0f);
        auto bv = UIK::Bouton(p, "Vitesse", "", Style::Parchemin, [this] { BotsRapides = !BotsRapides; Maj(); }, 20);
        UIK::Placer(Vers<RectTransform>(bv->transform()), 0, 1, 664, y, 320, 44, 0.5f, 1.0f);
        vitesseTexte = UIK::Libelle(bv);
        bCarte = UIK::Bouton(p, "Carte", "", Style::Parchemin, [this] { ChoisirCarte((Carte + 1) % nomsCartes.Count()); }, 19);
        UIK::Placer(Vers<RectTransform>(bCarte->transform()), 0, 1, 1062, y, 460, 44, 0.5f, 1.0f);
        carteTexte = UIK::Libelle(bCarte);
        erreur = UIK::Texte(p, "Erreur", "", 19, Color(0.65f, 0.12f, 0.08f), TextAnchor::MiddleCenter, false, false);
        UIK::Placer(erreur->rectTransform(), 0.5f, 0, 0, 104, 900, 28, 0.5f, 0);
        auto br = UIK::Bouton(p, "Retour", "Retour", Style::Bouton, [retour] { retour(); }, 24);
        UIK::Placer(Vers<RectTransform>(br->transform()), 0.5f, 0, -230, 30, 280, 62, 0.5f, 0);
        bLancer = UIK::Bouton(p, "Lancer", "Commencer la partie", Style::BoutonOr, [this] { Lancer(); }, 26);
        UIK::Placer(Vers<RectTransform>(bLancer->transform()), 0.5f, 0, 170, 30, 420, 66, 0.5f, 0);
        Maj();
    }

    void EcranSalon::Ligne(const Ref<RectTransform>& p, int i)
    {
        auto pl = Places[i];
        float y = static_cast<float>(-96 - i * 90);
        auto ligne = UIK::Placer(UIK::Panneau(p, "Place" + Texte::De(i), Style::Carte), 0.5f, 1.0f, 0, y, 1240, 80, 0.5f, 1.0f);
        auto pastille = UIK::Bouton(ligne, "Couleur", "", Style::Parchemin, nullptr, 18);
        UIK::Placer(Vers<RectTransform>(pastille->transform()), 0, 0.5f, 14, 0, 52, 52, 0, 0.5f);
        auto teinte = pastille->GetComponent<Image>();
        pastille->onClick().AddListener([this, pl, i] { pl->Couleur = CouleurLibre(pl->Couleur, i); Maj(); });
        auto type = UIK::Bouton(ligne, "Type", "", Style::Bouton, [this, pl] { pl->Type = static_cast<TypePlace>((static_cast<int>(pl->Type) + 1) % 3); Maj(); }, 21);
        UIK::Placer(Vers<RectTransform>(type->transform()), 0, 0.5f, 78, 0, 150, 52, 0, 0.5f);
        // champ du nom
        auto champFond = UIK::Image(ligne, "Nom", UIK::Plein(), Color(1, 1, 1, 0.85f));
        champFond->type = Image::Type::Sliced;
        champFond->raycastTarget = true;
        UIK::Placer(champFond->rectTransform(), 0, 0.5f, 244, 0, 250, 50, 0, 0.5f);
        auto champTexte = UIK::Texte(champFond->transform(), "Texte", "", 22, UIK::Encre, TextAnchor::MiddleLeft, true, false);
        UIK::Remplir(champTexte->rectTransform(), 12, 2, 10, 2);
        champTexte->supportRichText = false;
        auto champ = champFond->gameObject()->AddComponent<InputField>();
        champ->textComponent = champTexte;
        champ->characterLimit = 14;
        champ->text(pl->Nom);
        champ->onValueChanged().AddListener([pl](string s) { pl->Nom = s; });
        // classe
        auto moins = UIK::Bouton(ligne, "Classe moins", "◄", Style::Parchemin, [this, pl] { pl->Classe = static_cast<ClasseHeros>((static_cast<int>(pl->Classe) + 5) % 6); Maj(); }, 20);
        UIK::Placer(Vers<RectTransform>(moins->transform()), 0, 0.5f, 512, 0, 44, 50, 0, 0.5f);
        auto classe = UIK::Texte(ligne, "Classe", "", 24, UIK::Encre, TextAnchor::MiddleCenter, true, false);
        UIK::Placer(classe->rectTransform(), 0, 0.5f, 560, 0, 150, 50, 0, 0.5f);
        auto plus = UIK::Bouton(ligne, "Classe plus", "►", Style::Parchemin, [this, pl] { pl->Classe = static_cast<ClasseHeros>((static_cast<int>(pl->Classe) + 1) % 6); Maj(); }, 20);
        UIK::Placer(Vers<RectTransform>(plus->transform()), 0, 0.5f, 714, 0, 44, 50, 0, 0.5f);
        auto desc = UIK::Texte(ligne, "Description", "", 16, Color(0.3f, 0.22f, 0.15f), TextAnchor::MiddleLeft, false, false);
        UIK::Placer(desc->rectTransform(), 0, 0.5f, 774, 0, 450, 70, 0, 0.5f);
        auto cg = ligne->gameObject()->AddComponent<CanvasGroup>();
        majLignes.Add([teinte, pl, type, cg, champ, moins, plus, pastille, classe, desc]
        {
            teinte->color = Pal::Joueurs[pl->Couleur];
            UIK::Libelle(type)->text(pl->Type == TypePlace::Humain ? "Humain" : pl->Type == TypePlace::Bot ? "Bot" : "Fermée");
            bool ouvert = pl->Type != TypePlace::Fermee;
            cg->alpha = ouvert ? 1.0f : 0.45f;
            champ->interactable = ouvert; moins->interactable = ouvert; plus->interactable = ouvert; pastille->interactable = ouvert;
            auto d = Donnees::Classe(pl->Classe);
            classe->text(d->Nom);
            desc->text("<b>Att " + Texte::De(d->Attaque) + " · Déf " + Texte::De(d->Defense) + " · PV " + Texte::De(d->Vie) + "</b>  " + d->Passif);
        });
    }

    int EcranSalon::CouleurLibre(int actuelle, int place)
    {
        for (int k = 1; k <= 6; k++)
        {
            int c = (actuelle + k) % 6;
            bool prise = false;
            for (int i = 0; i < Places.Count(); i++) if (i != place && Places[i]->Type != TypePlace::Fermee && Places[i]->Couleur == c) prise = true;
            if (!prise) return c;
        }
        return actuelle;
    }

    /// La liste des cartes jouables (noms et niveaux conseillés) et celle choisie au départ.
    void EcranSalon::DefinirCartes(const Liste<string>& noms, int choisie, const Liste<int>& niveaux)
    {
        if (noms.Count() == 0) return;
        nomsCartes = noms; niveauxCartes = niveaux;
        ChoisirCarte(Mathf::Clamp(choisie, 0, noms.Count() - 1));
    }

    void EcranSalon::ChoisirCarte(int i)
    {
        Carte = i;
        if (i < niveauxCartes.Count()) NiveauCible = Mathf::Clamp(niveauxCartes[i], 3, 10);
        Maj();
    }

    void EcranSalon::Maj()
    {
        for (auto& m : majLignes) m();
        carteTexte->text("Carte : " + nomsCartes[Carte] + (nomsCartes.Count() > 1 ? "  ►" : ""));
        bCarte->interactable = nomsCartes.Count() > 1;
        niveauTexte->text("Niveau à atteindre : " + Texte::De(NiveauCible));
        vitesseTexte->text("Vitesse des bots : " + string(BotsRapides ? "rapide" : "normale"));
        int n = 0; Ensemble<int> couleurs; bool doublon = false;
        for (auto& pl : Places) if (pl->Type != TypePlace::Fermee) { n++; if (!couleurs.Add(pl->Couleur)) doublon = true; }
        erreur->text(n < 2 ? "Ouvrez au moins deux places." : doublon ? "Deux aventuriers ont la même couleur : cliquez sur une pastille pour en changer." : "");
        bLancer->interactable = n >= 2 && !doublon;
    }

    void EcranSalon::Lancer()
    {
        Liste<Ptr<ReglageJoueur>> l;
        int k = 1;
        for (auto& pl : Places)
        {
            if (pl->Type == TypePlace::Fermee) continue;
            string nom = Texte::Vide(pl->Nom) || Texte::Longueur(Texte::Rogner(pl->Nom)) == 0 ? "Aventurier " + Texte::De(k) : Texte::Rogner(pl->Nom);
            l.Add(Nouveau<ReglageJoueur>(nom, pl->Classe, pl->Type == TypePlace::Humain, pl->Couleur));
            k++;
        }
        if (l.Count() >= 2) lancer(l);
    }

    void EcranSalon::Visible(bool v) { R->gameObject()->SetActive(v); }
}
