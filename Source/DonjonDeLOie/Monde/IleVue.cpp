// Donjon de l'Oie — l'île dans la scène : terrain, mer, dalles, décor, lieux, lampes, et les marqueurs du plateau
// (cases à choisir, pièges, embuscades, cible de quête).
#include "IleVue.h"

#include "../Modeles/Catalogue.h"
#include "../Voxel/Pal.h"
#include "Animateur.h"
#include "DDMateriaux.h"
#include "Formes.h"
#include "RigVue.h"

namespace DonjonDeLOie::Monde
{
    using namespace DonjonDeLOie::Regles;
    using namespace DonjonDeLOie::Modeles;
    using namespace DonjonDeLOie::Voxel;

    namespace
    {
        // Remise à zéro des variables statiques (une partie relancée dans Unreal repart comme un lancement d'Unity).
        const bool reinit = Boucle::AuReinitialiser([] { Vacillement::Facteur = 1.0f; });
    }

    Ref<IleVue> IleVue::Construire(const Ptr<IleModele>& m, const Ref<Transform>& parent)
    {
        auto go = Neuf<GameObject>("Île");
        go->transform()->SetParent(parent, false);
        auto iv = go->AddComponent<IleVue>();
        iv->Modele = m;
        iv->Batir();
        return iv;
    }

    Ref<GameObject> IleVue::Objet(const string& nom, const Ref<Transform>& parent, const Ref<Mesh>& mesh, const Ref<Material>& mat, bool ombres, bool recoit)
    {
        auto go = Neuf<GameObject>(nom);
        go->transform()->SetParent(parent, false);
        go->AddComponent<MeshFilter>()->sharedMesh = mesh;
        auto mr = go->AddComponent<MeshRenderer>();
        mr->sharedMaterial = mat;
        mr->shadowCastingMode = ombres ? ShadowCastingMode::On : ShadowCastingMode::Off;
        mr->receiveShadows = recoit;
        return go;
    }

    Ref<Mesh> IleVue::Propre(const Ref<Mesh>& m)
    {
        maillagesPropres.Add(m);
        return m;
    }

    void IleVue::OnDestroy()
    {
        for (auto& m : maillagesPropres)
        {
            if (m == nullptr) continue;
            if (Application::isPlaying) Destroy(m);
            else DestroyImmediate(m);
        }
        maillagesPropres.Clear();
    }

