// Donjon de l'Oie — les matériaux, créés au lancement à partir des shaders du dossier Resources.
// Si un shader n'est pas pris en charge (autre pipeline, erreur de compilation), un shader d'Unity prend le relais.
#include "DDMateriaux.h"

namespace DonjonDeLOie::Monde
{
    using namespace DonjonDeLOie::Voxel;

    Ref<Shader> DDMateriaux::Trouver(const Liste<string>& noms)
    {
        for (auto& n : noms)
        {
            auto s = Shader::Find(n);
            if (s != nullptr && s->isSupported) return s;
        }
        Debug::LogWarning("Donjon de l'Oie : aucun des shaders " + Texte::Joindre(", ", noms) + " n'est disponible.");
        return Shader::Find("Sprites/Default");
    }

    Ref<Material> DDMateriaux::Voxel()
    {
        if (voxel == nullptr)
        {
            voxel = Neuf<Material>(Trouver({"DonjonDeLOie/Voxel", "Universal Render Pipeline/Particles/Simple Lit", "Particles/Standard Surface", "Sprites/Default"}));
            voxel->name("DD_Voxel");
            voxel->enableInstancing = true;
        }
        return voxel;
    }

    /// Matériau lumineux. intensite : 1 = couleur telle quelle, 2–4 = rayonne avec le halo. pulsation : vitesse du battement (0 = fixe).
    Ref<Material> DDMateriaux::Lueur(float intensite, float pulsation)
    {
        int cle = Mathf::RoundToInt(intensite * 100) * 1000 + Mathf::RoundToInt(pulsation * 100);
        Ref<Material> m;
        if (lueurs.TryGetValue(cle, m) && m != nullptr) return m;
        m = Neuf<Material>(Trouver({"DonjonDeLOie/Lueur", "Universal Render Pipeline/Particles/Unlit", "Sprites/Default"}));
        m->name("DD_Lueur_" + Texte::De(cle));
        m->SetFloat("_Intensite", intensite);
        m->SetFloat("_Pulsation", pulsation);
        m->enableInstancing = true;
        lueurs[cle] = m;
        return m;
    }

    Ref<Material> DDMateriaux::Fx(bool additif, bool pardessus)
    {
        auto m = Neuf<Material>(Trouver({"DonjonDeLOie/Fx", "Sprites/Default"}));
        m->SetFloat("_SrcBlend", additif ? static_cast<float>(BlendMode::One) : static_cast<float>(BlendMode::SrcAlpha));
        m->SetFloat("_DstBlend", additif ? static_cast<float>(BlendMode::One) : static_cast<float>(BlendMode::OneMinusSrcAlpha));
        m->SetFloat("_ZTest", pardessus ? static_cast<float>(CompareFunction::Always) : static_cast<float>(CompareFunction::LessEqual));
        m->SetFloat("_Additif", additif ? 1.0f : 0.0f);
        m->renderQueue = pardessus ? 3100 : 3000;
        return m;
    }

    /// Effets additifs (étincelles, halos).
    Ref<Material> DDMateriaux::FxAdd()
    {
        if (fxAdd == nullptr)
        {
            fxAdd = Fx(true, false);
            fxAdd->name("DD_FxAdd");
        }
        return fxAdd;
    }

    /// Effets en transparence (fumée, ombres, anneaux pleins).
    Ref<Material> DDMateriaux::FxAlpha()
    {
        if (fxAlpha == nullptr)
        {
            fxAlpha = Fx(false, false);
            fxAlpha->name("DD_FxAlpha");
        }
        return fxAlpha;
    }

    /// Marqueurs toujours visibles, même derrière le décor (cases à choisir).
    Ref<Material> DDMateriaux::FxDessus()
    {
        if (fxDessus == nullptr)
        {
            fxDessus = Fx(true, true);
            fxDessus->name("DD_FxDessus");
        }
        return fxDessus;
    }

    Ref<Material> DDMateriaux::Eau()
    {
        if (eau == nullptr)
        {
            eau = Fx(false, false);
            eau->name("DD_Eau");
            eau->SetFloat("_Vague", 0.16f);
            eau->renderQueue = 2950;
        }
        return eau;
    }

    /// Maillage Unity à partir d'un MeshData (index 32 bits au-delà de 65 000 sommets).
    Ref<Mesh> DDMateriaux::Maillage(const Ptr<MeshData>& md, const string& nom)
    {
        auto m = Neuf<Mesh>();
        m->name(nom);
        if (md->V.Count() > 65000) m->indexFormat = IndexFormat::UInt32;
        m->SetVertices(md->V);
        m->SetNormals(md->N);
        m->SetColors(md->C);
        m->SetTriangles(md->T, 0);
        m->RecalculateBounds();
        return m;
    }

    /// Maillage d'une pièce de rig, gardé en cache par clé (les mêmes modèles ne sont maillés qu'une fois).
    Ref<Mesh> DDMateriaux::MaillagePiece(const string& cle, const Ptr<VoxelPart>& p, float taille)
    {
        Ref<Mesh> m;
        if (!cle.empty() && cache.TryGetValue(cle, m) && m != nullptr) return m;
        m = Maillage(VoxelMesher::Build(*p->Model, p->Pivot, taille), !cle.empty() ? cle : p->Name);
        if (!cle.empty()) cache[cle] = m;
        return m;
    }

    // Remise à zéro des variables statiques (une partie relancée dans Unreal repart comme un lancement d'Unity).
    const bool DDMateriaux::remiseAZero = Boucle::AuReinitialiser([] {
        DDMateriaux::voxel = nullptr;
        DDMateriaux::fxAdd = nullptr;
        DDMateriaux::fxAlpha = nullptr;
        DDMateriaux::eau = nullptr;
        DDMateriaux::fxDessus = nullptr;
        DDMateriaux::lueurs.Clear();
        DDMateriaux::cache.Clear();
    });
}
