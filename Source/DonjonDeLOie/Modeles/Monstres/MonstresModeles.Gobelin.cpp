// Donjon de l'Oie — le gobelin et l'orc pillard (humanoïdes peaux-vertes).
#include "MonstresInterne.h"

namespace DonjonDeLOie::Modeles::MonstresModeles
{
    using namespace DonjonDeLOie::Voxel;

    namespace
    {
        /// Une plaque plate dans le plan (y, z), d'épaisseur x0..x1, définie par un prédicat (y, z) → dedans.
        void PlaqueYZ(VoxelModel& m, int x0, int x1, int y0, int y1, int z0, int z1, const std::function<bool(int, int)>& dedans, Color32 c)
        {
            for (int y = y0; y <= y1; y++)
                for (int z = z0; z <= z1; z++)
                    if (dedans(y, z))
                        for (int x = x0; x <= x1; x++) m.Set(x, y, z, c);
        }
    }

    // ------------------------------------------------------------------ gobelin (~0,95 m, voxel 0,05)
    Ptr<VoxelRig> Gobelin()
    {
        auto r = NouveauRig("gobelin", 0.05f);
        Color32 peau = C("7f9f45"), peauS = C("586f30"), peauC = C("a3bb62"), oreilleIn = C("b5786a");
        Color32 gue = C("7b6a4c"), gueS = C("564a36"), cuir = Pal::CuirSombre, bouche = C("3a1c1c"), dent = C("e7dcb0");
        Color32 rouille = C("8f5530"), lame = C("8e877c"), lameC = C("b4aca0"), oeil = C("ffd23a");

        // Corps : buste voûté en guenilles, ceinture, pagne effiloché.
        auto corps = Nouveau<VoxelModel>();
        ProfilY(*corps, 0, {5, 0, 3, 2.4f, 8, 0.4f, 3.3f, 2.6f, 10, 1.1f, 3.8f, 2.8f, 12, 2, 3.6f, 2.6f, 13, 2.4f, 2.4f, 2}, 0.4f, gue);
        ProfilY(*corps, 0, {2, -0.2f, 3.8f, 3, 6, 0, 3.4f, 2.6f}, 0.3f, gueS);
        corps->Box(-4, 6, -3, 3, 6, 3, cuir);
        corps->Carve([&](int x, int y, int z) { return y < 6 && std::abs(x + 0.5f) < 3.6f && std::abs(x + 0.5f) > 0.6f && y < 2 + static_cast<int>(Pal::Hash01(x, 1, z) * 3); });
        corps->Carve([&](int x, int y, int z) { return y < 6 && !corps->Has(x, y + 1, z); });
        corps->Set(-4, 6, 3, Pal::Fer); corps->Set(3, 6, 3, Pal::Fer);
        // déchirures : la peau apparaît par des trous
        corps->Paint([&](int x, int y, int z, Color32 c) { return (Meme(c, gue) && Pal::Hash01(x, y, z * 7) > 0.86f) ? peauS : c; });
        corps->Paint([&](int x, int y, int z, Color32 c) { return (Meme(c, gue) && y >= 11 && z >= 2) ? peau : c; });   // gorge nue
        // une bandoulière de corde
        corps->Line(-3.5f, 11.5f, 3, 3, 6.5f, 3, 0.55f, C("a08a5a"));
        // collier de crocs et de doigts séchés
        for (int x = -2; x <= 1; x++)
        {
            int zf = Avant(*corps, x, 12, -6, 10);
            if (zf != INT_MIN)
            {
                corps->Set(x, 12, zf + 1, C("5a4632"));
                if (x % 2 == 0) corps->Set(x, 11, zf + 1, dent);
            }
        }
        // sac de rapines dans le dos, cordé, un manche de louche qui dépasse
        BoiteC(*corps, -3, 7, -5, 2, 12, -3, gueS, 1);
        corps->Box(-3, 10, -6, 2, 10, -6, C("a08a5a"));
        corps->Line(1, 11, -4, 2.5f, 15.5f, -4.5f, 0.5f, Pal::Bois);
        corps->Set(2, 15, -5, Pal::FerSombre); corps->Set(3, 15, -5, Pal::FerSombre);
        r->Add("corps", corps, V(0, 7, 0));

        // Jambes : courtes, arquées, pieds plats griffus.
        for (int s = 0; s < 2; s++)
        {
            float sx = s == 0 ? -1.0f : 1.0f;
            auto j = Nouveau<VoxelModel>();
            Chaine(*j, {sx * 1.6f, 5.5f, 0, sx * 2.6f, 3, 0.8f, sx * 2.1f, 1.2f, 0}, 1.45f, 1.15f, peau);
            int px = Mathf::FloorToInt(sx * 2.1f);
            BoiteC(*j, px - 1, 0, -1, px + 1, 1, 3, peauS, 1);
            j->Set(px - 1, 0, 4, dent); j->Set(px + 1, 0, 4, dent);
            j->Box(px - 1, 3, -1, px + 1, 3, 1, gueS);                        // bande de chiffon au genou
            r->Add(s == 0 ? "jambeG" : "jambeD", j, V(sx * 1.6f, 6, 0), "corps");
        }

        // Tête : grosse, large, nez crochu, grandes oreilles en lame.
        auto tete = Nouveau<VoxelModel>();
        ProfilY(*tete, 0, {12, 3.2f, 2.8f, 2.8f, 13, 3.4f, 3.6f, 3.2f, 16, 3.2f, 4, 3.4f, 18, 2.8f, 3.4f, 3, 19, 2.6f, 2.4f, 2.2f}, 0.45f, peau);
        tete->Line(0, 15.5f, 6, 0, 13.8f, 8.6f, 1, 0.55f, peauS);              // nez
        for (int s = 0; s < 2; s++)
        {
            float sx = s == 0 ? -1.0f : 1.0f;
            Triangle(*tete, V(sx * 3.6f, 14.5f, 2), V(sx * 3.6f, 18, 3.5f), V(sx * 10.5f, 19.5f, 0.5f), peau);
            Triangle(*tete, V(sx * 4.6f, 15.6f, 2.7f), V(sx * 4.6f, 17.4f, 3.4f), V(sx * 8.5f, 18.6f, 1.6f), oreilleIn);
            tete->Set(s == 0 ? -10 : 9, 19, 0, peauS);
        }
        tete->Paint([&](int x, int y, int z, Color32 c) { return (Meme(c, peau) && y >= 19) ? peauS : c; });
        tete->Paint([&](int x, int y, int z, Color32 c) { return (Meme(c, peau) && Pal::Hash01(x, y, z) > 0.88f) ? peauC : c; });           // verrues
        tete->Line(-0.5f, 19.5f, 2, -1.5f, 21.5f, -0.5f, 0.5f, C("2a2620")); tete->Line(0.5f, 19.5f, 3, 1.5f, 21, 1, 0.5f, C("2a2620"));   // trois poils raides
        // bouche : large rictus aux dents pointues
        for (int x = -3; x <= 2; x++)
        {
            int zf = Avant(*tete, x, 13, -5, 12);
            if (zf == INT_MIN) continue;
            tete->Set(x, 13, zf, bouche);
            if (x % 2 == 0 || x == -3) tete->Set(x, 12, zf, dent);
            else tete->Set(x, 12, zf, bouche);
        }
        auto lueur = Nouveau<VoxelModel>();
        for (int x : {-3, -2, 1, 2})
        {
            int zf = Avant(*tete, x, 16, -5, 12);
            Oeil(*tete, *lueur, x, 16, zf, oeil);
            tete->Set(x, 17, zf, peauS);
            tete->Set(x, 17, zf + 1, (x == -3 || x == 2) ? peauS : peau);    // arcade froncée
        }
        tete->Set(-1, 17, Avant(*tete, -1, 17, -5, 12) + 1, peauS); tete->Set(0, 17, Avant(*tete, 0, 17, -5, 12) + 1, peauS);
        r->Add("tete", tete, V(0, 13, 2.5f), "corps");
        r->Add("lueur_yeux", lueur, V(0, 13, 2.5f), "tete");

        // Bras longs et maigres, mains griffues.
        for (int s = 0; s < 2; s++)
        {
            float sx = s == 0 ? -1.0f : 1.0f;
            auto b = Nouveau<VoxelModel>();
            Chaine(*b, {sx * 4.2f, 11.5f, 1.8f, sx * 5.2f, 8.5f, 1.8f, sx * 5.0f, 5.5f, 3.2f}, 1.2f, 0.95f, peau);
            b->Ellipsoid(sx * 4.4f, 11.6f, 1.6f, 1.6f, 1.3f, 1.8f, gueS);
            for (int y = 6; y <= 7; y++) Section(*b, 1, y, sx * 5.1f + 0.5f * 0, 2.9f, 1.3f, 1.3f, 0.5f, cuir);   // brassard
            int hx = Mathf::FloorToInt(sx * 5.0f);
            BoiteC(*b, hx - 1, 4, 2, hx + 1, 5, 4, peauS, 0);
            b->Set(hx - 1, 3, 4, dent); b->Set(hx + 1, 3, 4, dent);
            r->Add(s == 0 ? "brasG" : "brasD", b, V(sx * 4.2f, 11.5f, 1.8f), "corps");
        }

        // Lame rouillée : un coutelas ébréché tenu pointe en avant.
        auto arme = Nouveau<VoxelModel>();
        arme->Box(4, 4, 1, 5, 5, 4, cuir);
        arme->Box(4, 3, 5, 5, 6, 5, Pal::FerSombre);                            // garde
        PlaqueYZ(*arme, 5, 5, 3, 9, 6, 15, [&](int y, int z) { return y >= 4 + (z - 6) / 4 && y <= 6 + (z - 6) / 3 - (z >= 14 ? 2 * (z - 13) : 0) && !(y == 6 + (z - 6) / 3 && (z == 9 || z == 12)); }, lame);
        arme->Paint([&](int x, int y, int z, Color32 c) { return (Meme(c, lame) && Pal::Hash01(x, y * 5, z * 3) > 0.55f) ? rouille : c; });
        arme->Paint([&](int x, int y, int z, Color32 c) { return (Meme(c, lame) && !arme->Has(x, y - 1, z)) ? lameC : c; });
        r->Add("arme", arme, V(4.5f, 4.5f, 3), "brasD");
        return Fin(r);
    }

