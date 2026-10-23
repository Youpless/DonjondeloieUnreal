// Donjon de l'Oie — la vie du marché en diorama : il surgit pièce après pièce, les passants vont et viennent, les
// marchands haranguent, l'évadé guette en tournant la tête ; un clic sur une tête dit qui est là. À la fin, l'évadé
// est démasqué (il bondit hors de sa cachette) ou s'enfuit en courant hors du diorama, qui se replie.
#include "Diorama.h"

#include <cfloat>

namespace DonjonDeLOie::Monde
{
    Vector3 Diorama::PositionPassant(Vector3 a, Vector3 b, float phase) { return Vector3::Lerp(a, b, Mathf::PingPong(phase, 1.0f)); }

    // ------------------------------------------------------------------ apparaître, disparaître

    /// Le diorama surgit : le socle bondit, les maisons et les étals poussent, les habitants tombent du ciel.
    /// « pop » est appelé au surgissement de chaque pièce (sa position dans le monde), pour un son ou un nuage ;
    /// « vitesse » accélère le tout (les bots rapides).
    Tache Diorama::Apparaitre(std::function<void(Vector3)> pop, float vitesse)
    {
        float fin = 0.0f;
        for (auto& p : pieces) fin = Mathf::Max(fin, p->Delai + DureePiece);
        Liste<bool> vus(pieces.Count());
        float t = 0.0f;
        while (t < fin)
        {
            t += Time::deltaTime * vitesse;
            for (int i = 0; i < pieces.Count(); i++)
            {
                auto p = pieces[i];
                if (p->T == nullptr || t < p->Delai) continue;
                if (!vus[i])
                {
                    vus[i] = true;
                    p->T->gameObject()->SetActive(true);
                    if (pop) pop(p->T->position());
                }
                Poser(p, Mathf::Clamp01((t - p->Delai) / DureePiece));
            }
            co_yield nullptr;
        }
        Montrer();
    }

    /// Tout en place d'un coup (essais hors Unity, ou partie qui se termine pendant l'animation).
    void Diorama::Montrer()
    {
        for (auto& p : pieces)
            if (p->T != nullptr)
            {
                p->T->gameObject()->SetActive(true);
                Poser(p, 1.0f);
            }
        vivant = true;
    }

    float Diorama::Rebond(float k)
    {
        constexpr float c1 = 1.70158f, c3 = c1 + 1.0f;
        float x = k - 1.0f;
        return 1.0f + c3 * x * x * x + c1 * x * x;
    }

    void Diorama::Poser(const Ptr<Piece>& p, float k)
    {
        switch (p->Genre)
        {
            case 1:
                p->T->localScale = Vector3(p->Echelle.x, p->Echelle.y * Mathf::Max(0.01f, Rebond(k)), p->Echelle.z);
                break;
            case 2:
            {
                float h = 1.0f - k;
                p->T->localPosition = p->Position + Vector3::up * (1.4f * h * h);
                p->T->localScale = p->Echelle * Mathf::Lerp(0.5f, 1.0f, Mathf::Min(1.0f, k * 2.0f));
                break;
            }
            default:
                p->T->localScale = p->Echelle * Mathf::Max(0.01f, Rebond(k));
                break;
        }
    }

    /// Le diorama se replie sur lui-même en tournant, puis s'éteint.
    Tache Diorama::Disparaitre(float duree)
    {
        vivant = false;
        auto e0 = transform()->localScale;
        auto r0 = transform()->rotation();
        float t = 0.0f;
        while (t < duree)
        {
            t += Time::deltaTime;
            float k = Mathf::Clamp01(t / duree);
            transform()->localScale = e0 * Mathf::Max(0.001f, 1.0f - k * k);
            transform()->rotation(r0 * Quaternion::Euler(0.0f, 50.0f * k * k, 0.0f));
            co_yield nullptr;
        }
        gameObject()->SetActive(false);
    }

    // ------------------------------------------------------------------ la vie du marché

    void Diorama::Update()
    {
        if (!vivant) return;
        float dt = Time::deltaTime;
        temps += dt;
        for (auto& h : Foule)
        {
            if (h->Rig == nullptr) continue;
            auto tr = h->Rig->transform();
            if (h->Marche)
            {
                h->Phase += dt * h->Vitesse / Mathf::Max(0.5f, Vector3::Distance(h->A, h->B));
                tr->localPosition = PositionPassant(h->A, h->B, h->Phase);
                float cap = Mathf::Repeat(h->Phase, 2.0f) < 1.0f ? 90.0f : -90.0f;
                tr->localRotation = Quaternion::Slerp(tr->localRotation, Quaternion::Euler(0.0f, cap, 0.0f), Mathf::Min(1.0f, dt * 5.0f));
                h->Anim->EnMarche = true;
            }
            else if (h->Cache && !fini)
                tr->localRotation = Quaternion::Euler(0.0f, h->Lacet + Mathf::Sin(temps * 0.8f + h->Anim->Graine) * 12.0f, 0.0f);   // il guette
        }
        prochainGeste -= dt;
        if (prochainGeste <= 0.0f && Foule.Count() > 0)
        {
            prochainGeste = 0.7f + 2.0f * static_cast<float>(r->NextDouble());
            auto h = Foule[r->Next(Foule.Count())];
            if (!h->Cache && !h->Marche && h->Anim != nullptr && !h->Anim->EnGeste())
            {
                bool marchand = h->Rig->transform()->localPosition.z < EtalsZ;
                h->Anim->Jouer(marchand ? (r->Next(2) == 0 ? Geste::Sort : Geste::Lancer) : (r->Next(3) == 0 ? Geste::Joie : Geste::Lancer), 1.1f);
            }
        }
    }

