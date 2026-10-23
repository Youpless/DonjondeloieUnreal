// Donjon de l'Oie — boîte à outils d'interface (uGUI construit en code) : panneaux de bois et de parchemin,
// textes ombrés, boutons, icônes en pixel art, barres. Tous les sprites sont dessinés au lancement.
//
// (Dans les fonctions d'UIK, « Image », « Rect » et « Texte » désignent les fonctions d'UIK : le type d'image, celui
// du rectangle et l'espace de noms des textes s'écrivent Moteur::Image, Moteur::Rect, Moteur::Texte, comme le C#
// écrit « UnityEngine.UI.Image ».)
#include "UIK.h"

#include "../Modeles/HerosModeles.h"
#include "../Monde/Sons.h"
#include "../Voxel/PixelIcones.h"

namespace DonjonDeLOie::Interface
{
    using namespace DonjonDeLOie::Voxel;

    // Les variables statiques qui changent (police, sprites, cache des icônes) reprennent leur valeur de départ à
    // chaque nouvelle session, comme au lancement d'une partie dans Unity (les objets eux-mêmes sont détruits par le
    // moteur).
    const bool UIK::statiquesInscrits = Boucle::AuReinitialiser([] {
        UIK::Police = nullptr;
        UIK::plein = nullptr;
        UIK::contour = nullptr;
        UIK::lueur = nullptr;
        UIK::degrade = nullptr;
        UIK::blanc = nullptr;
        UIK::icones.Clear();
    });

    void UIK::Initialiser()
    {
        if (Police != nullptr && plein != nullptr) return;
        icones.Clear();
        Police = Resources::GetBuiltinResource<Font>("LegacyRuntime.ttf");
        plein = Arrondi(64, 15, 0.0f); plein->name("plein");
        contour = Arrondi(64, 15, 3.0f); contour->name("contour");
        lueur = Radial(64); lueur->name("lueur");
        degrade = Degrade(8, 64); degrade->name("degrade");
        auto t = Neuf<Texture2D>(4, 4); Liste<Color32> px(16);
        for (int i = 0; i < 16; i++) px[i] = Color32(255, 255, 255, 255);
        t->SetPixels32(px); t->Apply();
        blanc = Sprite::Create(t, Moteur::Rect(0, 0, 4, 4), Vector2(0.5f, 0.5f), 100.0f);
        blanc->name("blanc");
    }

    // ------------------------------------------------------------------ sprites dessinés

    Ref<Sprite> UIK::Arrondi(int n, int rayon, float epaisseur)
    {
        auto t = Neuf<Texture2D>(n, n, TextureFormat::RGBA32, false);
        t->wrapMode = TextureWrapMode::Clamp;
        t->filterMode = FilterMode::Bilinear;
        Liste<Color32> px(n * n);
        for (int y = 0; y < n; y++)
            for (int x = 0; x < n; x++)
            {
                float cx = Mathf::Clamp(x + 0.5f, static_cast<float>(rayon), static_cast<float>(n - rayon)), cy = Mathf::Clamp(y + 0.5f, static_cast<float>(rayon), static_cast<float>(n - rayon));
                float d = Mathf::Sqrt((x + 0.5f - cx) * (x + 0.5f - cx) + (y + 0.5f - cy) * (y + 0.5f - cy));
                float a = Mathf::Clamp01(rayon - d + 0.5f);
                if (epaisseur > 0) a *= Mathf::Clamp01(d - (rayon - epaisseur) + 0.5f);
                px[y * n + x] = Color32(255, 255, 255, static_cast<uint8_t>(a * 255));
            }
        t->SetPixels32(px); t->Apply();
        float b = static_cast<float>(rayon + 1);
        return Sprite::Create(t, Moteur::Rect(0, 0, static_cast<float>(n), static_cast<float>(n)), Vector2(0.5f, 0.5f), 100.0f, 0, SpriteMeshType::FullRect, Vector4(b, b, b, b));
    }

