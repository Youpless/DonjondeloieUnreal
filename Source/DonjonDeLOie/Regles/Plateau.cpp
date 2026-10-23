// Donjon de l'Oie — le plateau d'origine (grande boucle + raccourcis), et la conversion des cartes de l'éditeur.
#include "Plateau.h"

#include "Evenements.h"

#include "../Moteur/Maths.h"      // SinNet, CosNet : Math.Sin et Math.Cos de .NET

#include <cmath>
#include <deque>

namespace DonjonDeLOie::Regles
{
    string NomEnum(ModeEvenements v)
    {
        static const char* n[] = {"Standard", "Remplacer", "Ajouter"};
        return n[static_cast<int>(v)];
    }

    Liste<int> Plateau::Voisins(int id) const
    {
        auto& c = Cases[id];
        Liste<int> l;
        for (int i = 0; i < c->Suivants.Count(); i++) l.Add(c->Suivants[i]);
        for (int i = 0; i < c->Precedents.Count(); i++) l.Add(c->Precedents[i]);
        return l;
    }

    bool Plateau::SontVoisines(int a, int b) const
    {
        for (int v : Voisins(a))
            if (v == b) return true;
        return false;
    }

    int Plateau::DistanceEnAvant(int a, int b, int max) const
    {
        if (a == b) return 0;
        std::vector<int> dist(static_cast<size_t>(Cases.Count()), -1);
        std::deque<int> file;
        dist[static_cast<size_t>(a)] = 0;
        file.push_back(a);
        while (!file.empty())
        {
            int c = file.front();
            file.pop_front();
            int d = dist[static_cast<size_t>(c)];
            if (d >= max) continue;
            for (int s : Cases[c]->Suivants)
            {
                if (dist[static_cast<size_t>(s)] >= 0) continue;
                dist[static_cast<size_t>(s)] = d + 1;
                if (s == b) return d + 1;
                file.push_back(s);
            }
        }
        return -1;
    }

    Liste<int> Plateau::CasesDeType(TypeCase t) const
    {
        Liste<int> l;
        for (int i = 0; i < Cases.Count(); i++)
            if (Cases[i]->Type == t) l.Add(i);
        return l;
    }

    // ------------------------------------------------------------------ fabrication

    namespace
    {
        const int Boucle = 40;

        struct Pose
        {
            TypeCase T;
            Zone Z;
            string Nom;
            Pose(TypeCase t, Zone z, string n = "") : T(t), Z(z), Nom(std::move(n)) {}
        };

        /// Points (x, z) d'une courbe fermée, à intervalles réguliers. Valcreux est au sud (vers la caméra d'accueil).
        std::vector<float> Echantillonner(int n)
        {
            const int fin = 2000;
            const double pi = 3.14159265358979323846;
            std::vector<double> xs(fin + 1), zs(fin + 1), cum(fin + 1, 0.0);
            for (int i = 0; i <= fin; i++)
            {
                double t = -pi / 2 + 2 * pi * i / fin;
                double r = 1.0 + 0.07 * ::Moteur::SinNet(3 * t + 0.6) + 0.04 * ::Moteur::CosNet(5 * t);
                xs[static_cast<size_t>(i)] = 27.0 * ::Moteur::CosNet(t) * r;
                zs[static_cast<size_t>(i)] = 21.0 * ::Moteur::SinNet(t) * r;
                if (i > 0)
                {
                    double dx = xs[static_cast<size_t>(i)] - xs[static_cast<size_t>(i - 1)], dz = zs[static_cast<size_t>(i)] - zs[static_cast<size_t>(i - 1)];
                    cum[static_cast<size_t>(i)] = cum[static_cast<size_t>(i - 1)] + std::sqrt(dx * dx + dz * dz);
                }
            }
            std::vector<float> res(static_cast<size_t>(n * 2));
            int j = 0;
            for (int k = 0; k < n; k++)
            {
                double cible = cum[fin] * k / n;
                while (j < fin && cum[static_cast<size_t>(j + 1)] < cible) j++;
                double f = (cible - cum[static_cast<size_t>(j)]) / std::max(1e-9, cum[static_cast<size_t>(j + 1)] - cum[static_cast<size_t>(j)]);
                res[static_cast<size_t>(k * 2)] = static_cast<float>(xs[static_cast<size_t>(j)] + (xs[static_cast<size_t>(j + 1)] - xs[static_cast<size_t>(j)]) * f);
                res[static_cast<size_t>(k * 2 + 1)] = static_cast<float>(zs[static_cast<size_t>(j)] + (zs[static_cast<size_t>(j + 1)] - zs[static_cast<size_t>(j)]) * f);
            }
            return res;
        }
    }

    void Plateau::Lier(Plateau& p, int a, int b)
    {
        p.Cases[a]->Suivants.Add(b);
        p.Cases[b]->Precedents.Add(a);
    }