    // ------------------------------------------------------------------ le clic

    /// L'habitant dont la tête est sous ce point de l'écran (le plus proche de la caméra), ou null.
    Ptr<Diorama::Habitant> Diorama::Toucher(Vector2 souris, const Ref<Camera>& cam) const
    {
        Ptr<Habitant> best; float bz = FLT_MAX;
        for (auto& h : Foule)
        {
            if (h->Rig == nullptr || !h->Rig->gameObject()->activeInHierarchy()) continue;
            auto c = h->Tete();
            auto e = cam->WorldToScreenPoint(c);
            if (e.z <= 0.0f) continue;
            auto e2 = cam->WorldToScreenPoint(c + cam->transform()->right() * (0.24f * transform()->lossyScale().x));
            float rayon = Mathf::Max(10.0f, Mathf::Abs(e2.x - e.x));
            if ((Vector2(e.x, e.y) - souris).sqrMagnitude() > rayon * rayon) continue;
            if (e.z < bz) { bz = e.z; best = h; }
        }
        return best;
    }

    /// Un habitant au hasard dont on voit le visage (la loupe d'un bot qui cherche), sauf l'évadé.
    Ptr<Diorama::Habitant> Diorama::Badaud()
    {
        for (int k = 0; k < 20; k++)
        {
            auto h = Foule[r->Next(Foule.Count())];
            if (!h->Evade && !h->Marche && Vector3::Angle(h->Rig->transform()->forward(), transform()->rotation() * VersCamera) < 80.0f) return h;
        }
        return Foule[0] == evade_ && Foule.Count() > 1 ? Foule[1] : Foule[0];
    }

    /// Un sosie de l'évadé (ou n'importe qui), pour l'erreur d'un bot.
    Ptr<Diorama::Habitant> Diorama::UnSosie()
    {
        for (auto& h : Foule)
            if (h->Sosie) return h;
        return Badaud();
    }

    // ------------------------------------------------------------------ la fin

    /// Démasqué : l'évadé bondit par-dessus sa cachette et atterrit devant, penaud.
    Tache Diorama::Attrape()
    {
        fini = true;
        auto h = evade_;
        auto tr = h->Rig->transform();
        auto depart = tr->localPosition;
        auto arrivee = depart + VersCamera * (Recul + 2.0f * DemiAbris[h->Cachette] + 0.42f);
        auto r0 = tr->localRotation;
        h->Anim->Jouer(Geste::Touche, 0.5f);
        float t = 0.0f;
        while (t < 0.75f)
        {
            t += Time::deltaTime;
            float k = Mathf::Clamp01(t / 0.75f);
            tr->localPosition = Vector3::Lerp(depart, arrivee, k) + Vector3::up * (1.15f * Mathf::Sin(k * Mathf::Pi));
            tr->localRotation = r0 * Quaternion::Euler(0.0f, 360.0f * Mathf::SmoothStep(0.0f, 1.0f, k), 0.0f);
            co_yield nullptr;
        }
        tr->localPosition = arrivee;
        tr->localRotation = Quaternion::Euler(0.0f, LacetVue, 0.0f);
        h->Anim->Jouer(Geste::Ko);
    }

    /// Raté : l'évadé saute dans la rue, file vers le bord le plus proche et saute dans le vide.
    Tache Diorama::Fuite()
    {
        fini = true;
        auto h = evade_;
        auto tr = h->Rig->transform();
        auto depart = tr->localPosition;
        auto rue = Vector3(depart.x, 0.0f, 1.15f);
        float cote = depart.x < 0.0f ? -1.0f : 1.0f;
        float t = 0.0f;
        while (t < 0.55f)
        {
            t += Time::deltaTime;
            float k = Mathf::Clamp01(t / 0.55f);
            tr->localPosition = Vector3::Lerp(depart, rue, k) + Vector3::up * (1.05f * Mathf::Sin(k * Mathf::Pi));
            co_yield nullptr;
        }
        tr->localRotation = Quaternion::Euler(0.0f, cote * 90.0f, 0.0f);
        h->Anim->EnMarche = true;
        h->Anim->Vitesse = 2.4f;
        auto pos = rue;
        while (Mathf::Abs(pos.x) < 5.4f)
        {
            pos.x += cote * 2.6f * Time::deltaTime;
            tr->localPosition = pos;
            co_yield nullptr;
        }
        // il saute dans le vide et rapetisse
        auto e0 = tr->localScale;
        t = 0.0f;
        while (t < 0.5f)
        {
            t += Time::deltaTime;
            float k = Mathf::Clamp01(t / 0.5f);
            tr->localPosition = pos + Vector3(cote * 0.8f * k, 0.5f * Mathf::Sin(k * Mathf::Pi) - 2.2f * k * k, 0.0f);
            tr->localScale = e0 * (1.0f - k);
            co_yield nullptr;
        }
        h->Rig->gameObject()->SetActive(false);
    }
}
