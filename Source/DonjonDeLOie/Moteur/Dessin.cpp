// Donjon de l'Oie — la mise en page et la liste de dessin de l'interface (le travail d'uGUI).
#include "Dessin.h"

#include "Entrees.h"

#include <cmath>
#include <cstdlib>

namespace Moteur
{
    namespace Dessin
    {
        namespace
        {
            MesureurApproximatif& Approximatif()
            {
                static MesureurApproximatif m;
                return m;
            }
            MesureurTexte*& Courante()
            {
                static MesureurTexte* m = nullptr;
                return m;
            }

            int imageMiseEnPage = 0;

            // ---------------------------------------------------------- UTF-8

            /// Le point de code qui commence en s[i] ; i avance au caractère suivant.
            uint32_t Lire(const string& s, size_t& i)
            {
                unsigned char c = static_cast<unsigned char>(s[i]);
                int n = c < 0x80 ? 1 : (c >> 5) == 6 ? 2 : (c >> 4) == 14 ? 3 : (c >> 3) == 30 ? 4 : 1;
                uint32_t cp = n == 1 ? c : n == 2 ? (c & 0x1Fu) : n == 3 ? (c & 0x0Fu) : (c & 0x07u);
                for (int k = 1; k < n && i + static_cast<size_t>(k) < s.size(); k++) cp = (cp << 6) | (static_cast<unsigned char>(s[i + static_cast<size_t>(k)]) & 0x3Fu);
                i += static_cast<size_t>(n);
                return cp;
            }

            // ---------------------------------------------------------- texte riche

            struct Style
            {
                bool gras = false, italique = false;
                float taille = 14.0f;
                Color couleur;
                bool couleurPropre = false;     // une balise <color> l'a fixée
            };

            /// Un mot, une espace ou un retour à la ligne, découpé en morceaux de même style.
            struct Jeton
            {
                enum class Sorte { Mot, Espace, Retour };
                Sorte sorte = Sorte::Mot;
                Liste<std::pair<string, Style>> parts;
            };

            bool LireCouleur(const string& v, Color& c)
            {
                if (!v.empty() && v[0] == '#')
                {
                    string h = v.substr(1);
                    if (h.size() != 6 && h.size() != 8) return false;
                    auto oct = [&](size_t k) { return static_cast<float>(std::strtol(h.substr(k, 2).c_str(), nullptr, 16)) / 255.0f; };
                    for (char ch : h)
                        if (!std::isxdigit(static_cast<unsigned char>(ch))) return false;
                    c = Color(oct(0), oct(2), oct(4), h.size() == 8 ? oct(6) : 1.0f);
                    return true;
                }
                struct Nom
                {
                    const char* n;
                    float r, g, b;
                };
                static const Nom noms[] = {{"red", 1, 0, 0}, {"green", 0, 0.5f, 0}, {"blue", 0, 0, 1}, {"white", 1, 1, 1}, {"black", 0, 0, 0},
                                           {"yellow", 1, 0.92f, 0.016f}, {"cyan", 0, 1, 1}, {"magenta", 1, 0, 1}, {"grey", 0.5f, 0.5f, 0.5f},
                                           {"gray", 0.5f, 0.5f, 0.5f}, {"orange", 1, 0.65f, 0}, {"purple", 0.5f, 0, 0.5f}, {"brown", 0.65f, 0.16f, 0.16f},
                                           {"lime", 0, 1, 0}, {"silver", 0.75f, 0.75f, 0.75f}};
                for (auto& n : noms)
                    if (v == n.n)
                    {
                        c = Color(n.r, n.g, n.b, 1.0f);
                        return true;
                    }
                return false;
            }