    void IleVue::Batir()
    {
        auto terrain = Neuf<GameObject>("Terrain")->transform();
        terrain->SetParent(transform(), false);
        for (int mz = 0; mz * IleModele::Morceau < Modele->NZ; mz++)
            for (int mx = 0; mx * IleModele::Morceau < Modele->NX; mx++)
            {
                auto md = Modele->MaillageTerrain(mx, mz);
                if (md->V.Count() == 0) continue;
                // (arguments de Objet évalués de gauche à droite comme en C# : le maillage, puis le matériau)
                auto mesh = Propre(DDMateriaux::Maillage(md, "terrain"));
                auto mat = DDMateriaux::Voxel();
                auto o = Objet("Terrain_" + Texte::De(mx) + "_" + Texte::De(mz), terrain, mesh, mat, true);
                o->isStatic = true;
            }
        auto meshMer = Propre(DDMateriaux::Maillage(Modele->MaillageMer(), "mer"));
        auto matMer = DDMateriaux::Eau();
        Objet("Mer", transform(), meshMer, matMer, false, false);

        // Dalles et figurines des cases
        auto cases = Neuf<GameObject>("Cases")->transform();
        cases->SetParent(transform(), false);
        Dico<TypeCase, Ref<Mesh>> maillagesDalles;
        for (int c = 0; c < Modele->Plateau->Nombre(); c++)
        {
            TypeCase t = (*Modele->Plateau)[c]->Type;
            Ref<Mesh> mesh;
            if (!maillagesDalles.TryGetValue(t, mesh))
            {
                mesh = Propre(DDMateriaux::Maillage(IleModele::MaillageDalle(t), "dalle_" + NomEnum(t)));
                maillagesDalles[t] = mesh;
            }
            auto d = Objet("Case_" + Texte::De(c) + "_" + NomEnum(t), cases, mesh, DDMateriaux::Voxel(), true);
            d->transform()->position(Modele->Cases[c]);
            Dalles.Add(d->transform());
            auto icone = RigVue::Creer(Catalogue::IconeCase(t), d->transform(), "icone_" + NomEnum(t), true, 2.2f);
            icone->transform()->localPosition = Vector3(-0.62f, 0.0f, 0.62f);
            icone->transform()->localRotation = Quaternion::Euler(0.0f, 200.0f, 0.0f);
            icone->transform()->localScale = Vector3::one * 0.72f;   // figurine de pion : ne pas cacher les héros
        }

        // Décor semé
        racineDecor = Neuf<GameObject>("Décor")->transform();
        racineDecor->SetParent(transform(), false);
        for (const auto& p : Modele->Decors)
        {
            int g = p.Graine % 6 + 1;
            auto rig = Catalogue::Decor(p.Id, g);
            auto rv = RigVue::Creer(rig, racineDecor, "decor_" + p.Id + "_" + Texte::De(g), true, 2.2f);
            Poser(rv->transform(), p);
            if (rv->Piece("feuillage") != nullptr) Animateur::Sur(rv);
        }
        // Lieux
        for (const auto& p : Modele->Lieux)
        {
            if (Texte::CommencePar(p.Id, "monstre:"))
            {
                string id = Texte::Sous(p.Id, 8);
                auto rv = RigVue::Creer(Catalogue::Monstre(id), racineDecor, "monstre_" + id, true, 2.6f);
                Poser(rv->transform(), p);
                auto an = Animateur::Sur(rv);
                an->Vitesse = 0.45f;
                if (Dragon == nullptr) Dragon = rv;      // le premier monstre posé est le boss de l'antre
                continue;
            }
            auto lr = RigVue::Creer(Catalogue::Decor(p.Id, p.Graine), racineDecor, "lieu_" + p.Id + "_" + Texte::De(p.Graine), true, 2.4f);
            Poser(lr->transform(), p);
            if (lr->Piece("feuillage") != nullptr) Animateur::Sur(lr);
        }
        // Lampes
        auto lampes = Neuf<GameObject>("Lampes")->transform();
        lampes->SetParent(transform(), false);
        for (const auto& l : Modele->Lampes)
        {
            auto go = Neuf<GameObject>("Lampe");
            go->transform()->SetParent(lampes, false);
            go->transform()->position(l.Pos);
            auto li = go->AddComponent<Light>();
            li->type = LightType::Point;
            li->color = l.Couleur;
            li->range = l.Portee;
            li->intensity = l.Intensite;
            li->shadows = LightShadows::None;
            go->AddComponent<Vacillement>()->Base = l.Intensite;
        }
        racineMarqueurs = Neuf<GameObject>("Marqueurs")->transform();
        racineMarqueurs->SetParent(transform(), false);
    }

    /// Le plus haut sommet du décor (ou du terrain) dans un rayon autour d'un point : le diorama du mini-jeu de
    /// l'évadé flotte au-dessus.
    float IleVue::SommetAutour(Vector3 p, float rayon) const
    {
        float h = Modele->HauteurEn(p.x, p.z);
        for (int k = 0; k < 16; k++)                                   // le relief : deux cercles de huit points
        {
            float a = static_cast<float>(k) * Mathf::Pi / 4.0f, d = k < 8 ? rayon * 0.5f : rayon;
            h = Mathf::Max(h, Modele->HauteurEn(p.x + Mathf::Cos(a) * d, p.z + Mathf::Sin(a) * d));
        }
        if (racineDecor == nullptr) return h;
        for (auto& t : racineDecor->Enfants())
        {
            Vector3 d = t->position() - p; d.y = 0;
            if (d.sqrMagnitude() > rayon * rayon) continue;
            auto rv = t->GetComponent<RigVue>();
            if (rv != nullptr) h = Mathf::Max(h, t->position().y + rv->Hauteur * t->lossyScale().y);
        }
        return h;
    }

