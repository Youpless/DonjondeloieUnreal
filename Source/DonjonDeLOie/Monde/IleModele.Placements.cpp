// Donjon de l'Oie — où poser les lieux remarquables, le décor semé et les lampes.
#include "IleModele.h"

#include "../Modeles/Catalogue.h"

namespace DonjonDeLOie::Monde
{
    using namespace DonjonDeLOie::Regles;
    using namespace DonjonDeLOie::Modeles;
    using namespace DonjonDeLOie::Voxel;

    namespace
    {
        /// new Lampe { Pos = …, Couleur = …, Portee = …, Intensite = … }
        Lampe NouvelleLampe(Vector3 pos, Color32 couleur, float portee, float intensite)
        {
            Lampe l;
            l.Pos = pos;
            l.Couleur = couleur;
            l.Portee = portee;
            l.Intensite = intensite;
            return l;
        }
    }

    Vector3 IleModele::PosCase(int c) const { return Vector3((*Plateau)[c]->X, 0.0f, (*Plateau)[c]->Z); }

    /// Direction de la route au passage de la case c (moyenne entrée/sortie).
    Vector3 IleModele::Tangente(int c) const
    {
        const Regles::Plateau& pl = *Plateau;
        Vector3 p = PosCase(c);
        Vector3 avant = pl[c]->Suivants.Count() > 0 ? PosCase(pl[c]->Suivants[0]) : p;
        Vector3 arriere = pl[c]->Precedents.Count() > 0 ? PosCase(pl[c]->Precedents[0]) : p;
        Vector3 t = avant - arriere; t.y = 0;
        return t.sqrMagnitude() > 1e-4f ? t.normalized() : Vector3::forward;
    }

    /// Point au bord de la route : côté extérieur de l'île (exterieur = true) ou intérieur.
    Vector3 IleModele::Bord(int c, float dist, bool exterieur, float glisse) const
    {
        Vector3 p = PosCase(c);
        Vector3 t = Tangente(c);
        Vector3 n(-t.z, 0.0f, t.x);
        float s = Vector3::Dot(n, p - Centre) >= 0 ? 1.0f : -1.0f;
        if (!exterieur) s = -s;
        // Rien de haut entre la caméra (au sud, par défaut) et la case : on passe de l'autre côté du chemin.
        if ((n * s).z < -0.45f) s = -s;
        return p + n * (s * dist) + t * glisse;
    }

    float IleModele::CapVers(Vector3 de, Vector3 vers)
    {
        Vector3 d = vers - de;
        return Mathf::Atan2(d.x, d.z) * Mathf::Rad2Deg;
    }

    void IleModele::Lieu(const string& id, Vector3 pos, float rotY, float rayon, int g, float echelle)
    {
        Aplanir(pos, rayon * 0.8f);
        pos.y = HauteurEn(pos.x, pos.z);
        Placement pl;
        pl.Id = id; pl.Graine = g; pl.Pos = pos; pl.RotY = rotY; pl.Echelle = echelle; pl.Lieu = true;
        Lieux.Add(pl);
        zonesInterdites.Add(std::pair<Vector3, float>(pos, rayon));
    }

    void IleModele::LieuFace(const string& id, int c, float dist, bool exterieur, float rayon, int g, float glisse)
    {
        Vector3 pos = Bord(c, dist, exterieur, glisse);
        Lieu(id, pos, CapVers(pos, PosCase(c) + Tangente(c) * glisse * 0.5f), rayon, g);
    }

    /// Met le terrain d'un disque à la hauteur de son centre (sans toucher au chemin).
    void IleModele::Aplanir(Vector3 centre, float rayon)
    {
        float h = Mathf::Max(0.25f, HauteurEn(centre.x, centre.z));
        int r = Mathf::CeilToInt(rayon / Cell) + 1;
        int cx = static_cast<int>((centre.x - X0) / Cell), cz = static_cast<int>((centre.z - Z0) / Cell);
        for (int z = cz - r; z <= cz + r; z++)
            for (int x = cx - r; x <= cx + r; x++)
            {
                if (x < 0 || z < 0 || x >= NX || z >= NZ) continue;
                float dx = CX(x) - centre.x, dz = CZ(z) - centre.z;
                if (dx * dx + dz * dz > rayon * rayon) continue;
                if (Dist[I(x, z)] < 1.6f) continue;
                H[I(x, z)] = Quant(h);
            }
    }

