// Donjon de l'Oie — un dé en voxels lancé sur le plateau : il roule, rebondit et s'arrête sur la valeur tirée par les règles.
// Le D6 est ivoire à points rouges ; le D3 des duels est doré, ses faces portent 1, 2 et 3 points (deux fois chacune).
#include "DeVue.h"

#include "../Voxel/Pal.h"
#include "../Voxel/VoxelMesher.h"
#include "DDMateriaux.h"

namespace DonjonDeLOie::Monde
{
    using namespace DonjonDeLOie::Voxel;

    Ref<Mesh> DeVue::Maillage(int nbFaces)
    {
        if (nbFaces == 3 && d3 != nullptr) return d3;
        if (nbFaces != 3 && d6 != nullptr) return d6;
        Color32 corpsC = nbFaces == 3 ? Pal::C("e9bf55") : Pal::C("f1e9d6");
        Color32 bordC = nbFaces == 3 ? Pal::C("c9962f") : Pal::C("d9cfb8");
        Color32 pointC = nbFaces == 3 ? Pal::C("4a2a10") : Pal::C("b3241c");
        VoxelModel m;
        for (int x = 0; x < N; x++)
            for (int y = 0; y < N; y++)
                for (int z = 0; z < N; z++)
                {
                    int bx = (x == 0 || x == N - 1) ? 1 : 0, by = (y == 0 || y == N - 1) ? 1 : 0, bz = (z == 0 || z == N - 1) ? 1 : 0;
                    if (bx + by + bz >= 2) continue;     // arêtes et coins arrondis
                    bool surface = bx + by + bz == 1;
                    m.Set(x, y, z, surface ? corpsC : bordC);
                }
        const int* valeurs = nbFaces == 3 ? ValeursD3 : ValeursD6;
        for (int f = 0; f < 6; f++) Points(m, Normales[f], valeurs[f], pointC);
        auto md = VoxelMesher::Build(m, Vector3(static_cast<float>(N) / 2.0f, static_cast<float>(N) / 2.0f, static_cast<float>(N) / 2.0f), Taille, 0.015f, true);
        auto mesh = DDMateriaux::Maillage(md, "de" + Texte::De(nbFaces));
        if (nbFaces == 3) d3 = mesh;
        else d6 = mesh;
        return mesh;
    }

    /// Creuse les points d'une face (points de 2 × 2 voxels, sur une grille 3 × 3).
    void DeVue::Points(VoxelModel& m, Vector3 n, int v, Color32 c)
    {
        Liste<Vector2> pos;
        // positions sur la face, de −1 à 1
        switch (v)
        {
            case 1: pos.Add(Vector2(0.0f, 0.0f)); break;
            case 2: pos.Add(Vector2(-1.0f, -1.0f)); pos.Add(Vector2(1.0f, 1.0f)); break;
            case 3: pos.Add(Vector2(-1.0f, -1.0f)); pos.Add(Vector2(0.0f, 0.0f)); pos.Add(Vector2(1.0f, 1.0f)); break;
            case 4: pos.Add(Vector2(-1.0f, -1.0f)); pos.Add(Vector2(1.0f, 1.0f)); pos.Add(Vector2(-1.0f, 1.0f)); pos.Add(Vector2(1.0f, -1.0f)); break;
            case 5:
                pos.Add(Vector2(-1.0f, -1.0f)); pos.Add(Vector2(1.0f, 1.0f)); pos.Add(Vector2(-1.0f, 1.0f)); pos.Add(Vector2(1.0f, -1.0f)); pos.Add(Vector2(0.0f, 0.0f));
                break;
            default:
                pos.Add(Vector2(-1.0f, -1.0f)); pos.Add(Vector2(1.0f, 1.0f)); pos.Add(Vector2(-1.0f, 1.0f)); pos.Add(Vector2(1.0f, -1.0f)); pos.Add(Vector2(-1.0f, 0.0f));
                pos.Add(Vector2(1.0f, 0.0f));
                break;
        }
        for (const auto& p : pos)
        {
            // centre du point : 5 ± 3 voxels ; un point = 2 × 2 voxels creusés d'un cran
            int u0 = 4 + static_cast<int>(p.x) * 3, w0 = 4 + static_cast<int>(p.y) * 3;
            for (int du = 0; du < 2; du++)
                for (int dw = 0; dw < 2; dw++)
                {
                    int u = u0 + du, w = w0 + dw;
                    int x = 0, y = 0, z = 0, x2 = 0, y2 = 0, z2 = 0;
                    if (n == Vector3::up) { x = u; z = w; y = N - 1; x2 = x; y2 = y - 1; z2 = z; }
                    else if (n == Vector3::down) { x = u; z = w; y = 0; x2 = x; y2 = 1; z2 = z; }
                    else if (n == Vector3::right) { z = u; y = w; x = N - 1; x2 = x - 1; y2 = y; z2 = z; }
                    else if (n == Vector3::left) { z = u; y = w; x = 0; x2 = 1; y2 = y; z2 = z; }
                    else if (n == Vector3::forward) { x = u; y = w; z = N - 1; x2 = x; y2 = y; z2 = z - 1; }
                    else { x = u; y = w; z = 0; x2 = x; y2 = y; z2 = 1; }
                    m.Remove(x, y, z);
                    m.Set(x2, y2, z2, c);
                }
        }
    }

