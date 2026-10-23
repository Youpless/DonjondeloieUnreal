// Donjon de l'Oie — outils de construction des bâtiments : maçonnerie appareillée (joints en creux), planches à deux tons,
// toitures en rangs (tuiles, chaume, ardoise), pignons, fenêtres à volets, portes ferrées.
// La façade (FaceBat) et la déclaration du chantier (ChantierBat, avec ses commentaires) sont dans BatimentsInterne.h.
#include "BatimentsInterne.h"

namespace DonjonDeLOie::Modeles::DecorsModeles::Batiments
{
    using namespace DonjonDeLOie::Voxel;

    void ChantierBat::Rond(float cx, float cz, int y0, int y1, float r)
    {
        std::array<float, 5> q = {cx, cz, static_cast<float>(y0), static_cast<float>(y1), r};
        ronds.Add(q);
    }

    // graine * 7919 + 13 boucle comme en C# (calcul sur 32 bits non signés)
    ChantierBat::ChantierBat(int graine) : R(static_cast<int>(static_cast<uint32_t>(graine) * 7919u + 13u)) {}

    Ptr<VoxelModel> ChantierBat::Lueur(const string& nom)
    {
        for (int i = 0; i < lueurs.Count(); i++)
            if (lueurs[i].Key == nom) return lueurs[i].Value;
        auto m = Nouveau<VoxelModel>();
        lueurs.Add(LueurNommee{nom, m});
        return m;
    }

    int ChantierBat::Div(int a, int b) { return a >= 0 ? a / b : -((-a + b - 1) / b); }

    int ChantierBat::Mod(int a, int b)
    {
        int r = a % b;
        return r < 0 ? r + b : r;
    }

    float ChantierBat::H(int a, int b, int c) { return Pal::Hash01(a, b, c); }

    // ------------------------------------------------------------------ maçonnerie
    void ChantierBat::Pierre(int x0, int y0, int z0, int x1, int y1, int z1, Color32 c) { P->Box(x0, y0, z0, x1, y1, z1, c); }
    void ChantierBat::PierreCyl(float cx, float cz, int y0, int y1, float r, Color32 c) { P->Cylinder(cx, cz, y0, y1, r, c); }
    void ChantierBat::PierreCone(float cx, float cz, int y0, int y1, float r0, float r1, Color32 c) { P->Cone(cx, cz, y0, y1, r0, r1, c); }

    void ChantierBat::MursPierre(int x0, int y0, int z0, int x1, int y1, int z1, int ep, Color32 c)
    {
        Pierre(x0, y0, z0, x1, y1, z0 + ep - 1, c);
        Pierre(x0, y0, z1 - ep + 1, x1, y1, z1, c);
        Pierre(x0, y0, z0, x0 + ep - 1, y1, z1, c);
        Pierre(x1 - ep + 1, y0, z0, x1, y1, z1, c);
    }

    void ChantierBat::Vide(int x0, int y0, int z0, int x1, int y1, int z1)
    {
        if (x0 > x1) std::swap(x0, x1);
        if (y0 > y1) std::swap(y0, y1);
        if (z0 > z1) std::swap(z0, z1);
        for (int x = x0; x <= x1; x++)
            for (int y = y0; y <= y1; y++)
                for (int z = z0; z <= z1; z++)
                {
                    P->Remove(x, y, z);
                    M->Remove(x, y, z);
                }
    }

    bool ChantierBat::Plein(int x, int y, int z) const { return y < 0 || P->Has(x, y, z) || M->Has(x, y, z); }