    namespace
    {
        void Raccourci(Plateau& p, int de, int vers, const string& nom, const std::vector<Pose>& poses, float creux,
                       void (*lier)(Plateau&, int, int))
        {
            auto a = p.Cases[de];
            auto b = p.Cases[vers];
            // Courbe de Bézier quadratique tirée vers le centre de l'île.
            float mx = (a->X + b->X) * 0.5f, mz = (a->Z + b->Z) * 0.5f;
            float cx = mx * (1.0f - creux), cz = mz * (1.0f - creux);
            int prev = de;
            int n = static_cast<int>(poses.size());
            for (int k = 0; k < n; k++)
            {
                float t = (static_cast<float>(k) + 1.0f) / (static_cast<float>(n) + 1.0f);
                float u = 1.0f - t;
                float x = u * u * a->X + 2.0f * u * t * cx + t * t * b->X;
                float z = u * u * a->Z + 2.0f * u * t * cz + t * t * b->Z;
                auto c = Nouveau<Case>();
                c->Id = p.Cases.Count();
                c->Type = poses[static_cast<size_t>(k)].T;
                c->Zone = poses[static_cast<size_t>(k)].Z;
                c->Nom = poses[static_cast<size_t>(k)].Nom;
                c->X = x;
                c->Z = z;
                c->Raccourci = true;
                if (k == 0) c->Voie = nom;
                p.Cases.Add(c);
                lier(p, prev, c->Id);
                prev = c->Id;
            }
            lier(p, prev, vers);
        }
    }

    Ptr<Plateau> Plateau::Standard()
    {
        auto p = Nouveau<Plateau>();
        const Zone V = Zone::Village, B = Zone::Bois, M = Zone::Marais, C = Zone::Cryptes, P = Zone::Pics, A = Zone::Antre;
        using T = TypeCase;
        std::vector<Pose> poses = {
            Pose(T::Ville, V, "Valcreux"),
            Pose(T::Tresor, V), Pose(T::Evenement, V), Pose(T::Monstre, V),
            Pose(T::Taverne, V, "Auberge du Sanglier"),
            Pose(T::Monstre, B), Pose(T::Tresor, B), Pose(T::Evenement, B),
            Pose(T::Chemin, B, "Carrefour des Brumes"),
            Pose(T::Monstre, B), Pose(T::Marchand, B, "Étal du colporteur"),
            Pose(T::Malediction, B), Pose(T::Monstre, B),
            Pose(T::Sanctuaire, M, "Chapelle des Saules"),
            Pose(T::Evenement, M), Pose(T::Monstre, M), Pose(T::Tresor, M),
            Pose(T::Malediction, M), Pose(T::Monstre, M),
            Pose(T::Taverne, M, "Le Crapaud Borgne"),
            Pose(T::Evenement, C),
            Pose(T::Chemin, C, "Portail des Cryptes"),
            Pose(T::Monstre, C), Pose(T::Malediction, C), Pose(T::Tresor, C),
            Pose(T::Monstre, C), Pose(T::Sanctuaire, C, "Autel d'Ombrelune"),
            Pose(T::Marchand, C, "Comptoir des Fossoyeurs"),
            Pose(T::Monstre, P), Pose(T::Evenement, P), Pose(T::Tresor, P),
            Pose(T::Chemin, P, "Col du Dragon"),
            Pose(T::Monstre, P), Pose(T::Malediction, P),
            Pose(T::Marchand, P, "Forge des Pics"), Pose(T::Monstre, P),
            Pose(T::Evenement, V), Pose(T::Tresor, V),
            Pose(T::Sanctuaire, V, "Fontaine de Valcreux"), Pose(T::Monstre, V),
        };
        DD_VERIFIE(static_cast<int>(poses.size()) == Boucle, "Plateau::Standard : la boucle doit avoir 40 cases");

        // Grande boucle : une courbe fermée irrégulière, échantillonnée à pas égaux.
        auto pts = Echantillonner(Boucle);
        for (int i = 0; i < Boucle; i++)
        {
            auto c = Nouveau<Case>();
            c->Id = i;
            c->Type = poses[static_cast<size_t>(i)].T;
            c->Zone = poses[static_cast<size_t>(i)].Z;
            c->Nom = poses[static_cast<size_t>(i)].Nom;
            c->X = pts[static_cast<size_t>(i * 2)];
            c->Z = pts[static_cast<size_t>(i * 2 + 1)];
            p->Cases.Add(c);
        }
        for (int i = 0; i < Boucle; i++) Lier(*p, i, (i + 1) % Boucle);
        p->Cases[0]->Voie = "Grande route";

        // Raccourcis
        Raccourci(*p, 8, 16, "Sentier des Brumes", {Pose(T::Monstre, M), Pose(T::Tresor, M, "Îlot des Brumes"), Pose(T::Monstre, M)}, 0.55f, Lier);
        Raccourci(*p, 21, 27, "Catacombes", {Pose(T::Monstre, C), Pose(T::Tresor, C, "Ossuaire"), Pose(T::Malediction, C)}, 0.6f, Lier);
        Raccourci(*p, 31, 38, "Antre du Dragon",
                  {Pose(T::Monstre, A), Pose(T::Tresor, A), Pose(T::Antre, A, "Antre d'Ignathor"), Pose(T::Tresor, A, "Trésor d'Ignathor")}, 0.75f, Lier);

        // La grande route est la suite de la boucle à chaque embranchement.
        for (int b : {8, 21, 31}) p->Cases[(b + 1) % Boucle]->Voie = "Grande route";
        p->Depart = 0;
        return p;
    }

