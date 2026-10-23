// Donjon de l'Oie — l'écran titre (l'île tourne lentement derrière) et le menu pause.
#include "EcranTitre.h"

#include "UIK.h"
#include "../Monde/Sons.h"
#include "../Regles/Donnees.h"

namespace DonjonDeLOie::Interface
{
    using namespace DonjonDeLOie::Regles;

    EcranTitre::EcranTitre(const Ref<RectTransform>& racine, std::function<void()> nouvelle, std::function<void()> regles, std::function<void()> quitter)
    {
        R = UIK::Remplir(UIK::Rect(racine, "Écran titre"));
        auto voile = UIK::Image(R, "Voile", UIK::DegradeVertical(), Color(0.05f, 0.03f, 0.02f, 0.65f));
        UIK::Remplir(voile->rectTransform());
        voile->rectTransform()->localScale = Vector3(1, -1, 1);   // sombre en bas
        auto lueur = UIK::Image(R, "Lueur", UIK::Lueur(), Color(0, 0, 0, 0.55f));
        UIK::Placer(lueur->rectTransform(), 0.5f, 1.0f, 0, -40, 1500, 420, 0.5f, 1.0f);
        auto titre = UIK::Texte(R, "Titre", "DONJON DE L'OIE", 104, UIK::Or, TextAnchor::MiddleCenter, true);
        UIK::Placer(titre->rectTransform(), 0.5f, 1.0f, 0, -90, 1600, 130, 0.5f, 1.0f);
        auto ol = titre->gameObject()->AddComponent<Outline>();
        ol->effectColor = Color(0.25f, 0.1f, 0.02f, 1.0f); ol->effectDistance = Vector2(3, -3);
        auto sous = UIK::Texte(R, "Sous-titre", "Un jeu de plateau dans les donjons · de 1 à 6 aventuriers sur le même ordinateur", 28, UIK::Creme, TextAnchor::MiddleCenter, false);
        UIK::Placer(sous->rectTransform(), 0.5f, 1.0f, 0, -224, 1500, 40, 0.5f, 1.0f);
        but = UIK::Texte(R, "But", TexteBut(), 22, Color(0.95f, 0.85f, 0.65f), TextAnchor::MiddleCenter, false);
        UIK::Placer(but->rectTransform(), 0.5f, 1.0f, 0, -266, 1500, 34, 0.5f, 1.0f);

        auto b1 = UIK::Bouton(R, "Nouvelle partie", "Nouvelle partie", Style::BoutonOr, [nouvelle] { nouvelle(); }, 32);
        UIK::Placer(Vers<RectTransform>(b1->transform()), 0.5f, 0.0f, 0, 300, 420, 78, 0.5f, 0.0f);
        auto b2 = UIK::Bouton(R, "Règles", "Règles du jeu", Style::Bouton, [regles] { regles(); }, 26);
        UIK::Placer(Vers<RectTransform>(b2->transform()), 0.5f, 0.0f, 0, 210, 420, 64, 0.5f, 0.0f);
        auto b3 = UIK::Bouton(R, "Quitter", "Quitter", Style::Bouton, [quitter] { quitter(); }, 26);
        UIK::Placer(Vers<RectTransform>(b3->transform()), 0.5f, 0.0f, 0, 134, 420, 64, 0.5f, 0.0f);
        auto aide = UIK::Texte(R, "Commandes", "Molette : zoom  ·  clic droit glissé ou flèches : tourner  ·  clic milieu : déplacer  ·  Espace : tout le plateau  ·  Échap : menu", 19, Color(0.85f, 0.8f, 0.7f), TextAnchor::MiddleCenter, false);
        UIK::Placer(aide->rectTransform(), 0.5f, 0.0f, 0, 50, 1700, 30, 0.5f, 0.0f);
    }

    /// Le but du jeu, selon le catalogue (avec ou sans Dragonnoie).
    string EcranTitre::TexteBut()
    {
        return string(Donnees::Dragonnoie() != nullptr ? "Premier au niveau 10 ou vainqueur du Dragonnoie : la couronne est à vous."
                                                       : "Premier au niveau 10 : la couronne est à vous.")
               + " Monstres, duels, pièges et malédictions vous attendent.";
    }