    Ptr<VoxelModel> ChantierBat::Finir()
    {
        Liste<VoxelLu> liste = P->Tous();
        Liste<Vector3Int> aCreuser;
        for (const VoxelLu& kv : liste)
        {
            int x = kv.Key.x, y = kv.Key.y, z = kv.Key.z;
            Color32 b = kv.Value;
            bool xp = !Plein(x + 1, y, z), xm = !Plein(x - 1, y, z), zp = !Plein(x, y, z + 1), zm = !Plein(x, y, z - 1), yp = !Plein(x, y + 1, z);
            bool ex = xp || xm, ez = zp || zm;
            int assise = Div(y, Assise);
            int dec = Mod(assise, 2) == 1 ? Longueur / 2 : 0;
            bool jointH = Mod(y, Assise) == 0 && y > 0;
            bool jointV = false;
            int bloc = 0;
            int face = 0;
            const std::array<float, 5>* rz = nullptr;
            for (int i = 0; i < ronds.Count(); i++)
            {
                const std::array<float, 5>& q = ronds[i];
                float dx = x + 0.5f - q[0], dz = z + 0.5f - q[1];
                if (y >= q[2] && y <= q[3] && dx * dx + dz * dz <= (q[4] + 1.5f) * (q[4] + 1.5f) && dx * dx + dz * dz >= (q[4] - 2.5f) * (q[4] - 2.5f))
                {
                    rz = &q;
                    break;
                }
            }
            if (rz != nullptr && (ex || ez))
            {
                float ang = Mathf::Atan2(z + 0.5f - (*rz)[1], x + 0.5f - (*rz)[0]);
                int arc = Mathf::FloorToInt((ang + Mathf::Pi) * (*rz)[4]);
                jointV = Mod(arc + dec, Longueur + 1) == 0;
                bloc = Div(arc + dec, Longueur + 1);
                face = 3;
            }
            else if (ex && !ez)
            {
                jointV = Mod(z + dec, Longueur) == 0;
                bloc = Div(z + dec, Longueur);
                face = x;
            }
            else if (ez && !ex)
            {
                jointV = Mod(x + dec, Longueur) == 0;
                bloc = Div(x + dec, Longueur);
                face = z;
            }
            else if (yp && !ex && !ez)
            {
                jointH = Mod(x + dec, Longueur) == 0 || Mod(z, Assise + 1) == 0;
                bloc = Div(x + dec, Longueur) * 31 + Div(z, Assise + 1);
                face = y;
            }
            else
            {
                bloc = Div(x + z + dec, Longueur + 2);
                face = 7;
            }
            float t = 0.9f + 0.17f * H(assise * 3 + 1, bloc * 5 + 2, face);
            Color32 c = Pal::Shade(b, t);
            if (Mousse > 0 && (yp || y < 3) && H(x * 3, y * 7, z * 5) < Mousse * (y < 3 ? 1.0f : 0.5f)) c = Pal::Mix(c, CouleurMousse, 0.7f);
            bool joint = jointH || jointV;
            if (joint) c = Pal::Shade(b, 0.72f);
            P->Set(x, y, z, c);
            int nh = (xp ? 1 : 0) + (xm ? 1 : 0) + (zp ? 1 : 0) + (zm ? 1 : 0);
            if (joint && nh == 1 && !yp && y > 0)
            {
                int bx = xp ? x - 1 : xm ? x + 1 : x, bz = zp ? z - 1 : zm ? z + 1 : z;
                int bx2 = 2 * bx - x, bz2 = 2 * bz - z;   // il faut deux voxels pleins derrière, sinon le joint percerait le mur
                if (P->Has(bx, y, bz) && P->Has(bx2, y, bz2) && !M->Has(x, y, z)) aCreuser.Add(Vector3Int(x, y, z));
            }
        }
        // Les joints sont creusés sur les parements (on garde un voxel sombre derrière).
        for (const Vector3Int& v : aCreuser)
        {
            Color32 c;
            P->TryGet(v.x, v.y, v.z, c);
            [[maybe_unused]] int n = 0;   // (compté, jamais lu, comme en C#)
            if (!Plein(v.x + 1, v.y, v.z))
            {
                n++;
                P->Set(v.x - 1, v.y, v.z, Pal::Shade(c, 0.9f));
            }
            if (!Plein(v.x - 1, v.y, v.z))
            {
                n++;
                P->Set(v.x + 1, v.y, v.z, Pal::Shade(c, 0.9f));
            }
            if (!Plein(v.x, v.y, v.z + 1))
            {
                n++;
                P->Set(v.x, v.y, v.z - 1, Pal::Shade(c, 0.9f));
            }
            if (!Plein(v.x, v.y, v.z - 1))
            {
                n++;
                P->Set(v.x, v.y, v.z + 1, Pal::Shade(c, 0.9f));
            }
            P->Remove(v.x, v.y, v.z);
        }
        auto fin = Nouveau<VoxelModel>();
        fin->Merge(*P, 0, 0, 0);
        fin->Merge(*M, 0, 0, 0);
        if (Evidement > 0) Evider(*fin, Evidement);
        if (Miroir) fin = Retourner(*fin);
        return fin;
    }