    /// Case à k pas sur la grande route (k négatif : en arrière).
    int IleModele::Pas(int c, int k) const
    {
        const Regles::Plateau& pl = *Plateau;
        for (int i = 0; i < Mathf::Abs(k); i++)
        {
            const Liste<int>& l = k > 0 ? pl[c]->Suivants : pl[c]->Precedents;
            if (l.Count() == 0) break;
            int suivant = l[0];
            for (int x : l)
                if (!pl[x]->Raccourci)
                {
                    suivant = x;
                    break;
                }
            c = suivant;
        }
        return c;
    }

    /// Rayon occupé par un bâtiment ou un décor posé (pour aplanir le sol et écarter le reste).
    float IleModele::RayonLieu(const string& id)
    {
        if (id == "montagne_antre") return 7.5f;
        if (id == "maison" || id == "taverne" || id == "chapelle" || id == "mausolee" || id == "ruine") return 2.6f;
        if (id == "tour_garde" || id == "porte_ville" || id == "forge" || id == "comptoir_fossoyeurs" || id == "portail_cryptes" || id == "rempart" || id == "pont") return 2.2f;
        if (id == "ponton") return 1.8f;
        if (id == "etal" || id == "autel_ombrelune" || id == "puits") return 1.4f;
        if (id == "banniere" || id == "panneau") return 0.7f;
        return !id.empty() && Texte::CommencePar(id, "monstre:") ? 2.4f : 1.2f;
    }

    /// Pose un lieu au bord de la case : à l'extérieur d'abord, à l'intérieur si la place est prise.
    bool IleModele::LieuPres(const string& id, int c, int g)
    {
        float r = RayonLieu(id);
        float dist = Mathf::Max(1.7f, r + 1.4f);
        for (bool ext : {true, false})
        {
            Vector3 pos = Bord(c, dist, ext);
            if (Ellipse(pos.x, pos.z) > 0.93f || !LibrePourLieu(pos, r)) continue;
            Lieu(id, pos, CapVers(pos, PosCase(c)), r, g);
            return true;
        }
        return false;
    }

    /// Place libre pour un bâtiment de la ville : seules comptent les zones posées avant la ville, et le chemin.
    bool IleModele::LibrePourVille(Vector3 p, float r, int avantVille) const
    {
        for (int i = 0; i < avantVille && i < zonesInterdites.Count(); i++)
        {
            const std::pair<Vector3, float>& kv = zonesInterdites[i];
            float dx = p.x - kv.first.x, dz = p.z - kv.first.z, rr = kv.second * 0.8f + r * 0.5f;
            if (dx * dx + dz * dz < rr * rr) return false;
        }
        return DistanceChemin(p.x, p.z) > Mathf::Min(r, 2.2f) * 0.75f;
    }

    bool IleModele::LibrePourLieu(Vector3 p, float r) const
    {
        for (const auto& kv : zonesInterdites)
        {
            float dx = p.x - kv.first.x, dz = p.z - kv.first.z, rr = kv.second + r * 0.8f;
            if (dx * dx + dz * dz < rr * rr) return false;
        }
        return DistanceChemin(p.x, p.z) > Mathf::Min(r, 2.2f) * 0.75f;
    }