            /// Découpe un texte (balises <b>, <i>, <size=…>, <color=…> si riche) en jetons stylés.
            Liste<Jeton> Decouper(const string& texte, bool riche, const Style& base)
            {
                Liste<Jeton> jetons;
                int gras = 0, italique = 0;
                auto styleCourant = [&]() {
                    Style s = base;
                    s.gras = base.gras || gras > 0;
                    s.italique = base.italique || italique > 0;
                    return s;
                };
                Liste<float> tailles;
                Liste<Color> couleurs;
                auto ajouter = [&](Jeton::Sorte sorte, const string& ch) {
                    Style s = styleCourant();
                    if (tailles.Count() > 0) s.taille = tailles[tailles.Count() - 1];
                    if (couleurs.Count() > 0)
                    {
                        s.couleur = couleurs[couleurs.Count() - 1];
                        s.couleurPropre = true;
                    }
                    if (sorte == Jeton::Sorte::Retour)
                    {
                        Jeton j;
                        j.sorte = sorte;
                        jetons.Add(j);
                        return;
                    }
                    if (jetons.Count() == 0 || jetons[jetons.Count() - 1].sorte != sorte || sorte == Jeton::Sorte::Retour)
                    {
                        Jeton j;
                        j.sorte = sorte;
                        jetons.Add(j);
                    }
                    auto& j = jetons[jetons.Count() - 1];
                    if (j.parts.Count() > 0)
                    {
                        auto& d = j.parts[j.parts.Count() - 1];
                        if (d.second.gras == s.gras && d.second.italique == s.italique && d.second.taille == s.taille && d.second.couleurPropre == s.couleurPropre &&
                            d.second.couleur == s.couleur)
                        {
                            d.first += ch;
                            return;
                        }
                    }
                    j.parts.Add(std::make_pair(ch, s));
                };
                size_t i = 0;
                while (i < texte.size())
                {
                    char c = texte[i];
                    if (riche && c == '<')
                    {
                        size_t f = texte.find('>', i);
                        if (f != string::npos)
                        {
                            string tag = texte.substr(i + 1, f - i - 1);
                            bool reconnu = true;
                            if (tag == "b") gras++;
                            else if (tag == "/b") { if (gras > 0) gras--; }
                            else if (tag == "i") italique++;
                            else if (tag == "/i") { if (italique > 0) italique--; }
                            else if (tag.rfind("size=", 0) == 0)
                            {
                                int t = 0;
                                if (Texte::LireEntier(tag.substr(5), t)) tailles.Add(static_cast<float>(t));
                                else reconnu = false;
                            }
                            else if (tag == "/size") { if (tailles.Count() > 0) tailles.RemoveAt(tailles.Count() - 1); }
                            else if (tag.rfind("color=", 0) == 0)
                            {
                                Color col;
                                string v = tag.substr(6);
                                if (v.size() >= 2 && v[0] == '"' && v[v.size() - 1] == '"') v = v.substr(1, v.size() - 2);
                                if (LireCouleur(v, col)) couleurs.Add(col);
                                else reconnu = false;
                            }
                            else if (tag == "/color") { if (couleurs.Count() > 0) couleurs.RemoveAt(couleurs.Count() - 1); }
                            else reconnu = false;
                            if (reconnu)
                            {
                                i = f + 1;
                                continue;
                            }
                        }
                    }
                    size_t j = i;
                    uint32_t cp = Lire(texte, j);
                    string ch = texte.substr(i, j - i);
                    i = j;
                    if (cp == '\n') ajouter(Jeton::Sorte::Retour, ch);
                    else if (cp == '\r') continue;
                    else if (cp == ' ' || cp == '\t') ajouter(Jeton::Sorte::Espace, " ");
                    else ajouter(Jeton::Sorte::Mot, ch);
                }
                return jetons;
            }

            // ---------------------------------------------------------- mise en lignes

            struct Element
            {
                string texte;
                Style style;
                float x = 0.0f, largeur = 0.0f;
            };

            struct Ligne
            {
                Liste<Element> elements;
                float largeur = 0.0f, hauteur = 0.0f, tailleMax = 0.0f;
            };

            struct Disposition
            {
                Liste<Ligne> lignes;
                float hauteurTotale = 0.0f, largeurMax = 0.0f;
            };

            float Mesurer(MesureurTexte& m, const string& t, const Style& s) { return t.empty() ? 0.0f : m.Largeur(t, s.taille, s.gras, s.italique); }

