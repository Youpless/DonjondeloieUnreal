// Donjon de l'Oie — le pion d'un joueur : son héros animé, un socle à sa couleur, ses déplacements de case en case.
#include "PionVue.h"

#include "../Modeles/Catalogue.h"
#include "../Voxel/Pal.h"
#include "DDMateriaux.h"
#include "Formes.h"

namespace DonjonDeLOie::Monde
{
    Ref<PionVue> PionVue::Creer(const Ptr<Regles::Joueur>& j, const Ref<Transform>& parent)
    {
        auto go = Neuf<GameObject>("Pion_" + j->Nom);
        go->transform()->SetParent(parent, false);
        auto pv = go->AddComponent<PionVue>();
        pv->Joueur = j->Id;
        pv->Couleur = Voxel::Pal::Joueurs[j->Couleur];
        pv->Rig = RigVue::Creer(Modeles::Catalogue::Heros(j->Classe, Voxel::Pal::Joueurs[j->Couleur]), go->transform(),
                                "heros_" + Regles::NomEnum(j->Classe) + "_" + Texte::De(j->Couleur), true, 2.6f);
        pv->Anim = Animateur::Sur(pv->Rig);
        auto c = pv->Couleur;
        pv->socle = Formes::Disque(go->transform(), 0.36f, Color(c.r, c.g, c.b, 0.55f), Color(c.r, c.g, c.b, 0.0f), DDMateriaux::FxAlpha());
        pv->socle->transform()->localPosition = Vector3::up * 0.015f;
        auto anneau = Formes::Anneau(go->transform(), 0.33f, 0.4f, Color(c.r, c.g, c.b, 0.95f), DDMateriaux::FxAlpha());
        anneau->transform()->localPosition = Vector3::up * 0.02f;
        pv->halo = Formes::Colonne(go->transform(), 0.42f, 1.8f, Color(c.r, c.g, c.b, 0.35f));
        pv->halo->SetActive(false);
        return pv;
    }

    /// Le joueur dont c'est le tour a une colonne de lumière à sa couleur.
    void PionVue::Actif(bool oui)
    {
        if (halo != nullptr) halo->SetActive(oui);
    }

    void PionVue::Regarder(Vector3 point)
    {
        auto d = point - transform()->position(); d.y = 0.0f;
        if (d.sqrMagnitude() < 1e-4f) return;
        regard = d.normalized();
    }

    void PionVue::Update()
    {
        auto cible = Quaternion::LookRotation(regard, Vector3::up);
        Rig->transform()->rotation(Quaternion::Slerp(Rig->transform()->rotation(), cible, Time::deltaTime * 10.0f));
    }

    /// Saut d'une case à l'autre (arc), en durée fixe.
    Tache PionVue::Sauter(Vector3 vers, float duree, float hauteur)
    {
        auto de = transform()->position();
        Regarder(vers);
        Anim->EnMarche = true;
        float t = 0.0f;
        while (t < duree)
        {
            t += Time::deltaTime;
            float k = Mathf::Clamp01(t / duree);
            float e = k * k * (3.0f - 2.0f * k);
            auto p = Vector3::Lerp(de, vers, e);
            p.y += Mathf::Sin(k * Mathf::Pi) * hauteur;
            transform()->position(p);
            co_yield nullptr;
        }
        transform()->position(vers);
        Anim->EnMarche = false;
    }

    /// Glisser doucement vers une place (quand plusieurs pions se partagent une case).
    Tache PionVue::Glisser(Vector3 vers, float duree)
    {
        auto de = transform()->position();
        float t = 0.0f;
        while (t < duree)
        {
            t += Time::deltaTime;
            transform()->position(Vector3::Lerp(de, vers, Mathf::SmoothStep(0.0f, 1.0f, t / duree)));
            co_yield nullptr;
        }
        transform()->position(vers);
    }

    /// Disparaître dans un éclair puis réapparaître ailleurs.
    Tache PionVue::Teleporter(Vector3 vers)
    {
        float t = 0.0f;
        auto e0 = Rig->transform()->localScale;
        while (t < 0.35f) { t += Time::deltaTime; Rig->transform()->localScale = Vector3(e0.x * (1.0f - t / 0.35f), e0.y * (1.0f + t / 0.35f), e0.z * (1.0f - t / 0.35f)); co_yield nullptr; }
        transform()->position(vers);
        t = 0.0f;
        while (t < 0.35f) { t += Time::deltaTime; float k = t / 0.35f; Rig->transform()->localScale = Vector3(e0.x * k, e0.y * (2.0f - k), e0.z * k); co_yield nullptr; }
        Rig->transform()->localScale = e0;
    }
}
