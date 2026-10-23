// Donjon de l'Oie — jouer un événement décrit en données : annonce, effets (avec chance et cible), choix éventuel.
#include "Partie.h"

namespace DonjonDeLOie::Regles
{
    using Moteur::Texte::De;

    /// Un événement de la liste, tiré au hasard selon les poids (ceux de la zone d'abord, s'il y en a).
    Ptr<DefEvenement> Partie::TirerEvenement(const Liste<string>& ids, Zone zone)
    {
        Liste<Ptr<DefEvenement>> l;
        for (auto& id : ids)
        {
            auto e = Donnees::Evenement(id);
            if (e != nullptr && e->PossibleEn(zone)) l.Add(e);
        }
        if (l.Count() == 0)
            for (auto& id : ids)
            {
                auto e = Donnees::Evenement(id);
                if (e != nullptr) l.Add(e);
            }
        return Tirer(l);
    }

    /// Un événement de la pioche commune, possible dans cette zone.
    Ptr<DefEvenement> Partie::TirerEvenementPioche(Zone zone)
    {
        Liste<Ptr<DefEvenement>> l;
        for (auto& e : Donnees::Evenements)
            if (e->DansLaPioche && e->PossibleEn(zone)) l.Add(e);
        return Tirer(l);
    }

    Ptr<DefEvenement> Partie::Tirer(const Liste<Ptr<DefEvenement>>& l)
    {
        if (l.Count() == 0) return nullptr;
        int tot = 0;
        for (auto& e : l) tot += std::max(1, e->Poids);
        int v = Des.Entre(1, tot);
        for (auto& e : l)
        {
            v -= std::max(1, e->Poids);
            if (v <= 0) return e;
        }
        return l[l.Count() - 1];
    }

    string Partie::TexteEvenement(const string& t, const Ptr<Joueur>& j) { return Moteur::Texte::Remplacer(t, "{joueur}", j->Nom); }

    Signaux Partie::JouerEvenement(Ptr<Joueur> j, Ptr<DefEvenement> ev)
    {
        if (ev == nullptr)
        {
            co_yield Nouveau<SAnnonce>(j->Nom + " ne croise personne.", j->Id);
            co_return;
        }
        {
            auto e = Nouveau<SEffet>(Effet::Evenement);
            e->Joueur = j->Id;
            e->Texte = ev->Nom;
            co_yield e;
        }
        if (!ev->Texte.empty()) co_yield Nouveau<SAnnonce>(TexteEvenement(ev->Texte, j), j->Id, ev->Ton, true);
        for (auto& ef : ev->Effets)
        {
            for (auto& s : Appliquer(j, ef, ev->Nom)) co_yield s;
            if (Arret()) co_return;
        }
        if (ev->Choix.Count() == 0) co_return;
        auto q = Nouveau<SQuestion>(j->Id, GenreQuestion::ChoixEvenement, ev->Nom, TexteEvenement(ev->Texte, j));
        for (int i = 0; i < ev->Choix.Count(); i++)
        {
            q->Ajouter(ev->Choix[i]->Libelle, "choix", ev->Choix[i]->Aide, PeutChoisir(j, ev->Choix[i]));
            q->EffetsChoix.Add(ev->Choix[i]->Effets);
        }
        if (!q->Actives.Contains(true)) co_return;
        co_yield q;
        int r = q->Reponse;
        if (r < 0 || r >= ev->Choix.Count() || !q->Actives[r]) r = q->Actives.IndexOf(true);
        auto ch = ev->Choix[r];
        co_yield Nouveau<SAnnonce>(j->Nom + " : " + Moteur::Texte::Minuscules(ch->Libelle) + ".", j->Id, ev->Ton);
        for (auto& ef : ch->Effets)
        {
            for (auto& s : Appliquer(j, ef, ev->Nom)) co_yield s;
            if (Arret()) co_return;
        }
    }

    /// Un choix qui coûte de l'or n'est possible que si l'on a cet or.
    bool Partie::PeutChoisir(const Ptr<Joueur>& j, const Ptr<ChoixEvenement>& c)
    {
        int cout = 0;
        for (auto& e : c->Effets)
            if (e->Type == TypeEffet::Or && e->Valeur < 0 && e->Chance >= 100 && e->Cible == CibleEffet::Joueur) cout -= e->Valeur;
        return j->Or >= cout;
    }