    Ptr<VoxelModel> ChantierBat::Retourner(const VoxelModel& m)
    {
        auto r = Nouveau<VoxelModel>();
        for (const VoxelLu& kv : m.Tous()) r->Set(-1 - kv.Key.x, kv.Key.y, kv.Key.z, kv.Value);
        return r;
    }

    void ChantierBat::Evider(VoxelModel& m, int p)
    {
        Liste<Vector3Int> aRetirer;
        for (const VoxelLu& kv : m.Tous())
        {
            int x = kv.Key.x, y = kv.Key.y, z = kv.Key.z;
            bool enfoui = true;
            for (int dx = -p; dx <= p && enfoui; dx++)
                for (int dy = -p; dy <= p && enfoui; dy++)
                    for (int dz = -p; dz <= p && enfoui; dz++)
                        if (!m.Has(x + dx, y + dy, z + dz)) enfoui = false;
            if (enfoui && y > 0) aRetirer.Add(kv.Key);
        }
        for (const Vector3Int& v : aRetirer) m.Remove(v.x, v.y, v.z);
    }

    Ptr<VoxelRig> ChantierBat::Rig(const string& id, float taille)
    {
        auto corps = Finir();
        auto r = Nouveau<VoxelRig>();
        r->Name = id;
        r->VoxelSize = taille;
        r->Origin = Vector3::zero;
        r->Add("corps", corps, Vector3::zero);
        for (int i = 0; i < lueurs.Count(); i++)
            if (lueurs[i].Value->Count() > 0) r->Add(lueurs[i].Key, Miroir ? Retourner(*lueurs[i].Value) : lueurs[i].Value, Vector3::zero, "corps");
        return r;
    }

    // ------------------------------------------------------------------ bois
    void ChantierBat::Bois(int x0, int y0, int z0, int x1, int y1, int z1, Color32 c, char axe, int larg)
    {
        if (x0 > x1) std::swap(x0, x1);
        if (y0 > y1) std::swap(y0, y1);
        if (z0 > z1) std::swap(z0, z1);
        for (int x = x0; x <= x1; x++)
            for (int y = y0; y <= y1; y++)
                for (int z = z0; z <= z1; z++)
                {
                    int k = Div(axe == 'x' ? x : axe == 'y' ? y : z, larg);
                    float f = (Mod(k, 2) == 0 ? 1.0f : 0.84f) * (0.94f + 0.12f * H(k, 77, x0 + z0 * 3));
                    int along = axe == 'y' ? x + z : y;
                    if (H(k * 13, along, 5) < 0.04f) f *= 0.75f;   // nœud
                    M->Set(x, y, z, Pal::Shade(c, f));
                }
    }

    void ChantierBat::Poutre(int x0, int y0, int z0, int x1, int y1, int z1, Color32 c)
    {
        if (x0 > x1) std::swap(x0, x1);
        if (y0 > y1) std::swap(y0, y1);
        if (z0 > z1) std::swap(z0, z1);
        for (int x = x0; x <= x1; x++)
            for (int y = y0; y <= y1; y++)
                for (int z = z0; z <= z1; z++) M->Set(x, y, z, Pal::Var(c, x, y, z, 0.05f));
    }

    void ChantierBat::Trait(float x0, float y0, float z0, float x1, float y1, float z1, float r, Color32 c) { M->Line(x0, y0, z0, x1, y1, z1, r, c); }

