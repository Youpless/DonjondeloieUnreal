// Donjon de l'Oie — le marché de Valcreux en diorama, pour le mini-jeu de l'évadé : un socle de rue pavée, quatre
// maisons, trois étals et leurs marchands, une vingtaine d'habitants (dont deux ou trois sosies de l'évadé) et l'évadé
// lui-même, caché derrière des caisses, des tonneaux ou des sacs : seule sa tête dépasse. Il se bâtit une pièce par
// image pendant que la caméra zoome, puis surgit pièce après pièce (la vie du marché : Diorama.Vie.cpp).
// Repère : origine au centre du sol, la rue le long de x, les maisons au fond (z < 0) ; la caméra le regarde de
// l'avant-droite (LacetVue), d'en haut (TangageVue).
#include "Diorama.h"

#include "../Modeles/Catalogue.h"

#include <cstdlib>
#include <iterator>

namespace DonjonDeLOie::Monde
{
    /// La direction de la caméra, à l'horizontale, dans le repère du diorama.
    const Vector3 Diorama::VersCamera = Vector3(Mathf::Sin(LacetVue * Mathf::Deg2Rad), 0.0f, Mathf::Cos(LacetVue * Mathf::Deg2Rad));

    const bool Diorama::remiseAZero = Boucle::AuReinitialiser([] { actif_ = nullptr; });

    /// Le milieu de la tête, dans le monde.
    Vector3 Diorama::Habitant::Tete() const { return Rig->transform()->TransformPoint(Vector3(-0.025f, 0.99f, 0.0f)); }

    Ref<Diorama> Diorama::Creer(const Ref<Transform>& parent, Vector3 position, float lacet, int graine, const Ptr<Modeles::Allure>& evade)
    {
        auto go = Neuf<GameObject>("Diorama du marché");
        go->transform()->SetParent(parent, false);
        go->transform()->position(position);
        go->transform()->rotation(Quaternion::Euler(0.0f, lacet, 0.0f));
        auto d = go->AddComponent<Diorama>();
        d->r = Nouveau<AleaNet>(static_cast<int>(static_cast<uint32_t>(graine) * 13u + 5u));      // graine * 13 + 5 (débordement comme en C#)
        d->allure = evade;
        actif_ = d;
        return d;
    }

    void Diorama::OnDestroy()
    {
        if (actif_.get() == this) actif_ = nullptr;
        for (auto& m : propres)
            if (m != nullptr) Destroy(m);
        propres.Clear();
    }

    /// La caméra qui cadre le diorama : orientation fixe (LacetVue, TangageVue sur son repère) ; distance et cible
    /// cherchées pour que l'emprise tienne, perspective comprise, dans la fenêtre libre de l'écran : à droite de l'avis
    /// de recherche (« marge » : part de la largeur), sous le sablier (« haut ») et au-dessus de la consigne (« bas »).
    void Diorama::Cadrage(const Ref<Transform>& d, float champ, float aspect, float marge, float haut, float bas,
                          Vector3& cible, float& distance, float& lacet)
    {
        lacet = d->eulerAngles().y + 180.0f + LacetVue;
        auto q = Quaternion::Euler(TangageVue, lacet, 0.0f);
        Vector3 av = q * Vector3::forward, dr = q * Vector3::right, ht = q * Vector3::up;
        float ty = Mathf::Tan(champ * 0.5f * Mathf::Deg2Rad), tx = ty * aspect;
        float g = -1.0f + 2.0f * marge + 0.03f, dx = 0.97f, b = -1.0f + 2.0f * bas + 0.02f, hh = 1.0f - 2.0f * haut - 0.02f;
        const int n = static_cast<int>(std::size(Emprise));
        Liste<Vector3> pts(n);
        cible = Vector3::zero;
        for (int i = 0; i < pts.Count(); i++)
        {
            pts[i] = d->TransformPoint(Emprise[i]);
            cible += pts[i] / static_cast<float>(pts.Count());
        }
        distance = 40.0f;
        for (int it = 0; it < 14; it++)
        {
            auto oeil = cible - av * distance;
            float x0 = 1e9f, x1 = -1e9f, y0 = 1e9f, y1 = -1e9f;
            for (auto& p : pts)
            {
                auto v = p - oeil;
                float z = Mathf::Max(0.1f, Vector3::Dot(v, av));
                float sx = Vector3::Dot(v, dr) / (z * tx), sy = Vector3::Dot(v, ht) / (z * ty);
                x0 = Mathf::Min(x0, sx); x1 = Mathf::Max(x1, sx); y0 = Mathf::Min(y0, sy); y1 = Mathf::Max(y1, sy);
            }
            cible += dr * (((x0 + x1) - (g + dx)) * 0.5f * distance * tx) + ht * (((y0 + y1) - (b + hh)) * 0.5f * distance * ty);
            distance *= Mathf::Max((x1 - x0) / (dx - g), (y1 - y0) / (hh - b));
        }
    }

