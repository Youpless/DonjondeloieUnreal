// Donjon de l'Oie — l'interface : toile, boutons, champ de saisie, système d'événements du pointeur.
#include "UI.h"

#include "Dessin.h"
#include "Entrees.h"

#include <cmath>

namespace Moteur
{
    // ------------------------------------------------------------------ toile

    float Canvas::scaleFactor() const
    {
        auto sc = GetComponent<CanvasScaler>();
        if (sc == nullptr) return 1.0f;
        if (sc->uiScaleMode == CanvasScaler::ScaleMode::ScaleWithScreenSize)
        {
            float w = static_cast<float>(std::max(1, Screen::width)), h = static_cast<float>(std::max(1, Screen::height));
            float rx = std::max(1.0f, sc->referenceResolution.x), ry = std::max(1.0f, sc->referenceResolution.y);
            switch (sc->screenMatchMode)
            {
                case CanvasScaler::ScreenMatchMode::Expand: return std::min(w / rx, h / ry);
                case CanvasScaler::ScreenMatchMode::Shrink: return std::max(w / rx, h / ry);
                default:
                {
                    // comme Unity : moyenne logarithmique de la largeur et de la hauteur
                    double lw = std::log2(static_cast<double>(w / rx)), lh = std::log2(static_cast<double>(h / ry));
                    double l = lw + (lh - lw) * static_cast<double>(Mathf::Clamp01(sc->matchWidthOrHeight));
                    return static_cast<float>(std::pow(2.0, l));
                }
            }
        }
        return 1.0f;
    }

    Ref<RectTransform> Graphic::rectTransform() const { return Comme<RectTransform>(transform()); }

    ColorBlock ColorBlock::defaultColorBlock()
    {
        ColorBlock b;
        b.normalColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
        b.highlightedColor = Color(0.9607843f, 0.9607843f, 0.9607843f, 1.0f);
        b.pressedColor = Color(0.7843137f, 0.7843137f, 0.7843137f, 1.0f);
        b.selectedColor = Color(0.9607843f, 0.9607843f, 0.9607843f, 1.0f);
        b.disabledColor = Color(0.7843137f, 0.7843137f, 0.7843137f, 0.5019608f);
        b.colorMultiplier = 1.0f;
        b.fadeDuration = 0.1f;
        return b;
    }

    // ------------------------------------------------------------------ Selectable, Button

    bool Selectable::Interactif() const
    {
        if (!interactable) return false;
        for (auto t = transform(); t != nullptr; t = t->parent())
        {
            auto g = t->gameObject();
            if (g == nullptr) break;
            auto cg = g->GetComponent<CanvasGroup>();
            if (cg != nullptr && cg->enabled)
            {
                if (!cg->interactable) return false;
                if (cg->ignoreParentGroups) break;
            }
        }
        return true;
    }

    void Selectable::OnPointerEnter(PointerEventData&) { survole = true; }
    void Selectable::OnPointerExit(PointerEventData&) { survole = false; }

    void Selectable::OnPointerDown(PointerEventData& e)
    {
        if (e.bouton != 0) return;
        if (Interactif()) enfonce = true;
    }

    void Selectable::AnimerTeinte(float dt)
    {
        Color cible;
        if (!Interactif()) cible = c.disabledColor;
        else if (enfonce) cible = c.pressedColor;
        else if (survole) cible = c.highlightedColor;
        else cible = c.normalColor;
        cible = cible * c.colorMultiplier;
        if (transition != Transition::ColorTint) cible = Color::white;
        if (!teinteInitiale)
        {
            teinteInitiale = true;
            teinte = teinteDepart = teinteCible = cible;
            fondu = 1.0f;
        }
        else if (!(cible == teinteCible))
        {
            teinteDepart = teinte;
            teinteCible = cible;
            fondu = 0.0f;
        }
        if (fondu < 1.0f)
        {
            fondu = c.fadeDuration > 0.0f ? std::min(1.0f, fondu + dt / c.fadeDuration) : 1.0f;
            teinte = Color::Lerp(teinteDepart, teinteCible, fondu);
        }
        if (targetGraphic != nullptr) targetGraphic->TeinteTransition = teinte;
    }

    void Button::OnPointerClick(PointerEventData& e)
    {
        if (e.bouton != 0) return;
        if (!isActiveAndEnabled() || !Interactif()) return;
        clic.Invoke();
    }

    // ------------------------------------------------------------------ InputField

    void InputField::text(const string& v)
    {
        t = v;
        if (textComponent != nullptr) textComponent->text(t);
        change.Invoke(t);
    }

    void InputField::OnPointerClick(PointerEventData& e)
    {
        if (e.bouton != 0 || !Interactif()) return;
        if (!focus)
        {
            focus = true;
            debutClignotement = Time::unscaledTime;
        }
    }

    void InputField::PerdreFocus()
    {
        if (!focus) return;
        focus = false;
        fin.Invoke(t);
    }

    bool InputField::CurseurVisible() const
    {
        if (!focus) return false;
        float p = Time::unscaledTime - debutClignotement;
        return std::fmod(p, 1.06f) < 0.53f;
    }

    namespace
    {
        // le dernier caractère UTF-8 d'un texte (ses octets de suite compris)
        void EnleverDernier(string& s)
        {
            if (s.empty()) return;
            size_t i = s.size() - 1;
            while (i > 0 && (static_cast<unsigned char>(s[i]) & 0xC0) == 0x80) i--;
            s.erase(i);
        }
    }