    void EcranTitre::Visible(bool v)
    {
        if (v) but->text(TexteBut());
        R->gameObject()->SetActive(v);
    }

    MenuPause::MenuPause(const Ref<RectTransform>& racine, std::function<void()> reprendre, std::function<void()> regles, std::function<void()> abandonner)
    {
        R = UIK::Remplir(UIK::Rect(racine, "Pause"));
        auto voile = UIK::Image(R, "Voile", UIK::Blanc(), Color(0, 0, 0, 0.55f));
        UIK::Remplir(voile->rectTransform());
        voile->raycastTarget = true;
        auto p = UIK::Placer(UIK::Panneau(R, "Panneau", Style::Bois), 0.5f, 0.5f, 0, 0, 520, 520);
        auto t = UIK::Texte(p, "Titre", "Pause", 44, UIK::Or, TextAnchor::MiddleCenter, true);
        UIK::Placer(t->rectTransform(), 0.5f, 1.0f, 0, -24, 400, 56, 0.5f, 1.0f);
        auto b1 = UIK::Bouton(p, "Reprendre", "Reprendre", Style::BoutonOr, [reprendre] { reprendre(); }, 26);
        UIK::Placer(Vers<RectTransform>(b1->transform()), 0.5f, 1.0f, 0, -100, 380, 62, 0.5f, 1.0f);
        auto b2 = UIK::Bouton(p, "Règles", "Règles du jeu", Style::Bouton, [regles] { regles(); }, 24);
        UIK::Placer(Vers<RectTransform>(b2->transform()), 0.5f, 1.0f, 0, -176, 380, 58, 0.5f, 1.0f);
        musique = Reglage(p, -260, "Musique", [] { return Monde::Sons::VolumeMusique; }, [](float v) { Monde::Sons::ReglerVolumes(Monde::Sons::VolumeEffets, v); });
        effets = Reglage(p, -320, "Effets", [] { return Monde::Sons::VolumeEffets; }, [](float v) { Monde::Sons::ReglerVolumes(v, Monde::Sons::VolumeMusique); });
        auto b3 = UIK::Bouton(p, "Abandonner", "Retour au menu principal", Style::BoutonRouge, [abandonner] { abandonner(); }, 22);
        UIK::Placer(Vers<RectTransform>(b3->transform()), 0.5f, 0.0f, 0, 28, 380, 58, 0.5f, 0.0f);
        Visible(false);
    }

    Ref<Text> MenuPause::Reglage(const Ref<RectTransform>& p, float y, const string& nom, std::function<float()> lire, std::function<void(float)> ecrire)
    {
        auto l = UIK::Texte(p, nom, "", 22, UIK::Creme, TextAnchor::MiddleCenter, false);
        UIK::Placer(l->rectTransform(), 0.5f, 1.0f, 0, y, 240, 44, 0.5f, 1.0f);
        std::function<void()> maj = [l, nom, lire] { l->text(nom + " : " + Texte::De(Mathf::RoundToInt(lire() * 100)) + " %"); };
        maj();
        auto moins = UIK::Bouton(p, "Moins", "−", Style::Bouton, [ecrire, lire, maj] { ecrire(Mathf::Clamp01(lire() - 0.1f)); maj(); }, 28);
        UIK::Placer(Vers<RectTransform>(moins->transform()), 0.5f, 1.0f, -170, y, 56, 44, 0.5f, 1.0f);
        auto plus = UIK::Bouton(p, "Plus", "+", Style::Bouton, [ecrire, lire, maj] { ecrire(Mathf::Clamp01(lire() + 0.1f)); maj(); }, 28);
        UIK::Placer(Vers<RectTransform>(plus->transform()), 0.5f, 1.0f, 170, y, 56, 44, 0.5f, 1.0f);
        return l;
    }

    void MenuPause::Visible(bool v) { R->gameObject()->SetActive(v); R->SetAsLastSibling(); }
}