    // ------------------------------------------------------------------ la construction

    /// Bâtit le diorama, une pièce par image (« parImage » faux : tout d'un coup). Tout reste caché jusqu'à Apparaitre().
    Tache Diorama::Batir(bool parImage)
    {
        Ajouter(RigVue::Creer(Modeles::DecorsModeles::PlateauMarche(104, 80, -15), transform(), "marche_plateau", true, 2.2f), Vector3::zero, 0.0f, 0.0f, 0);
        if (parImage) co_yield nullptr;
        int gm = r->Next(6);
        for (int i = 0; i < static_cast<int>(std::size(MaisonsX)); i++)
        {
            int g = 101 + (gm + i) % 6;
            Ajouter(RigVue::Creer(Modeles::DecorsModeles::Maison(g, 12, false), transform(), "marche_maison_" + Texte::De(g), true, 2.2f), Vector3(MaisonsX[i], 0.0f, MaisonsZ), 0.0f, 0.12f + 0.09f * static_cast<float>(i), 1);
            if (parImage) co_yield nullptr;
        }
        int ge = r->Next(5);
        for (int i = 0; i < static_cast<int>(std::size(EtalsX)); i++)
        {
            int g = 200 + (ge + i) % 5;
            Ajouter(RigVue::Creer(Modeles::Catalogue::Decor("etal_marche", g), transform(), "marche_etal_" + Texte::De(g), true, 2.2f), Vector3(EtalsX[i], 0.0f, EtalsZ), 0.0f, 0.5f + 0.08f * static_cast<float>(i), 1);
            if (parImage) co_yield nullptr;
        }
        Ajouter(RigVue::Creer(Modeles::Catalogue::Decor("puits", 3), transform(), "decor_puits_3", true, 2.2f), Puits, 15.0f, 0.78f, 1);
        Ajouter(RigVue::Creer(Modeles::Catalogue::Decor("charrette_foin", 2), transform(), "marche_charrette", true, 2.2f), Charrette, 90.0f, 0.84f, 0);
        for (int i = 0; i < static_cast<int>(std::size(Cachettes)); i++)
        {
            auto pos = Cachettes[i] + VersCamera * (Recul + DemiAbris[i]);
            Ajouter(RigVue::Creer(Modeles::Catalogue::Decor(Abris[i], 5 + i), transform(), string("marche_") + Abris[i] + "_" + Texte::De(5 + i), true, 2.2f), pos, LacetVue, 0.9f + 0.05f * static_cast<float>(i), 0);
        }
        if (parImage) co_yield nullptr;
        // l'avis de recherche, tourné vers la caméra
        auto ev = Modeles::HerosModeles::Habitant(*allure);
        portrait_ = Modeles::HerosModeles::Portrait(*ev);
        auto avis = RigVue::Creer(Modeles::DecorsModeles::PanneauAvis(portrait_), transform(), "", true, 2.2f);
        Propres(avis);
        Ajouter(avis, Avis, LacetVue - 8.0f, 1.06f, 1);
        if (parImage) co_yield nullptr;
        // la foule
        for (bool s : Peupler(ev))
            if (parImage && s) co_yield nullptr;
        pret_ = true;
    }

    void Diorama::Ajouter(const Ref<RigVue>& rv, Vector3 pos, float lacet, float delai, int genre)
    {
        auto t = rv->transform();
        t->localPosition = pos;
        t->localRotation = Quaternion::Euler(0.0f, lacet, 0.0f);
        auto p = Nouveau<Piece>();
        p->T = t; p->Echelle = t->localScale; p->Position = pos; p->Delai = delai; p->Genre = genre;
        pieces.Add(p);
        t->gameObject()->SetActive(false);
    }

