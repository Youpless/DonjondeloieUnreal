// Donjon de l'Oie — l'écran du mini-jeu de l'évadé : l'avis de recherche à gauche (portrait, nom, récompense), le
// sablier en haut, la consigne en bas, les messages (« Ce n'est pas lui ! », « Le voilà ! ») et la loupe d'un bot
// qui cherche à sa place.
#include "EcranEvade.h"

#include "UIK.h"

namespace DonjonDeLOie::Interface
{
    using namespace DonjonDeLOie::Regles;

    EcranEvade::EcranEvade(const Ref<RectTransform>& racine_, const Ref<Canvas>& canvas_) : racine(racine_), canvas(canvas_) {}

    void EcranEvade::Cacher() { if (p != nullptr) Object::Destroy(p->gameObject()); p = nullptr; loupe = nullptr; message = nullptr; }

    /// « icone » : l'icône du portrait ; « chercheur » : le nom du héros (pour un bot) ; humain : consigne et clics.
    void EcranEvade::Montrer(const Ptr<Quete>& qu, const string& icone, const string& chercheur, bool humain, Color couleur)
    {
        Cacher();
        p = UIK::Remplir(UIK::Rect(racine, "Mini-jeu de l'évadé"));
        auto encre = Color(0.3f, 0.2f, 0.12f);
        // l'avis de recherche
        auto avis = UIK::Placer(UIK::Panneau(p, "Avis de recherche", Style::Parchemin), 0.0f, 0.5f, 24, 0, 400, 650, 0.0f, 0.5f);
        auto rech = UIK::Texte(avis, "Recherché", qu->Feminin ? "RECHERCHÉE" : "RECHERCHÉ", 48, Color(0.64f, 0.15f, 0.11f), TextAnchor::UpperCenter, true, false);
        UIK::Placer(rech->rectTransform(), 0.5f, 1, 0, -16, 380, 58, 0.5f, 1);
        auto cadre = UIK::Image(avis, "Cadre", UIK::Plein(), Color(0.36f, 0.24f, 0.14f));
        UIK::Placer(cadre->rectTransform(), 0.5f, 1, 0, -82, 306, 306, 0.5f, 1);
        auto fond = UIK::Image(cadre->transform(), "Fond", UIK::Plein(), Color(0.86f, 0.8f, 0.66f));
        UIK::Remplir(fond->rectTransform(), 8, 8, 8, 8);
        auto portrait = UIK::IconeImage(cadre->transform(), icone, 286);
        UIK::Placer(portrait->rectTransform(), 0.5f, 0.5f, 0, 0, 286, 286);
        auto nom = UIK::Texte(avis, "Nom", qu->NomEvade, 31, UIK::Encre, TextAnchor::UpperCenter, true, false);
        UIK::Placer(nom->rectTransform(), 0.5f, 1, 0, -400, 380, 42, 0.5f, 1);
        auto motif = UIK::Texte(avis, "Motif", string(qu->Feminin ? "Évadée" : "Évadé") + " des geôles de " + (Texte::Vide(qu->Ville) ? string("Valcreux") : qu->Ville), 19, encre, TextAnchor::UpperCenter, false, false);
        UIK::Placer(motif->rectTransform(), 0.5f, 1, 0, -444, 380, 26, 0.5f, 1);
        auto rec = UIK::Texte(avis, "Récompense", "Récompense : <b>" + Texte::De(qu->RecompenseOr) + " or</b> et <b>" + Texte::De(qu->RecompenseXP) + " XP</b>", 23,
            Color(0.5f, 0.32f, 0.05f), TextAnchor::UpperCenter, false, false);
        UIK::Placer(rec->rectTransform(), 0.5f, 1, 0, -482, 380, 30, 0.5f, 1);
        auto note = UIK::Texte(avis, "Indice", "Seule sa tête dépasse : comparez bien les visages.\nGare aux sosies !", 18, encre, TextAnchor::UpperCenter, false, false);
        UIK::Placer(note->rectTransform(), 0.5f, 1, 0, -538, 360, 60, 0.5f, 1);
        // le sablier, au-dessus du diorama
        float cx = LargeurAvis * 0.5f;
        auto haut = UIK::Placer(UIK::Panneau(p, "Sablier", Style::Bois), 0.5f, 1.0f, cx, -14, 660, 96, 0.5f, 1.0f);
        auto bande = UIK::Image(haut, "Couleur", UIK::Plein(), couleur);
        UIK::Placer(bande->rectTransform(), 0.0f, 0.5f, 10, 0, 12, 72, 0.0f, 0.5f);
        auto ic = UIK::IconeImage(haut, "affiche", 44);
        UIK::Placer(ic->rectTransform(), 0.0f, 0.5f, 30, 0, 44, 44, 0.0f, 0.5f);
        string consigne = humain ? "Trouvez " + qu->NomEvade + " !" : chercheur + " cherche " + qu->NomEvade + "…";
        auto titre = UIK::Texte(haut, "Titre", consigne, 25, UIK::Creme, TextAnchor::UpperLeft, true);
        UIK::Placer(titre->rectTransform(), 0.0f, 1.0f, 88, -10, 540, 34, 0.0f, 1.0f);
        Ref<RectTransform> cadreBarre;
        barre = UIK::Barre(haut, "Temps", UIK::Or, cadreBarre);
        UIK::Placer(cadreBarre, 0.0f, 0.0f, 88, 16, 450, 24, 0.0f, 0.0f);
        temps = UIK::Texte(haut, "Secondes", "", 26, UIK::Or, TextAnchor::MiddleRight, true);
        UIK::Placer(temps->rectTransform(), 1.0f, 0.0f, -16, 10, 90, 36, 1.0f, 0.0f);
        // la consigne
        if (humain)
        {
            auto bas = UIK::Placer(UIK::Panneau(p, "Consigne", Style::Verre), 0.5f, 0.0f, cx, 18, 780, 52, 0.5f, 0.0f);
            auto tb = UIK::Texte(bas, "Texte", "Cliquez sur son visage  ·  molette : zoomer  ·  une erreur coûte 3 secondes", 20, UIK::Creme, TextAnchor::MiddleCenter, false);
            UIK::Remplir(tb->rectTransform(), 12, 2, 12, 2);
        }
        // les messages
        message = UIK::Texte(p, "Message", "", 46, UIK::Or, TextAnchor::MiddleCenter, true);
        UIK::Placer(message->rectTransform(), 0.5f, 1.0f, cx, -170, 1000, 70, 0.5f, 0.5f);
        auto ol = message->gameObject()->AddComponent<Outline>();
        ol->effectColor = Color(0.12f, 0.06f, 0.02f, 0.9f); ol->effectDistance = Vector2(2, -2);
        messageGroupe = message->gameObject()->AddComponent<CanvasGroup>();
        messageGroupe->alpha = 0; messageGroupe->blocksRaycasts = false;
        messageT = 99.0f;
        // la loupe du bot
        if (!humain)
        {
            auto im = UIK::IconeImage(p, "loupe", 104);
            loupe = im->rectTransform();
            loupe->anchorMin = loupe->anchorMax = Vector2::zero;
            loupe->pivot = Vector2(4.5f / 11.0f, 6.5f / 11.0f);
            loupe->gameObject()->SetActive(false);
        }
    }