    Liste<Ptr<Joueur>> Partie::Cibles(const Ptr<Joueur>& j, CibleEffet c) const
    {
        Liste<Ptr<Joueur>> l;
        switch (c)
        {
            case CibleEffet::Tous: l.AddRange(Joueurs); break;
            case CibleEffet::Autres:
                for (auto& o : Joueurs)
                    if (o != j) l.Add(o);
                break;
            case CibleEffet::Meneur:
            {
                Ptr<Joueur> m;
                for (auto& o : Joueurs)
                    if (m == nullptr || o->Niveau > m->Niveau || (o->Niveau == m->Niveau && o->XP > m->XP)) m = o;
                l.Add(m);
                break;
            }
            case CibleEffet::Dernier:
            {
                Ptr<Joueur> m;
                for (auto& o : Joueurs)
                    if (m == nullptr || o->Niveau < m->Niveau || (o->Niveau == m->Niveau && o->XP < m->XP)) m = o;
                l.Add(m);
                break;
            }
            default: l.Add(j); break;
        }
        return l;
    }

    Signaux Partie::Appliquer(Ptr<Joueur> j, Ptr<EffetEvenement> ef, string raison)
    {
        if (ef == nullptr) co_return;
        if (ef->Chance < 100 && Des.Entre(1, 100) > std::max(0, ef->Chance)) co_return;
        if (EffetEvenement::SeulementLeJoueur(ef->Type))
        {
            for (auto& s : AppliquerA(j, j, ef, raison)) co_yield s;
            co_return;
        }
        for (auto& cible : Cibles(j, ef->Cible))
        {
            for (auto& s : AppliquerA(j, cible, ef, raison)) co_yield s;
            if (Fini) co_return;
        }
    }

    namespace
    {
        /// Enum.TryParse de C# (sans casse) pour un type d'objet : un nom (« Potion ») ou un nombre.
        bool LireTypeObjet(const string& texte, TypeObjet& sortie)
        {
            string t = Moteur::Texte::Rogner(texte);
            int n;
            if (Moteur::Texte::LireEntier(t, n))
            {
                sortie = static_cast<TypeObjet>(n);
                return true;
            }
            for (int i = 0; i <= static_cast<int>(TypeObjet::Parchemin); i++)
                if (Moteur::Texte::EgauxSansCasse(t, NomEnum(static_cast<TypeObjet>(i))))
                {
                    sortie = static_cast<TypeObjet>(i);
                    return true;
                }
            return false;
        }
    }

