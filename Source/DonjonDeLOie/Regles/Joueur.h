// Donjon de l'Oie — un joueur (humain ou bot) et son héros, ses quêtes, ses pièges, son embuscade.
#pragma once

#include "Donnees.h"

namespace DonjonDeLOie::Regles
{
    struct Quete
    {
        /// Atteindre un lieu, chasser un monstre précis, vaincre des ennemis quelconques, tendre une embuscade qui prend
        /// un joueur, défier un joueur précis, retrouver un évadé caché dans la ville (mini-jeu).
        enum class Genre { Atteindre, Chasser, Vaincre, Embuscade, Defier, Evade };
        Genre Type = Genre::Atteindre;
        int CaseCible = -1;          // Atteindre ; Evade : la ville où il se cache
        string MonstreCible;         // Chasser (id)
        int JoueurCible = -1;        // Defier
        int Graine = 0;              // Evade : l'allure de l'évadé et sa cachette
        string NomEvade;             // Evade
        bool Feminin = false;        // Evade : une évadée (pour les textes)
        string Ville;                // Evade : le nom de la ville de l'avis (celle de CaseCible)
        int Nombre = 1, Fait = 0;
        int RecompenseOr = 0, RecompenseXP = 0;
        string Titre, Texte;

        string Progression() const
        {
            switch (Type)
            {
                case Genre::Chasser:
                case Genre::Vaincre: return Moteur::Texte::De(Fait) + " / " + Moteur::Texte::De(Nombre);
                case Genre::Atteindre: return Fait > 0 ? "atteint" : "en route";
                case Genre::Evade: return "à la ville";
                default: return "en cours";
            }
        }
    };

    struct Piege
    {
        int Proprietaire = 0;
        Ptr<DefObjet> Objet;
        int Case = 0;
    };

    struct Embuscade
    {
        int Case = 0, CaseVoisine = 0;
        bool Couvre(int c) const { return c == Case || c == CaseVoisine; }
    };

    struct Joueur
    {
        int Id = 0;
        string Nom;
        ClasseHeros Classe = ClasseHeros::Guerrier;
        bool Humain = false;
        int Couleur = 0;              // index de couleur (0..5)

        int Niveau = 1, XP = 0, PV = 0, Or = 0;
        int Case = 0;
        Ptr<DefObjet> Arme, Armure;
        Liste<Ptr<DefObjet>> Accessoires;
        Liste<Ptr<DefObjet>> Sac;
        Ptr<Regles::Quete> Quete;

        // états
        int ToursImmobilise = 0;      // passe ses prochains tours
        int ToursMaudit = 0;          // −2 en attaque
        bool Lenteur = false;         // prochain jet divisé par deux
        bool ProtegeDuel = false;     // vient de perdre un duel : on ne peut plus le défier jusqu'à la fin de son tour
        Ptr<Regles::Embuscade> Embuscade;
        int Benediction = 0;          // bonus au prochain combat

        // statistiques de fin de partie
        int MonstresTues = 0, DuelsGagnes = 0, DuelsPerdus = 0, QuetesFinies = 0, OrGagne = 0, PiegesDeclenches = 0, KO = 0, NiveauMax = 1;

        Ptr<DefClasse> Def() const { return Donnees::Classe(Classe); }

        int PVMax() const
        {
            int v = Def()->Vie + 2 * (Niveau - 1);
            for (int i = 0; i < Accessoires.Count(); i++)
                if (Accessoires[i]->Effet == EffetObjet::Anneau) v += Accessoires[i]->Bonus;
            return v;
        }

        int Defense() const { return Def()->Defense + (Armure != nullptr ? Armure->Bonus : 0); }

        /// Attaque affichée et utilisée en combat (sans le dé).
        int Attaque() const
        {
            int a = Def()->Attaque + (Arme != nullptr ? Arme->Bonus : 0);
            if (ToursMaudit > 0) a -= 2;
            if (Classe == ClasseHeros::Barbare && PV * 2 <= PVMax()) a += 2;
            return std::max(0, a);
        }

        bool A(EffetObjet e) const
        {
            for (int i = 0; i < Accessoires.Count(); i++)
                if (Accessoires[i]->Effet == e) return true;
            return false;
        }

        bool RetirerAccessoire(EffetObjet e)
        {
            for (int i = 0; i < Accessoires.Count(); i++)
                if (Accessoires[i]->Effet == e)
                {
                    Accessoires.RemoveAt(i);
                    return true;
                }
            return false;
        }

        int CompterSac(TypeObjet t) const
        {
            int n = 0;
            for (int i = 0; i < Sac.Count(); i++)
                if (Sac[i]->Type == t) n++;
            return n;
        }

        int PrixPour(const Ptr<DefObjet>& o) const
        {
            int p = o->Prix;
            if (Classe == ClasseHeros::Mage && o->Type == TypeObjet::Parchemin) p = (p + 1) / 2;
            if (Classe == ClasseHeros::Voleur && o->Type == TypeObjet::Piege) p = (p + 1) / 2;
            return p;
        }

        string Titre() const { return Nom + " (" + Def()->Nom + ")"; }
    };
}