    // ------------------------------------------------------------------ toitures
    void ChantierBat::SetUV(bool faitageX, int u, int y, int v, Color32 c)
    {
        if (faitageX) M->Set(u, y, v, c);
        else M->Set(v, y, u, c);
    }

    void ChantierBat::Toit(bool faitageX, int u0, int u1, int v0, int v1, int yb, int montee, int ep, Color32 c, int style)
    {
        int largeur = v1 - v0 + 1, pas = (largeur + 1) / 2;
        for (int d = 0; d < pas; d++)
        {
            int va = v0 + d, vb = v1 - d;
            int yh = yb + d * montee + montee - 1;
            bool fait = d == pas - 1;
            for (int u = u0; u <= u1; u++)
                for (int y = yh - ep - montee + 2; y <= yh; y++)
                {
                    Color32 col = TuileCouleur(c, style, d, u, y, yh, fait);
                    SetUV(faitageX, u, y, va, col);
                    SetUV(faitageX, u, y, vb, col);
                    if (fait && style != 1)
                    {
                        SetUV(faitageX, u, yh + 1, va, Pal::Shade(c, 0.62f));
                        SetUV(faitageX, u, yh + 1, vb, Pal::Shade(c, 0.62f));
                    }
                }
            if (style == 1 && d == 0)   // le chaume déborde et pend un peu à l'égout
                for (int u = u0; u <= u1; u++)
                {
                    SetUV(faitageX, u, yb - ep, va, Pal::Shade(c, 0.78f));
                    SetUV(faitageX, u, yb - ep, vb, Pal::Shade(c, 0.78f));
                }
        }
        if (style == 1)   // faîtage de chaume en bourrelet
            for (int u = u0; u <= u1; u++)
            {
                int yh = yb + (pas - 1) * montee + montee;
                for (int v = v0 + pas - 2; v <= v1 - pas + 2; v++) SetUV(faitageX, u, yh, v, Pal::Shade(c, 0.8f));
            }
    }

    Color32 ChantierBat::TuileCouleur(Color32 c, int style, int d, int u, int y, int yh, bool fait) const
    {
        if (style == 1)
        {
            float f = 0.86f + 0.22f * H(u, d / 3, 41) + (Mod(d, 3) == 0 ? -0.08f : 0.0f);
            return Pal::Shade(c, f);
        }
        int rang = style == 3 ? d : d / 2;
        float g = Mod(rang, 2) == 0 ? 1.0f : 0.9f;
        int lu = style == 2 ? 4 : 3;
        g *= 0.9f + 0.18f * H(Div(u + (Mod(rang, 2) * lu / 2), lu), rang, style);
        if (y < yh) g *= 0.8f;                         // dessous du rang
        if (style != 3 && Mod(d, 2) == 0 && y == yh) g *= 0.9f;
        if (fait) g *= 0.85f;
        return Pal::Shade(c, g);
    }

    void ChantierBat::Pignon(bool faitageX, int uPlan, int v0, int v1, int yb, int montee, Color32 c, bool pierre)
    {
        int largeur = v1 - v0 + 1, pas = (largeur + 1) / 2;
        for (int d = 0; d < pas; d++)
            for (int y = yb + d * montee; y < yb + d * montee + montee; y++)
                for (int v = v0 + d; v <= v1 - d; v++)
                {
                    int x = faitageX ? uPlan : v, z = faitageX ? v : uPlan;
                    if (pierre) P->Set(x, y, z, c);
                    else M->Set(x, y, z, Pal::Var(c, x, y, z, 0.03f));
                }
    }

    // ------------------------------------------------------------------ ouvertures
    void ChantierBat::SetF(FaceBat f, int u, int y, int d, Color32 c) { M->Set(f.X(u, d), y, f.Z(u, d), c); }

    void ChantierBat::VideF(FaceBat f, int u, int y, int d)
    {
        int x = f.X(u, d), z = f.Z(u, d);
        P->Remove(x, y, z);
        M->Remove(x, y, z);
    }

