// Donjon de l'Oie — les événements en données : un texte, une suite d'effets, et éventuellement un choix.
// Les 17 événements d'origine sont écrits dans Evenements.cpp ; le catalogue permet d'en changer et d'en créer.
#pragma once

#include "Donnees.h"

namespace DonjonDeLOie::Regles
{
    /// Ce que fait un effet. « Valeur » et « Parametre » changent de sens selon le type (voir Aide).
    enum class TypeEffet
    {
        Annonce,            // Parametre : texte affiché ({joueur} = son nom)
        Or,                 // Valeur : or gagné (négatif : perdu)
        PV,                 // Valeur : PV rendus (négatif : blessure)
        SoinTotal,          // tous les PV
        XP,                 // Valeur : XP gagnée (négatif : perdue, sans perdre de niveau)
        Niveau,             // Valeur : niveaux gagnés (négatif : perdus)
        Avancer,            // Valeur : cases à avancer (la case d'arrivée fait son effet)
        Reculer,            // Valeur : cases à reculer
        TeleporterHasard,   // vers une case de la grande boucle ; Valeur 1 : la case d'arrivée fait son effet
        TeleporterDepart,   // vers la case de départ (Valcreux)
        TeleporterCase,     // Parametre : nom du lieu ; Valeur 1 : la case d'arrivée fait son effet
        Combat,             // Parametre : id du monstre (vide : un monstre de la zone)
        Boutique,           // Valeur : remise en % ; Parametre : ids d'objets séparés par des virgules (vide : 5 au hasard)
        Quetes,             // Parametre : nom de celui qui les propose
        ObjetHasard,        // Parametre : Potion, Piege ou Parchemin (vide : un consommable) ; sac plein : Valeur or à la place
        ObjetDonne,         // Parametre : id de l'objet ; sac plein : Valeur or à la place
        Faiblesse,          // Valeur : tours à −2 en attaque
        Immobiliser,        // Valeur : tours passés
        Lenteur,            // le prochain jet de dé est divisé par deux
        Benediction,        // Valeur : bonus au prochain combat contre un monstre
        JetMalediction,     // le tableau des malédictions (1D6)
        LeverMalediction,   // retire faiblesse, lenteur et immobilisation
    };

    /// Sur qui porte l'effet. Les effets de déplacement, de combat, de boutique et de quêtes portent toujours sur le joueur.
    enum class CibleEffet { Joueur, Tous, Autres, Meneur, Dernier };

    struct EffetEvenement
    {
        TypeEffet Type = TypeEffet::Annonce;
        int Valeur = 0;
        string Parametre;
        CibleEffet Cible = CibleEffet::Joueur;
        int Chance = 100;            // % de chances que l'effet se produise

        EffetEvenement() = default;
        EffetEvenement(TypeEffet t, int v = 0, string p = "", CibleEffet c = CibleEffet::Joueur, int chance = 100)
            : Type(t), Valeur(v), Parametre(std::move(p)), Cible(c), Chance(chance) {}
        Ptr<EffetEvenement> Copie() const { return Nouveau<EffetEvenement>(*this); }

        static bool SeulementLeJoueur(TypeEffet t);
        /// Aide affichée dans l'éditeur.
        static string Aide(TypeEffet t);
    };

    struct ChoixEvenement
    {
        string Libelle = "Choix", Aide;
        Liste<Ptr<EffetEvenement>> Effets;

        ChoixEvenement() = default;
        ChoixEvenement(string libelle, string aide, Liste<Ptr<EffetEvenement>> effets = {})
            : Libelle(std::move(libelle)), Aide(std::move(aide)), Effets(std::move(effets)) {}
        Ptr<ChoixEvenement> Copie() const;
    };

    struct DefEvenement
    {
        string Id, Nom = "Nouvel événement";
        string Texte;                                   // annonce ({joueur} = son nom)
        Regles::Ton Ton = Regles::Ton::Neutre;
        bool DansLaPioche = true;                       // tiré sur les cases « Événement » qui n'ont pas leur propre liste
        int Poids = 10;                                 // plus il est grand, plus il sort souvent
        Liste<Zone> Zones;                              // vide : toutes les zones
        Liste<Ptr<EffetEvenement>> Effets;
        Liste<Ptr<ChoixEvenement>> Choix;               // vide : pas de choix

        DefEvenement() = default;
        DefEvenement(string id, string nom, string texte, Regles::Ton ton, Liste<Ptr<EffetEvenement>> effets = {})
            : Id(std::move(id)), Nom(std::move(nom)), Texte(std::move(texte)), Ton(ton), Effets(std::move(effets)) {}

        Ptr<DefEvenement> Copie() const;
        bool PossibleEn(Zone z) const { return Zones.Count() == 0 || Zones.Contains(z); }
    };

    struct EvenementsParDefaut
    {
        static Liste<Ptr<DefEvenement>> Creer();
    };

    string NomEnum(TypeEffet v);
    string NomEnum(CibleEffet v);
}
