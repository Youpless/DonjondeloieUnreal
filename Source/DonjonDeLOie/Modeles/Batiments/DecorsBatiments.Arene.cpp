// Donjon de l'Oie — l'arène du diable : une île de basalte qui flotte dans la nuit, un cercle de lave, huit piliers
// d'obsidienne coiffés de braseros et deux grandes cornes à l'entrée. 1 voxel = 10 cm, origine au centre du sol de
// l'arène (y = 0 : le dessus des dalles), l'entrée vers −z.
#include "BatimentsInterne.h"

namespace DonjonDeLOie::Modeles::DecorsModeles::Batiments
{
    using namespace DonjonDeLOie::Voxel;

    namespace
    {
        constexpr Color32 ArBasalte = Pal::C("2b2630"), ArBasalteClair = Pal::C("3b3442"), ArDalle = Pal::C("443a46"),
            ArDalleSombre = Pal::C("352d38"), ArObsidienne = Pal::C("1f1726"), ArObsidienneReflet = Pal::C("4a3a5e"),
            ArFer = Pal::C("3a3633"), ArCorne = Pal::C("d9cdb0"), ArCorneSombre = Pal::C("9c8f74"), ArBraise = Pal::C("ff6a1a"),
            ArLave = Pal::C("ff4a12"), ArLaveClaire = Pal::C("ffb43a");
    }

    Ptr<VoxelRig> AreneDuDiable(int graine)
    {
        AleaNet R(graine);
        auto socle = Nouveau<VoxelModel>();
        auto lave = Nouveau<VoxelModel>();
        auto feu = Nouveau<VoxelModel>();
        const int Rayon = 38;
        // le sol de dalles (deux voxels d'épaisseur), et le cercle de lave
        for (int x = -Rayon; x <= Rayon; x++)
            for (int z = -Rayon; z <= Rayon; z++)
            {
                float d = Mathf::Sqrt(static_cast<float>(x * x + z * z));
                if (d > Rayon + 0.5f) continue;
                bool joint = ((x + 100) % 6 == 0) || ((z + 100) % 6 == 0);
                Color32 dalle = joint ? ArDalleSombre : Pal::Var(ArDalle, x, 0, z, 0.08f);
                socle->Set(x, -1, z, dalle);
                socle->Set(x, -2, z, ArBasalte);
                if (d > 25.2f && d < 27.4f)
                {
                    socle->Remove(x, -1, z);
                    lave->Set(x, -1, z, Pal::Var(ArLave, x, 1, z, 0.12f));
                }
                else if (d > Rayon - 1.5f) socle->Set(x, 0, z, ArBasalteClair);               // margelle
            }
        // quelques fissures de lave qui rayonnent du centre
        for (int k = 0; k < 6; k++)
        {
            float a = k * Mathf::Pi / 3.0f + static_cast<float>(R.NextDouble()) * 0.4f;
            for (float t = 6.0f; t < 24.0f; t += 0.5f)
            {
                int x = Mathf::RoundToInt(Mathf::Cos(a) * t + Mathf::Sin(t * 0.7f) * 1.2f), z = Mathf::RoundToInt(Mathf::Sin(a) * t + Mathf::Cos(t * 0.6f) * 1.2f);
                if (!socle->Has(x, -1, z)) continue;
                socle->Remove(x, -1, z);
                lave->Set(x, -1, z, t < 14.0f ? ArLaveClaire : ArLave);
            }
        }
        // le dessous : un rocher qui s'effile, comme arraché à la terre
        for (int y = -3; y >= -34; y--)
        {
            float k = (-y - 2) / 32.0f;
            float r = Rayon * Mathf::Pow(1.0f - k, 0.75f);
            for (int x = -Rayon; x <= Rayon; x++)
                for (int z = -Rayon; z <= Rayon; z++)
                {
                    float bruit = Mathf::Sin(x * 0.45f + y * 0.3f) * 1.6f + Mathf::Cos(z * 0.38f - y * 0.27f) * 1.6f;
                    if (x * x + z * z > (r + bruit) * (r + bruit)) continue;
                    bool braise = ((x * 7 + z * 13 + y * 5) & 63) == 0 && y < -6;
                    if (braise) lave->Set(x, y, z, ArBraise);
                    else socle->Set(x, y, z, ((x + z + y) & 3) == 0 ? ArBasalteClair : ArBasalte);
                }
        }
        // huit piliers d'obsidienne et leurs braseros (l'entrée, vers −z, reste libre)
        for (int i = 0; i < 8; i++)
        {
            float a = (i + 0.5f) * Mathf::Pi / 4.0f;
            int px = Mathf::RoundToInt(Mathf::Sin(a) * 33.0f), pz = Mathf::RoundToInt(Mathf::Cos(a) * 33.0f);
            if (pz < -24 && Mathf::Abs(px) < 14) continue;
            socle->Box(px - 2, 0, pz - 2, px + 2, 1, pz + 2, ArBasalteClair);
            socle->Box(px - 1, 2, pz - 1, px + 1, 20, pz + 1, ArObsidienne);
            for (int y = 4; y <= 18; y += 5) socle->Set(px + 1, y, pz + 1, ArObsidienneReflet);
            socle->Box(px - 2, 21, pz - 2, px + 2, 21, pz + 2, ArFer);
            socle->Box(px - 2, 22, pz - 2, px + 2, 23, pz + 2, ArFer);
            socle->Box(px - 1, 22, pz - 1, px + 1, 23, pz + 1, ArFer);
            feu->Box(px - 1, 23, pz - 1, px + 1, 24, pz + 1, ArBraise);
            feu->Set(px, 25, pz, ArLaveClaire);
            feu->Set(px, 26, pz, ArLaveClaire);
            feu->Set(px + (i % 2 == 0 ? 1 : -1), 25, pz, ArLave);
        }
        // l'entrée : deux grandes cornes qui se recourbent l'une vers l'autre
        for (int s : {-1, 1})
        {
            for (int k = 0; k <= 40; k++)
            {
                float t = k / 40.0f;
                float x = s * (14.0f - 9.0f * t * t), y = 1.0f + 34.0f * Mathf::Sin(t * 1.35f), z = -34.0f + 6.0f * t;
                float r = Mathf::Lerp(3.2f, 0.6f, t);
                Color32 c = (k / 4) % 2 == 0 ? ArCorne : ArCorneSombre;
                socle->Line(x, y, z, x, y + 1, z, r, c);
            }
        }
        auto rig = Nouveau<VoxelRig>();
        rig->Name = "arene_diable";
        rig->VoxelSize = 0.1f;
        rig->Origin = Vector3::zero;
        rig->Add("socle", socle, Vector3::zero, "");   // (parent null : racine)
        rig->Add("lueur_lave", lave, Vector3::zero, "socle");
        rig->Add("lueur_feu", feu, Vector3::zero, "socle");
        return rig;
    }
}