    void ChantierBat::Fenetre(FaceBat f, int u0, int y0, int w, int h, Color32 cadre, Color32 volet, bool volets, const Ptr<VoxelModel>& vitre)
    {
        Color32 verre = Pal::C("2a2f3a");
        for (int u = u0; u < u0 + w; u++)
            for (int y = y0; y < y0 + h; y++)
            {
                VideF(f, u, y, 0);
                bool crois = (w >= 3 && u == u0 + w / 2) || (h >= 4 && y == y0 + h / 2);
                if (crois) SetF(f, u, y, -1, cadre);
                else if (vitre != nullptr)
                {
                    SetF(f, u, y, -1, verre);
                    vitre->Set(f.X(u, -1), y, f.Z(u, -1), Pal::C("ffc070"));
                    M->Remove(f.X(u, -1), y, f.Z(u, -1));
                }
                else SetF(f, u, y, -1, Pal::Var(verre, u, y, 3, 0.15f));
            }
        for (int u = u0 - 1; u <= u0 + w; u++)
        {
            SetF(f, u, y0 - 1, 1, Pal::Shade(cadre, 0.9f));
            SetF(f, u, y0 - 1, 0, cadre);
            SetF(f, u, y0 + h, 0, cadre);
        }
        for (int y = y0; y < y0 + h; y++)
        {
            SetF(f, u0 - 1, y, 0, cadre);
            SetF(f, u0 + w, y, 0, cadre);
        }
        if (!volets) return;
        int lv = Mathf::Max(1, (w + 1) / 2);
        for (int k = 0; k < lv; k++)
            for (int y = y0; y < y0 + h; y++)
            {
                float t = Mod(k, 2) == 0 ? 1.0f : 0.86f;
                if (y == y0 + 1 || y == y0 + h - 2) t *= 0.78f;   // barres
                SetF(f, u0 - 2 - k, y, 1, Pal::Shade(volet, t));
                SetF(f, u0 + w + 1 + k, y, 1, Pal::Shade(volet, t));
            }
    }

    void ChantierBat::Porte(FaceBat f, int u0, int y0, int w, int h, int prof, Color32 bois, bool arche, Color32 encadrement)
    {
        float cu = u0 + w * 0.5f, r = w * 0.5f;
        int yr = y0 + h - (arche ? Mathf::CeilToInt(r) : 0);   // (int)Math.Ceiling(r)
        for (int u = u0; u < u0 + w; u++)
            for (int y = y0; y < y0 + h; y++)
            {
                if (arche && y >= yr)
                {
                    float du = u + 0.5f - cu, dy = y + 0.5f - yr;
                    if (du * du + dy * dy > r * r) continue;
                }
                for (int d = 0; d > -prof; d--) VideF(f, u, y, d);
                int k = Div(u - u0, 2);
                float t = Mod(k, 2) == 0 ? 1.0f : 0.85f;
                Color32 c = Pal::Shade(bois, t);
                if (y == y0 + 3 || y == y0 + h - 5) c = Pal::FerSombre;
                SetF(f, u, y, -prof, c);
            }
        SetF(f, u0 + w - 2, y0 + h / 2 - 1, -prof + 1, Pal::Fer);
        // encadrement
        for (int y = y0; y < y0 + h + 1; y++)
            for (int u = u0 - 1; u <= u0 + w; u++)
            {
                bool dedans;
                if (arche && y >= yr)
                {
                    float du = u + 0.5f - cu, dy = y + 0.5f - yr;
                    float q = du * du + dy * dy;
                    dedans = q <= (r + 1.2f) * (r + 1.2f) && q > r * r;
                }
                else dedans = ((u == u0 - 1 || u == u0 + w) && y < (arche ? yr : y0 + h)) || (!arche && y == y0 + h);
                if (dedans) SetF(f, u, y, 1, Pal::Var(encadrement, u, y, 9, 0.08f));
            }
        for (int u = u0 - 1; u <= u0 + w; u++)
        {
            SetF(f, u, 0, 1, Pal::PierreClaire);
            SetF(f, u, 0, 2, Pal::Pierre);
        }
    }
}
