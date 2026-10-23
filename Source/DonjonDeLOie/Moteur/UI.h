// Donjon de l'Oie — l'interface (les équivalents d'uGUI et de son système d'événements) : toiles, rectangles
// ancrés, images, textes riches, boutons, champ de saisie, survol. Le jeu construit son interface en code avec ces
// composants ; le moteur en calcule la mise en page, gère le pointeur (survol, clic, teintes des boutons) et en tire
// une liste de dessin que la couche Unreal peint (Dessin.h).
#pragma once

#include "Rendu.h"

namespace Moteur
{
    // ------------------------------------------------------------------ événements (UnityEngine.Events)

    using UnityAction = std::function<void()>;

    class UnityEvent
    {
    public:
        void AddListener(UnityAction a)
        {
            DD_VERIFIE(a != nullptr, "UnityEvent.AddListener : action nulle");
            l.Add(std::move(a));
        }
        void RemoveAllListeners() { l.Clear(); }
        void Invoke() const
        {
            auto copie = l;
            for (auto& a : copie) a();
        }
        int Nombre() const { return l.Count(); }

    private:
        Liste<UnityAction> l;
    };

    template <class T0>
    class UnityEvent1
    {
    public:
        void AddListener(std::function<void(T0)> a) { l.Add(std::move(a)); }
        void RemoveAllListeners() { l.Clear(); }
        void Invoke(T0 v) const
        {
            auto copie = l;
            for (auto& a : copie) a(v);
        }

    private:
        Liste<std::function<void(T0)>> l;
    };

    // ------------------------------------------------------------------ système d'événements (UnityEngine.EventSystems)

    class UIBehaviour : public MonoBehaviour
    {
        DD_TYPE_OBJET(UIBehaviour, MonoBehaviour)
    };

    class PointerEventData
    {
    public:
        Vector2 position;
        int bouton = 0;           // 0 gauche, 1 droit, 2 milieu
    };

    class IPointerEnterHandler
    {
    public:
        virtual ~IPointerEnterHandler() = default;
        virtual void OnPointerEnter(PointerEventData& e) = 0;
    };
    class IPointerExitHandler
    {
    public:
        virtual ~IPointerExitHandler() = default;
        virtual void OnPointerExit(PointerEventData& e) = 0;
    };
    class IPointerClickHandler
    {
    public:
        virtual ~IPointerClickHandler() = default;
        virtual void OnPointerClick(PointerEventData& e) = 0;
    };
    class IPointerDownHandler
    {
    public:
        virtual ~IPointerDownHandler() = default;
        virtual void OnPointerDown(PointerEventData& e) = 0;
    };

    /// Le système d'événements : à chaque image (dans son Update), il cherche l'élément sous le pointeur et envoie
    /// survol, sortie, appui et clic ; il fait aussi vivre le champ de saisie qui a le focus et les teintes des boutons.
    class EventSystem : public UIBehaviour
    {
        DD_TYPE_OBJET(EventSystem, UIBehaviour)
    public:
        static Ref<EventSystem> current();
        /// Le pointeur est-il au-dessus d'un élément d'interface qui arrête les clics ?
        bool IsPointerOverGameObject() const;
        Ref<GameObject> currentSelectedGameObject;
        void SetSelectedGameObject(const Ref<GameObject>& g) { currentSelectedGameObject = g; }

        void Update() override;

    private:
        Ref<GameObject> survole;          // l'élément sous le pointeur (le plus haut qui reçoit les clics)
        Liste<Ref<GameObject>> chaineSurvol;   // lui et ses parents qui ont reçu « entrée »
        Ref<GameObject> appuye;           // l'élément qui a reçu l'appui (pour le clic au relâchement)
    };

    class BaseInputModule : public UIBehaviour
    {
        DD_TYPE_OBJET(BaseInputModule, UIBehaviour)
    };
    class StandaloneInputModule : public BaseInputModule
    {
        DD_TYPE_OBJET(StandaloneInputModule, BaseInputModule)
    };
    class InputSystemUIInputModule : public BaseInputModule
    {
        DD_TYPE_OBJET(InputSystemUIInputModule, BaseInputModule)
    public:
        void AssignDefaultActions() {}
    };
    class BaseMeshEffect : public UIBehaviour
    {
        DD_TYPE_OBJET(BaseMeshEffect, UIBehaviour)
    };

    // ------------------------------------------------------------------ toile