    Ref<Sprite> UIK::Radial(int n)
    {
        auto t = Neuf<Texture2D>(n, n, TextureFormat::RGBA32, false);
        t->wrapMode = TextureWrapMode::Clamp;
        Liste<Color32> px(n * n);
        for (int y = 0; y < n; y++)
            for (int x = 0; x < n; x++)
            {
                float dx = (x + 0.5f) / static_cast<float>(n) * 2 - 1, dy = (y + 0.5f) / static_cast<float>(n) * 2 - 1;
                float a = Mathf::Clamp01(1.0f - Mathf::Sqrt(dx * dx + dy * dy));
                px[y * n + x] = Color32(255, 255, 255, static_cast<uint8_t>(a * a * 255));
            }
        t->SetPixels32(px); t->Apply();
        return Sprite::Create(t, Moteur::Rect(0, 0, static_cast<float>(n), static_cast<float>(n)), Vector2(0.5f, 0.5f), 100.0f);
    }

    Ref<Sprite> UIK::Degrade(int w, int h)
    {
        auto t = Neuf<Texture2D>(w, h, TextureFormat::RGBA32, false);
        t->wrapMode = TextureWrapMode::Clamp;
        Liste<Color32> px(w * h);
        for (int y = 0; y < h; y++) for (int x = 0; x < w; x++) px[y * w + x] = Color32(255, 255, 255, static_cast<uint8_t>(255 * y / (h - 1)));
        t->SetPixels32(px); t->Apply();
        return Sprite::Create(t, Moteur::Rect(0, 0, static_cast<float>(w), static_cast<float>(h)), Vector2(0.5f, 0.5f), 100.0f);
    }

    Ref<Sprite> UIK::Icone(const string& id)
    {
        Ref<Sprite> s;
        if (icones.TryGetValue(id, s)) return s;
        int n = PixelIcones::Taille;
        auto t = Neuf<Texture2D>(n, n, TextureFormat::RGBA32, false);
        t->filterMode = FilterMode::Point;
        t->wrapMode = TextureWrapMode::Clamp;
        Liste<Color32> px(n * n);
        for (int y = 0; y < n; y++) for (int x = 0; x < n; x++) px[y * n + x] = PixelIcones::Pixel(id, x, y);
        t->SetPixels32(px); t->Apply();
        s = Sprite::Create(t, Moteur::Rect(0, 0, static_cast<float>(n), static_cast<float>(n)), Vector2(0.5f, 0.5f), 100.0f);
        s->name("icone:" + id);
        icones[id] = s;
        return s;
    }

    /// Oublie les icônes dont le nom commence ainsi (les portraits des évadés d'une partie terminée) : leurs
    /// textures sont libérées ; elles seraient redessinées si on les redemandait.
    void UIK::OublierIcones(const string& prefixe)
    {
        Liste<string> noms;
        for (auto& k : icones.Keys()) if (Moteur::Texte::CommencePar(k, prefixe)) noms.Add(k);
        for (auto& k : noms)
        {
            auto s = icones[k];
            icones.Remove(k);
            if (s != nullptr) { Object::Destroy(s->texture); Object::Destroy(s); }
        }
        noms.Clear();
        for (auto& k : PixelIcones::Dynamiques().Keys()) if (Moteur::Texte::CommencePar(k, prefixe)) noms.Add(k);
        for (auto& k : noms) PixelIcones::Dynamiques().Remove(k);
    }

    // ------------------------------------------------------------------ placement

    Ref<RectTransform> UIK::Rect(const Ref<Transform>& parent, const string& nom)
    {
        auto go = Neuf<GameObject>(nom, TypeDe<RectTransform>());
        go->transform()->SetParent(parent, false);
        return Vers<RectTransform>(go->transform());
    }

    /// Ancre en un point (ax, ay de 0 à 1), décalage et taille en pixels de référence (1920 × 1080).
    Ref<RectTransform> UIK::Placer(const Ref<RectTransform>& r, float ax, float ay, float x, float y, float w, float h, float px, float py)
    {
        r->anchorMin = r->anchorMax = Vector2(ax, ay);
        r->pivot = Vector2(px, py);
        r->anchoredPosition = Vector2(x, y);
        r->sizeDelta = Vector2(w, h);
        return r;
    }

    /// Remplit le parent avec des marges.
    Ref<RectTransform> UIK::Remplir(const Ref<RectTransform>& r, float g, float b, float d, float h)
    {
        r->anchorMin = Vector2::zero; r->anchorMax = Vector2::one;
        r->offsetMin(Vector2(g, b)); r->offsetMax(Vector2(-d, -h));
        return r;
    }

    // ------------------------------------------------------------------ éléments