    void IleVue::Poser(const Ref<Transform>& t, const Placement& p)
    {
        t->position(p.Pos);
        t->rotation(Quaternion::Euler(0.0f, p.RotY, 0.0f));
        t->localScale = Vector3::one * (p.Echelle <= 0 ? 1.0f : p.Echelle);
    }

    // ------------------------------------------------------------------ positions

    Vector3 IleVue::PositionCase(int c) const { return Modele->Cases[c]; }

    /// Place d'un pion sur une case partagée (jusqu'à six autour du centre).
    Vector3 IleVue::Emplacement(int c, int rang, int total) const
    {
        Vector3 p = Modele->Cases[c];
        if (total <= 1) return p;
        float a = (static_cast<float>(rang) / static_cast<float>(total)) * Mathf::Pi * 2.0f + 0.4f;
        float r = total <= 2 ? 0.38f : 0.52f;
        return p + Vector3(Mathf::Cos(a) * r, 0.0f, Mathf::Sin(a) * r);
    }

    // ------------------------------------------------------------------ marqueurs

    /// Allume des anneaux sur les cases proposées (choix d'embranchement, de piège, de case d'embuscade…).
    void IleVue::MontrerChoix(const Liste<int>& cases, Color c)
    {
        EffacerChoix();
        for (int k : cases)
        {
            if (k < 0) continue;
            auto go = Formes::Anneau(racineMarqueurs, 0.95f, 1.12f, c, DDMateriaux::FxDessus());
            go->transform()->position(Modele->Cases[k] + Vector3::up * 0.04f);
            go->AddComponent<Pulsation>();
            auto col = Formes::Colonne(go->transform(), 0.9f, 2.6f, Color(c.r, c.g, c.b, 0.35f));
            col->transform()->localPosition = Vector3::zero;
            choix.Add(go);
        }
    }

    void IleVue::EffacerChoix()
    {
        for (auto& g : choix)
            if (g != nullptr) Destroy(g);
        choix.Clear();
    }

    /// Met en valeur une case (survol d'une option liée à une case) ; -1 pour effacer.
    void IleVue::Survoler(int c)
    {
        if (c == caseSurvolee) return;
        caseSurvolee = c;
        if (survol != nullptr) Destroy(survol);
        survol = nullptr;
        if (c < 0 || c >= Modele->Cases.Count()) return;
        survol = Neuf<GameObject>("Survol");
        survol->transform()->SetParent(racineMarqueurs, false);
        survol->transform()->position(Modele->Cases[c]);
        Formes::Colonne(survol->transform(), 1.05f, 5.0f, Color(1.0f, 0.9f, 0.55f, 0.6f));
        auto a = Formes::Anneau(survol->transform(), 1.1f, 1.3f, Color(1.0f, 0.92f, 0.6f, 1.0f), DDMateriaux::FxDessus());
        a->transform()->localPosition = Vector3::up * 0.05f;
        a->AddComponent<Pulsation>();
    }

    /// Pièges : visibles seulement pour leur poseur (et pour un voleur).
    void IleVue::MontrerPieges(const Liste<Ptr<Piege>>& liste, int joueurQuiRegarde, bool voitTout)
    {
        Ensemble<int> vus;
        for (const auto& p : liste)
        {
            bool visible = voitTout || p->Proprietaire == joueurQuiRegarde;
            if (!visible) continue;
            vus.Add(p->Case);
            if (!pieges.ContainsKey(p->Case))
            {
                auto go = Neuf<GameObject>("Piège");
                go->transform()->SetParent(racineMarqueurs, false);
                go->transform()->position(Modele->Cases[p->Case] + Vector3(0.55f, 0.02f, -0.5f));
                Formes::Icone(go->transform(), "piege", 0.07f, true)->transform()->localPosition = Vector3(0.0f, 0.05f, 0.0f);
                auto an = Formes::Anneau(go->transform(), 0.18f, 0.3f, Color(1.0f, 0.35f, 0.25f, 0.9f), DDMateriaux::FxAdd());
                an->transform()->localPosition = Vector3::up * 0.02f;
                pieges[p->Case] = go;
            }
        }
        Liste<int> aEnlever;
        for (auto& kv : pieges)
            if (!vus.Contains(kv.Key)) aEnlever.Add(kv.Key);
        for (int k : aEnlever)
        {
            Destroy(pieges[k]);
            pieges.Remove(k);
        }
    }