            Disposition Disposer(const Liste<Jeton>& jetons, float largeurMax, bool couper, float interligne, float tailleBase, MesureurTexte& m)
            {
                Disposition d;
                Ligne ligne;
                Liste<Element> espaces;          // espaces en attente (perdues si la ligne se coupe là)
                float largeurEspaces = 0.0f;
                bool ligneCoupee = false;        // la ligne en cours commence après une coupure automatique
                auto finir = [&]() {
                    float t = ligne.tailleMax > 0.0f ? ligne.tailleMax : tailleBase;
                    ligne.tailleMax = t;
                    ligne.hauteur = m.HauteurLigne(t);
                    d.lignes.Add(ligne);
                    ligne = Ligne();
                    espaces.Clear();
                    largeurEspaces = 0.0f;
                };
                auto poser = [&](const string& t, const Style& s, float w) {
                    Element e;
                    e.texte = t;
                    e.style = s;
                    e.x = ligne.largeur;
                    e.largeur = w;
                    ligne.elements.Add(e);
                    ligne.largeur += w;
                    ligne.tailleMax = std::max(ligne.tailleMax, s.taille);
                };
                for (auto& j : jetons)
                {
                    if (j.sorte == Jeton::Sorte::Retour)
                    {
                        finir();
                        ligneCoupee = false;
                        continue;
                    }
                    if (j.sorte == Jeton::Sorte::Espace)
                    {
                        for (auto& p : j.parts)
                        {
                            Element e;
                            e.texte = p.first;
                            e.style = p.second;
                            e.largeur = Mesurer(m, p.first, p.second);
                            espaces.Add(e);
                            largeurEspaces += e.largeur;
                        }
                        continue;
                    }
                    float w = 0.0f;
                    Liste<float> largeurs;
                    for (auto& p : j.parts)
                    {
                        float l = Mesurer(m, p.first, p.second);
                        largeurs.Add(l);
                        w += l;
                    }
                    if (couper && ligne.elements.Count() > 0 && ligne.largeur + largeurEspaces + w > largeurMax + 0.01f)
                    {
                        finir();
                        ligneCoupee = true;
                    }
                    // les espaces en attente vont sur la ligne (sauf en tête d'une ligne coupée)
                    if (ligne.elements.Count() > 0 || !ligneCoupee)
                        for (auto& e : espaces) poser(e.texte, e.style, e.largeur);
                    espaces.Clear();
                    largeurEspaces = 0.0f;
                    if (couper && w > largeurMax + 0.01f && largeurMax > 0.0f)
                    {
                        // un mot plus long que la ligne : coupé entre deux caractères
                        for (auto& p : j.parts)
                        {
                            size_t i = 0;
                            while (i < p.first.size())
                            {
                                size_t k = i;
                                Lire(p.first, k);
                                string ch = p.first.substr(i, k - i);
                                i = k;
                                float lc = Mesurer(m, ch, p.second);
                                if (ligne.elements.Count() > 0 && ligne.largeur + lc > largeurMax + 0.01f)
                                {
                                    finir();
                                    ligneCoupee = true;
                                }
                                poser(ch, p.second, lc);
                            }
                        }
                        continue;
                    }
                    for (int k = 0; k < j.parts.Count(); k++) poser(j.parts[k].first, j.parts[k].second, largeurs[k]);
                }
                if (ligne.elements.Count() > 0 || d.lignes.Count() == 0 || (jetons.Count() > 0 && jetons[jetons.Count() - 1].sorte == Jeton::Sorte::Retour)) finir();
                for (int i = 0; i < d.lignes.Count(); i++)
                {
                    d.hauteurTotale += d.lignes[i].hauteur * (i < d.lignes.Count() - 1 ? interligne : 1.0f);
                    d.largeurMax = std::max(d.largeurMax, d.lignes[i].largeur);
                }
                return d;
            }

            Style StyleDeBase(const Text& t, float taille, Color couleur)
            {
                Style s;
                s.gras = t.fontStyle == FontStyle::Bold || t.fontStyle == FontStyle::BoldAndItalic;
                s.italique = t.fontStyle == FontStyle::Italic || t.fontStyle == FontStyle::BoldAndItalic;
                s.taille = taille;
                s.couleur = couleur;
                return s;
            }

            /// La disposition d'un texte dans son rectangle (taille ajustée si « meilleur ajustement »).
            Disposition DisposerTexte(const Text& t, const Rect& r, Color couleur, MesureurTexte& m, float& taille)
            {
                bool couper = t.horizontalOverflow == HorizontalWrapMode::Wrap;
                taille = static_cast<float>(t.fontSize);
                if (t.resizeTextForBestFit)
                {
                    int lo = std::min(t.resizeTextMinSize, t.resizeTextMaxSize), hi = std::max(t.resizeTextMinSize, t.resizeTextMaxSize), meilleur = lo;
                    while (lo <= hi)
                    {
                        int mi = (lo + hi) / 2;
                        auto d = Disposer(Decouper(t.text(), t.supportRichText, StyleDeBase(t, static_cast<float>(mi), couleur)), r.width, couper, t.lineSpacing, static_cast<float>(mi), m);
                        bool tient = d.hauteurTotale <= r.height + 0.01f && (couper || d.largeurMax <= r.width + 0.01f);
                        if (tient)
                        {
                            meilleur = mi;
                            lo = mi + 1;
                        }
                        else hi = mi - 1;
                    }
                    taille = static_cast<float>(meilleur);
                }
                auto d = Disposer(Decouper(t.text(), t.supportRichText, StyleDeBase(t, taille, couleur)), r.width, couper, t.lineSpacing, taille, m);
                if (t.verticalOverflow == VerticalWrapMode::Truncate && d.hauteurTotale > r.height + 0.01f)
                {
                    // seules les lignes entières qui tiennent
                    Disposition g;
                    float h = 0.0f;
                    for (int i = 0; i < d.lignes.Count(); i++)
                    {
                        float hl = d.lignes[i].hauteur;
                        if (h + hl > r.height + 0.01f) break;
                        g.lignes.Add(d.lignes[i]);
                        h += hl * t.lineSpacing;
                    }
                    g.hauteurTotale = 0.0f;
                    for (int i = 0; i < g.lignes.Count(); i++)
                    {
                        g.hauteurTotale += g.lignes[i].hauteur * (i < g.lignes.Count() - 1 ? t.lineSpacing : 1.0f);
                        g.largeurMax = std::max(g.largeurMax, g.lignes[i].largeur);
                    }
                    d = g;
                }
                return d;
            }