    void Diorama::Propres(const Ref<RigVue>& rv)
    {
        for (auto& r0 : rv->Rendus)
        {
            auto mf = r0 != nullptr ? r0->GetComponent<MeshFilter>() : Ref<MeshFilter>();
            if (mf != nullptr && mf->sharedMesh != nullptr) propres.Add(mf->sharedMesh);
        }
    }

    Ptr<Diorama::Habitant> Diorama::Placer(const Ptr<Modeles::Allure>& a, const Ptr<Voxel::VoxelRig>& rig, Vector3 pos, float lacet, float delai)
    {
        auto rv = RigVue::Creer(rig != nullptr ? rig : Modeles::HerosModeles::Habitant(*a), transform(), "", true, 2.2f);
        rv->name("Habitant");
        Propres(rv);
        auto h = Nouveau<Habitant>();
        h->Rig = rv; h->Allure = a; h->Lacet = lacet;
        h->Anim = Animateur::Sur(rv);
        h->Anim->Vitesse = 0.75f + 0.5f * static_cast<float>(r->NextDouble());
        Foule.Add(h);
        Ajouter(rv, pos, lacet, delai, 2);
        return h;
    }

    float Diorama::Hasard(float a, float b) { return a + (b - a) * static_cast<float>(r->NextDouble()); }

    /// Un visage qu'on ne confond pas avec celui de l'évadé : au moins trois pixels de différence sur l'avis, à partir
    /// de la ligne « depuis » (4 : du nez aux cheveux, ce qui dépasse d'une cachette).
    bool Diorama::Distinct(const Ptr<Voxel::VoxelRig>& rig, int depuis)
    {
        auto p = Modeles::HerosModeles::Portrait(*rig);
        int n = 0;
        for (int i = depuis * 11; i < p.Count(); i++)
        {
            Color32 a = p[i], b = portrait_[i];
            if (a.a != b.a || std::abs(static_cast<int>(a.r) - static_cast<int>(b.r)) + std::abs(static_cast<int>(a.g) - static_cast<int>(b.g)) + std::abs(static_cast<int>(a.b) - static_cast<int>(b.b)) > 40) n++;
        }
        return n >= 3;
    }

    /// Une allure tirée au hasard, ou un sosie de l'évadé, et son modèle — toujours un visage distinct du sien (pour
    /// qui se cache : distinct dans ce qui dépasse de la cachette).
    Ptr<Modeles::Allure> Diorama::Tirer(bool sosie, bool cache, Ptr<Voxel::VoxelRig>& rig)
    {
        for (int k = 0; k < 40; k++)
        {
            auto a = sosie ? allure->Sosie(*r) : Modeles::Allure::Hasard(*r);
            rig = Modeles::HerosModeles::Habitant(*a);
            if (Distinct(rig, cache ? 4 : 0)) return a;
        }
        auto b = Modeles::Allure::Hasard(*r);
        b->Peau = (allure->Peau + 1 + r->Next(Modeles::Allure::NPeaux - 1)) % Modeles::Allure::NPeaux;
        rig = Modeles::HerosModeles::Habitant(*b);
        return b;
    }

    Ptr<Diorama::Habitant> Diorama::Gens(bool sosie, Vector3 pos, float lacet, float delai, bool cache)
    {
        Ptr<Voxel::VoxelRig> rig;
        auto a = Tirer(sosie, cache, rig);
        auto h = Placer(a, rig, pos, lacet, delai);
        h->Sosie = sosie;
        return h;
    }

