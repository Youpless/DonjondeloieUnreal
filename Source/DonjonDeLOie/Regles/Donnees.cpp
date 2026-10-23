// Donjon de l'Oie — les données d'origine : classes, monstres, objets, noms des zones et des types de case.
#include "Donnees.h"

#include "Evenements.h"

namespace DonjonDeLOie::Regles
{
    namespace
    {
        Liste<Ptr<DefClasse>> ClassesParDefaut()
        {
            return {
                Nouveau<DefClasse>(ClasseHeros::Guerrier, "Guerrier", 3, 1, 11, "Vétéran : +1 en combat contre les monstres.",
                                   "Solide et bien protégé, il nettoie les routes des bêtes qui les infestent."),
                Nouveau<DefClasse>(ClasseHeros::Mage, "Mage", 3, 0, 9,
                                   "Arcaniste : parchemins à moitié prix, sorts plus durs à esquiver, +1 contre les monstres (trait de feu).",
                                   "Fragile, mais ses parchemins font trembler les plus hauts niveaux."),
                Nouveau<DefClasse>(ClasseHeros::Voleur, "Voleur", 3, 0, 10, "Ombre : pièges à moitié prix, ne déclenche jamais un piège, +5 or sur les trésors.",
                                   "Il sème le plateau de pièges et se faufile entre ceux des autres."),
                Nouveau<DefClasse>(ClasseHeros::Clerc, "Clerc", 3, 1, 11, "Foi : +2 PV au début de chaque tour, résiste aux malédictions.",
                                   "Béni des dieux, il se relève de tout et brave les cases maudites."),
                Nouveau<DefClasse>(ClasseHeros::Rodeur, "Rôdeur", 2, 0, 10, "Pisteur : après son jet, avance d'une case de plus ou de moins s'il le veut.",
                                   "Il choisit où il pose le pied : trésors, marchands, proies."),
                Nouveau<DefClasse>(ClasseHeros::Barbare, "Barbare", 3, 0, 13, "Rage : +2 en attaque quand il lui reste la moitié de ses PV ou moins.",
                                   "Une force brute ; blessé, il devient terrifiant."),
            };
        }

        Liste<Ptr<DefMonstre>> MonstresParDefaut()
        {
            using Z = Zone;
            Liste<Ptr<DefMonstre>> l = {
                Nouveau<DefMonstre>("rat", "Rat géant", 1, 2, 5, 1, Liste<Z>{Z::Village, Z::Bois}, "Gros comme un chien, plus vorace encore."),
                Nouveau<DefMonstre>("gobelin", "Gobelin", 2, 2, 7, 1, Liste<Z>{Z::Bois}, "Petit, vicieux, toujours une lame rouillée à la main."),
                Nouveau<DefMonstre>("loup", "Loup gris", 3, 3, 6, 2, Liste<Z>{Z::Bois, Z::Village}, "Il chasse en silence entre les troncs."),
                Nouveau<DefMonstre>("araignee", "Araignée géante", 4, 3, 10, 2, Liste<Z>{Z::Bois, Z::Marais}, "Huit yeux, huit pattes, aucun scrupule."),
                Nouveau<DefMonstre>("gelee", "Gelée verte", 5, 4, 9, 3, Liste<Z>{Z::Marais}, "Elle dissout tout ce qu'elle avale, armure comprise."),
                Nouveau<DefMonstre>("squelette", "Squelette", 5, 3, 11, 3, Liste<Z>{Z::Marais, Z::Cryptes}, "Un ancien soldat qui ne sait pas qu'il est mort."),
                Nouveau<DefMonstre>("orc", "Orc pillard", 6, 4, 14, 3, Liste<Z>{Z::Pics, Z::Bois}, "Brute couverte de cicatrices, hache au poing."),
                Nouveau<DefMonstre>("spectre", "Spectre", 7, 4, 16, 4, Liste<Z>{Z::Cryptes}, "Une plainte glacée qui traverse les murs."),
                Nouveau<DefMonstre>("troll", "Troll des pics", 8, 5, 20, 5, Liste<Z>{Z::Pics}, "Il se régénère plus vite qu'on ne le frappe."),
                Nouveau<DefMonstre>("wyverne", "Wyverne", 10, 6, 28, 6, Liste<Z>{Z::Pics, Z::Antre}, "Cousine du dragon, le venin en plus."),
                Nouveau<DefMonstre>("dragon", "Ignathor, le Dragon rouge", 12, 7, 80, 8, Liste<Z>{Z::Antre}, "Le seigneur de l'antre. Son trésor est légendaire.", true),
            };
            auto oie = Nouveau<DefMonstre>("dragonnoie", "Le Dragonnoie", 9, 0, 0, 0, Liste<Z>{},
                                           "Mi-oie, mi-dragon, entièrement mauvais. Il fond sur les aventuriers et leur arrache leur bourse. Qui le terrasse gagne la partie.");
            oie->Errant = true;
            l.Add(oie);
            return l;
        }

