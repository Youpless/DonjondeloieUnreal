// Donjon de l'Oie — le squelette et le spectre (morts-vivants des Cryptes d'Ombrelune).
#include "MonstresInterne.h"

namespace DonjonDeLOie::Modeles::MonstresModeles
{
    using namespace DonjonDeLOie::Voxel;

    namespace
    {
        constexpr double MathPi = 3.14159265358979323846;      // Math.PI (double)
    }

    // ------------------------------------------------------------------ squelette (~1,45 m, voxel 0,05)
    Ptr<VoxelRig> Squelette()
    {
        auto r = NouveauRig("squelette", 0.05f);
        Color32 os = C("e2d8bf"), osS = C("b2a587"), osT = C("8c7f64"), creux = C("2a2420"), fer = C("7f868c"), ferS = C("4d5358");
        Color32 rouille = C("8a5532"), bois = C("6b4a30"), boisS = C("47301f"), tissu = C("5a3c4a"), oeil = C("7fd0ff");

        auto corps = Nouveau<VoxelModel>();
        // Bassin : ailes iliaques en plaques
        BoiteC(*corps, -4, 12, -2, 3, 14, 1, os, 1);
        corps->Box(-1, 11, -1, 0, 12, 0, osS);
        corps->Carve([&](int x, int y, int z) { return y == 13 && (x == -3 || x == 2) && z >= 0; });
        // Colonne vertébrale : vertèbres en disques alternés
        for (int y = 15; y <= 22; y++)
        {
            BoiteC(*corps, -1, y, -2, 0, y, -1, y % 2 == 0 ? os : osS, 0);
            if (y % 2 == 0) { corps->Set(-2, y, -2, osS); corps->Set(1, y, -2, osS); }
        }
        // Cage thoracique : côtes en arcs, sternum, entre les côtes du vide.
        for (int k = 0; k < 5; k++)
        {
            int y = 18 + k * 2;
            float w = 3.4f + (k < 3 ? k * 0.6f : (4 - k) * 0.5f + 0.4f);
            for (int x = -6; x <= 5; x++)
                for (int z = -4; z <= 4; z++)
                {
                    float dx = (x + 0.5f) / w, dz = (z + 1) / 3.2f;
                    float d = dx * dx + dz * dz;
                    if (d >= 0.62f && d <= 1.05f && !(std::abs(x + 0.5f) < 1.2f && z > 1)) corps->Set(x, y, z, os);
                }
        }
        for (int y = 18; y <= 26; y++) { corps->Set(-1, y, 2, osS); corps->Set(0, y, 2, osS); }   // sternum
        // Clavicules et épaules
        corps->Line(-5.5f, 27.5f, 0, 4.5f, 27.5f, 0, 0.7f, os);
        // Restes d'armure : épaulière rouillée à gauche, tassette de mailles, ceinturon
        for (int y = 26; y <= 28; y++) Section(*corps, 1, y, -6, 0, 2.6f - (y - 26) * 0.5f, 2.8f - (y - 26) * 0.5f, 1, y == 28 ? rouille : fer);
        corps->Box(-8, 25, -2, -4, 25, 2, ferS);
        corps->Box(-5, 11, -3, 4, 11, 2, C("4a3424"));
        corps->Box(-4, 8, 2, -1, 10, 2, ferS); corps->Box(0, 8, 2, 3, 10, 2, fer);
        corps->Paint([&](int x, int y, int z, Color32 c) { return (Meme(c, fer) && Pal::Hash01(x * 3, y, z) > 0.6f) ? rouille : c; });
        // lambeau de tabard violet passé
        corps->Box(-2, 6, 2, 1, 10, 3, tissu); corps->Carve([&](int x, int y, int z) { return Meme(Couleur(*corps, x, y, z), tissu) && y <= 7 && (x == -2 || x == 1); });
        r->Add("corps", corps, V(0, 13, 0));

        // Jambes : fémur, genou, tibia, pied en éventail
        for (int s = 0; s < 2; s++)
        {
            float sx = s == 0 ? -1.0f : 1.0f;
            auto j = Nouveau<VoxelModel>();
            j->Line(sx * 2.4f, 12, 0, sx * 2.6f, 6.5f, 0.6f, 0.75f, os);
            BoiteC(*j, Mathf::FloorToInt(sx * 2.6f) - 1, 5, -1, Mathf::FloorToInt(sx * 2.6f) + 1, 7, 1, osS, 1);
            j->Line(sx * 2.6f, 5.5f, 0, sx * 2.6f, 1.5f, -0.4f, 0.7f, os);
            int px = Mathf::FloorToInt(sx * 2.6f);
            j->Box(px - 1, 0, -2, px + 1, 1, 0, osS);
            for (int t = -1; t <= 1; t++) j->Line(px + 0.5f + t * 0.6f, 0.5f, 0, px + 0.5f + t * 1.1f, 0.5f, 3.2f, 0.5f, os);
            // grève rouillée sur une jambe
            if (s == 1)
                for (int y = 2; y <= 4; y++) Section(*j, 1, y, sx * 2.6f + 0.5f, 0.2f, 1.6f, 1.6f, 0.6f, rouille);
            r->Add(s == 0 ? "jambeG" : "jambeD", j, V(sx * 2.4f, 12.5f, 0), "corps");
        }

        // Crâne : grosse boîte chanfreinée, orbites profondes, mâchoire séparée
        auto tete = Nouveau<VoxelModel>();
        ProfilY(*tete, 0, {30, 1, 2.8f, 3, 31, 1, 3.6f, 3.8f, 35, 0.8f, 4, 4, 37, 0.6f, 3.4f, 3.5f, 38, 0.4f, 2.4f, 2.6f}, 0.45f, os);
        tete->Line(-0.5f, 28.5f, 0, -0.5f, 30, 0, 0.7f, osS); // atlas
        // orbites (creux) + lueur au fond
        auto lueur = Nouveau<VoxelModel>();
        for (int x : {-3, -2, 1, 2})
            for (int y = 33; y <= 34; y++) { tete->Remove(x, y, 4); tete->Set(x, y, 3, creux); }
        Oeil(*tete, *lueur, -3, 33, 3, oeil); Oeil(*tete, *lueur, 2, 33, 3, oeil);
        tete->Set(-1, 32, 4, creux); tete->Set(0, 32, 4, creux); tete->Set(-1, 31, 4, creux); // nez
        tete->Box(-4, 35, 3, 3, 35, 4, osS);                                              // arcade
        for (int x = -3; x <= 2; x++) tete->Set(x, 30, 4, x % 2 == 0 ? os : creux);       // dents du haut
        // fêlure
        tete->Set(1, 37, 3, osT); tete->Set(2, 36, 3, osT); tete->Set(2, 38, 1, osT);
        // reste de casque : calotte cabossée à nasal
        for (int y = 36; y <= 39; y++) Section(*tete, 1, y, 0, 0.5f, 4.3f - std::max(0, y - 37) * 1.1f, 4.3f - std::max(0, y - 37) * 1.1f, 1.4f, y == 36 ? ferS : fer);
        tete->Carve([&](int x, int y, int z) { return y >= 37 && x >= 1 && z >= 2; });                            // casque fendu, crâne visible
        tete->Paint([&](int x, int y, int z, Color32 c) { return (Meme(c, fer) && Pal::Hash01(x, y * 2, z) > 0.55f) ? rouille : c; });
        r->Add("tete", tete, V(0, 30, 0), "corps");
        r->Add("lueur_yeux", lueur, V(0, 30, 0), "tete");

        auto mach = Nouveau<VoxelModel>();
        BoiteC(*mach, -3, 28, 0, 2, 29, 4, os, 1);
        mach->Carve([&](int x, int y, int z) { return y == 29 && z <= 2 && std::abs(x + 0.5f) < 2; });
        for (int x = -2; x <= 1; x++) mach->Set(x, 29, 4, x % 2 == 0 ? creux : os);
        Soustraire(*tete, *mach);
        r->Add("machoire", mach, V(0, 29.5f, 0), "tete");

        // Bras : humérus, radius/cubitus (deux traits), main en griffes
        for (int s = 0; s < 2; s++)
        {
            float sx = s == 0 ? -1.0f : 1.0f;
            auto b = Nouveau<VoxelModel>();
            b->Line(sx * 5.5f, 27, 0, sx * 6.8f, 21, 0.5f, 0.75f, os);
            b->Set(Mathf::FloorToInt(sx * 6.8f), 21, 0, osS);
            b->Line(sx * 6.6f, 20.5f, 0.2f, sx * 6.6f, 16, 2.6f, 0.55f, os);
            b->Line(sx * 7.4f, 20.5f, 0.6f, sx * 7.2f, 16, 3, 0.5f, osS);
            int hx = Mathf::FloorToInt(sx * 6.9f);
            b->Box(hx - 1, 14, 2, hx + 1, 15, 4, os);
            b->Set(hx - 1, 13, 4, osS); b->Set(hx + 1, 13, 4, osS); b->Set(hx, 13, 3, osS);
            r->Add(s == 0 ? "brasG" : "brasD", b, V(sx * 5.5f, 27, 0), "corps");
        }

        // Bouclier rond, fendu, bordé de fer (porté au bras gauche)
        auto bouc = Nouveau<VoxelModel>();
        DisqueZ(*bouc, -8, 17, 5, 5, 6.3f, bois);
        DisqueZ(*bouc, -8, 17, 4, 4, 5.5f, boisS);
        bouc->Paint([&](int x, int y, int z, Color32 c)
        {
            float dx = x + 0.5f + 8, dy = y + 0.5f - 17;
            float d = dx * dx + dy * dy;
            return d > 30 ? ferS : (std::abs(dx - dy * 0.2f) < 0.6f ? boisS : c);
        });
        for (int x = -9; x <= -8; x++)
            for (int y = 16; y <= 17; y++) bouc->Set(x, y, 6, fer);     // umbo
        bouc->Carve([&](int x, int y, int z)
        {
            float dx = x + 0.5f + 8, dy = y + 0.5f - 17;
            return dy > 1 && std::abs(dx - 1.8f - (dy - 1) * 0.35f) < 0.6f + (dy - 1) * 0.15f;
        });   // fente
        bouc->Carve([&](int x, int y, int z) { return x >= -4 && y >= 20; });                                            // éclat manquant
        r->Add("bouclier", bouc, V(-7, 16, 3.5f), "brasG");

        // Épée : lame droite ébréchée, garde en croix
        auto arme = Nouveau<VoxelModel>();
        arme->Box(6, 13, 2, 7, 16, 3, Pal::CuirSombre);
        arme->Box(4, 16, 2, 9, 17, 3, ferS);
        arme->Box(6, 12, 2, 7, 12, 3, fer);
        for (int y = 18; y <= 31; y++) { arme->Set(6, y, 2, fer); arme->Set(7, y, 2, y % 4 == 1 ? ferS : fer); arme->Set(6, y, 3, Pal::Acier); arme->Set(7, y, 3, fer); }
        arme->Set(6, 32, 2, fer); arme->Set(6, 32, 3, Pal::Acier); arme->Remove(7, 25, 2); arme->Remove(7, 25, 3);
        arme->Paint([&](int x, int y, int z, Color32 c) { return (y < 26 && Pal::Hash01(x, y, z * 5) > 0.75f) ? rouille : c; });
        r->Add("arme", arme, V(6.8f, 14.5f, 3), "brasD");
        return Fin(r);
    }