    Ref<Moteur::Image> UIK::Image(const Ref<Transform>& parent, const string& nom, const Ref<Sprite>& s, Color c)
    {
        auto r = Rect(parent, nom);
        auto im = r->gameObject()->AddComponent<Moteur::Image>();
        im->sprite = s;
        im->color = c;
        if (s == plein || s == contour) im->type = Moteur::Image::Type::Sliced;
        im->raycastTarget = false;
        return im;
    }

    /// Panneau stylé : fond + liseré. Rend le RectTransform du panneau.
    Ref<RectTransform> UIK::Panneau(const Ref<Transform>& parent, const string& nom, Style st)
    {
        Color fond, bord;
        switch (st)
        {
            case Style::Parchemin: fond = Color(0.93f, 0.87f, 0.73f, 0.98f); bord = Color(0.45f, 0.3f, 0.16f); break;
            case Style::Sombre: fond = Color(0.05f, 0.04f, 0.04f, 0.72f); bord = Color(0.55f, 0.45f, 0.3f, 0.6f); break;
            case Style::Carte: fond = Color(0.98f, 0.94f, 0.84f, 1.0f); bord = Color(0.55f, 0.38f, 0.2f); break;
            case Style::Verre: fond = Color(0.1f, 0.08f, 0.07f, 0.62f); bord = Color(0.93f, 0.76f, 0.36f, 0.35f); break;
            case Style::Puce: fond = Color(0.2f, 0.15f, 0.11f, 0.9f); bord = Color(0.72f, 0.55f, 0.28f, 0.8f); break;
            default: fond = BoisFonce; bord = Color(0.78f, 0.6f, 0.3f); break;
        }
        auto im = Image(parent, nom, plein, fond);
        im->raycastTarget = true;
        auto b = Image(im->transform(), "Liseré", contour, bord);
        Remplir(b->rectTransform());
        if (st == Style::Bois)
        {
            auto ref_ = Image(im->transform(), "Reflet", degrade, Color(1.0f, 0.85f, 0.6f, 0.06f));
            Remplir(ref_->rectTransform(), 3, 3, 3, 3);
        }
        return im->rectTransform();
    }

    Ref<Text> UIK::Texte(const Ref<Transform>& parent, const string& nom, const string& s, int taille, Color c, TextAnchor ancre, bool gras, bool ombre)
    {
        auto r = Rect(parent, nom);
        auto t = r->gameObject()->AddComponent<Text>();
        t->font = Police;
        t->text(s);
        t->fontSize = taille;
        t->color = c;
        t->alignment = ancre;
        t->fontStyle = gras ? FontStyle::Bold : FontStyle::Normal;
        t->horizontalOverflow = HorizontalWrapMode::Wrap;
        t->verticalOverflow = VerticalWrapMode::Overflow;
        t->supportRichText = true;
        t->raycastTarget = false;
        t->lineSpacing = 1.05f;
        if (ombre)
        {
            auto sh = r->gameObject()->AddComponent<Shadow>();
            sh->effectColor = Color(0, 0, 0, c.grayscale() > 0.5f ? 0.75f : 0.18f);
            sh->effectDistance = Vector2(1.5f, -1.5f);
        }
        return t;
    }

    Ref<Button> UIK::Bouton(const Ref<Transform>& parent, const string& nom, const string& libelle, Style st, std::function<void()> clic, int taille)
    {
        Color fond, bord, encre;
        switch (st)
        {
            case Style::BoutonOr: fond = Color(0.9f, 0.68f, 0.27f); bord = Color(0.45f, 0.28f, 0.1f); encre = Encre; break;
            case Style::BoutonRouge: fond = Color(0.62f, 0.2f, 0.16f); bord = Color(0.95f, 0.6f, 0.4f); encre = Creme; break;
            case Style::Parchemin: fond = Color(0.98f, 0.94f, 0.84f); bord = Color(0.55f, 0.38f, 0.2f); encre = Encre; break;
            default: fond = Color(0.25f, 0.18f, 0.13f, 0.97f); bord = Color(0.82f, 0.64f, 0.32f); encre = Creme; break;
        }
        auto im = Image(parent, nom, plein, fond);
        im->raycastTarget = true;
        auto b = Image(im->transform(), "Liseré", contour, bord);
        Remplir(b->rectTransform());
        auto btn = im->gameObject()->AddComponent<Button>();
        btn->targetGraphic = im;
        auto cb = btn->colors();
        cb.normalColor = Color(0.9f, 0.9f, 0.9f, 1.0f);
        cb.highlightedColor = Color::white;
        cb.selectedColor = Color(0.9f, 0.9f, 0.9f, 1.0f);
        cb.pressedColor = Color(0.72f, 0.72f, 0.72f, 1.0f);
        cb.disabledColor = Color(0.55f, 0.55f, 0.55f, 0.55f);
        cb.colorMultiplier = 1.15f;
        cb.fadeDuration = 0.08f;
        btn->colors(cb);
        if (clic) btn->onClick().AddListener(clic);
        btn->onClick().AddListener([] { Monde::Sons::Jouer(Monde::Son::Clic, 0.6f); });
        auto t = Texte(im->transform(), "Libellé", libelle, taille, encre, TextAnchor::MiddleCenter, st == Style::BoutonOr, st != Style::Parchemin && st != Style::BoutonOr);
        Remplir(t->rectTransform(), 10, 2, 10, 2);
        return btn;
    }

