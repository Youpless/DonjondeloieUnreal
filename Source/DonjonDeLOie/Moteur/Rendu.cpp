// Donjon de l'Oie — maillages, matériaux, caméra, textures, sprites.
#include "Rendu.h"

#include <cmath>

namespace Moteur
{
    // ------------------------------------------------------------------ Mesh

    void Mesh::SetVertices(const Liste<Vector3>& v)
    {
        DD_VERIFIE(!(v.Count() > 65535 && indexFormat == IndexFormat::UInt16), "Mesh : plus de 65 535 sommets en index 16 bits");
        V = v;
        version++;
    }

    void Mesh::SetNormals(const Liste<Vector3>& n)
    {
        N = n;
        version++;
    }

    void Mesh::SetColors(const Liste<Color32>& c)
    {
        C = c;
        version++;
    }

    void Mesh::SetColors(const Liste<Color>& c)
    {
        C.Clear();
        for (auto& x : c) C.Add(Color32(x));
        version++;
    }

    void Mesh::SetTriangles(const Liste<int>& t, int)
    {
        for (int i : t) DD_VERIFIE(i >= 0 && i < V.Count(), "Mesh : indice de triangle hors limites");
        T = t;
        version++;
    }

    void Mesh::vertices(const Liste<Vector3>& v)
    {
        V = v;
        version++;
    }

    void Mesh::colors32(const Liste<Color32>& c)
    {
        C = c;
        version++;
    }

    void Mesh::normals(const Liste<Vector3>& n)
    {
        N = n;
        version++;
    }

    void Mesh::RecalculateBounds()
    {
        if (V.Count() == 0)
        {
            bounds = Bounds();
            return;
        }
        Vector3 mn = V[0], mx = V[0];
        for (auto& v : V)
        {
            mn = Vector3::Min(mn, v);
            mx = Vector3::Max(mx, v);
        }
        bounds = Bounds((mn + mx) * 0.5f, mx - mn);
    }

    void Mesh::Clear()
    {
        V.Clear();
        N.Clear();
        C.Clear();
        T.Clear();
        version++;
    }

    // ------------------------------------------------------------------ MaterialPropertyBlock

    void MaterialPropertyBlock::SetColor(const string& n, Color c)
    {
        nombres.Remove(n);
        couleurs[n] = c;
    }

    void MaterialPropertyBlock::SetFloat(const string& n, float f)
    {
        couleurs.Remove(n);
        nombres[n] = f;
    }

    // ------------------------------------------------------------------ Shader, Material

    Ref<Shader> Shader::Find(const string& n)
    {
        static const char* connus[] = {"DonjonDeLOie/Voxel", "DonjonDeLOie/Lueur", "DonjonDeLOie/Fx", "Sprites/Default", "Universal Render Pipeline/Lit",
                                       "Universal Render Pipeline/Particles/Simple Lit", "Universal Render Pipeline/Particles/Unlit", "Particles/Standard Surface", "Standard"};
        bool connu = false;
        for (const char* c : connus)
            if (n == c) connu = true;
        if (!connu) return Ref<Shader>();
        auto s = Neuf<Shader>();
        s->name(n);
        return s;
    }

    int Shader::PropertyToID(const string& n)
    {
        // un identifiant stable par nom (FNV-1a)
        uint32_t h = 2166136261u;
        for (char c : n)
        {
            h ^= static_cast<uint8_t>(c);
            h *= 16777619u;
        }
        return static_cast<int>(h);
    }

    Material::Material(const Ref<Shader>& s)
    {
        DD_VERIFIE(s.get() != nullptr, "new Material(shader) : shader nul");
        shader = s;
    }

    Material::Material(const Ref<Material>& m)
    {
        DD_VERIFIE(m.get() != nullptr, "new Material(matériau) : matériau nul");
        shader = m->shader;
        renderQueue = m->renderQueue;
        nombres = m->nombres;
        couleurs = m->couleurs;
        vecteurs = m->vecteurs;
    }

    void Material::SetFloat(const string& n, float f)
    {
        couleurs.Remove(n);
        vecteurs.Remove(n);
        nombres[n] = f;
        version++;
    }

    void Material::SetColor(const string& n, Color c)
    {
        nombres.Remove(n);
        vecteurs.Remove(n);
        couleurs[n] = c;
        version++;
    }

    void Material::SetVector(const string& n, Vector4 v)
    {
        nombres.Remove(n);
        couleurs.Remove(n);
        vecteurs[n] = v;
        version++;
    }

    float Material::GetFloat(const string& n) const
    {
        float f = 0.0f;
        return nombres.TryGetValue(n, f) ? f : 0.0f;
    }

    Color Material::GetColor(const string& n) const
    {
        Color c;
        return couleurs.TryGetValue(n, c) ? c : Color::white;
    }

    // ------------------------------------------------------------------ Camera

    Ref<Camera> Camera::main()
    {
        for (auto& g : Boucle::Objets())
            if (g->tag == "MainCamera" && !g->EstDetruit())
            {
                auto c = g->GetComponent<Camera>();
                if (c != nullptr) return c;
            }
        return Ref<Camera>();
    }

    Vector3 Camera::WorldToScreenPoint(Vector3 p) const
    {
        auto t = transform();
        Vector3 d = p - t->position();
        Vector3 f = t->forward(), r = t->right(), u = t->up();
        float z = Vector3::Dot(d, f);
        float k = (static_cast<float>(Screen::height) * 0.5f) / Mathf::Tan(fieldOfView * 0.5f * Mathf::Deg2Rad);
        if (Mathf::Abs(z) < 1e-5f) z = 1e-5f;
        return Vector3(static_cast<float>(Screen::width) * 0.5f + Vector3::Dot(d, r) / z * k, static_cast<float>(Screen::height) * 0.5f + Vector3::Dot(d, u) / z * k, z);
    }

    Ray Camera::ScreenPointToRay(Vector3 s) const
    {
        auto t = transform();
        float k = (static_cast<float>(Screen::height) * 0.5f) / Mathf::Tan(fieldOfView * 0.5f * Mathf::Deg2Rad);
        Vector3 d = t->forward() * k + t->right() * (s.x - static_cast<float>(Screen::width) * 0.5f) + t->up() * (s.y - static_cast<float>(Screen::height) * 0.5f);
        return Ray(t->position(), d);
    }

    // ------------------------------------------------------------------ Texture2D, Sprite

    Texture2D::Texture2D(int w, int h)
    {
        width = w;
        height = h;
        px = Liste<Color32>(w * h);
    }

    void Texture2D::SetPixels32(const Liste<Color32>& c)
    {
        DD_VERIFIE(c.Count() == px.Count(), "Texture2D : taille");
        px = c;
    }

    void Texture2D::SetPixel(int x, int y, Color c) { px[y * width + x] = Color32(c); }

    Ref<Texture2D> Texture2D::whiteTexture()
    {
        auto t = Neuf<Texture2D>(4, 4);
        t->px = Liste<Color32>(16, Color32(255, 255, 255, 255));
        t->Apply();
        return t;
    }

    Ref<Sprite> Sprite::Create(const Ref<Texture2D>& t, Rect r, Vector2, float)
    {
        auto s = Neuf<Sprite>();
        s->texture = t;
        s->rect = r;
        return s;
    }

    Ref<Sprite> Sprite::Create(const Ref<Texture2D>& t, Rect r, Vector2, float, unsigned int, SpriteMeshType, Vector4 bord)
    {
        auto s = Neuf<Sprite>();
        s->texture = t;
        s->rect = r;
        s->border = bord;
        return s;
    }
}