        Liste<Ptr<DefObjet>> ObjetsParDefaut()
        {
            using T = TypeObjet;
            using E = EffetObjet;
            return {
                // Armes : bonus d'attaque
                Nouveau<DefObjet>("dague", "Dague", T::Arme, 10, 1, E::Aucun, "+1 en attaque."),
                Nouveau<DefObjet>("epee", "Épée courte", T::Arme, 25, 2, E::Aucun, "+2 en attaque."),
                Nouveau<DefObjet>("hache", "Hache de guerre", T::Arme, 45, 3, E::Aucun, "+3 en attaque."),
                Nouveau<DefObjet>("runique", "Épée runique", T::Arme, 70, 4, E::Aucun, "+4 en attaque."),
                Nouveau<DefObjet>("marteau", "Marteau-tonnerre", T::Arme, 100, 5, E::Aucun, "+5 en attaque."),
                Nouveau<DefObjet>("lamedragon", "Lame du Dragon", T::Arme, 150, 7, E::Aucun, "+7 en attaque. Forgée à Valcreux seulement.", true),
                // Armures : réduisent les dégâts des monstres et des sorts de foudre
                Nouveau<DefObjet>("cuir", "Armure de cuir", T::Armure, 15, 1, E::Aucun, "−1 aux dégâts subis."),
                Nouveau<DefObjet>("mailles", "Cotte de mailles", T::Armure, 35, 2, E::Aucun, "−2 aux dégâts subis."),
                Nouveau<DefObjet>("plates", "Armure de plates", T::Armure, 65, 3, E::Aucun, "−3 aux dégâts subis."),
                Nouveau<DefObjet>("ecailles", "Écailles de dragon", T::Armure, 110, 4, E::Aucun, "−4 aux dégâts subis. Valcreux seulement.", true),
                // Accessoires (2 au plus)
                Nouveau<DefObjet>("amulette", "Amulette de garde", T::Accessoire, 30, 0, E::Amulette, "Bloque la prochaine malédiction, rune ou sort reçu, puis se brise."),
                Nouveau<DefObjet>("bottes", "Bottes de célérité", T::Accessoire, 45, 1, E::Bottes, "+1 à chaque jet de déplacement."),
                Nouveau<DefObjet>("anneau", "Anneau de vigueur", T::Accessoire, 35, 4, E::Anneau, "+4 PV maximum."),
                // Potions
                Nouveau<DefObjet>("potion", "Potion de soin", T::Potion, 8, 5, E::Soin, "Rend 5 PV."),
                Nouveau<DefObjet>("elixir", "Élixir de vie", T::Potion, 20, 0, E::SoinTotal, "Rend tous les PV."),
                // Pièges (posés sur sa case ou une case voisine)
                Nouveau<DefObjet>("loups", "Piège à loups", T::Piege, 12, 2, E::PiegeLoups, "La victime perd 2 PV et passe son prochain tour."),
                Nouveau<DefObjet>("fosse", "Fosse à pieux", T::Piege, 10, 4, E::PiegeFosse, "La victime perd 4 PV."),
                Nouveau<DefObjet>("filet", "Filet du voleur", T::Piege, 15, 15, E::PiegeFilet, "Vole 15 or à la victime, pour vous."),
                Nouveau<DefObjet>("rune", "Rune d'oubli", T::Piege, 40, 1, E::PiegeRune, "La victime perd 1 niveau."),
                // Parchemins (sorts lancés sur n'importe quel joueur)
                Nouveau<DefObjet>("malediction", "Parchemin de malédiction", T::Parchemin, 40, 1, E::SortMalediction,
                                  "La cible perd 1 niveau, sauf si elle résiste (5+ au dé)."),
                Nouveau<DefObjet>("foudre", "Parchemin de foudre", T::Parchemin, 20, 5, E::SortFoudre, "La cible perd 5 PV (moins son armure)."),
                Nouveau<DefObjet>("lenteur", "Parchemin de lenteur", T::Parchemin, 12, 0, E::SortLenteur,
                                  "Le prochain jet de déplacement de la cible est divisé par deux."),
                Nouveau<DefObjet>("rappel", "Parchemin de rappel", T::Parchemin, 15, 0, E::SortRappel, "Vous téléporte sur Valcreux ou un sanctuaire."),
            };
        }
    }