    Ref<Text> UIK::Libelle(const Ref<Button>& b) { return b->GetComponentInChildren<Text>(); }

    Ref<Moteur::Image> UIK::IconeImage(const Ref<Transform>& parent, const string& id, float taille)
    {
        auto im = Image(parent, "Icône_" + id, Icone(id), Color::white);
        im->rectTransform()->sizeDelta = Vector2(taille, taille);
        im->preserveAspect = true;
        return im;
    }

    /// Barre de jauge : fond sombre + remplissage (Image en mode « Filled »).
    Ref<Moteur::Image> UIK::Barre(const Ref<Transform>& parent, const string& nom, Color c, Ref<RectTransform>& cadre)
    {
        auto fond = Image(parent, nom, plein, Color(0, 0, 0, 0.55f));
        cadre = fond->rectTransform();
        auto r = Image(fond->transform(), "Remplissage", blanc, c);
        Remplir(r->rectTransform(), 2, 2, 2, 2);
        r->type = Moteur::Image::Type::Filled;
        r->fillMethod = Moteur::Image::FillMethod::Horizontal;
        r->fillOrigin = 0;
        r->fillAmount(1.0f);
        return r;
    }

    string UIK::Hex(Color c)
    {
        Color32 k = c;
        return Moteur::Texte::Hex2(k.r) + Moteur::Texte::Hex2(k.g) + Moteur::Texte::Hex2(k.b);
    }

    string UIK::Colorer(const string& s, Color c) { return "<color=#" + Hex(c) + ">" + s + "</color>"; }

    void UIK::Vider(const Ref<Transform>& t)
    {
        for (int i = t->childCount() - 1; i >= 0; i--) Object::Destroy(t->GetChild(i)->gameObject());
    }

    /// L'icône d'une quête selon son genre ; celle d'un avis de recherche est le portrait de l'évadé.
    string UIK::IconeQuete(const Ptr<Regles::Quete>& q)
    {
        switch (q->Type)
        {
            case Regles::Quete::Genre::Chasser: return "crane";
            case Regles::Quete::Genre::Vaincre: return "epee";
            case Regles::Quete::Genre::Defier: return "epees_croisees";
            case Regles::Quete::Genre::Embuscade: return "oeil";
            case Regles::Quete::Genre::Evade: return Modeles::HerosModeles::IconePortrait(q->Graine, q->NomEvade, q->Feminin);
            default: return "parchemin";
        }
    }

    string UIK::IconeObjet(const Ptr<Regles::DefObjet>& o)
    {
        switch (o->Type)
        {
            case Regles::TypeObjet::Arme: return "epee";
            case Regles::TypeObjet::Armure: return "bouclier";
            case Regles::TypeObjet::Accessoire: return o->Effet == Regles::EffetObjet::Amulette ? "oeil" : o->Effet == Regles::EffetObjet::Bottes ? "fleche" : "couronne";
            case Regles::TypeObjet::Potion: return "fiole";
            case Regles::TypeObjet::Piege: return "piege";
            default: return o->Effet == Regles::EffetObjet::SortFoudre ? "eclair" : o->Effet == Regles::EffetObjet::SortMalediction ? "crane" : "parchemin";
        }
    }
}