    Vector3 DeVue::NormalePour(int nbFaces, int valeur)
    {
        const int* valeurs = nbFaces == 3 ? ValeursD3 : ValeursD6;
        for (int f = 0; f < 6; f++)
            if (valeurs[f] == valeur) return Normales[f];
        return Vector3::up;
    }

    Ref<DeVue> DeVue::Creer(const Ref<Transform>& parent, int nbFaces)
    {
        auto go = Neuf<GameObject>("Dé");
        go->transform()->SetParent(parent, false);
        // C# : go.AddComponent<MeshFilter>().sharedMesh = Maillage(faces) — C# évalue la gauche d'abord (le
        // MeshFilter, puis le maillage) ; en C++ une affectation évalue la droite d'abord : deux lignes.
        auto mf = go->AddComponent<MeshFilter>();
        mf->sharedMesh = Maillage(nbFaces);
        auto mr = go->AddComponent<MeshRenderer>();
        mr->sharedMaterial = DDMateriaux::Voxel();
        mr->shadowCastingMode = ShadowCastingMode::On;
        auto dv = go->AddComponent<DeVue>();
        dv->faces = nbFaces;
        return dv;
    }

    /// Lance le dé depuis « depart » jusqu'à « arrivee » (sol) et le fait s'arrêter sur « valeur ».
    Tache DeVue::Lancer(Vector3 depart, Vector3 arrivee, int valeur, float duree)
    {
        fini = false;
        Quaternion finale = Quaternion::AngleAxis(Random::Range(0.0f, 360.0f), Vector3::up) * Quaternion::FromToRotation(NormalePour(faces, valeur), Vector3::up);
        Vector3 axe = Random::onUnitSphere();
        if (axe.sqrMagnitude() < 0.01f) axe = Vector3::right;
        float tours = Random::Range(2.5f, 3.5f) * 360.0f;
        Quaternion rot0 = Random::rotation();
        float t = 0;
        float demi = static_cast<float>(N) * Taille * 0.5f;
        while (t < duree)
        {
            t += Time::deltaTime;
            float k = Mathf::Clamp01(t / duree);
            // trajectoire : arc puis deux rebonds amortis
            Vector3 p = Vector3::Lerp(depart, arrivee, 1.0f - (1.0f - k) * (1.0f - k));
            float h;
            if (k < 0.55f) h = Mathf::Lerp(depart.y - arrivee.y, 0.0f, k / 0.55f) + Mathf::Sin(k / 0.55f * Mathf::Pi) * 0.9f;
            else if (k < 0.8f) h = Mathf::Sin((k - 0.55f) / 0.25f * Mathf::Pi) * 0.32f;
            else h = Mathf::Sin((k - 0.8f) / 0.2f * Mathf::Pi) * 0.08f;
            p.y = arrivee.y + demi + Mathf::Max(0.0f, h);
            transform()->position(p);
            // rotation : la culbute ralentit puis rejoint la face tirée
            Quaternion culbute = Quaternion::AngleAxis(tours * (1.0f - (1.0f - k) * (1.0f - k)), axe) * rot0;
            transform()->rotation(Quaternion::Slerp(culbute, finale, Mathf::SmoothStep(0.0f, 1.0f, (k - 0.55f) / 0.45f)));
            co_yield nullptr;
        }
        transform()->rotation(finale);
        transform()->position(Vector3(arrivee.x, arrivee.y + demi, arrivee.z));
        fini = true;
    }

    // Remise à zéro des variables statiques (une partie relancée dans Unreal repart comme un lancement d'Unity).
    const bool DeVue::remiseAZero = Boucle::AuReinitialiser([] {
        DeVue::d6 = nullptr;
        DeVue::d3 = nullptr;
    });
}
