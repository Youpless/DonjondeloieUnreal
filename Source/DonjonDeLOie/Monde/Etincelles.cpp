// Donjon de l'Oie — particules en petits cubes, toutes dans un seul maillage rebâti à chaque image (peu d'objets).
// Deux systèmes : un additif (étincelles, magie, or) et un en transparence (fumée, poussière).
#include "Etincelles.h"

#include "DDMateriaux.h"

namespace DonjonDeLOie::Monde
{
    Ref<Etincelles> Etincelles::Creer(const Ref<Transform>& parent, bool estAdditif)
    {
        auto go = Neuf<GameObject>(estAdditif ? "Étincelles" : "Fumées");
        go->transform()->SetParent(parent, false);
        auto e = go->AddComponent<Etincelles>();
        e->additif = estAdditif;
        e->mesh = Neuf<Mesh>();
        e->mesh->name("particules");
        e->mesh->MarkDynamic();
        go->AddComponent<MeshFilter>()->sharedMesh = e->mesh;
        auto mr = go->AddComponent<MeshRenderer>();
        mr->sharedMaterial = estAdditif ? DDMateriaux::FxAdd() : DDMateriaux::FxAlpha();
        mr->shadowCastingMode = ShadowCastingMode::Off;
        mr->receiveShadows = false;
        return e;
    }

    void Etincelles::Emettre(Vector3 pos, Vector3 vel, Color col, float duree, float taille, float gravite, float tournoie)
    {
        if (ps.Count() > 1500) return;
        P p;
        p.pos = pos; p.vel = vel; p.vie = 0.0f; p.duree = duree; p.taille = taille; p.gravite = gravite; p.c = col; p.tournoie = tournoie; p.centre = pos;
        ps.Add(p);
    }

    // ------------------------------------------------------------------ recettes
    // (Les arguments qui tirent au hasard sont sortis dans des variables, dans l'ordre où C# les évalue.)

    void Etincelles::Explosion(Vector3 p, Color col, int nombre, float vitesse, float taille, float gravite)
    {
        for (int i = 0; i < nombre; i++)
        {
            Vector3 d = Random::onUnitSphere(); d.y = Mathf::Abs(d.y) * 0.8f + 0.2f;
            float kv = Random::Range(0.4f, 1.0f);
            Color teinte = Teinte(col);
            float duree = Random::Range(0.4f, 0.9f);
            float kt = Random::Range(0.6f, 1.3f);
            Emettre(p, d * vitesse * kv, teinte, duree, taille * kt, gravite);
        }
    }

    void Etincelles::Pieces(Vector3 p, int nombre)
    {
        for (int i = 0; i < nombre; i++)
        {
            float dx = Random::Range(-1.0f, 1.0f), dy = Random::Range(2.2f, 3.6f), dz = Random::Range(-1.0f, 1.0f);
            Vector3 d(dx, dy, dz);
            Color teinte = Teinte(Color(1.0f, 0.78f, 0.25f));
            float duree = Random::Range(0.8f, 1.2f);
            float taille = Random::Range(0.06f, 0.1f);
            Emettre(p + Vector3::up * 0.8f, d * 1.1f, teinte, duree, taille, 7.0f);
        }
    }

    /// Spirale montante (niveau gagné, soin, bénédiction).
    void Etincelles::Spirale(Vector3 p, Color col, int nombre, float hauteur)
    {
        for (int i = 0; i < nombre; i++)
        {
            float a = static_cast<float>(i) / static_cast<float>(nombre) * Mathf::Pi * 6.0f;
            Vector3 off(Mathf::Cos(a) * 0.55f, static_cast<float>(i) / static_cast<float>(nombre) * 0.4f, Mathf::Sin(a) * 0.55f);
            float kh = Random::Range(0.6f, 1.1f);
            Color teinte = Teinte(col);
            float duree = Random::Range(0.9f, 1.4f);
            float taille = Random::Range(0.05f, 0.09f);
            Emettre(p + off, Vector3(0.0f, hauteur * kh, 0.0f), teinte, duree, taille, 0.0f, 3.2f);
        }
    }