            // ---------------------------------------------------------- mise en page

            struct Groupe
            {
                float alpha = 1.0f;
                bool bloque = true;
            };

            void Placer(const Ref<Transform>& t, const Rect& rectParent, Vector2 origineParent, Vector2 echelleParent)
            {
                if (!t->gameObject()->activeSelf()) return;
                auto rt = Comme<RectTransform>(t);
                Rect r = rectParent;
                Vector2 o = origineParent, e = echelleParent;
                if (rt != nullptr)
                {
                    float ax0 = rectParent.x + rt->anchorMin.x * rectParent.width, ay0 = rectParent.y + rt->anchorMin.y * rectParent.height;
                    float ax1 = rectParent.x + rt->anchorMax.x * rectParent.width, ay1 = rectParent.y + rt->anchorMax.y * rectParent.height;
                    float w = ax1 - ax0 + rt->sizeDelta.x, h = ay1 - ay0 + rt->sizeDelta.y;
                    float px = ax0 + (ax1 - ax0) * rt->pivot.x + rt->anchoredPosition.x, py = ay0 + (ay1 - ay0) * rt->pivot.y + rt->anchoredPosition.y;
                    r = Rect(-rt->pivot.x * w, -rt->pivot.y * h, w, h);
                    o = Vector2(origineParent.x + echelleParent.x * px, origineParent.y + echelleParent.y * py);
                    e = Vector2(echelleParent.x * rt->localScale.x, echelleParent.y * rt->localScale.y);
                    rt->RectCalcule = r;
                    rt->OrigineEcran = o;
                    rt->EchelleEcran = e;
                    rt->ImageMiseEnPage = imageMiseEnPage;
                }
                for (auto& enfant : t->Enfants()) Placer(enfant, r, o, e);
            }

            /// Les toiles à la racine (sans toile parente), dans l'ordre de dessin.
            Liste<Ref<Canvas>> Toiles()
            {
                Liste<Ref<Canvas>> l;
                for (auto& c : Object::FindObjectsByType<Canvas>())
                {
                    if (!c->isActiveAndEnabled()) continue;
                    bool imbriquee = false;
                    for (auto p = c->transform()->parent(); p != nullptr; p = p->parent())
                        if (p->gameObject()->GetComponent<Canvas>() != nullptr) imbriquee = true;
                    if (!imbriquee) l.Add(c);
                }
                // tri stable par ordre de tri
                for (int i = 1; i < l.Count(); i++)
                    for (int j = i; j > 0 && l[j - 1]->sortingOrder > l[j]->sortingOrder; j--) std::swap(l[j - 1], l[j]);
                return l;
            }

            Vector2 VersEcran(const Ref<RectTransform>& rt, float x, float y) { return Vector2(rt->OrigineEcran.x + rt->EchelleEcran.x * x, rt->OrigineEcran.y + rt->EchelleEcran.y * y); }

            Color Pince(Color c) { return Color(Mathf::Clamp01(c.r), Mathf::Clamp01(c.g), Mathf::Clamp01(c.b), Mathf::Clamp01(c.a)); }

            // ---------------------------------------------------------- dessin

            struct Effet
            {
                Vector2 decalage;
                Color couleur;
                bool alphaDuGraphique;
            };

            /// Les effets (ombre, contour) posés sur l'objet, dans l'ordre de ses composants.
            Liste<Effet> Effets(const GameObject& g)
            {
                Liste<Effet> l;
                for (auto& c : g.Composants())
                {
                    auto s = Comme<Shadow>(c);
                    if (s == nullptr || s->EstDetruit() || !s->enabled) continue;
                    Vector2 d = s->effectDistance;
                    if (Est<Outline>(c))
                    {
                        l.Add(Effet{Vector2(d.x, d.y), s->effectColor, s->useGraphicAlpha});
                        l.Add(Effet{Vector2(d.x, -d.y), s->effectColor, s->useGraphicAlpha});
                        l.Add(Effet{Vector2(-d.x, d.y), s->effectColor, s->useGraphicAlpha});
                        l.Add(Effet{Vector2(-d.x, -d.y), s->effectColor, s->useGraphicAlpha});
                    }
                    else l.Add(Effet{d, s->effectColor, s->useGraphicAlpha});
                }
                return l;
            }