    Signaux Partie::AppliquerA(Ptr<Joueur> j, Ptr<Joueur> t, Ptr<EffetEvenement> ef, string raison)
    {
        string r = Moteur::Texte::Minuscules(raison);
        switch (ef->Type)
        {
            case TypeEffet::Annonce:
                co_yield Nouveau<SAnnonce>(TexteEvenement(ef->Parametre, j), j->Id, Ton::Neutre);
                break;
            case TypeEffet::Or:
                if (ef->Valeur >= 0)
                    for (auto& s : GagnerOr(t, ef->Valeur, r)) co_yield s;
                else
                    for (auto& s : PerdreOr(t, -ef->Valeur, r)) co_yield s;
                break;
            case TypeEffet::PV:
                if (ef->Valeur >= 0)
                    for (auto& s : Soigner(t, ef->Valeur, raison)) co_yield s;
                else
                    for (auto& s : Blesser(t, -ef->Valeur, r)) co_yield s;
                break;
            case TypeEffet::SoinTotal:
                for (auto& s : Soigner(t, t->PVMax(), raison)) co_yield s;
                break;
            case TypeEffet::XP:
                if (ef->Valeur >= 0)
                    for (auto& s : GagnerXP(t, ef->Valeur)) co_yield s;
                else
                {
                    int x = std::min(t->XP, -ef->Valeur);
                    if (x > 0)
                    {
                        t->XP -= x;
                        co_yield Nouveau<SStat>(t->Id, Stat::XP, -x);
                    }
                }
                break;
            case TypeEffet::Niveau:
                if (ef->Valeur > 0)
                    for (auto& s : GagnerXP(t, ef->Valeur * Donnees::XPParNiveau())) co_yield s;
                else if (ef->Valeur < 0)
                {
                    bool bloque = false;
                    for (auto& s : Amulette(t, [&bloque](bool b) { bloque = b; })) co_yield s;
                    if (!bloque)
                        for (auto& s : PerdreNiveau(t, -ef->Valeur, r)) co_yield s;
                }
                break;
            case TypeEffet::Avancer:
                for (auto& s : Marcher(j, std::max(1, ef->Valeur), true)) co_yield s;
                break;
            case TypeEffet::Reculer:
                for (int i = 0; i < std::max(1, ef->Valeur) && Plateau[j->Case]->Precedents.Count() > 0; i++)
                {
                    int de = j->Case, vers = Plateau[j->Case]->Precedents[0];
                    j->Case = vers;
                    co_yield Nouveau<SPas>(j->Id, de, vers);
                }
                for (auto& s : ArretDragonnoie(j, nullptr)) co_yield s;
                break;
            case TypeEffet::TeleporterHasard:
            {
                Liste<int> cases;
                for (auto& c : Plateau.Cases)
                    if (!c->Raccourci && c->Type != TypeCase::Ville && c->Id != j->Case) cases.Add(c->Id);
                if (cases.Count() == 0) break;
                for (auto& s : Teleporter(j, Des.Choisir(cases), ef->Valeur != 0, "Portail")) co_yield s;
                break;
            }
            case TypeEffet::TeleporterDepart:
                for (auto& s : Teleporter(t, Plateau.Depart, false, "Rappel")) co_yield s;
                break;
            case TypeEffet::TeleporterCase:
            {
                int vers = -1;
                for (auto& c : Plateau.Cases)
                    if (!c->Nom.empty() && Moteur::Texte::EgauxSansCasse(c->Nom, ef->Parametre)) vers = c->Id;
                if (vers < 0)
                {
                    co_yield Nouveau<SAnnonce>("(lieu introuvable : « " + ef->Parametre + " »)", j->Id, Ton::Systeme);
                    break;
                }
                for (auto& s : Teleporter(j, vers, ef->Valeur != 0, raison)) co_yield s;
                break;
            }
            case TypeEffet::Combat:
            {
                auto m = ef->Parametre.empty() ? TirerMonstre(j, Plateau[j->Case]->Zone) : Donnees::Monstre(Moteur::Texte::Rogner(ef->Parametre));
                if (m == nullptr || m->Errant) m = TirerMonstre(j, Plateau[j->Case]->Zone);     // le Dragonnoie ne se combat que sur sa case
                for (auto& s : CombatMonstre(j, m, true)) co_yield s;
                break;
            }
            case TypeEffet::Boutique:
            {
                Liste<Ptr<DefObjet>> offre;
                if (!ef->Parametre.empty())
                    for (auto& id : Moteur::Texte::Decouper(ef->Parametre, ','))
                    {
                        auto o = Donnees::Objet(Moteur::Texte::Rogner(id));
                        if (o != nullptr && !offre.Contains(o)) offre.Add(o);
                    }
                if (offre.Count() == 0) offre = OffreColporteur();
                float remise = std::max(0.0f, 1.0f - static_cast<float>(std::max(0, std::min(100, ef->Valeur))) / 100.0f);
                for (auto& s : Boutique(j, offre, Moteur::Texte::Ou(raison, "Le colporteur"), remise)) co_yield s;
                break;
            }
            case TypeEffet::Quetes:
                for (auto& s : ProposerQuetes(j, ef->Parametre.empty() ? Moteur::Texte::Ou(raison, "Un voyageur") : ef->Parametre)) co_yield s;
                break;
            case TypeEffet::ObjetHasard:
            case TypeEffet::ObjetDonne:
            {
                Ptr<DefObjet> o;
                if (ef->Type == TypeEffet::ObjetDonne) o = Donnees::Objet(Moteur::Texte::Rogner(ef->Parametre));
                else
                {
                    Liste<Ptr<DefObjet>> pool;
                    TypeObjet voulu = TypeObjet::Arme;
                    bool filtre = LireTypeObjet(ef->Parametre, voulu) && Moteur::Texte::Rogner(ef->Parametre).size() > 0;
                    for (auto& x : Donnees::Objets)
                        if (filtre ? x->Type == voulu : x->Consommable()) pool.Add(x);
                    if (pool.Count() > 0) o = Des.Choisir(pool);
                }
                if (o == nullptr) break;
                bool place = o->Consommable() ? t->Sac.Count() < Donnees::TailleSac() : PeutEquiper(t, o);
                if (place)
                {
                    Equiper(t, o);
                    {
                        auto e = Nouveau<SEffet>(Effet::Achat);
                        e->Joueur = t->Id;
                        e->Objet = o->Id;
                        co_yield e;
                    }
                    co_yield Nouveau<SAnnonce>(t->Nom + " obtient : " + o->Nom + " !", t->Id, Ton::Bon);
                }
                else if (ef->Valeur > 0)
                {
                    co_yield Nouveau<SAnnonce>(t->Nom + " n'a pas de place pour " + o->Nom + " et le revend.", t->Id, Ton::Neutre);
                    for (auto& s : GagnerOr(t, ef->Valeur, "")) co_yield s;
                }
                break;
            }
            case TypeEffet::Faiblesse:
            {
                t->ToursMaudit = std::max(t->ToursMaudit, std::max(1, ef->Valeur));
                auto e = Nouveau<SEffet>(Effet::Malediction);
                e->Joueur = t->Id;
                co_yield e;
                break;
            }
            case TypeEffet::Immobiliser:
                t->ToursImmobilise = std::max(t->ToursImmobilise, std::max(1, ef->Valeur));
                co_yield Nouveau<SAnnonce>(t->Nom + " passera " + (t->ToursImmobilise > 1 ? De(t->ToursImmobilise) + " tours." : string("son prochain tour.")),
                                           t->Id, Ton::Mauvais);
                break;
            case TypeEffet::Lenteur:
                t->Lenteur = true;
                co_yield Nouveau<SAnnonce>(t->Nom + " est ralenti.", t->Id, Ton::Mauvais);
                break;
            case TypeEffet::Benediction:
                t->Benediction += std::max(1, ef->Valeur);
                co_yield Nouveau<SAnnonce>(t->Nom + " est béni : +" + De(t->Benediction) + " à son prochain combat.", t->Id, Ton::Bon);
                break;
            case TypeEffet::JetMalediction:
                for (auto& s : Malediction(t)) co_yield s;
                break;
            case TypeEffet::LeverMalediction:
                if (t->ToursMaudit > 0 || t->Lenteur || t->ToursImmobilise > 0)
                {
                    t->ToursMaudit = 0;
                    t->Lenteur = false;
                    t->ToursImmobilise = 0;
                    co_yield Nouveau<SAnnonce>("Les malédictions de " + t->Nom + " sont levées.", t->Id, Ton::Bon);
                }
                break;
        }
    }