    void EcranEvade::Temps(float reste, float total)
    {
        if (p == nullptr) return;
        reste = Mathf::Max(0.0f, reste);
        barre->fillAmount(total > 0 ? Mathf::Clamp01(reste / total) : 0.0f);
        barre->color = reste <= 10.0f ? Color::Lerp(UIK::Rouge, UIK::Or, reste / 10.0f) : UIK::Or;
        temps->text(Texte::De(Mathf::CeilToInt(reste)) + " s");
        temps->color = reste <= 5.0f ? UIK::Rouge : UIK::Or;
    }

    void EcranEvade::Message(const string& s, Color c)
    {
        if (message == nullptr) return;
        message->text(s); message->color = c; messageT = 0.0f;
    }

    /// À chaque image : le message apparaît, reste, s'efface.
    void EcranEvade::Animer(float dt)
    {
        if (message == nullptr) return;
        messageT += dt;
        messageGroupe->alpha = messageT < 0.12f ? messageT / 0.12f : messageT < 1.3f ? 1.0f : Mathf::Clamp01(1.0f - (messageT - 1.3f) / 0.4f);
        message->transform()->localScale = Vector3::one * (messageT < 0.12f ? Mathf::Lerp(1.35f, 1.0f, messageT / 0.12f) : 1.0f);
    }

    /// La loupe du bot sur ce point de l'écran (en pixels) ; « montrer » faux : cachée.
    void EcranEvade::Loupe(Vector2 ecran, bool montrer)
    {
        if (loupe == nullptr) return;
        loupe->gameObject()->SetActive(montrer);
        float s = canvas != nullptr && canvas->scaleFactor() > 0 ? canvas->scaleFactor() : 1.0f;
        loupe->anchoredPosition = ecran / s;
    }
}