            /// Couleur d'un effet pour un sommet de cette couleur (« useGraphicAlpha » : l'alpha du sommet s'y multiplie).
            Color CouleurEffet(const Effet& e, Color sommet)
            {
                Color c = e.couleur;
                if (e.alphaDuGraphique) c.a *= sommet.a;
                return c;
            }

            struct Peintre
            {
                Liste<Commande>& sortie;
                MesureurTexte& mesure;
                Dico<const Text*, Ref<InputField>> champs;     // les textes des champs de saisie qui ont le focus

                Commande& Quads(const Ref<Texture2D>& tex)
                {
                    if (sortie.Count() > 0)
                    {
                        auto& d = sortie[sortie.Count() - 1];
                        if (d.sorte == Commande::Sorte::Quads && d.texture.get() == tex.get()) return d;
                    }
                    Commande c;
                    c.sorte = Commande::Sorte::Quads;
                    c.texture = tex;
                    sortie.Add(c);
                    return sortie[sortie.Count() - 1];
                }

                void Quad(const Ref<Texture2D>& tex, const Ref<RectTransform>& rt, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, Color c)
                {
                    if (x1 <= x0 || y1 <= y0 || c.a <= 0.0f) return;
                    auto& cmd = Quads(tex);
                    cmd.sommets.Add(Sommet{VersEcran(rt, x0, y0), Vector2(u0, v0), c});
                    cmd.sommets.Add(Sommet{VersEcran(rt, x0, y1), Vector2(u0, v1), c});
                    cmd.sommets.Add(Sommet{VersEcran(rt, x1, y1), Vector2(u1, v1), c});
                    cmd.sommets.Add(Sommet{VersEcran(rt, x1, y0), Vector2(u1, v0), c});
                }

                /// Une image (simple, découpée en neuf, jauge, proportions gardées), dans la couleur finale c.
                void ImageDe(const Image& im, const Ref<RectTransform>& rt, Color c, float dx, float dy)
                {
                    Rect r = rt->RectCalcule;
                    float x0 = r.x + dx, y0 = r.y + dy, x1 = r.x + r.width + dx, y1 = r.y + r.height + dy;
                    Ref<Texture2D> tex = im.sprite != nullptr ? im.sprite->texture : Ref<Texture2D>();
                    float u0 = 0, v0 = 0, u1 = 1, v1 = 1;
                    float tw = 1, th = 1;
                    if (im.sprite != nullptr && tex != nullptr && tex->width > 0 && tex->height > 0)
                    {
                        tw = static_cast<float>(tex->width);
                        th = static_cast<float>(tex->height);
                        Rect sr = im.sprite->rect;
                        u0 = sr.x / tw;
                        v0 = sr.y / th;
                        u1 = (sr.x + sr.width) / tw;
                        v1 = (sr.y + sr.height) / th;
                    }
                    if (im.type == Image::Type::Sliced && im.sprite != nullptr && (im.sprite->border.x > 0 || im.sprite->border.y > 0 || im.sprite->border.z > 0 || im.sprite->border.w > 0))
                    {
                        Vector4 b = im.sprite->border;
                        float m = std::max(0.0001f, im.pixelsPerUnitMultiplier);
                        float bl = b.x / m, bb = b.y / m, br = b.z / m, bt = b.w / m;
                        float w = x1 - x0, h = y1 - y0;
                        if (bl + br > w && bl + br > 0)
                        {
                            float k = w / (bl + br);
                            bl *= k;
                            br *= k;
                        }
                        if (bb + bt > h && bb + bt > 0)
                        {
                            float k = h / (bb + bt);
                            bb *= k;
                            bt *= k;
                        }
                        float xs[4] = {x0, x0 + bl, x1 - br, x1}, ys[4] = {y0, y0 + bb, y1 - bt, y1};
                        float us[4] = {u0, u0 + b.x / tw, u1 - b.z / tw, u1}, vs[4] = {v0, v0 + b.y / th, v1 - b.w / th, v1};
                        for (int j = 0; j < 3; j++)
                            for (int i = 0; i < 3; i++)
                            {
                                if (i == 1 && j == 1 && !im.fillCenter) continue;
                                Quad(tex, rt, xs[i], ys[j], xs[i + 1], ys[j + 1], us[i], vs[j], us[i + 1], vs[j + 1], c);
                            }
                        return;
                    }
                    if (im.type == Image::Type::Filled)
                    {
                        float f = im.fillAmount();
                        if (f <= 0.0f) return;
                        if (im.fillMethod == Image::FillMethod::Horizontal)
                        {
                            if (im.fillOrigin == 1)
                            {
                                x0 = x1 - (x1 - x0) * f;
                                u0 = u1 - (u1 - u0) * f;
                            }
                            else
                            {
                                x1 = x0 + (x1 - x0) * f;
                                u1 = u0 + (u1 - u0) * f;
                            }
                        }
                        else if (im.fillMethod == Image::FillMethod::Vertical)
                        {
                            if (im.fillOrigin == 1)
                            {
                                y0 = y1 - (y1 - y0) * f;
                                v0 = v1 - (v1 - v0) * f;
                            }
                            else
                            {
                                y1 = y0 + (y1 - y0) * f;
                                v1 = v0 + (v1 - v0) * f;
                            }
                        }
                    }
                    else if (im.preserveAspect && im.sprite != nullptr && im.sprite->rect.height > 0)
                    {
                        float as = im.sprite->rect.width / im.sprite->rect.height;
                        float w = x1 - x0, h = y1 - y0;
                        if (h > 0 && w / h > as)
                        {
                            float nw = h * as;
                            x0 += (w - nw) * rt->pivot.x;
                            x1 = x0 + nw;
                        }
                        else if (as > 0)
                        {
                            float nh = w / as;
                            y0 += (h - nh) * rt->pivot.y;
                            y1 = y0 + nh;
                        }
                    }
                    Quad(tex, rt, x0, y0, x1, y1, u0, v0, u1, v1, c);
                }