    Color32 Couleur(const VoxelModel& m, int x, int y, int z)
    {
        Color32 c;
        return m.TryGet(x, y, z, c) ? c : Color32(0, 0, 0, 0);
    }

    // ------------------------------------------------------------------ spectre (~1,65 m, voxel 0,05)
    Ptr<VoxelRig> Spectre()
    {
        auto r = NouveauRig("spectre", 0.05f);
        Color32 robe = C("5d6a7e"), robeS = C("3c4658"), robeC = C("8a98ac"), robeT = C("2a3040"), nuit = C("10121a");
        Color32 main = C("b7c2c8"), griffe = C("e6ecf0"), oeil = C("9ff0ff"), chaine = C("6d7378");

        auto corps = Nouveau<VoxelModel>();
        // Robe : cône qui s'évase puis se déchire en lambeaux pointus ; le bas ne touche pas le sol.
        ProfilY(*corps, 0, {4, -1, 6.5f, 5.5f, 10, -0.5f, 5.6f, 4.8f, 18, 0.2f, 4.4f, 3.6f, 24, 0.8f, 4.8f, 3.6f, 27, 1.2f, 5.8f, 3.8f, 28, 1.4f, 4, 3}, 0.4f, robe);
        // lambeaux : on découpe le bas en languettes de longueurs variées, et on en étire certaines vers le bas/arrière
        corps->Carve([&](int x, int y, int z)
        {
            double ang = std::atan2(z + 0.5, x + 0.5);
            int bande = static_cast<int>(std::floor((ang + MathPi) / (2 * MathPi) * 14));
            float lim = 4 + Pal::Hash01(bande, 3, 7) * 8;
            return y < lim && (static_cast<int>(std::floor((ang + MathPi) / (2 * MathPi) * 28))) % 2 == 0;
        });
        AleaNet rnd(3);
        for (int k = 0; k < 12; k++)
        {
            double a = k / 12.0 * 2 * MathPi + rnd.NextDouble() * 0.3;
            float rx = static_cast<float>(::Moteur::CosNet(a)) * 5.4f, rz = static_cast<float>(::Moteur::SinNet(a)) * 4.8f - 1;
            float l = 2 + static_cast<float>(rnd.NextDouble()) * 3;
            corps->Line(rx, 5.5f, rz, rx * 1.15f, 5.5f - l, rz * 1.1f - (rz < 0 ? 2.5f : 0.5f), 0.9f, 0.45f, k % 3 == 0 ? robeT : robeS);
        }
        // plis verticaux : bandes sombres et claires
        corps->Paint([&](int x, int y, int z, Color32 c)
        {
            if (!Meme(c, robe)) return c;
            double ang = std::atan2(z + 0.5, x + 0.5);
            int pli = static_cast<int>(std::floor((ang + MathPi) / (2 * MathPi) * 16));
            if (pli % 4 == 0) return robeS;
            if (pli % 4 == 2 && y > 10) return robeC;
            if (y < 9) return robeS;
            return c;
        });
        // chaîne rouillée qui ceint la taille et pend
        for (int i = 0; i < 26; i++)
        {
            double a = i / 26.0 * 2 * MathPi;
            int x = static_cast<int>(std::floor(::Moteur::CosNet(a) * 4.8f)), z = static_cast<int>(std::floor(::Moteur::SinNet(a) * 4.0f + 0.3f));
            corps->Set(x, 18 + (i % 2), z, chaine);
        }
        corps->Line(3, 18, 3, 3.5f, 11, 4, 0.5f, chaine); corps->Box(3, 10, 3, 4, 11, 4, C("6e6a5e"));
        r->Add("corps", corps, V(0, 16, 0));

        // Capuche : grande, pointue vers l'arrière, ouverture noire où brillent deux yeux.
        auto tete = Nouveau<VoxelModel>();
        ProfilY(*tete, 0, {26, 1.8f, 4.4f, 4.6f, 30, 1.6f, 4.6f, 4.8f, 33, 1, 3.8f, 4.2f, 35, 0, 2.4f, 3, 36, -1.5f, 1, 1.5f}, 0.45f, robeS);
        tete->Line(0, 34, -2, -0.5f, 32.5f, -6.5f, 1.5f, 0.6f, robeS);                 // pointe de capuche
        // ouverture du visage : creux profond
        tete->Carve([&](int x, int y, int z) { return z >= 4 && y >= 27 && y <= 32 && std::abs(x + 0.5f) < 3 - (y >= 31 ? 1 : 0); });
        for (int x = -3; x <= 2; x++)
            for (int y = 27; y <= 32; y++)
                for (int z = 3; z <= 4; z++)
                    if (!tete->Has(x, y, z) || z == 3)
                    {
                        if (std::abs(x + 0.5f) < 3) tete->Set(x, y, z == 4 ? 3 : z, nuit);
                    }
        tete->Paint([&](int x, int y, int z, Color32 c) { return (Meme(c, robeS) && z >= 5 && y >= 26) ? robeC : c; });   // bord de capuche éclairé
        auto lueur = Nouveau<VoxelModel>();
        Oeil(*tete, *lueur, -2, 30, 4, oeil); Oeil(*tete, *lueur, 1, 30, 4, oeil);
        lueur->Set(-2, 29, 4, oeil); lueur->Set(1, 29, 4, oeil);
        r->Add("tete", tete, V(0, 27, 1), "corps");
        r->Add("lueur_yeux", lueur, V(0, 27, 1), "tete");

        // Bras : manches larges en lambeaux, mains osseuses aux longues griffes, tendues vers l'avant.
        for (int s = 0; s < 2; s++)
        {
            float sx = s == 0 ? -1.0f : 1.0f;
            auto b = Nouveau<VoxelModel>();
            Chaine(*b, {sx * 5, 26, 1, sx * 6.8f, 22, 3, sx * 7.4f, 19, 6.5f}, 1.8f, 2.6f, robe);
            b->Paint([&](int x, int y, int z, Color32 c) { return (y < 20) ? robeS : c; });
            // lambeaux de manche pendants
            for (int k = 0; k < 3; k++) b->Line(sx * (6.4f + k * 0.8f), 18.5f, 5 + k, sx * (6.6f + k * 0.8f), 14 - k, 4 + k, 0.6f, 0.4f, robeT);
            // main : paume + quatre griffes recourbées
            float hx = sx * 7.4f;
            b->Box(Mathf::FloorToInt(hx) - 1, 18, 7, Mathf::FloorToInt(hx) + 1, 19, 9, main);
            for (int f = 0; f < 4; f++)
            {
                float fx = hx + (f - 1.5f) * 0.9f;
                b->Line(fx, 18.5f, 9, fx + (f - 1.5f) * 0.3f, 17, 12, 0.5f, main);
                b->Line(fx + (f - 1.5f) * 0.3f, 17, 12, fx + (f - 1.5f) * 0.4f, 14.5f, 12.8f, 0.5f, 0.35f, griffe);
            }
            r->Add(s == 0 ? "brasG" : "brasD", b, V(sx * 5, 26, 1), "corps");
        }
        Fin(r);
        r->Origin = Vector3(r->Origin.x, -4, r->Origin.z);      // il flotte à 20 cm du sol
        return r;
    }
}