    Liste<Ptr<DefClasse>> Donnees::Classes = ClassesParDefaut();

    const Liste<string> Donnees::NomsZones = {"Valcreux", "Bois des Murmures", "Marais Putride", "Cryptes d'Ombrelune", "Pics de Braise", "Antre du Dragon"};

    Liste<Ptr<DefMonstre>> Donnees::Monstres = MonstresParDefaut();

    Liste<Ptr<DefObjet>> Donnees::Objets = ObjetsParDefaut();

    const Liste<string> Donnees::NomsTypesCase = {"Valcreux", "Chemin", "Monstre", "Trésor", "Événement", "Malédiction", "Sanctuaire", "Marchand", "Taverne", "Antre du dragon"};

    const Liste<string> Donnees::AidesTypesCase = {
        "La cité : boutique complète, soins gratuits et quêtes. De l'or et de l'XP de renommée à chaque passage.",
        "Un chemin tranquille.",
        "Un monstre de la région surgit. Combattez-le ou fuyez.",
        "Un coffre ! De l'or, parfois un objet.",
        "Un événement tiré au hasard, heureux ou non.",
        "Une malédiction vous frappe : niveau, or ou force peuvent y passer.",
        "Soins complets, malédictions levées.",
        "Un marchand itinérant et son étal.",
        "Une taverne : quêtes et rumeurs.",
        "Le repaire d'Ignathor. Un combat légendaire.",
    };

    Ptr<DefReglages> Donnees::Reglages = Nouveau<DefReglages>();

    Liste<Ptr<DefEvenement>> Donnees::Evenements = EvenementsParDefaut::Creer();

    Ptr<DefMonstre> Donnees::Boss()
    {
        for (auto& m : Monstres)
            if (m->Boss && !m->Errant) return m;
        for (int i = Monstres.Count() - 1; i >= 0; i--)
            if (!Monstres[i]->Errant) return Monstres[i];
        return nullptr;
    }

    Ptr<DefMonstre> Donnees::Dragonnoie()
    {
        if (!Reglages->DragonnoieActif) return nullptr;
        for (auto& m : Monstres)
            if (m->Errant) return m;
        return nullptr;
    }

    Ptr<DefMonstre> Donnees::Monstre(const string& id)
    {
        for (int i = 0; i < Monstres.Count(); i++)
            if (Monstres[i]->Id == id) return Monstres[i];
        return nullptr;
    }

    Ptr<DefObjet> Donnees::Objet(const string& id)
    {
        for (int i = 0; i < Objets.Count(); i++)
            if (Objets[i]->Id == id) return Objets[i];
        return nullptr;
    }

    Ptr<DefEvenement> Donnees::Evenement(const string& id)
    {
        for (auto& e : Evenements)
            if (e->Id == id) return e;
        return nullptr;
    }