                /// Un texte : ses lignes, puis (dessinés avant) les copies de ses ombres et contours. teinte : la couleur
                /// du « CanvasRenderer » (teinte du bouton, alpha des groupes), qui se multiplie à tout.
                void TexteDe(const Text& t, const Ref<RectTransform>& rt, Color teinte, const GameObject& g)
                {
                    auto champ = champs.Chercher(&t);
                    if (t.text().empty() && champ == nullptr) return;
                    Rect r = rt->RectCalcule;
                    float taille = 0.0f;
                    auto d = DisposerTexte(t, r, t.color, mesure, taille);
                    int a = static_cast<int>(t.alignment);
                    int colonne = a % 3, rangee = a / 3;       // rangée 0 : en haut, 1 : au milieu, 2 : en bas
                    float haut = rangee == 0 ? r.y + r.height : rangee == 1 ? r.y + (r.height + d.hauteurTotale) * 0.5f : r.y + d.hauteurTotale;
                    float ex = std::fabs(rt->EchelleEcran.x), ey = std::fabs(rt->EchelleEcran.y);
                    Liste<Morceau> base;
                    Liste<Color> sommets;              // la couleur de chaque morceau avant la teinte (pour les effets)
                    float finX = r.x, finHaut = haut, finH = mesure.HauteurLigne(taille);
                    if (colonne == 1) finX = r.x + r.width * 0.5f;
                    else if (colonne == 2) finX = r.x + r.width;
                    float y = haut;
                    for (int i = 0; i < d.lignes.Count(); i++)
                    {
                        auto& l = d.lignes[i];
                        float x = colonne == 0 ? r.x : colonne == 1 ? r.x + (r.width - l.largeur) * 0.5f : r.x + r.width - l.largeur;
                        for (auto& e : l.elements)
                        {
                            if (e.texte.empty() || e.texte == " ") continue;
                            Morceau m;
                            m.texte = e.texte;
                            m.position = VersEcran(rt, x + e.x, y);
                            m.taillePx = e.style.taille * ey;
                            m.gras = e.style.gras;
                            m.italique = e.style.italique;
                            Color vc = e.style.couleurPropre ? e.style.couleur : t.color;
                            m.couleur = Pince(vc * teinte);
                            sommets.Add(vc);
                            base.Add(m);
                        }
                        finX = x + l.largeur;
                        finHaut = y;
                        finH = l.hauteur;
                        y -= l.hauteur * t.lineSpacing;
                    }
                    auto effets = Effets(g);
                    Commande cmd;
                    cmd.sorte = Commande::Sorte::Texte;
                    for (auto& ef : effets)
                        for (int k = 0; k < base.Count(); k++)
                        {
                            Morceau o = base[k];
                            o.position = Vector2(base[k].position.x + ef.decalage.x * rt->EchelleEcran.x, base[k].position.y + ef.decalage.y * rt->EchelleEcran.y);
                            o.couleur = Pince(CouleurEffet(ef, sommets[k]) * teinte);
                            if (o.couleur.a > 0.0f) cmd.morceaux.Add(o);
                        }
                    for (auto& m : base)
                        if (m.couleur.a > 0.0f) cmd.morceaux.Add(m);
                    if (cmd.morceaux.Count() > 0) sortie.Add(cmd);
                    // le curseur clignotant du champ de saisie, au bout du texte
                    if (champ != nullptr && (*champ)->CurseurVisible())
                    {
                        float cx = finX + 1.0f, largeur = 2.0f / std::max(0.0001f, ex);
                        Quad(Ref<Texture2D>(), rt, cx, finHaut - finH * 0.92f, cx + largeur, finHaut - finH * 0.08f, 0, 0, 1, 1, Pince(t.color * teinte));
                    }
                }

