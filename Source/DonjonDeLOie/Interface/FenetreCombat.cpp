// Donjon de l'Oie — le tableau d'un combat : les deux adversaires, leur attaque, leurs dés, les totaux et le verdict.
#include "FenetreCombat.h"

#include "UIK.h"
#include "../Voxel/Pal.h"

namespace DonjonDeLOie::Interface
{
    using namespace DonjonDeLOie::Regles;
    using namespace DonjonDeLOie::Voxel;

    FenetreCombat::FenetreCombat(const Ref<RectTransform>& racine_) : racine(racine_) {}

    void FenetreCombat::Cacher() { if (p != nullptr) Object::Destroy(p->gameObject()); p = nullptr; }

    Ref<Text> FenetreCombat::Cote(const string& nom, Color c, const string& attaque, float x, Ref<Text>& de, Ref<Text>& total, Ref<Image>& fond)
    {
        auto bloc = UIK::Placer(UIK::Rect(p, "Côté"), 0.5f, 0.5f, x, 0, 330, 180);
        auto n = UIK::Texte(bloc, "Nom", nom, 26, c, TextAnchor::UpperCenter, true);
        UIK::Placer(n->rectTransform(), 0.5f, 1, 0, -8, 330, 32, 0.5f, 1);
        auto a = UIK::Texte(bloc, "Attaque", attaque, 18, UIK::Creme, TextAnchor::UpperCenter, false);
        UIK::Placer(a->rectTransform(), 0.5f, 1, 0, -42, 330, 26, 0.5f, 1);
        fond = UIK::Image(bloc, "Dé", UIK::Plein(), Color(0.95f, 0.91f, 0.82f));
        fond->type = Image::Type::Sliced;
        UIK::Placer(fond->rectTransform(), 0.5f, 0, -60, 16, 74, 74, 0.5f, 0);
        de = UIK::Texte(fond->transform(), "Valeur", "?", 44, UIK::Encre, TextAnchor::MiddleCenter, true, false);
        UIK::Remplir(de->rectTransform());
        total = UIK::Texte(bloc, "Total", "", 46, UIK::Or, TextAnchor::MiddleCenter, true);
        UIK::Placer(total->rectTransform(), 0.5f, 0, 56, 16, 120, 74, 0.5f, 0);
        return n;
    }

    void FenetreCombat::Montrer(const Ptr<SCombat>& c, const Ptr<Partie>& partie)
    {
        Cacher();
        p = UIK::Placer(UIK::Panneau(racine, "Combat", Style::Bois), 0.5f, 1.0f, 0, -90, 860, 196, 0.5f, 1.0f);
        auto ja = partie->Joueurs[c->Attaquant];
        string atkA = "Attaque " + Texte::De(c->AtkA) + (c->BonusA > 0 ? " + " + Texte::De(c->BonusA) + (!c->RaisonBonusA.empty() ? " (" + c->RaisonBonusA + ")" : string()) : string()) + " + 1D" + Texte::De(c->FacesDe);
        Cote(ja->Nom, Pal::Joueurs[ja->Couleur], atkA, -230, deA, totA, fondA);
        string nomB; Color colB; string atkB;
        if (c->Duel)
        {
            auto jb = partie->Joueurs[c->Defenseur];
            nomB = jb->Nom; colB = Pal::Joueurs[jb->Couleur];
            atkB = "Attaque " + Texte::De(c->AtkB) + (c->BonusB > 0 ? " + " + Texte::De(c->BonusB) : string()) + " + 1D3";
        }
        else
        {
            nomB = c->Monstre->Nom; colB = Color(1.0f, 0.45f, 0.38f);
            atkB = "Force " + Texte::De(c->AtkB) + " + 1D6";
        }
        Cote(nomB, colB, atkB, 230, deB, totB, fondB);
        verdict = UIK::Texte(p, "VS", c->Duel ? (c->Arene ? "ARÈNE" : c->Embuscade ? "EMBUSCADE" : "DUEL") : "COMBAT", 30, Color(1.0f, 0.6f, 0.35f), TextAnchor::MiddleCenter, true);
        UIK::Placer(verdict->rectTransform(), 0.5f, 0.5f, 0, 0, 160, 120);
    }

    void FenetreCombat::Des(const Ptr<SCombat>& c)
    {
        if (p == nullptr) return;
        deA->text(Texte::De(c->DeA)); deB->text(Texte::De(c->DeB));
        totA->text("= " + Texte::De(c->TotalA())); totB->text("= " + Texte::De(c->TotalB()));
    }

    void FenetreCombat::Verdict(const Ptr<SCombat>& c)
    {
        if (p == nullptr) return;
        if (c->Egalite)
        {
            verdict->text("ÉGALITÉ");
            verdict->color = UIK::Creme;
            return;
        }
        bool a = c->AttaquantGagne;
        totA->color = a ? UIK::Vert : UIK::Rouge;
        totB->color = a ? UIK::Rouge : UIK::Vert;
        fondA->color = a ? Color(0.75f, 0.95f, 0.65f) : Color(0.95f, 0.7f, 0.65f);
        fondB->color = a ? Color(0.95f, 0.7f, 0.65f) : Color(0.75f, 0.95f, 0.65f);
        verdict->text(a ? "◄ VICTOIRE" : "VICTOIRE ►");
        verdict->color = UIK::Or;
        verdict->fontSize = 26;
    }
}