    /// Le bâtiment qui va avec une case quand l'auteur de la carte n'a rien choisi.
    string IleModele::LieuAuto(const Ptr<Case>& c, const Ptr<Regles::Plateau>& p)
    {
        switch (c->Type)
        {
            case TypeCase::Taverne: return "taverne";
            case TypeCase::Sanctuaire: return c->Zone == Zone::Cryptes || c->Zone == Zone::Pics || c->Zone == Zone::Antre ? "autel_ombrelune" : "chapelle";
            case TypeCase::Marchand:
                if (c->Zone == Zone::Cryptes) return "comptoir_fossoyeurs";
                if (c->Zone == Zone::Pics || c->Zone == Zone::Antre) return "forge";
                return "etal";
            case TypeCase::Malediction: return c->Zone == Zone::Cryptes ? "mausolee" : (c->Zone == Zone::Pics || c->Zone == Zone::Antre ? "ruine" : "");
            case TypeCase::Ville: return "maison";
            default: break;
        }
        if (c->Raccourci && c->Precedents.Count() > 0 && !(*p)[c->Precedents[0]]->Raccourci && c->Zone == Zone::Cryptes) return "portail_cryptes";
        if (c->Zone == Zone::Marais && c->Type == TypeCase::Evenement) return "ponton";
        return "";
    }

    void IleModele::PlacerLieux()
    {
        const Regles::Plateau& pl = *Plateau;
        int d = pl.Depart;
        Vector3 p0 = PosCase(d);
        bool automatique = Options->LieuxAutomatiques;          // (« auto » en C#)
        // La montagne de l'antre d'abord (elle prend le plus de place), et le boss endormi devant.
        int antre = -1;
        for (int i = 0; i < pl.Nombre(); i++)
            if (pl[i]->Type == TypeCase::Antre) antre = i;
        if (antre >= 0 && pl[antre]->Lieu != "-")
        {
            Vector3 pa = PosCase(antre);
            Lieu("montagne_antre", Montagne, CapVers(Montagne, pa), 7.5f);
            auto boss = Donnees::Boss();
            if (boss != nullptr)
            {
                Vector3 bp = Vector3::Lerp(pa, Montagne, 0.42f);
                Lieu("monstre:" + boss->Modele(), bp, CapVers(bp, pa) + 20.0f, 2.6f);
            }
        }
        if (automatique && pl[d]->Lieu != "-")
        {
            // La ville de départ : bannières, maisons serrées de part et d'autre, une tour, un puits, une porte sur
            // la route. Les maisons de la ville peuvent se toucher (c'est une ville) : on ne vérifie la place que
            // par rapport à ce qui était posé avant elle (montagne, boss) et au chemin.
            int avantVille = zonesInterdites.Count();
            Lieu("banniere", Bord(d, 3.4f, true, 1.6f), CapVers(Bord(d, 3.4f, true, 1.6f), p0), 0.8f, 1);
            Lieu("banniere", Bord(d, 3.4f, true, -1.6f), CapVers(Bord(d, 3.4f, true, -1.6f), p0), 0.8f, 2);
            int maison = 0;
            for (int k = -3; k <= 3; k++)
            {
                if (k == 0) continue;
                int c = Pas(d, k);
                Vector3 b = Bord(c, 6.2f + static_cast<float>(Mathf::Abs(k) % 2) * 1.6f, true, 0.0f);
                if (Ellipse(b.x, b.z) < 0.9f && LibrePourVille(b, 2.4f, avantVille)) Lieu("maison", b, CapVers(b, PosCase(c)), 2.4f, 11 + maison++);
                Vector3 b2 = Bord(c, 10.8f, true, 1.2f);
                if (Ellipse(b2.x, b2.z) < 0.9f && LibrePourVille(b2, 2.4f, avantVille)) Lieu("maison", b2, CapVers(b2, p0), 2.4f, 31 + maison++);
            }
            Vector3 tour = Bord(Pas(d, 2), 7.5f, false, 0.0f);
            if (LibrePourVille(tour, 2.2f, avantVille)) Lieu("tour_garde", tour, 0.0f, 2.2f, 1);
            for (int k : {1, -2})
            {
                Vector3 b = Bord(Pas(d, k), k < 0 ? 4.8f : 4.6f, false, k < 0 ? 0.8f : 0.0f);
                if (LibrePourVille(b, 2.4f, avantVille)) Lieu("maison", b, CapVers(b, PosCase(Pas(d, k))), 2.4f, 50 + maison++);
            }
            Vector3 puits = Bord(Pas(d, -2), 3.0f, true);
            if (LibrePourVille(puits, 1.2f, avantVille)) Lieu("puits", puits, CapVers(puits, PosCase(Pas(d, -2))), 1.2f);
            // la porte enjambe la route, quelques cases avant la ville
            int a = Pas(d, -5), b3 = Pas(d, -4);
            if (a != d && b3 != d && a != b3)
            {
                Vector3 porte = (PosCase(a) + PosCase(b3)) * 0.5f;
                Lieu("porte_ville", porte, CapVers(PosCase(a), PosCase(b3)), 2.2f);
            }
        }
        // Chaque case : le bâtiment choisi par l'auteur, sinon celui de son type.
        int g = 1;
        for (int i = 0; i < pl.Nombre(); i++)
        {
            const Ptr<Case>& c = pl[i];
            if (i == d || c->Type == TypeCase::Antre || c->Lieu == "-") continue;
            string id = !c->Lieu.empty() ? c->Lieu : (automatique ? LieuAuto(c, Plateau) : string());
            if (!id.empty()) LieuPres(id, i, g++);
            if (automatique && c->Suivants.Count() > 1) LieuPres("panneau", i, i);
        }
        // Les décors posés à la main dans l'éditeur.
        for (const auto& dp : Options->Decors)
        {
            if (dp->Id.empty()) continue;
            Vector3 pos(dp->X, 0.0f, dp->Z);
            Lieu(dp->Id, pos, dp->RotY, RayonLieu(dp->Id), dp->Graine, dp->Echelle <= 0 ? 1.0f : dp->Echelle);
        }
    }