    enum class RenderMode { ScreenSpaceOverlay, ScreenSpaceCamera, WorldSpace };

    class Canvas : public Behaviour
    {
        DD_TYPE_OBJET(Canvas, Behaviour)
    public:
        RenderMode renderMode = RenderMode::ScreenSpaceOverlay;
        int sortingOrder = 0;
        /// Le facteur d'échelle entre les pixels de référence de l'interface et ceux de l'écran (CanvasScaler).
        float scaleFactor() const;
        void scaleFactor(float) {}
        bool pixelPerfect = false, overrideSorting = false;
        Ref<Camera> worldCamera;
    };

    class CanvasGroup : public Behaviour
    {
        DD_TYPE_OBJET(CanvasGroup, Behaviour)
    public:
        float alpha = 1.0f;
        bool blocksRaycasts = true, interactable = true, ignoreParentGroups = false;
    };

    class CanvasScaler : public UIBehaviour
    {
        DD_TYPE_OBJET(CanvasScaler, UIBehaviour)
    public:
        enum class ScaleMode { ConstantPixelSize, ScaleWithScreenSize, ConstantPhysicalSize };
        enum class ScreenMatchMode { MatchWidthOrHeight, Expand, Shrink };
        ScaleMode uiScaleMode = ScaleMode::ConstantPixelSize;
        Vector2 referenceResolution{800.0f, 600.0f};
        float matchWidthOrHeight = 0.0f;
        ScreenMatchMode screenMatchMode = ScreenMatchMode::MatchWidthOrHeight;
    };

    class GraphicRaycaster : public UIBehaviour
    {
        DD_TYPE_OBJET(GraphicRaycaster, UIBehaviour)
    };

    // ------------------------------------------------------------------ éléments graphiques (UnityEngine.UI)

    class Graphic : public UIBehaviour
    {
        DD_TYPE_OBJET(Graphic, UIBehaviour)
    public:
        Color color = Color::white;
        bool raycastTarget = true;
        Ref<Material> material;
        Ref<RectTransform> rectTransform() const;
        Ref<Canvas> canvas() const { return GetComponentInParent<Canvas>(); }
        void SetAllDirty() {}
        void CrossFadeAlpha(float, float, bool) {}

        // --- usage interne : la teinte posée par un bouton (survol, appui, désactivé), multipliée à la couleur
        Color TeinteTransition = Color::white;
    };

    class MaskableGraphic : public Graphic
    {
        DD_TYPE_OBJET(MaskableGraphic, Graphic)
    public:
        bool maskable = true;
    };

    class Image : public MaskableGraphic
    {
        DD_TYPE_OBJET(Image, MaskableGraphic)
    public:
        enum class Type { Simple, Sliced, Tiled, Filled };
        enum class FillMethod { Horizontal, Vertical, Radial90, Radial180, Radial360 };
        Ref<Sprite> sprite;
        Type type = Type::Simple;
        FillMethod fillMethod = FillMethod::Horizontal;
        int fillOrigin = 0;
        float fillAmount() const { return fill; }
        void fillAmount(float v) { fill = Mathf::Clamp01(v); }
        bool preserveAspect = false, fillCenter = true;
        float pixelsPerUnitMultiplier = 1.0f;

    private:
        float fill = 1.0f;
    };

    class RawImage : public MaskableGraphic
    {
        DD_TYPE_OBJET(RawImage, MaskableGraphic)
    public:
        Ref<Texture> texture;
    };

    class Text : public MaskableGraphic
    {
        DD_TYPE_OBJET(Text, MaskableGraphic)
    public:
        Ref<Font> font;
        string text() const { return t; }
        void text(const string& v) { t = v; }
        int fontSize = 14;
        TextAnchor alignment = TextAnchor::UpperLeft;
        FontStyle fontStyle = FontStyle::Normal;
        HorizontalWrapMode horizontalOverflow = HorizontalWrapMode::Wrap;
        VerticalWrapMode verticalOverflow = VerticalWrapMode::Truncate;
        bool supportRichText = true, resizeTextForBestFit = false;
        int resizeTextMinSize = 10, resizeTextMaxSize = 40;
        float lineSpacing = 1.0f;
        /// Largeur et hauteur naturelles du texte (en pixels de référence), d'après la mesure des polices quand la
        /// couche d'affichage en fournit une, sinon une estimation.
        float preferredWidth() const;
        float preferredHeight() const;