    Signaux Partie::Teleporter(Ptr<Joueur> j, int vers, bool effetArrivee, string raison)
    {
        j->Case = vers;
        j->Embuscade = nullptr;
        co_yield Nouveau<STeleport>(j->Id, vers, raison);
        for (auto& s : VerifierQueteLieu(j)) co_yield s;
        bool oie = false;
        for (auto& s : ArretDragonnoie(j, [&oie](bool r) { oie = r; })) co_yield s;
        if (oie) co_return;                       // le Dragonnoie occupe la case : pas d'effet d'arrivée
        if (effetArrivee && !Arret() && j == JoueurCourant())
            for (auto& s : Arrivee(j)) co_yield s;
    }

    bool Partie::PeutEquiper(const Ptr<Joueur>& j, const Ptr<DefObjet>& o)
    {
        switch (o->Type)
        {
            case TypeObjet::Arme: return j->Arme == nullptr || j->Arme->Bonus < o->Bonus;
            case TypeObjet::Armure: return j->Armure == nullptr || j->Armure->Bonus < o->Bonus;
            case TypeObjet::Accessoire: return !j->A(o->Effet) && j->Accessoires.Count() < Donnees::AccessoiresMax();
            default: break;
        }
        return j->Sac.Count() < Donnees::TailleSac();
    }

    void Partie::Equiper(const Ptr<Joueur>& j, const Ptr<DefObjet>& o)
    {
        switch (o->Type)
        {
            case TypeObjet::Arme: j->Arme = o; break;
            case TypeObjet::Armure: j->Armure = o; break;
            case TypeObjet::Accessoire:
                j->Accessoires.Add(o);
                if (o->Effet == EffetObjet::Anneau) j->PV += o->Bonus;
                break;
            default: j->Sac.Add(o); break;
        }
    }
}