    void InputField::Saisir(const string& caracteres, bool effacer, bool valider)
    {
        if (!focus) return;
        string avant = t, n = t;
        if (effacer) EnleverDernier(n);
        size_t i = 0;
        while (i < caracteres.size())
        {
            size_t j = i + 1;
            while (j < caracteres.size() && (static_cast<unsigned char>(caracteres[j]) & 0xC0) == 0x80) j++;
            string ch = caracteres.substr(i, j - i);
            i = j;
            if (ch.size() == 1 && static_cast<unsigned char>(ch[0]) < 32) continue;     // touches de contrôle
            if (characterLimit > 0 && Texte::Longueur(n) >= characterLimit) continue;
            n += ch;
        }
        if (n != avant)
        {
            text(n);
            debutClignotement = Time::unscaledTime;
        }
        if (valider) PerdreFocus();
    }

    // ------------------------------------------------------------------ système d'événements

    Ref<EventSystem> EventSystem::current() { return FindAnyObjectByType<EventSystem>(); }

    bool EventSystem::IsPointerOverGameObject() const
    {
        auto m = Mouse::current();
        if (!m->Presente) return false;
        Dessin::MettreEnPage(Screen::width, Screen::height);
        return Dessin::ElementSous(m->position.ReadValue()) != nullptr;
    }

    namespace
    {
        // le premier objet de la chaîne (lui puis ses parents) dont un composant accepte cet événement
        template <class F>
        Ref<GameObject> Gestionnaire(const Ref<GameObject>& depart, F a)
        {
            for (auto t = depart != nullptr ? depart->transform() : Ref<Transform>(); t != nullptr; t = t->parent())
            {
                auto g = t->gameObject();
                if (g == nullptr) break;
                for (auto& c : g->Composants())
                    if (!c->EstDetruit() && a(*c) != nullptr) return g;
            }
            return Ref<GameObject>();
        }

        bool ActifPourEvenements(const Ref<Component>& c)
        {
            if (c->EstDetruit()) return false;
            auto b = Comme<Behaviour>(c);
            if (b != nullptr && !b->isActiveAndEnabled()) return false;
            return true;
        }
    }

    void EventSystem::Update()
    {
        auto m = Mouse::current();
        auto k = Keyboard::current();
        float dt = Time::unscaledDeltaTime;
        if (m->Presente)
        {
            Dessin::MettreEnPage(Screen::width, Screen::height);
            PointerEventData e;
            e.position = m->position.ReadValue();
            auto g = Dessin::ElementSous(e.position);
            Ref<GameObject> sous = g != nullptr ? g->gameObject() : Ref<GameObject>();

            // survol : « sortie » pour ce qui n'est plus sous le pointeur, « entrée » pour ce qui l'est devenu
            Liste<Ref<GameObject>> chaine;
            for (auto t = sous != nullptr ? sous->transform() : Ref<Transform>(); t != nullptr; t = t->parent())
                if (auto og = t->gameObject()) chaine.Add(og);
            for (auto& ancien : chaineSurvol)
            {
                bool encore = false;
                for (auto& n : chaine)
                    if (n.get() == ancien.get()) encore = true;
                if (encore || ancien == nullptr) continue;
                auto composants = ancien->Composants();
                for (auto& c : composants)
                    if (ActifPourEvenements(c))
                        if (auto h = c->GestionnaireSortie()) h->OnPointerExit(e);
            }
            for (auto& nouveau : chaine)
            {
                bool deja = false;
                for (auto& a : chaineSurvol)
                    if (a.get() == nouveau.get()) deja = true;
                if (deja) continue;
                auto composants = nouveau->Composants();
                for (auto& c : composants)
                    if (ActifPourEvenements(c))
                        if (auto h = c->GestionnaireEntree()) h->OnPointerEnter(e);
            }
            chaineSurvol = chaine;
            survole = sous;

            // appui : le champ de saisie qui avait le focus le perd si l'on clique ailleurs
            if (m->leftButton.wasPressedThisFrame)
            {
                auto champs = FindObjectsByType<InputField>();
                for (auto& f : champs)
                    if (f->isFocused() && (sous == nullptr || !sous->transform()->IsChildOf(f->transform()))) f->PerdreFocus();
                auto gAppui = Gestionnaire(sous, [](Component& c) { return c.GestionnaireAppui(); });
                if (gAppui != nullptr)
                {
                    auto composants = gAppui->Composants();
                    for (auto& c : composants)
                        if (ActifPourEvenements(c))
                            if (auto h = c->GestionnaireAppui()) h->OnPointerDown(e);
                }
                appuye = Gestionnaire(sous, [](Component& c) { return c.GestionnaireClic(); });
            }
            if (m->leftButton.wasReleasedThisFrame)
            {
                auto gClic = Gestionnaire(sous, [](Component& c) { return c.GestionnaireClic(); });
                auto presse = appuye;
                appuye = nullptr;
                auto selectables = FindObjectsByType<Selectable>();
                for (auto& s : selectables) s->Relacher();
                if (presse != nullptr && gClic != nullptr && presse.get() == gClic.get())
                {
                    auto composants = gClic->Composants();
                    for (auto& c : composants)
                        if (ActifPourEvenements(c))
                            if (auto h = c->GestionnaireClic()) h->OnPointerClick(e);
                }
            }
        }
        // le clavier va au champ de saisie qui a le focus
        auto champs = FindObjectsByType<InputField>();
        for (auto& f : champs)
            if (f->isFocused()) f->Saisir(k->CaracteresTapes, k->backspaceKey.wasPressedThisFrame, k->enterKey.wasPressedThisFrame);
        // les teintes des boutons (survol, appui, désactivé), en temps réel même quand le jeu est en pause
        if (m->Presente)
        {
            auto selectables = FindObjectsByType<Selectable>();
            for (auto& s : selectables)
                if (s->isActiveAndEnabled()) s->AnimerTeinte(dt);
        }
    }
}