                void Parcourir(const Ref<Transform>& t, Groupe groupe)
                {
                    auto g = t->gameObject();
                    if (!g->activeSelf()) return;
                    auto cg = g->GetComponent<CanvasGroup>();
                    if (cg != nullptr && cg->enabled)
                    {
                        if (cg->ignoreParentGroups) groupe.alpha = cg->alpha;
                        else groupe.alpha *= cg->alpha;
                    }
                    auto rt = Comme<RectTransform>(t);
                    if (rt != nullptr)
                    {
                        auto composants = g->Composants();
                        for (auto& c : composants)
                        {
                            if (c->EstDetruit()) continue;
                            auto gr = Comme<Graphic>(c);
                            if (gr == nullptr || !gr->enabled) continue;
                            Color teinte = gr->TeinteTransition;
                            teinte.a *= groupe.alpha;
                            if (auto im = Comme<Image>(c))
                            {
                                for (auto& ef : Effets(*g)) ImageDe(*im, rt, Pince(CouleurEffet(ef, im->color) * teinte), ef.decalage.x, ef.decalage.y);
                                ImageDe(*im, rt, Pince(im->color * teinte), 0, 0);
                            }
                            else if (auto ri = Comme<RawImage>(c))
                            {
                                Rect r = rt->RectCalcule;
                                Quad(Comme<Texture2D>(ri->texture), rt, r.x, r.y, r.x + r.width, r.y + r.height, 0, 0, 1, 1, Pince(ri->color * teinte));
                            }
                            else if (auto tx = Comme<Text>(c)) TexteDe(*tx, rt, teinte, *g);
                        }
                    }
                    for (auto& e : t->Enfants()) Parcourir(e, groupe);
                }
            };

            // ---------------------------------------------------------- lancer de rayon

            void Toucher(const Ref<Transform>& t, Vector2 p, bool bloque, Ref<Graphic>& meilleur)
            {
                auto g = t->gameObject();
                if (!g->activeSelf()) return;
                auto cg = g->GetComponent<CanvasGroup>();
                if (cg != nullptr && cg->enabled)
                {
                    if (cg->ignoreParentGroups) bloque = cg->blocksRaycasts;
                    else bloque = bloque && cg->blocksRaycasts;
                }
                auto rt = Comme<RectTransform>(t);
                if (rt != nullptr && bloque)
                    for (auto& c : g->Composants())
                    {
                        if (c->EstDetruit()) continue;
                        auto gr = Comme<Graphic>(c);
                        if (gr == nullptr || !gr->enabled || !gr->raycastTarget) continue;
                        Rect r = RectEcran(rt);
                        if (p.x >= r.x && p.x <= r.x + r.width && p.y >= r.y && p.y <= r.y + r.height) meilleur = gr;
                    }
                for (auto& e : t->Enfants()) Toucher(e, p, bloque, meilleur);
            }
        }

        // -------------------------------------------------------------- l'interface publique

        float MesureurApproximatif::Largeur(const string& texte, float taillePx, bool gras, bool)
        {
            float w = 0.0f;
            size_t i = 0;
            while (i < texte.size())
            {
                uint32_t cp = Lire(texte, i);
                float k;
                if (cp == ' ') k = 0.28f;
                else if (cp == 'i' || cp == 'l' || cp == 'j' || cp == '!' || cp == '.' || cp == ',' || cp == '\'' || cp == ':' || cp == ';' || cp == '|' || cp == 'I') k = 0.25f;
                else if (cp == 'f' || cp == 't' || cp == 'r' || cp == '(' || cp == ')' || cp == '-') k = 0.34f;
                else if (cp == 'm' || cp == 'w' || cp == 'M' || cp == 'W') k = 0.84f;
                else if (cp >= 'A' && cp <= 'Z') k = 0.66f;
                else if (cp >= '0' && cp <= '9') k = 0.56f;
                else if (cp >= 0xC0 && cp <= 0xDE) k = 0.66f;        // capitales accentuées
                else k = 0.52f;
                w += k * taillePx;
            }
            return gras ? w * 1.07f : w;
        }

