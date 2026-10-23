// Donjon de l'Oie — un VoxelRig posé dans la scène : une hiérarchie d'objets, une pièce par objet, pivots respectés.
#include "RigVue.h"

#include "DDMateriaux.h"

namespace DonjonDeLOie::Monde
{
    Ref<RigVue> RigVue::Creer(const Ptr<Voxel::VoxelRig>& rig, const Ref<Transform>& parent, const string& cle, bool ombres, float intensiteLueur)
    {
        auto go = Neuf<GameObject>(Texte::Ou(rig->Name, "rig"));
        go->transform()->SetParent(parent, false);
        auto rv = go->AddComponent<RigVue>();
        rv->Rig = rig;
        Liste<Ptr<Voxel::VoxelPart>> restants(rig->Parts);
        int garde = 0;
        float haut = 0.0f;
        while (restants.Count() > 0 && garde++ < 64)
        {
            for (int i = restants.Count() - 1; i >= 0; i--)
            {
                auto p = restants[i];
                Ref<Transform> tp; Vector3 pivotParent;
                if (p->Parent.empty() || rig->Get(p->Parent) == nullptr) { tp = go->transform(); pivotParent = rig->Origin; }
                else if (rv->Pieces.ContainsKey(p->Parent)) { tp = rv->Pieces.At(p->Parent); pivotParent = rig->Get(p->Parent)->Pivot; }
                else continue;
                auto pg = Neuf<GameObject>(p->Name);
                pg->transform()->SetParent(tp, false);
                pg->transform()->localPosition = (p->Pivot - pivotParent) * rig->VoxelSize;
                rv->Pieces[p->Name] = pg->transform();
                rv->Repos[p->Name] = Quaternion::identity;
                rv->PositionsRepos[p->Name] = pg->transform()->localPosition;
                if (p->Model->Count() > 0)
                {
                    auto mf = pg->AddComponent<MeshFilter>();
                    mf->sharedMesh = DDMateriaux::MaillagePiece(!cle.empty() ? cle + "/" + p->Name : string(), p, rig->VoxelSize);
                    auto mr = pg->AddComponent<MeshRenderer>();
                    bool lueur = Texte::CommencePar(p->Name, "lueur");
                    mr->sharedMaterial = lueur ? DDMateriaux::Lueur(intensiteLueur, 1.6f) : DDMateriaux::Voxel();
                    mr->shadowCastingMode = (ombres && !lueur) ? ShadowCastingMode::On : ShadowCastingMode::Off;
                    mr->receiveShadows = !lueur;
                    rv->Rendus.Add(mr);
                    Vector3Int a, b;
                    if (p->Model->Bounds(a, b)) haut = Mathf::Max(haut, (static_cast<float>(b.y + 1) - rig->Origin.y) * rig->VoxelSize);
                }
                restants.RemoveAt(i);
            }
        }
        rv->Hauteur = haut;
        return rv;
    }

    Ref<Transform> RigVue::Piece(const string& nom) const
    {
        Ref<Transform> t;
        return Pieces.TryGetValue(nom, t) ? t : Ref<Transform>();
    }

    void RigVue::Appliquer()
    {
        if (bloc == nullptr) bloc = Nouveau<MaterialPropertyBlock>();
        for (auto& r : Rendus)
        {
            if (r == nullptr || r->sharedMaterial != DDMateriaux::Voxel()) continue;
            r->GetPropertyBlock(*bloc);
            bloc->SetColor("_BaseColor", teinte);
            bloc->SetColor("_EmissionColor", emission);
            r->SetPropertyBlock(*bloc);
        }
    }

    void RigVue::Teinte(Color c)
    {
        teinte = c;
        Appliquer();
    }

    void RigVue::Emission(Color c)
    {
        emission = c;
        Appliquer();
    }

    /// Éclair de couleur (coup reçu, soin, niveau) qui s'éteint en « duree » secondes.
    void RigVue::Eclair(Color c, float duree)
    {
        if (!gameObject()->activeInHierarchy()) return;
        if (eclairCo != nullptr) StopCoroutine(eclairCo);
        eclairCo = StartCoroutine(EclairCo(c, duree));
    }

    Tache RigVue::EclairCo(Color c, float duree)
    {
        float t = 0.0f;
        while (t < duree)
        {
            t += Time::deltaTime;
            Emission(c * (1.0f - t / duree));
            co_yield nullptr;
        }
        Emission(Color::black);
    }

    void RigVue::Visible(bool v)
    {
        for (auto& r : Rendus)
            if (r != nullptr) r->enabled = v;
    }
}