    /// La foule : marchands, chalands, passants, badauds, quelqu'un qui lit l'avis, un ou deux autres cachés, et
    /// l'évadé. Chaque habitant bâti rend « vrai » (une image de pause possible).
    Gen<bool> Diorama::Peupler(Ptr<Voxel::VoxelRig> rigEvade)
    {
        float d = 1.2f;
        // l'évadé dans une cachette, un ou deux autres (dont peut-être un sosie) dans d'autres
        Liste<int> cachettes = {0, 1, 2, 3};
        int ce = cachettes[r->Next(cachettes.Count())]; cachettes.Remove(ce);
        evade_ = Placer(allure, rigEvade, Cachettes[ce] + Vector3::down * Accroupis[ce], LacetVue + Hasard(-14.0f, 14.0f), d + 0.28f);
        evade_->Evade = true; evade_->Cache = true; evade_->Cachette = ce;
        evade_->Rig->name("Évadé");
        co_yield true;
        int nCaches = 1 + r->Next(2);
        for (int k = 0; k < nCaches && cachettes.Count() > 0; k++)
        {
            int c = cachettes[r->Next(cachettes.Count())]; cachettes.Remove(c);
            // (deux tirages dans les arguments : dans l'ordre du C#, le sosie puis le lacet)
            bool sosieCache = k == 0 && r->Next(2) == 0;
            float lacetCache = LacetVue + Hasard(-18.0f, 18.0f);
            auto h = Gens(sosieCache, Cachettes[c] + Vector3::down * Accroupis[c], lacetCache, d + 0.3f + 0.25f * static_cast<float>(k), true);
            h->Cache = true; h->Cachette = c;
            co_yield true;
        }
        // deux sosies parmi les visages bien visibles : un marchand, et un badaud
        int marchandSosie = r->Next(static_cast<int>(std::size(EtalsX)));
        for (int i = 0; i < static_cast<int>(std::size(EtalsX)); i++)
        {
            float xMarchand = EtalsX[i] + Hasard(-0.3f, 0.3f);
            float lacetMarchand = Hasard(-12.0f, 12.0f);
            Gens(i == marchandSosie, Vector3(xMarchand, 0.0f, EtalsZ - 0.35f), lacetMarchand, d + 0.05f * static_cast<float>(i));
            co_yield true;
        }
        int badaudSosie = r->Next(2);
        Gens(badaudSosie == 0, Vector3(-3.2f, 0.0f, 2.45f), Hasard(45.0f, 70.0f), d + 0.2f);                // près du puits, ils causent
        Gens(false, Vector3(-2.55f, 0.0f, 2.25f), Hasard(-110.0f, -80.0f), d + 0.22f);
        Gens(badaudSosie == 1, Vector3(-0.95f, 0.0f, 2.55f), Hasard(10.0f, 35.0f), d + 0.26f);           // adossé au puits
        co_yield true;
        // les chalands devant les étals, de dos ; l'un d'eux de profil
        for (int i = 0; i < static_cast<int>(std::size(EtalsX)); i++)
        {
            for (int k = 0; k < 2; k++)
            {
                float xChaland = EtalsX[i] + (k == 0 ? -0.45f : 0.4f) + Hasard(-0.1f, 0.1f);
                float zChaland = Hasard(0.05f, 0.25f);
                float lacetChaland = 180.0f + Hasard(-25.0f, 25.0f);
                Gens(false, Vector3(xChaland, 0.0f, zChaland), lacetChaland, d + 0.35f + 0.04f * static_cast<float>(i * 2 + k));
            }
            co_yield true;
        }
        Gens(false, Vector3(EtalsX[1] + 0.95f, 0.0f, 0.35f), Hasard(-100.0f, -75.0f), d + 0.6f);
        // quelqu'un lit l'avis de recherche
        Gens(false, Avis - VersCamera * 0.75f + Vector3(-0.2f, 0.0f, 0.0f), LacetVue + 180.0f + Hasard(-15.0f, 15.0f), d + 0.62f);
        co_yield true;
        // les passants
        const float allees[] = {Allee1, Allee1, Allee2, Allee2};
        int nPassants = 3 + r->Next(2);
        for (int k = 0; k < nPassants; k++)
        {
            float z = allees[k] + Hasard(-0.08f, 0.08f);
            auto a = Vector3(-4.4f + Hasard(0.0f, 0.6f), 0.0f, z); auto b = Vector3(4.4f - Hasard(0.0f, 0.6f), 0.0f, z);
            float phase = Hasard(0.0f, 2.0f);
            auto h = Gens(false, PositionPassant(a, b, phase), Mathf::Repeat(phase, 2.0f) < 1.0f ? 90.0f : -90.0f, d + 0.66f + 0.04f * static_cast<float>(k));
            h->Marche = true; h->A = a; h->B = b; h->Phase = phase;
            h->Vitesse = Hasard(0.42f, 0.62f);
            co_yield true;
        }
    }
}