        MesureurTexte& MesureCourante() { return Courante() != nullptr ? *Courante() : static_cast<MesureurTexte&>(Approximatif()); }
        void ChoisirMesure(MesureurTexte* m) { Courante() = m; }

        void MettreEnPage(int largeurPx, int hauteurPx)
        {
            imageMiseEnPage++;
            for (auto& c : Toiles())
            {
                auto rt = Comme<RectTransform>(c->transform());
                if (rt == nullptr) continue;
                float s = c->scaleFactor();
                if (s <= 0.0f) s = 1.0f;
                float w = static_cast<float>(largeurPx) / s, h = static_cast<float>(hauteurPx) / s;
                rt->RectCalcule = Rect(-w * 0.5f, -h * 0.5f, w, h);
                rt->OrigineEcran = Vector2(static_cast<float>(largeurPx) * 0.5f, static_cast<float>(hauteurPx) * 0.5f);
                rt->EchelleEcran = Vector2(s, s);
                rt->ImageMiseEnPage = imageMiseEnPage;
                for (auto& e : rt->Enfants()) Placer(e, rt->RectCalcule, rt->OrigineEcran, rt->EchelleEcran);
            }
        }

        Rect RectEcran(const Ref<RectTransform>& r)
        {
            Vector2 a = VersEcran(r, r->RectCalcule.x, r->RectCalcule.y), b = VersEcran(r, r->RectCalcule.x + r->RectCalcule.width, r->RectCalcule.y + r->RectCalcule.height);
            return Rect(std::min(a.x, b.x), std::min(a.y, b.y), std::fabs(b.x - a.x), std::fabs(b.y - a.y));
        }

        void Interface(int largeurPx, int hauteurPx, MesureurTexte& mesure, Liste<Commande>& sortie)
        {
            sortie.Clear();
            MettreEnPage(largeurPx, hauteurPx);
            Peintre p{sortie, mesure, {}};
            for (auto& f : Object::FindObjectsByType<InputField>())
                if (f->isFocused() && f->textComponent != nullptr) p.champs[f->textComponent.get()] = f;
            for (auto& c : Toiles())
            {
                Groupe g;
                p.Parcourir(c->transform(), g);
            }
        }

        Ref<Graphic> ElementSous(Vector2 pointPx)
        {
            auto toiles = Toiles();
            for (int i = toiles.Count() - 1; i >= 0; i--)
            {
                if (toiles[i]->GetComponent<GraphicRaycaster>() == nullptr) continue;
                Ref<Graphic> meilleur;
                Toucher(toiles[i]->transform(), pointPx, true, meilleur);
                if (meilleur != nullptr) return meilleur;
            }
            return Ref<Graphic>();
        }
    }

    // ------------------------------------------------------------------ Text : tailles naturelles

    float Text::preferredWidth() const
    {
        auto& m = Dessin::MesureCourante();
        float taille = static_cast<float>(fontSize);
        Dessin::Style s;
        s.gras = fontStyle == FontStyle::Bold || fontStyle == FontStyle::BoldAndItalic;
        s.italique = fontStyle == FontStyle::Italic || fontStyle == FontStyle::BoldAndItalic;
        s.taille = taille;
        auto d = Dessin::Disposer(Dessin::Decouper(t, supportRichText, s), 0.0f, false, lineSpacing, taille, m);
        return d.largeurMax;
    }

    float Text::preferredHeight() const
    {
        auto& m = Dessin::MesureCourante();
        float taille = static_cast<float>(fontSize);
        Dessin::Style s;
        s.gras = fontStyle == FontStyle::Bold || fontStyle == FontStyle::BoldAndItalic;
        s.italique = fontStyle == FontStyle::Italic || fontStyle == FontStyle::BoldAndItalic;
        s.taille = taille;
        auto rt = Comme<RectTransform>(transform());
        float largeur = rt != nullptr ? rt->RectCalcule.width : 0.0f;
        bool couper = horizontalOverflow == HorizontalWrapMode::Wrap && largeur > 0.0f;
        auto d = Dessin::Disposer(Dessin::Decouper(t, supportRichText, s), largeur, couper, lineSpacing, taille, m);
        return d.hauteurTotale;
    }
}