    // ------------------------------------------------------------------ décor semé

    float IleModele::Poids(const string& id)
    {
        if (id == "chene" || id == "pin" || id == "herbes_hautes" || id == "roseaux") return 3.0f;
        if (id == "saule" || id == "arbre_mort" || id == "buisson" || id == "fleurs" || id == "fougere" || id == "tombe" || id == "pin_brule" || id == "stalagmite") return 2.0f;
        if (id == "rocher" || id == "rocher_mousse" || id == "pic_rocheux" || id == "champignon_marais") return 1.5f;
        if (id == "os" || id == "lave" || id == "tas_or") return 0.7f;
        if (id == "cristal_braise" || id == "cierge" || id == "feu_follet") return 0.5f;
        return 1.0f;
    }

    float IleModele::Densite(Zone z)
    {
        switch (z)
        {
            case Zone::Bois: return 0.8f;
            case Zone::Village: return 0.22f;
            case Zone::Marais: return 0.45f;
            case Zone::Cryptes: return 0.42f;
            case Zone::Pics: return 0.42f;
            default: return 0.32f;
        }
    }

    bool IleModele::Libre(Vector3 p, float marge) const
    {
        for (const auto& kv : zonesInterdites)
        {
            float dx = p.x - kv.first.x, dz = p.z - kv.first.z, r = kv.second + marge;
            if (dx * dx + dz * dz < r * r) return false;
        }
        const Regles::Plateau& pl = *Plateau;
        for (int c = 0; c < pl.Nombre(); c++)
        {
            float dx = p.x - pl[c]->X, dz = p.z - pl[c]->Z;
            if (dx * dx + dz * dz < 2.2f * 2.2f) return false;
        }
        return true;
    }

    const Ensemble<string>& IleModele::Grands()
    {
        static const Ensemble<string> g = {"chene", "pin", "saule", "arbre_mort", "pin_brule", "pic_rocheux", "stalagmite"};
        return g;
    }