    // ------------------------------------------------------------------ cartes de l'éditeur

    Ptr<Plateau> Plateau::Depuis(const CarteDef& carte)
    {
        auto p = Nouveau<Plateau>();
        for (int i = 0; i < carte.Cases.Count(); i++)
        {
            auto& k = carte.Cases[i];
            auto c = Nouveau<Case>();
            c->Id = i;
            c->Type = k->Type;
            c->Zone = k->Zone;
            c->Nom = k->Nom;
            c->X = k->X;
            c->Z = k->Z;
            c->Voie = k->Voie;
            c->Mode = k->Mode;
            c->Lieu = k->Lieu;
            for (auto& e : k->Evenements)
                if (!e.empty()) c->Evenements.Add(e);
            p->Cases.Add(c);
        }
        for (int i = 0; i < carte.Cases.Count(); i++)
            for (int s : carte.Cases[i]->Suivants)
                if (s >= 0 && s < p->Cases.Count() && s != i && !p->Cases[i]->Suivants.Contains(s)) Lier(*p, i, s);
        p->Depart = std::max(0, std::min(carte.Depart, p->Cases.Count() - 1));
        p->MarquerRaccourcis();
        return p;
    }

    void Plateau::MarquerRaccourcis()
    {
        for (auto& c : Cases) c->Raccourci = true;
        int k = Depart, garde = 0;
        while (garde++ <= Cases.Count() && Cases.Count() > 0)
        {
            Cases[k]->Raccourci = false;
            if (Cases[k]->Suivants.Count() == 0) break;
            k = Cases[k]->Suivants[0];
            if (k == Depart) break;
        }
        // la première case d'un embranchement porte le nom de la voie
        for (auto& c : Cases)
            if (c->Suivants.Count() > 1)
                for (int s : c->Suivants)
                    if (Cases[s]->Voie.empty()) Cases[s]->Voie = Cases[s]->Raccourci ? "Raccourci" : "Grande route";
    }

    Ptr<CarteDef> Plateau::VersCarte(const string& nom) const
    {
        auto d = Nouveau<CarteDef>();
        d->Nom = nom;
        d->Depart = Depart;
        for (auto& c : Cases)
        {
            auto k = Nouveau<CaseCarte>();
            k->Nom = c->Nom;
            k->Type = c->Type;
            k->Zone = c->Zone;
            k->X = c->X;
            k->Z = c->Z;
            k->Voie = c->Voie;
            k->Mode = c->Mode;
            k->Lieu = c->Lieu;
            k->Suivants.AddRange(c->Suivants);
            k->Evenements.AddRange(c->Evenements);
            d->Cases.Add(k);
        }
        return d;
    }

    namespace
    {
        Liste<bool> Atteints(const CarteDef& carte, int depuis, bool aRebours)
        {
            int n = carte.Cases.Count();
            Liste<bool> vus(n);
            std::deque<int> file;
            vus[depuis] = true;
            file.push_back(depuis);
            while (!file.empty())
            {
                int c = file.front();
                file.pop_front();
                if (!aRebours)
                {
                    for (int s : carte.Cases[c]->Suivants)
                        if (s >= 0 && s < n && !vus[s])
                        {
                            vus[s] = true;
                            file.push_back(s);
                        }
                }
                else
                {
                    for (int i = 0; i < n; i++)
                        if (!vus[i] && carte.Cases[i]->Suivants.Contains(c))
                        {
                            vus[i] = true;
                            file.push_back(i);
                        }
                }
            }
            return vus;
        }
    }

