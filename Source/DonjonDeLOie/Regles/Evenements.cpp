// Donjon de l'Oie — les événements d'origine, et l'aide des effets.
#include "Evenements.h"

namespace DonjonDeLOie::Regles
{
    bool EffetEvenement::SeulementLeJoueur(TypeEffet t)
    {
        switch (t)
        {
            case TypeEffet::Avancer:
            case TypeEffet::Reculer:
            case TypeEffet::TeleporterHasard:
            case TypeEffet::TeleporterCase:
            case TypeEffet::Combat:
            case TypeEffet::Boutique:
            case TypeEffet::Quetes:
            case TypeEffet::Annonce: return true;
            default: return false;
        }
    }

    string EffetEvenement::Aide(TypeEffet t)
    {
        switch (t)
        {
            case TypeEffet::Annonce: return "Paramètre : le texte à afficher ({joueur} = son nom).";
            case TypeEffet::Or: return "Valeur : or gagné (négatif : perdu).";
            case TypeEffet::PV: return "Valeur : PV rendus (négatif : blessure, peut mettre K.O.).";
            case TypeEffet::SoinTotal: return "Rend tous les PV.";
            case TypeEffet::XP: return "Valeur : XP gagnée (négatif : perdue, sans perdre de niveau).";
            case TypeEffet::Niveau: return "Valeur : niveaux gagnés (négatif : perdus).";
            case TypeEffet::Avancer: return "Valeur : cases à avancer. La case d'arrivée fait son effet.";
            case TypeEffet::Reculer: return "Valeur : cases à reculer (sans effet d'arrivée).";
            case TypeEffet::TeleporterHasard: return "Vers une case de la grande boucle. Valeur 1 : la case d'arrivée fait son effet.";
            case TypeEffet::TeleporterDepart: return "Vers la case de départ.";
            case TypeEffet::TeleporterCase: return "Paramètre : nom du lieu (ex. Chapelle des Saules). Valeur 1 : la case fait son effet.";
            case TypeEffet::Combat: return "Paramètre : id du monstre (vide : un monstre de la zone de la case).";
            case TypeEffet::Boutique: return "Valeur : remise en %. Paramètre : ids d'objets séparés par des virgules (vide : 5 au hasard).";
            case TypeEffet::Quetes: return "Propose des quêtes. Paramètre : nom de celui qui les propose.";
            case TypeEffet::ObjetHasard: return "Paramètre : Potion, Piege ou Parchemin (vide : n'importe quel consommable). Sac plein : Valeur or.";
            case TypeEffet::ObjetDonne: return "Paramètre : id de l'objet. Sac plein : Valeur or à la place.";
            case TypeEffet::Faiblesse: return "Valeur : nombre de tours à −2 en attaque.";
            case TypeEffet::Immobiliser: return "Valeur : nombre de tours passés.";
            case TypeEffet::Lenteur: return "Le prochain jet de déplacement est divisé par deux.";
            case TypeEffet::Benediction: return "Valeur : bonus au prochain combat contre un monstre.";
            case TypeEffet::JetMalediction: return "Le tableau des malédictions (1D6) : niveau, or, faiblesse, pétrification, brûlure ou rien.";
            default: return "Retire faiblesse, lenteur et immobilisation.";
        }
    }

    Ptr<ChoixEvenement> ChoixEvenement::Copie() const
    {
        auto c = Nouveau<ChoixEvenement>();
        c->Libelle = Libelle;
        c->Aide = Aide;
        for (auto& e : Effets) c->Effets.Add(e->Copie());
        return c;
    }

    Ptr<DefEvenement> DefEvenement::Copie() const
    {
        auto e = Nouveau<DefEvenement>();
        e->Id = Id;
        e->Nom = Nom;
        e->Texte = Texte;
        e->Ton = Ton;
        e->DansLaPioche = DansLaPioche;
        e->Poids = Poids;
        e->Zones = Zones;
        for (auto& x : Effets) e->Effets.Add(x->Copie());
        for (auto& c : Choix) e->Choix.Add(c->Copie());
        return e;
    }

    namespace
    {
        Ptr<EffetEvenement> E(TypeEffet t, int v = 0, string p = "", CibleEffet c = CibleEffet::Joueur, int chance = 100)
        {
            return Nouveau<EffetEvenement>(t, v, std::move(p), c, chance);
        }
        Ptr<DefEvenement> Ev(string id, string nom, string texte, Ton ton, Liste<Ptr<EffetEvenement>> effets = {})
        {
            return Nouveau<DefEvenement>(std::move(id), std::move(nom), std::move(texte), ton, std::move(effets));
        }
    }