    void IleModele::Semer()
    {
        const Regles::Plateau& pl = *Plateau;
        AleaNet r(static_cast<int>(static_cast<uint32_t>(graine) * 977u + 3u));
        float densite = Mathf::Max(0.0f, Options->DensiteDecor);
        if (!Options->DecorAutomatique || densite <= 0.0f)
        {
            PlacerLampes();
            return;
        }
        const float pas = 1.7f;
        for (float z = Z0 + 1; z < Z0 + static_cast<float>(NZ) * Cell - 1; z += pas)
            for (float x = X0 + 1; x < X0 + static_cast<float>(NX) * Cell - 1; x += pas)
            {
                float px = x + static_cast<float>(r.NextDouble() - 0.5) * pas * 0.9f, pz = z + static_cast<float>(r.NextDouble() - 0.5) * pas * 0.9f;
                float e = Ellipse(px, pz);
                if (e > 0.95f) continue;
                Zone zo = ZoneEn(px, pz);
                float d = DistanceChemin(px, pz);
                float h = HauteurEn(px, pz);
                if (d < 2.3f) continue;
                Liste<string> ids = Catalogue::DecorsDeZone(zo);
                if (ids.Count() == 0) continue;
                if (r.NextDouble() > Mathf::Min(1.0f, Densite(zo) * densite) * (d < 4.0f ? 0.55f : 1.0f)) continue;
                string id = Tirer(ids, r);
                bool eau = h < NiveauEau + 0.01f;
                if (eau && id != "roseaux")
                {
                    if (zo == Zone::Marais && r.NextDouble() < 0.5) id = "roseaux";
                    else continue;
                }
                if (!eau && id == "roseaux" && zo == Zone::Marais && r.NextDouble() < 0.6) continue;
                // les grands éléments s'écartent du chemin pour ne pas cacher les cases
                if (Grands().Contains(id) && d < 3.6f) continue;
                if (Grands().Contains(id) && CacheUneCase(px, pz)) continue;
                Vector3 pos(px, h, pz);
                if (!Libre(pos, Grands().Contains(id) ? 1.2f : 0.4f)) continue;
                // new Placement { Id = id, Graine = r.Next(1, 100000), Pos = pos, RotY = …, Echelle = … } : dans cet ordre
                Placement pd;
                pd.Id = id;
                pd.Graine = r.Next(1, 100000);
                pd.Pos = pos;
                pd.RotY = static_cast<float>(r.NextDouble()) * 360.0f;
                pd.Echelle = 0.85f + static_cast<float>(r.NextDouble()) * 0.35f;
                Decors.Add(pd);
            }
        // Petits détails le long du chemin
        for (int c = 0; c < pl.Nombre(); c++)
            for (int s : pl[c]->Suivants)
            {
                Vector3 a = PosCase(c); Vector3 b = PosCase(s);
                for (int k = 0; k < 3; k++)
                {
                    float t = static_cast<float>(r.NextDouble());
                    Vector3 p = Vector3::Lerp(a, b, t);
                    Vector3 dir = (b - a).normalized(); Vector3 n(-dir.z, 0.0f, dir.x);
                    // p += n * ((r.NextDouble() < 0.5 ? -1f : 1f) * (1.6f + (float)r.NextDouble() * 1.2f)) : deux tirages, de gauche à droite
                    float signe = r.NextDouble() < 0.5 ? -1.0f : 1.0f;
                    float ecart = 1.6f + static_cast<float>(r.NextDouble()) * 1.2f;
                    p += n * (signe * ecart);
                    if (DistanceChemin(p.x, p.z) < 1.4f || !Libre(p, 0.2f)) continue;
                    Zone zo = ZoneEn(p.x, p.z);
                    string id;
                    switch (zo)
                    {
                        case Zone::Village: id = r.NextDouble() < 0.6 ? "fleurs" : "herbes_hautes"; break;
                        case Zone::Bois: id = r.NextDouble() < 0.5 ? "fougere" : "champignon"; break;
                        case Zone::Marais: id = r.NextDouble() < 0.6 ? "roseaux" : "champignon_marais"; break;
                        case Zone::Cryptes: id = r.NextDouble() < 0.5 ? "os" : "herbes_hautes"; break;
                        case Zone::Pics: id = r.NextDouble() < 0.5 ? "os" : "rocher"; break;
                        default: id = r.NextDouble() < 0.5 ? "os" : "tas_or"; break;
                    }
                    p.y = HauteurEn(p.x, p.z);
                    if (p.y < NiveauEau && id != "roseaux") continue;
                    Placement pd;
                    pd.Id = id;
                    pd.Graine = r.Next(1, 100000);
                    pd.Pos = p;
                    pd.RotY = static_cast<float>(r.NextDouble()) * 360.0f;
                    pd.Echelle = 0.8f + static_cast<float>(r.NextDouble()) * 0.3f;
                    Decors.Add(pd);
                }
            }
        PlacerLampes();
    }

