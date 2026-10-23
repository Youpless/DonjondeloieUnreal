// Donjon de l'Oie — Ignathor (suite) : pattes, ailes à demi repliées, queue enroulée.
#include "MonstresInterne.h"

namespace DonjonDeLOie::Modeles::MonstresModeles
{
    using namespace DonjonDeLOie::Voxel;

    namespace
    {
        void Griffes(VoxelModel& m, float px, float pz, float dirZ, int n, float ecart)
        {
            for (int t = 0; t < n; t++)
            {
                float fx = px + (t - (n - 1) * 0.5f) * ecart;
                float bx = fx + (t - (n - 1) * 0.5f) * 0.5f;
                m.Line(fx, 1.5f, pz, bx, 1, pz + dirZ * 3.5f, 1.2f, 0.9f, DrRouge);
                m.Line(bx, 1.2f, pz + dirZ * 3.5f, bx, 0.5f, pz + dirZ * 5.5f, 0.8f, 0.4f, DrCorne);
            }
        }
    }

    void AjouterPattesDragon(const Ptr<VoxelRig>& r)
    {
        for (int s = 0; s < 2; s++)
        {
            float sx = s == 0 ? -1.0f : 1.0f;
            // Avant : épaule, avant-bras droit et puissant, main large à quatre griffes
            auto av = Nouveau<VoxelModel>();
            Chaine(*av, {sx * 8.5f, 16, 7, sx * 12.5f, 10.5f, 7.5f, sx * 12.5f, 5, 11, sx * 12.5f, 2, 12}, 4.4f, 2.6f, DrRouge);
            BoiteC(*av, Mathf::FloorToInt(sx * 12.5f) - 3, 0, 10, Mathf::FloorToInt(sx * 12.5f) + 3, 2, 15, DrRouge, 1);
            Griffes(*av, sx * 12.5f + 0.5f, 14, 1, 4, 1.6f);
            av->Line(sx * 13, 10, 5.5f, sx * 14, 11, 2.5f, 0.9f, 0.4f, DrCorne);            // ergot de coude
            PlaquesDragon(*av);
            r->Add(s == 0 ? "pattesAvG" : "pattesAvD", av, V(sx * 8.5f, 16, 7), "corps");

            // Arrière : cuisse énorme, jarret, pied allongé
            auto ar = Nouveau<VoxelModel>();
            ar->Ellipsoid(sx * 9, 13, -8, 4, 6, 6.5f, DrRouge);
            Chaine(*ar, {sx * 10, 11, -5, sx * 12, 7, -1.5f, sx * 12, 4, -9, sx * 12, 1.5f, -6.5f}, 3.8f, 2.4f, DrRouge);
            BoiteC(*ar, Mathf::FloorToInt(sx * 12) - 3, 0, -9, Mathf::FloorToInt(sx * 12) + 3, 2, -4, DrRouge, 1);
            Griffes(*ar, sx * 12 + 0.5f, -4.5f, 1, 3, 2);
            PlaquesDragon(*ar);
            r->Add(s == 0 ? "pattesArG" : "pattesArD", ar, V(sx * 9, 14, -8), "corps");
        }
    }

    void AjouterAilesDragon(const Ptr<VoxelRig>& r)
    {
        // Ailes à demi repliées : le coude se dresse au-dessus du dos, le poignet est en arrière,
        // les doigts redescendent vers le sol et la membrane drape les flancs.
        Liste<Vector3> doigts = {V(27, 47, -17), V(26, 34, -25), V(21, 22, -24), V(15, 15, -15)};
        Liste<Vector3> flanc = {V(9.6f, 15, -9), V(9.4f, 18, -1)};
        for (int s = 0; s < 2; s++)
        {
            float sx = s == 0 ? -1.0f : 1.0f;
            auto a = Aile(sx, V(6.5f, 21, 5), V(13, 34, 3), V(18, 46, -5), doigts, flanc, 1.8f, DrRougeS, DrMembrane, DrMembraneS, DrCorne);
            // la membrane près des os d'un rouge plus sombre, le bord d'attaque en plaques
            // griffe au coude
            Vector3 co = V(18 * sx, 46, -5);
            a->Line(co.x, co.y, co.z, co.x + sx * 0.8f, co.y + 3.5f, co.z + 1.2f, 1.1f, 0.4f, DrCorne);
            r->Add(s == 0 ? "aileG" : "aileD", a, V(sx * 6.5f, 21, 5), "corps");
        }
    }

    void AjouterQueueDragon(const Ptr<VoxelRig>& r)
    {
        // Queue : part de la croupe, descend au sol puis s'enroule vers la droite et revient vers l'avant.
        auto q = Nouveau<VoxelModel>();
        Liste<float> pts = {0, 13.5f, -14, 0, 10.5f, -20, 1.5f, 7, -26, 5, 5, -31, 11, 4.5f, -33.5f, 17, 4, -31, 20.5f, 4, -25, 21.5f, 4, -18, 20, 4.5f, -12.5f};
        Chaine(*q, pts, 5.5f, 1.2f, DrRouge);
        PlaquesDragon(*q);
        // ventre doré de la queue (dessous)
        q->Paint([&](int x, int y, int z, Color32 c) { return (Meme(c, DrRouge) || Meme(c, DrRougeS) || Meme(c, DrRougeC)) && !q->Has(x, y - 1, z) && y <= 4 + (z < -20 ? 0 : 6) ? DrOr : c; });
        // crête dorsale qui continue sur la queue
        int n = pts.Length() / 3;
        for (int i = 0; i < n - 1; i++)
        {
            for (int k = 0; k < 2; k++)
            {
                float t = k / 2.0f;
                float x = Lerp(pts[i * 3], pts[i * 3 + 3], t), z = Lerp(pts[i * 3 + 2], pts[i * 3 + 5], t);
                int h = Haut(*q, Mathf::FloorToInt(x), Mathf::FloorToInt(z), 0, 40);
                if (h == INT_MIN) continue;
                float hh = 3.2f - (i + t) * 0.32f;
                if (hh < 1) hh = 1;
                q->Line(x, h + 0.5f, z, x, h + 0.5f + hh, z - 0.8f, 0.9f, 0.4f, DrOr);
            }
        }
        // pointe : fer de lance d'os
        Vector3 b = V(20, 4.5f, -12.5f);
        for (int k = 0; k < 4; k++) q->Line(b.x, b.y, b.z, b.x - 0.8f + k * 0.5f, b.y + (k - 1.5f) * 0.7f, b.z + 4.5f - std::abs(k - 1.5f), 1.1f - std::abs(k - 1.5f) * 0.2f, 0.3f, DrCorne);
        r->Add("queue", q, V(0, 13.5f, -14), "corps");
    }
}