    void Etincelles::Fumee(Vector3 p, Color col, int nombre, float rayon)
    {
        for (int i = 0; i < nombre; i++)
        {
            Vector3 d = Random::insideUnitSphere() * rayon; d.y = Mathf::Abs(d.y);
            float vy = 0.6f + Random::value() * 0.6f;
            Color teinte = Teinte(col);
            float duree = Random::Range(0.8f, 1.6f);
            float taille = Random::Range(0.12f, 0.26f);
            Emettre(p + d, Vector3(d.x, vy, d.z), teinte, duree, taille, -0.3f);
        }
    }

    /// Un jet d'un point à un autre (sorts lancés).
    void Etincelles::Trait(Vector3 de, Vector3 vers, Color col, int nombre)
    {
        for (int i = 0; i < nombre; i++)
        {
            float k = static_cast<float>(i) / static_cast<float>(nombre);
            Vector3 p = Vector3::Lerp(de, vers, k) + Random::insideUnitSphere() * 0.12f;
            Vector3 vel = Random::insideUnitSphere() * 0.4f;
            Color teinte = Teinte(col);
            float taille = Random::Range(0.05f, 0.1f);
            Emettre(p, vel, teinte, 0.35f + k * 0.5f, taille, 0.0f);
        }
    }

    Color Etincelles::Teinte(Color couleur)
    {
        float k = Random::Range(0.85f, 1.15f);
        return Color(Mathf::Clamp01(couleur.r * k), Mathf::Clamp01(couleur.g * k), Mathf::Clamp01(couleur.b * k), couleur.a);
    }

    // ------------------------------------------------------------------ animation

    void Etincelles::Update()
    {
        float dt = Time::deltaTime;
        v.Clear(); n.Clear(); c.Clear(); t.Clear();
        for (int i = ps.Count() - 1; i >= 0; i--)
        {
            P p = ps[i];
            p.vie += dt;
            if (p.vie >= p.duree) { ps.RemoveAt(i); continue; }
            p.vel += Vector3::down * p.gravite * dt;
            p.vel *= 1.0f - Mathf::Min(0.9f, dt * 1.2f);
            p.pos += p.vel * dt;
            if (p.tournoie > 0)
            {
                Vector3 r = p.pos - p.centre; r.y = 0;
                p.pos = p.centre + Quaternion::AngleAxis(p.tournoie * 60.0f * dt, Vector3::up) * r + Vector3::up * (p.pos.y - p.centre.y);
            }
            ps[i] = p;
            float k = p.vie / p.duree;
            float s = p.taille * (k < 0.15f ? k / 0.15f : 1.0f - (k - 0.15f) / 0.85f * 0.7f);
            Color32 col = p.c;
            if (additif)
            {
                float f = 1.0f - k;
                col = Color32(static_cast<uint8_t>(col.r * f), static_cast<uint8_t>(col.g * f), static_cast<uint8_t>(col.b * f), 255);
            }
            else col.a = static_cast<uint8_t>(col.a * (1.0f - k));
            Cube(p.pos, s, col);
        }
        mesh->Clear();
        if (v.Count() == 0) return;
        mesh->SetVertices(v); mesh->SetNormals(n); mesh->SetColors(c); mesh->SetTriangles(t, 0);
        mesh->RecalculateBounds();
    }

    void Etincelles::Cube(Vector3 p, float s, Color32 col)
    {
        int b = v.Count();
        float h = s * 0.5f;
        for (int i = 0; i < 8; i++) { v.Add(p + Coins[i] * h); n.Add(Coins[i]); c.Add(col); }
        for (int i = 0; i < static_cast<int>(std::size(Faces)); i++) t.Add(b + Faces[i]);
    }
}