    /// Un grand élément juste au sud d'une case la cacherait à la caméra.
    bool IleModele::CacheUneCase(float x, float z) const
    {
        const Regles::Plateau& pl = *Plateau;
        for (int c = 0; c < pl.Nombre(); c++)
        {
            float dz = pl[c]->Z - z, dx = Mathf::Abs(pl[c]->X - x);
            if (dz > 0 && dz < 7.5f && dx < 3.2f) return true;
        }
        return false;
    }

    string IleModele::Tirer(const Liste<string>& ids, AleaNet& r)
    {
        float tot = 0;
        for (auto& id : ids) tot += Poids(id);
        float v = static_cast<float>(r.NextDouble()) * tot;
        for (auto& id : ids)
        {
            v -= Poids(id);
            if (v <= 0) return id;
        }
        return ids[ids.Count() - 1];
    }

    void IleModele::PlacerLampes()
    {
        for (const auto& l : Lieux)
        {
            if (l.Id == "taverne") Lampes.Add(NouvelleLampe(l.Pos + Vector3::up * 2.2f + Quaternion::AngleAxis(l.RotY, Vector3::up) * Vector3(0.0f, 0.0f, 2.3f), Pal::C("ffb45a"), 7.0f, 2.2f));
            else if (l.Id == "forge") Lampes.Add(NouvelleLampe(l.Pos + Vector3::up * 1.2f, Pal::C("ff7a2a"), 7.0f, 3.0f));
            else if (l.Id == "chapelle") Lampes.Add(NouvelleLampe(l.Pos + Vector3::up * 2.5f + Quaternion::AngleAxis(l.RotY, Vector3::up) * Vector3(0.0f, 0.0f, 2.6f), Pal::C("cfe4ff"), 6.0f, 1.4f));
            else if (l.Id == "autel_ombrelune") Lampes.Add(NouvelleLampe(l.Pos + Vector3::up * 1.5f, Pal::C("b9a6ff"), 5.0f, 1.6f));
            else if (l.Id == "comptoir_fossoyeurs") Lampes.Add(NouvelleLampe(l.Pos + Vector3::up * 2.0f, Pal::C("b5ff9a"), 5.0f, 1.3f));
            else if (l.Id == "porte_ville")
            {
                Quaternion q = Quaternion::AngleAxis(l.RotY, Vector3::up);
                Lampes.Add(NouvelleLampe(l.Pos + q * Vector3(2.6f, 2.6f, -1.0f), Pal::C("ffb45a"), 6.0f, 2.0f));
                Lampes.Add(NouvelleLampe(l.Pos + q * Vector3(-2.6f, 2.6f, -1.0f), Pal::C("ffb45a"), 6.0f, 2.0f));
            }
            else if (l.Id == "montagne_antre") Lampes.Add(NouvelleLampe(l.Pos + Quaternion::AngleAxis(l.RotY, Vector3::up) * Vector3(0.0f, 1.5f, 5.5f), Pal::C("ff5a1e"), 12.0f, 3.5f));
        }
        int restantes = 30 - Lampes.Count();
        for (const auto& d : Decors)
        {
            if (restantes <= 0) break;
            Color32 c;
            float portee = 0.0f, intensite = 0.0f;
            if (d.Id == "lave") { c = Pal::C("ff6a1e"); portee = 4.5f; intensite = 2.4f; }
            else if (d.Id == "cristal_braise") { c = Pal::C("ff8a3a"); portee = 3.5f; intensite = 1.8f; }
            else if (d.Id == "feu_follet") { c = Pal::C("7ff5d0"); portee = 4.0f; intensite = 2.0f; }
            else continue;
            bool proche = false;
            for (const auto& l : Lampes)
                if ((l.Pos - d.Pos).sqrMagnitude() < 36.0f)
                {
                    proche = true;
                    break;
                }
            if (proche) continue;
            Lampes.Add(NouvelleLampe(d.Pos + Vector3::up * 0.8f, c, portee, intensite));
            restantes--;
        }
    }
}
