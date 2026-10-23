// Donjon de l'Oie — formes simples pour les effets et les marqueurs : anneau, colonne de lumière, disque, icône en voxels.
#include "Formes.h"

#include "../Voxel/PixelIcones.h"
#include "../Voxel/VoxelMesher.h"
#include "DDMateriaux.h"

namespace DonjonDeLOie::Monde
{
    using namespace DonjonDeLOie::Voxel;

    namespace
    {
        /// ((Color32)c).GetHashCode() de C# : sous Mono (moteur des essais), un Color32 sans GetHashCode propre se
        /// hache en K ^ r ^ g ^ b ^ a, où K change d'un lancement à l'autre (il vient du type). La clé ne sert qu'au
        /// cache : seules comptent ses égalités, que r ^ g ^ b ^ a donne à l'identique (mêmes maillages partagés).
        int HacheCouleur32(Color32 k) { return k.r ^ k.g ^ k.b ^ k.a; }
    }

    Ref<GameObject> Formes::Objet(const string& nom, const Ref<Transform>& parent, const Ref<Mesh>& m, const Ref<Material>& mat)
    {
        auto go = Neuf<GameObject>(nom);
        go->transform()->SetParent(parent, false);
        go->AddComponent<MeshFilter>()->sharedMesh = m;
        auto mr = go->AddComponent<MeshRenderer>();
        mr->sharedMaterial = mat;
        mr->shadowCastingMode = ShadowCastingMode::Off;
        mr->receiveShadows = false;
        return go;
    }

    /// Anneau à plat (dans le plan xz), en couleur de sommets.
    Ref<GameObject> Formes::Anneau(const Ref<Transform>& parent, float r0, float r1, Color c, const Ref<Material>& mat)
    {
        Color32 kc = c;                                                 // ((Color32)c)
        string cle = "anneau_" + Texte::De(r0) + "_" + Texte::De(r1) + "_" + Texte::De(static_cast<int>(kc.r)) + "_" + Texte::De(static_cast<int>(kc.g)) + "_" +
                     Texte::De(static_cast<int>(kc.b)) + "_" + Texte::De(static_cast<int>(kc.a));
        Ref<Mesh> mc;
        if (cache.TryGetValue(cle, mc) && mc != nullptr) return Objet("Anneau", parent, mc, mat);
        auto md = Nouveau<MeshData>();
        const int n = 48;
        Color32 cc = c;
        for (int i = 0; i <= n; i++)
        {
            float a = static_cast<float>(i) / static_cast<float>(n) * Mathf::Pi * 2.0f;
            Vector3 d(Mathf::Cos(a), 0.0f, Mathf::Sin(a));
            md->V.Add(d * r0); md->V.Add(d * r1);
            md->N.Add(Vector3::up); md->N.Add(Vector3::up);
            md->C.Add(cc); md->C.Add(cc);
            if (i < n)
            {
                int k = i * 2;
                md->T.Add(k); md->T.Add(k + 1); md->T.Add(k + 3);
                md->T.Add(k); md->T.Add(k + 3); md->T.Add(k + 2);
            }
        }
        mc = DDMateriaux::Maillage(md, cle);
        cache[cle] = mc;
        return Objet("Anneau", parent, mc, mat);
    }

    /// Disque plein (ombre portée sous un pion, socle).
    Ref<GameObject> Formes::Disque(const Ref<Transform>& parent, float r, Color centre, Color bord, const Ref<Material>& mat)
    {
        Color32 k0 = centre, k1 = bord;
        string cle = "disque_" + Texte::De(r) + "_" + Texte::De(static_cast<int>(k0.r)) + "," + Texte::De(static_cast<int>(k0.g)) + "," + Texte::De(static_cast<int>(k0.b)) + "," +
                     Texte::De(static_cast<int>(k0.a)) + "_" + Texte::De(static_cast<int>(k1.r)) + "," + Texte::De(static_cast<int>(k1.g)) + "," + Texte::De(static_cast<int>(k1.b)) +
                     "," + Texte::De(static_cast<int>(k1.a));
        Ref<Mesh> mc;
        if (cache.TryGetValue(cle, mc) && mc != nullptr) return Objet("Disque", parent, mc, mat);
        auto md = Nouveau<MeshData>();
        const int n = 40;
        md->V.Add(Vector3::zero); md->N.Add(Vector3::up); md->C.Add(Color32(centre));
        for (int i = 0; i <= n; i++)
        {
            float a = static_cast<float>(i) / static_cast<float>(n) * Mathf::Pi * 2.0f;
            md->V.Add(Vector3(Mathf::Cos(a) * r, 0.0f, Mathf::Sin(a) * r)); md->N.Add(Vector3::up); md->C.Add(Color32(bord));
            if (i < n) { md->T.Add(0); md->T.Add(i + 2); md->T.Add(i + 1); }
        }
        mc = DDMateriaux::Maillage(md, cle);
        cache[cle] = mc;
        return Objet("Disque", parent, mc, mat);
    }