    void Donnees::Appliquer(const Liste<Ptr<DefClasse>>* classes, const Liste<Ptr<DefMonstre>>* monstres, const Liste<Ptr<DefObjet>>* objets,
                            const Liste<Ptr<DefEvenement>>* evenements, const Ptr<DefReglages>& reglages)
    {
        if (classes != nullptr)
        {
            auto d = ClassesParDefaut();
            for (auto& c : *classes)
                if (c != nullptr && static_cast<int>(c->Classe) >= 0 && static_cast<int>(c->Classe) < d.Count()) d[static_cast<int>(c->Classe)] = c->Copie();
            Classes = d;
        }
        if (monstres != nullptr)
        {
            Liste<Ptr<DefMonstre>> l;
            for (auto& m : *monstres)
                if (m != nullptr && !m->Id.empty()) l.Add(m->Copie());
            // Sans monstre errant, on garde le Dragonnoie d'origine (pour s'en passer : Reglages->DragonnoieActif).
            if (l.Count() > 0 && !l.Exists([](const Ptr<DefMonstre>& m) { return m->Errant; }))
                for (auto& m : MonstresParDefaut())
                    if (m->Errant) l.Add(m);
            if (l.Count() > 0) Monstres = l;
        }
        if (objets != nullptr)
        {
            Liste<Ptr<DefObjet>> l;
            for (auto& o : *objets)
                if (o != nullptr && !o->Id.empty()) l.Add(o->Copie());
            if (l.Count() > 0) Objets = l;
        }
        if (evenements != nullptr)
        {
            Liste<Ptr<DefEvenement>> l;
            for (auto& e : *evenements)
                if (e != nullptr && !e->Id.empty()) l.Add(e->Copie());
            if (l.Count() > 0) Evenements = l;
        }
        if (reglages != nullptr) Reglages = reglages->Copie();
    }

    void Donnees::Reinitialiser()
    {
        Classes = ClassesParDefaut();
        Monstres = MonstresParDefaut();
        Objets = ObjetsParDefaut();
        Evenements = EvenementsParDefaut::Creer();
        Reglages = Nouveau<DefReglages>();
    }

    Liste<Ptr<DefClasse>> Donnees::CopieClassesParDefaut() { return ClassesParDefaut(); }
    Liste<Ptr<DefMonstre>> Donnees::CopieMonstresParDefaut() { return MonstresParDefaut(); }
    Liste<Ptr<DefObjet>> Donnees::CopieObjetsParDefaut() { return ObjetsParDefaut(); }

    string NomEnum(ClasseHeros v)
    {
        static const char* n[] = {"Guerrier", "Mage", "Voleur", "Clerc", "Rodeur", "Barbare"};
        return n[static_cast<int>(v)];
    }
    string NomEnum(Zone v)
    {
        static const char* n[] = {"Village", "Bois", "Marais", "Cryptes", "Pics", "Antre"};
        return n[static_cast<int>(v)];
    }
    string NomEnum(TypeCase v)
    {
        static const char* n[] = {"Ville", "Chemin", "Monstre", "Tresor", "Evenement", "Malediction", "Sanctuaire", "Marchand", "Taverne", "Antre"};
        return n[static_cast<int>(v)];
    }
    string NomEnum(TypeObjet v)
    {
        static const char* n[] = {"Arme", "Armure", "Accessoire", "Potion", "Piege", "Parchemin"};
        return n[static_cast<int>(v)];
    }
    string NomEnum(EffetObjet v)
    {
        static const char* n[] = {"Aucun", "Amulette", "Bottes", "Anneau", "Soin", "SoinTotal", "PiegeLoups", "PiegeFosse", "PiegeFilet", "PiegeRune",
                                  "SortMalediction", "SortFoudre", "SortLenteur", "SortRappel"};
        return n[static_cast<int>(v)];
    }
    string NomEnum(Ton v)
    {
        static const char* n[] = {"Neutre", "Bon", "Mauvais", "Combat", "Quete", "Systeme"};
        return n[static_cast<int>(v)];
    }
}