    // ------------------------------------------------------------------ orc pillard (~1,65 m, voxel 0,05)
    Ptr<VoxelRig> Orc()
    {
        auto r = NouveauRig("orc", 0.05f);
        [[maybe_unused]] Color32 peau = C("71805a"), peauS = C("4f5c3f"), peauC = C("94a274"), cuir = C("6e4527"), cuirS = C("4a2e1a");   // (peauC ne sert pas, comme en C#)
        Color32 fer = C("7d848a"), ferS = C("50565c"), ferC = C("b5bcc2"), clou = C("cfc7a8"), os = C("e3d6b2");
        Color32 cheveux = C("24201e"), oeil = C("ff7a24"), bouche = C("2c1616"), sang = C("7a2a22");

        auto corps = Nouveau<VoxelModel>();
        // Bassin, pagne de cuir, ceinture à boucle de crâne
        ProfilY(*corps, 0, {12, 0, 5, 3.6f, 15, 0.3f, 5.2f, 3.8f}, 0.4f, cuirS);
        corps->Box(-6, 14, -4, 5, 15, 4, cuir);
        BoiteC(*corps, -2, 13, 4, 1, 16, 5, os, 1); corps->Set(-1, 14, 5, bouche); corps->Set(0, 14, 5, bouche);
        corps->Box(-3, 6, 3, 2, 12, 4, cuir); corps->Box(-3, 7, -4, 2, 12, -3, cuirS);
        corps->Carve([&](int x, int y, int z) { return y <= 7 && (x == -3 || x == 2 || x == -1) && std::abs(z) >= 3; });
        // Torse massif, épaules hautes et projetées vers l'avant
        ProfilY(*corps, 0, {15, 0.4f, 5, 3.8f, 19, 0.8f, 6, 4.4f, 23, 1.6f, 7.4f, 4.8f, 25, 2.2f, 6.6f, 4.2f, 26, 2.6f, 4, 3.4f}, 0.35f, cuir);
        corps->Paint([&](int x, int y, int z, Color32 c) { return (Meme(c, cuir) && y >= 24 && std::abs(x + 0.5f) < 3.2f && z >= 3) ? peau : c; });   // encolure
        // ventre nu, couturé de cicatrices
        corps->Paint([&](int x, int y, int z, Color32 c) { return (Meme(c, cuir) && y >= 15 && y <= 18 && z >= 3) ? peau : c; });
        corps->Paint([&](int x, int y, int z, Color32 c) { return (Meme(c, peau) && y <= 18 && (x + y) % 5 == 0 && z >= 4) ? peauS : c; });
        // plastron de métal clouté
        for (int x = -6; x <= 5; x++)
            for (int y = 19; y <= 24; y++)
            {
                int zf = Avant(*corps, x, y, -8, 12);
                if (zf == INT_MIN || std::abs(x + 0.5f) > 5.2f) continue;
                corps->Set(x, y, zf + 1, (y == 19 || std::abs(x + 0.5f) > 4.6f) ? ferS : fer);
                if ((x == -5 || x == 4 || x == -1) && (y == 20 || y == 23)) corps->Set(x, y, zf + 2, clou);
            }
        // sangle diagonale cloutée dans le dos et devant
        for (int i = 0; i <= 12; i++)
        {
            int x = -6 + i, y = 25 - i / 2 - 2;
            int zb = -8;
            for (int z = -8; z < 8; z++)
                if (corps->Has(x, y, z))
                {
                    zb = z;
                    break;
                }
            corps->Set(x, y, zb - 1, cuirS);
            if (i % 3 == 0) corps->Set(x, y, zb - 2, clou);
        }
        // trophée : une rangée de crânes de gobelin? on garde des os en colliers
        for (int x = -3; x <= 2; x++)
        {
            int zf = Avant(*corps, x, 25, -8, 12);
            if (zf != INT_MIN && x % 2 == 0) corps->Set(x, 25, zf + 1, os);
        }
        r->Add("corps", corps, V(0, 13, 0));

        // Jambes épaisses, genouillères, bottes ferrées
        for (int s = 0; s < 2; s++)
        {
            float sx = s == 0 ? -1.0f : 1.0f;
            auto j = Nouveau<VoxelModel>();
            Chaine(*j, {sx * 3.2f, 13, 0, sx * 3.8f, 8, 0.8f, sx * 3.6f, 3, 0}, 2.6f, 2.1f, peauS);
            j->Paint([&](int x, int y, int z, Color32 c) { return y >= 9 ? cuirS : c; });
            int px = Mathf::FloorToInt(sx * 3.6f);
            BoiteC(*j, px - 2, 0, -2, px + 2, 4, 4, cuirS, 1);
            j->Box(px - 2, 0, 3, px + 2, 1, 5, ferS);
            BoiteC(*j, px - 2, 6, 2, px + 2, 9, 3, fer, 1); j->Set(px, 8, 4, clou);
            r->Add(s == 0 ? "jambeG" : "jambeD", j, V(sx * 3.2f, 13, 0), "corps");
        }

        // Tête : petite par rapport au buste, avancée, mâchoire carrée, défenses
        auto tete = Nouveau<VoxelModel>();
        ProfilY(*tete, 0, {24, 4.2f, 3.6f, 3.4f, 26, 4.6f, 4.2f, 3.6f, 29, 4.4f, 3.8f, 3.6f, 31, 4, 3, 3}, 0.45f, peau);
        BoiteC(*tete, -4, 24, 4, 3, 26, 8, peau, 1);                                   // mâchoire prognathe
        tete->Box(-4, 28, 6, 3, 28, 8, peauS);                                          // arcade lourde
        tete->Line(-0.5f, 27, 7.5f, -0.5f, 26.5f, 8.8f, 0.9f, peauS); tete->Line(0.5f, 27, 7.5f, 0.5f, 26.5f, 8.8f, 0.9f, peauS);   // nez écrasé
        for (int x = -3; x <= 2; x++) tete->Set(x, 26, 9, x == -3 || x == 2 ? os : bouche);
        tete->Line(-3.5f, 25.5f, 9, -4, 28.2f, 9.5f, 0.7f, 0.5f, os); tete->Line(3.5f, 25.5f, 9, 4, 28.2f, 9.5f, 0.7f, 0.5f, os);   // défenses
        for (int s = 0; s < 2; s++)
        {
            float sx = s == 0 ? -1.0f : 1.0f;
            Triangle(*tete, V(sx * 3.8f, 27, 3), V(sx * 3.8f, 29, 4.5f), V(sx * 6.5f, 29.5f, 2), peau);
            tete->Set(s == 0 ? -6 : 5, 29, 2, Pal::Or);                                  // anneau
        }
        // crâne rasé, crête de cheveux noirs tressée vers l'arrière
        tete->Paint([&](int x, int y, int z, Color32 c) { return (Meme(c, peau) && y >= 30) ? peauS : c; });
        Chaine(*tete, {0, 31.5f, 6, 0, 32.5f, 3, 0, 31.5f, 0.5f, 0, 28, -0.5f, 0, 24, -1}, 1.1f, 0.8f, cheveux);
        auto lueur = Nouveau<VoxelModel>();
        for (int x : {-3, -2, 1, 2}) { tete->Set(x, 27, 6, C("2a2420")); }
        Oeil(*tete, *lueur, -3, 27, 7, oeil); Oeil(*tete, *lueur, 2, 27, 7, oeil);
        tete->Set(-2, 27, 7, peauS); tete->Set(1, 27, 7, peauS);
        tete->Paint([&](int x, int y, int z, Color32 c) { return (Meme(c, peau) && x == 2 && y >= 26 && y <= 29 && z >= 6) ? sang : c; });   // cicatrice
        r->Add("tete", tete, V(0, 25, 3.5f), "corps");
        r->Add("lueur_yeux", lueur, V(0, 25, 3.5f), "tete");

        // Bras énormes : épaulière de fer à pointes (gauche), brassards
        for (int s = 0; s < 2; s++)
        {
            float sx = s == 0 ? -1.0f : 1.0f;
            auto b = Nouveau<VoxelModel>();
            Chaine(*b, {sx * 7.6f, 23, 1.4f, sx * 9.2f, 17.5f, 1.6f, sx * 9.4f, 12.5f, 4}, 2.9f, 2.2f, peau);
            b->Paint([&](int x, int y, int z, Color32 c) { return (Meme(c, peau) && Pal::Hash01(x, y, z) > 0.8f) ? peauS : c; });
            // brassard
            for (int y = 12; y <= 15; y++) Section(*b, 1, y, sx * 9.4f, 3.4f, 2.6f, 2.6f, 1.2f, ferS);
            b->Box(Mathf::FloorToInt(sx * 9.4f) - 2, 14, 3, Mathf::FloorToInt(sx * 9.4f) + 2, 14, 3, clou);
            // poing
            int hx = Mathf::FloorToInt(sx * 9.4f);
            BoiteC(*b, hx - 2, 8, 2, hx + 2, 11, 6, peauS, 1);
            // épaulière : trois plaques en gradins
            for (int k = 0; k < 3; k++)
                BoiteC(*b, Mathf::FloorToInt(sx * (5.5f + k * 0.6f)) - (s == 0 ? 3 : 0) + (s == 0 ? 0 : 0), 25 - k * 2, -4 + k, Mathf::FloorToInt(sx * (5.5f + k * 0.6f)) + (s == 0 ? 0 : 3), 26 - k * 2, 6 - k, k == 0 ? fer : ferS, 1);
            if (s == 0) { b->Line(-8, 27, 1, -9.5f, 31, 0, 0.9f, 0.4f, ferC); b->Line(-8, 27, -2, -9, 30, -3, 0.8f, 0.4f, ferC); }
            else { b->Box(5, 24, -3, 9, 25, 5, cuir); b->Set(7, 26, 1, clou); }
            r->Add(s == 0 ? "brasG" : "brasD", b, V(sx * 7.6f, 23, 1.4f), "corps");
        }

        // Hache : manche épais tenu droit, grosse lame en croissant ébréchée vers l'avant, pointe à l'arrière.
        auto arme = Nouveau<VoxelModel>();
        arme->Line(9.5f, 3, 4, 9.5f, 29, 5, 0.95f, Pal::BoisSombre);
        for (int y = 4; y <= 12; y += 3) arme->Box(8, y, 3, 10, y, 5, cuirS);
        constexpr float cyA = 23.5f;
        PlaqueYZ(*arme, 9, 10, 12, 35, 6, 18, [&](int y, int z)
        {
            float dy = y + 0.5f - cyA, dz = z + 0.5f - 5.5f;
            return dz > 0.5f && std::abs(dy) <= 2.2f + dz * 0.55f && dz <= 11 - dy * dy / 9.0f && !(dz > 7 && std::abs(dy + 2.5f) < 0.8f);
        }, fer);
        arme->Paint([&](int x, int y, int z, Color32 c)
        {
            if (!Meme(c, fer)) return c;
            float dy = y + 0.5f - cyA, dz = z + 0.5f - 5.5f;
            if (dz > 9.6f - dy * dy / 9.0f) return ferC;
            if (Pal::Hash01(x, y, z) > 0.82f) return ferS;
            return c;
        });
        BoiteC(*arme, 8, 21, 3, 11, 26, 7, ferS, 1);                                  // douille
        arme->Line(9.5f, 23.5f, 3, 9.5f, 24.5f, -1.5f, 1.1f, 0.4f, ferS);             // pointe arrière
        arme->Line(9.5f, 29, 5, 9.5f, 31.5f, 5.5f, 0.8f, 0.3f, ferC);                 // pique sommitale
        arme->Set(9, 20, 15, sang); arme->Set(10, 19, 16, sang); arme->Set(9, 26, 15, sang);
        r->Add("arme", arme, V(9.5f, 9.5f, 4.5f), "brasD");
        return Fin(r);
    }
}