    /// Colonne de lumière qui s'efface vers le haut (additive).
    Ref<GameObject> Formes::Colonne(const Ref<Transform>& parent, float r, float h, Color c)
    {
        string cle = "colonne_" + Texte::De(r) + "_" + Texte::De(h) + "_" + Texte::De(HacheCouleur32(Color32(c)));
        Ref<Mesh> m;
        if (!cache.TryGetValue(cle, m) || m == nullptr)
        {
            auto md = Nouveau<MeshData>();
            const int n = 24;
            Color32 bas = c, haut = Color(c.r, c.g, c.b, 0.0f);
            bas = Color(c.r * c.a, c.g * c.a, c.b * c.a, c.a);
            haut = Color(0.0f, 0.0f, 0.0f, 0.0f);
            for (int i = 0; i <= n; i++)
            {
                float a = static_cast<float>(i) / static_cast<float>(n) * Mathf::Pi * 2.0f;
                Vector3 d(Mathf::Cos(a), 0.0f, Mathf::Sin(a));
                md->V.Add(d * r); md->V.Add(d * r * 0.85f + Vector3::up * h);
                md->N.Add(d); md->N.Add(d);
                md->C.Add(bas); md->C.Add(haut);
                if (i < n)
                {
                    int k = i * 2;
                    md->T.Add(k); md->T.Add(k + 1); md->T.Add(k + 3);
                    md->T.Add(k); md->T.Add(k + 3); md->T.Add(k + 2);
                }
            }
            m = DDMateriaux::Maillage(md, cle);
            cache[cle] = m;
        }
        return Objet("Colonne", parent, m, DDMateriaux::FxAdd());
    }

    /// Une icône en pixel art, extrudée en voxels (taille : mètres par pixel). debout : face vers −z ; sinon à plat.
    Ref<GameObject> Formes::Icone(const Ref<Transform>& parent, const string& id, float taille, bool aPlat)
    {
        string cle = "icone_" + id + "_" + Texte::De(taille) + (aPlat ? "_plat" : "");
        Ref<Mesh> m;
        if (!cache.TryGetValue(cle, m) || m == nullptr)
        {
            VoxelModel vm;
            int n = PixelIcones::Taille, o = n / 2;
            for (int x = 0; x < n; x++)
                for (int y = 0; y < n; y++)
                {
                    Color32 c = PixelIcones::Pixel(id, x, y);
                    if (c.a == 0) continue;
                    if (aPlat) vm.Set(x - o, 0, y - o, c);
                    else { vm.Set(x - o, y, 0, c); vm.Set(x - o, y, 1, Pal::Shade(c, 0.7f)); }
                }
            m = DDMateriaux::Maillage(VoxelMesher::Build(vm, Vector3(0.5f, aPlat ? 0.0f : static_cast<float>(n) * 0.5f, 0.5f), taille, 0.0f, false), cle);
            cache[cle] = m;
        }
        auto go = Objet("Icône_" + id, parent, m, DDMateriaux::Lueur(1.15f, 0.0f));
        return go;
    }

    // Remise à zéro des variables statiques (une partie relancée dans Unreal repart comme un lancement d'Unity).
    const bool Formes::remiseAZero = Boucle::AuReinitialiser([] { Formes::cache.Clear(); });
}