    void Plateau::Verifier(const CarteDef& carte, Liste<string>& erreurs, Liste<string>& conseils)
    {
        using Moteur::Texte::De;
        int n = carte.Cases.Count();
        if (n < 6)
        {
            erreurs.Add("Il faut au moins 6 cases (il y en a " + De(n) + ").");
            if (n == 0) return;
        }
        if (carte.Depart < 0 || carte.Depart >= n)
        {
            erreurs.Add("La case de départ n'existe pas.");
            return;
        }
        if (carte.Cases[carte.Depart]->Type != TypeCase::Ville)
            conseils.Add("La case de départ n'est pas une « Ville » : pas de soins ni de grande boutique au départ.");
        for (int i = 0; i < n; i++)
        {
            auto& c = carte.Cases[i];
            string nom = "Case " + De(i + 1) + (c->Nom.empty() ? string() : " (« " + c->Nom + " »)");
            if (c->Suivants.Count() == 0) erreurs.Add(nom + " : aucune case suivante, les pions seraient bloqués.");
            else
                for (int s : c->Suivants)
                    if (s < 0 || s >= n || s == i) erreurs.Add(nom + " : un lien mène nulle part.");
            if (c->Suivants.Count() > 2) conseils.Add(nom + " : plus de deux chemins ; le joueur choisira parmi " + De(c->Suivants.Count()) + ".");
            for (auto& e : c->Evenements)
                if (Donnees::Evenement(e) == nullptr) erreurs.Add(nom + " : événement inconnu « " + e + " ».");
            if (c->Mode != ModeEvenements::Standard && c->Evenements.Count() == 0)
                conseils.Add(nom + " : mode « " + NomEnum(c->Mode) + " » sans événement dans sa liste (il ne fera rien de plus).");
            if (c->Type == TypeCase::Antre && Donnees::Boss() == nullptr) erreurs.Add(nom + " : une case Antre demande un monstre « Boss » dans le catalogue.");
        }
        if (erreurs.Count() > 0) return;
        // tout doit être atteignable depuis le départ, et le départ atteignable depuis partout (pas de cul-de-sac)
        auto vus = Atteints(carte, carte.Depart, false);
        for (int i = 0; i < n; i++)
            if (!vus[i]) erreurs.Add("Case " + De(i + 1) + " : on ne peut pas l'atteindre depuis le départ.");
        auto retour = Atteints(carte, carte.Depart, true);
        for (int i = 0; i < n; i++)
            if (vus[i] && !retour[i]) erreurs.Add("Case " + De(i + 1) + " : on ne peut plus revenir au départ en partant d'ici (cul-de-sac).");
        Ensemble<string> noms;
        for (auto& c : carte.Cases)
            if (!c->Nom.empty() && !noms.Add(c->Nom)) conseils.Add("Deux cases s'appellent « " + c->Nom + " » : les quêtes de voyage visent la première.");
        // cases qui se chevauchent (les dalles font environ 2 m)
        int proches = 0;
        for (int i = 0; i < n; i++)
            for (int j = i + 1; j < n; j++)
            {
                float dx = carte.Cases[i]->X - carte.Cases[j]->X, dz = carte.Cases[i]->Z - carte.Cases[j]->Z;
                if (dx * dx + dz * dz < 2.1f * 2.1f && proches++ < 5)
                    conseils.Add("Cases " + De(i + 1) + " et " + De(j + 1) + " : à moins de 2 m l'une de l'autre, leurs dalles se chevauchent.");
            }
        int lieux = 0;
        for (auto& c : carte.Cases)
            if (!c->Nom.empty()) lieux++;
        if (lieux < 3) conseils.Add("Moins de 3 lieux nommés : il y aura peu de quêtes de voyage.");
    }

    Ptr<CarteDef> Plateau::CarteVide(const string& nom, int nombre, float rayon)
    {
        auto d = Nouveau<CarteDef>();
        d->Nom = nom;
        using T = TypeCase;
        const T types[] = {T::Ville, T::Tresor, T::Monstre, T::Evenement, T::Chemin, T::Monstre, T::Marchand, T::Tresor,
                           T::Malediction, T::Monstre, T::Taverne, T::Evenement, T::Monstre, T::Sanctuaire, T::Tresor, T::Monstre};
        const int nbTypes = 16;
        const double pi = 3.14159265358979323846;
        for (int i = 0; i < nombre; i++)
        {
            double a = -pi / 2 + 2 * pi * i / nombre;
            auto k = Nouveau<CaseCarte>();
            k->Type = types[i % nbTypes];
            k->Zone = i < nombre / 2 ? Zone::Village : Zone::Bois;
            k->X = static_cast<float>(::Moteur::CosNet(a) * rayon);
            k->Z = static_cast<float>(::Moteur::SinNet(a) * rayon * 0.8);
            if (i == 0) k->Nom = "Valcreux";
            if (k->Type == T::Taverne) k->Nom = "La Taverne";
            if (k->Type == T::Sanctuaire) k->Nom = "Le Sanctuaire";
            if (k->Type == T::Marchand) k->Nom = "L'Échoppe";
            k->Suivants.Add((i + 1) % nombre);
            d->Cases.Add(k);
        }
        return d;
    }
}