    /// Embuscades : deux cases teintées de la couleur du joueur embusqué, visibles de tous.
    void IleVue::MontrerEmbuscades(const Liste<Ptr<Joueur>>& joueurs)
    {
        Ensemble<int> vus;
        for (const auto& j : joueurs)
        {
            if (j->Embuscade == nullptr) continue;
            vus.Add(j->Id);
            if (embuscades.ContainsKey(j->Id)) continue;
            auto go = Neuf<GameObject>("Embuscade_" + j->Nom);
            go->transform()->SetParent(racineMarqueurs, false);
            Color c = Pal::Joueurs[j->Couleur];
            for (int k : {j->Embuscade->Case, j->Embuscade->CaseVoisine})
            {
                auto a = Formes::Anneau(go->transform(), 0.8f, 1.02f, Color(c.r, c.g, c.b, 0.85f), DDMateriaux::FxAdd());
                a->transform()->position(Modele->Cases[k] + Vector3::up * 0.03f);
                auto ic = Formes::Icone(go->transform(), "oeil", 0.06f, false);
                ic->transform()->position(Modele->Cases[k] + Vector3::up * 1.9f);
                ic->AddComponent<Flotter>();
            }
            embuscades[j->Id] = go;
        }
        Liste<int> aEnlever;
        for (auto& kv : embuscades)
            if (!vus.Contains(kv.Key)) aEnlever.Add(kv.Key);
        for (int k : aEnlever)
        {
            Destroy(embuscades[k]);
            embuscades.Remove(k);
        }
    }

    /// Le lieu à atteindre de la quête du joueur qui joue (ou rien).
    void IleVue::MontrerQuete(int caseCible)
    {
        if (quete != nullptr) Destroy(quete);
        quete = nullptr;
        if (caseCible < 0) return;
        quete = Neuf<GameObject>("Quête");
        quete->transform()->SetParent(racineMarqueurs, false);
        quete->transform()->position(Modele->Cases[caseCible]);
        auto ic = Formes::Icone(quete->transform(), "parchemin", 0.08f, false);
        ic->transform()->localPosition = Vector3::up * 2.4f;
        ic->AddComponent<Flotter>();
        Formes::Colonne(quete->transform(), 0.5f, 3.2f, Color(1.0f, 0.85f, 0.4f, 0.28f));
    }

    // ------------------------------------------------------------------ Vacillement, Pulsation, Flotter

    void Vacillement::Start()
    {
        l = GetComponent<Light>();
        graine = Random::value() * 100.0f;
    }

    void Vacillement::Update()
    {
        if (l == nullptr) return;
        float t = Time::time * 3.1f + graine;
        l->intensity = Base * Facteur * (0.86f + 0.1f * Mathf::Sin(t) + 0.06f * Mathf::Sin(t * 2.7f + 1.3f));
    }

    void Pulsation::Start()
    {
        echelle = transform()->localScale;
        graine = Random::value() * 3.0f;
    }

    void Pulsation::Update() { transform()->localScale = echelle * (1.0f + 0.06f * Mathf::Sin(Time::time * 4.0f + graine)); }

    void Flotter::Start()
    {
        basePos = transform()->localPosition;
        graine = Random::value() * 3.0f;
    }

    void Flotter::Update()
    {
        transform()->localPosition = basePos + Vector3::up * (Mathf::Sin(Time::time * 2.0f + graine) * 0.12f);
        transform()->Rotate(0.0f, 50.0f * Time::deltaTime, 0.0f, Space::World);
    }
}