    Liste<Ptr<DefEvenement>> EvenementsParDefaut::Creer()
    {
        using T = TypeEffet;
        Liste<Ptr<DefEvenement>> l = {
            Ev("fontaine", "Fontaine bénie", "Fontaine bénie : l'eau claire referme les plaies de {joueur}.", Ton::Bon, {E(T::PV, 5)}),
            Ev("bourse", "Bourse oubliée", "Bourse oubliée au bord du chemin !", Ton::Bon, {E(T::Or, 12)}),
            Ev("tirelaine", "Tire-laine", "Un tire-laine bouscule {joueur} et file avec sa bourse.", Ton::Mauvais, {E(T::Or, -10)}),
            Ev("barde", "Barde errant", "Un barde chante les exploits de {joueur}.", Ton::Bon, {E(T::XP, 1)}),
            Ev("orage", "Orage arcanique", "Orage arcanique ! La foudre frappe tous les aventuriers.", Ton::Mauvais, {E(T::PV, -2, "", CibleEffet::Tous)}),
            Ev("vent", "Vent favorable", "Vent favorable : {joueur} avance de 3 cases.", Ton::Bon, {E(T::Avancer, 3)}),
            Ev("eboulement", "Éboulement", "Éboulement ! {joueur} recule de 2 cases.", Ton::Mauvais, {E(T::Reculer, 2), E(T::PV, -1)}),
            Ev("chevalier", "Chevalier errant", "Un chevalier errant bénit l'arme de {joueur} : +2 à son prochain combat contre un monstre.", Ton::Bon,
               {E(T::Benediction, 2)}),
            Ev("colporteur", "Colporteur", "Un colporteur ouvre sa carriole : tout à −30 % !", Ton::Bon, {E(T::Boutique, 30)}),
            Ev("piece", "Pièce maudite", "Une pièce d'or brille dans la boue… maudite ! +20 or, mais −2 en attaque pendant 3 tours.", Ton::Mauvais,
               {E(T::Or, 20), E(T::Faiblesse, 3)}),
            Ev("ermite", "Vieil ermite", "Un vieil ermite a des missions à confier.", Ton::Quete, {E(T::Quetes, 0, "Le vieil ermite")}),
            Ev("portail", "Portail instable", "Un portail instable aspire {joueur}…", Ton::Neutre, {E(T::TeleporterHasard, 1)}),
            Ev("fees", "Festin des fées", "Festin des fées : {joueur} retrouve toutes ses forces.", Ton::Bon, {E(T::SoinTotal)}),
            Ev("coffrepiege", "Coffre piégé", "Coffre piégé ! Une aiguille empoisonnée, mais 15 or à l'intérieur.", Ton::Neutre, {E(T::Or, 15), E(T::PV, -3)}),
            Ev("parchemin", "Parchemin perdu", "{joueur} trouve un parchemin dans les herbes.", Ton::Bon, {E(T::ObjetHasard, 10, "Parchemin")}),
            Ev("gobelins", "Embuscade gobeline", "Des gobelins surgissent des fourrés !", Ton::Combat, {E(T::Combat, 0, "gobelin")}),
        };
        // un exemple d'événement à choix, à copier pour créer les vôtres
        auto idole = Ev("idole", "Idole oubliée", "Une idole couverte de mousse veille au bord du chemin.", Ton::Neutre);
        idole->Poids = 8;
        idole->Choix = {
            Nouveau<ChoixEvenement>("Prier (10 or d'offrande)", "−10 or, +1 XP, et vos malédictions sont levées.",
                                    Liste<Ptr<EffetEvenement>>{E(T::Or, -10), E(T::XP, 1), E(T::LeverMalediction)}),
            Nouveau<ChoixEvenement>("Piller ses gemmes", "+25 or… mais l'idole maudit les voleurs (1D6).",
                                    Liste<Ptr<EffetEvenement>>{E(T::Or, 25), E(T::JetMalediction)}),
            Nouveau<ChoixEvenement>("Passer son chemin", "Il ne se passe rien."),
        };
        l.Add(idole);
        return l;
    }

    string NomEnum(TypeEffet v)
    {
        static const char* n[] = {"Annonce", "Or", "PV", "SoinTotal", "XP", "Niveau", "Avancer", "Reculer", "TeleporterHasard", "TeleporterDepart",
                                  "TeleporterCase", "Combat", "Boutique", "Quetes", "ObjetHasard", "ObjetDonne", "Faiblesse", "Immobiliser", "Lenteur",
                                  "Benediction", "JetMalediction", "LeverMalediction"};
        return n[static_cast<int>(v)];
    }
    string NomEnum(CibleEffet v)
    {
        static const char* n[] = {"Joueur", "Tous", "Autres", "Meneur", "Dernier"};
        return n[static_cast<int>(v)];
    }
}