    private:
        string t;
    };

    class Shadow : public BaseMeshEffect
    {
        DD_TYPE_OBJET(Shadow, BaseMeshEffect)
    public:
        Color effectColor{0.0f, 0.0f, 0.0f, 0.5f};
        Vector2 effectDistance{1.0f, -1.0f};
        bool useGraphicAlpha = true;
    };

    class Outline : public Shadow
    {
        DD_TYPE_OBJET(Outline, Shadow)
    };

    struct ColorBlock
    {
        Color normalColor, highlightedColor, pressedColor, selectedColor, disabledColor;
        float colorMultiplier = 1.0f, fadeDuration = 0.1f;
        static ColorBlock defaultColorBlock();
    };

    class Selectable : public UIBehaviour, public IPointerEnterHandler, public IPointerExitHandler, public IPointerDownHandler
    {
        DD_TYPE_OBJET(Selectable, UIBehaviour)
    public:
        enum class Transition { None, ColorTint, SpriteSwap, Animation };
        bool interactable = true;
        Ref<Graphic> targetGraphic;
        ColorBlock colors() const { return c; }
        void colors(const ColorBlock& v) { c = v; }
        Transition transition = Transition::ColorTint;
        void Select() {}

        void OnPointerEnter(PointerEventData& e) override;
        void OnPointerExit(PointerEventData& e) override;
        void OnPointerDown(PointerEventData& e) override;
        IPointerEnterHandler* GestionnaireEntree() override { return this; }
        IPointerExitHandler* GestionnaireSortie() override { return this; }
        IPointerDownHandler* GestionnaireAppui() override { return this; }

        // --- usage interne : l'état du pointeur et la teinte qui en découle (posée sur targetGraphic)
        void Relacher() { enfonce = false; }
        void AnimerTeinte(float dt);
        /// Interactif compte tenu des CanvasGroup parents.
        bool Interactif() const;

    protected:
        ColorBlock c = ColorBlock::defaultColorBlock();
        bool survole = false, enfonce = false;
        Color teinte = Color::white, teinteDepart = Color::white, teinteCible = Color::white;
        float fondu = 1.0f;
        bool teinteInitiale = false;
    };

    class Button : public Selectable, public IPointerClickHandler
    {
        DD_TYPE_OBJET(Button, Selectable)
    public:
        using ButtonClickedEvent = UnityEvent;
        ButtonClickedEvent& onClick() { return clic; }
        void onClick(const ButtonClickedEvent& e) { clic = e; }
        void OnPointerClick(PointerEventData& e) override;
        IPointerClickHandler* GestionnaireClic() override { return this; }

    private:
        ButtonClickedEvent clic;
    };

    class InputField : public Selectable, public IPointerClickHandler
    {
        DD_TYPE_OBJET(InputField, Selectable)
    public:
        using OnChangeEvent = UnityEvent1<string>;
        using SubmitEvent = UnityEvent1<string>;
        Ref<Text> textComponent;
        Ref<Graphic> placeholder;
        int characterLimit = 0;
        string text() const { return t; }
        void text(const string& v);
        OnChangeEvent& onValueChanged() { return change; }
        SubmitEvent& onEndEdit() { return fin; }
        bool isFocused() const { return focus; }

        void OnPointerClick(PointerEventData& e) override;
        IPointerClickHandler* GestionnaireClic() override { return this; }

        // --- usage interne : la saisie au clavier quand le champ a le focus
        void Saisir(const string& caracteres, bool effacer, bool valider);
        void PerdreFocus();
        /// Le curseur clignotant est-il visible ?
        bool CurseurVisible() const;

    private:
        string t;
        OnChangeEvent change;
        SubmitEvent fin;
        bool focus = false;
        float debutClignotement = 0.0f;
    };

    class LayoutElement : public UIBehaviour
    {
        DD_TYPE_OBJET(LayoutElement, UIBehaviour)
    public:
        float preferredWidth = -1.0f, preferredHeight = -1.0f, minWidth = -1.0f, minHeight = -1.0f, flexibleWidth = -1.0f;
    };

    class RectMask2D : public UIBehaviour
    {
        DD_TYPE_OBJET(RectMask2D, UIBehaviour)
    };

    class Mask : public UIBehaviour
    {
        DD_TYPE_OBJET(Mask, UIBehaviour)
    public:
        bool showMaskGraphic = true;
    };
}
