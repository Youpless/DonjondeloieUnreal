// Donjon de l'Oie — la racine d'une carte dans la scène de l'éditeur : ses réglages, sa case de départ, et la conversion
// des marqueurs (enfants DDCaseMarqueur et DDDecorMarqueur) en une carte de jeu (CarteDef).
// En jeu, DDJeu lit la carte de cette racine s'il en trouve une dans la scène, puis la masque.
#pragma once

#include "../Moteur/Moteur.h"
#include "../Regles/Donnees.h"
#include "../Regles/Plateau.h"
#include "DDCaseMarqueur.h"
#include "DDDecorMarqueur.h"

namespace DonjonDeLOie::Cartes
{
    /// [DisallowMultipleComponent]
    class DDEditeurCarte : public MonoBehaviour
    {
        DD_TYPE_OBJET(DDEditeurCarte, MonoBehaviour)
    public:
        /// Le nom de la carte, affiché dans le salon.
        string NomCarte = "Nouvelle carte";
        string Description;                  // [TextArea]
        /// La case de départ : les pions y commencent ; on y gagne de l'or et de l'XP à chaque passage.
        Ref<DDCaseMarqueur> Depart;
        /// Niveau à atteindre proposé dans le salon pour cette carte.
        int NiveauConseille = 10;            // [Range(3, 10)]

        // [Header("Île")]
        /// Largeur de terre autour du chemin, en mètres.
        float Marge = 12.0f;                 // [Range(4f, 30f)]
        /// Arbres, rochers, fleurs… semés selon les zones.
        bool DecorAutomatique = true;
        float DensiteDecor = 1.0f;           // [Range(0f, 2f)]
        /// Bâtiments posés selon le type des cases (taverne, chapelle, échoppe…), sauf choix contraire d'une case.
        bool LieuxAutomatiques = true;
        /// Change le relief et le décor semé.
        int Graine = 7;

        Liste<Ref<DDCaseMarqueur>> Cases() const;

        /// La carte de jeu décrite par les marqueurs. Les positions sont celles de la scène (en mètres, sur le plan horizontal).
        Ptr<Regles::CarteDef> Exporter() const;

        /// Crée les marqueurs d'une carte de jeu sous cette racine (cases puis décors). Rend les objets créés,
        /// pour que l'éditeur puisse les inscrire dans l'historique d'annulation.
        Liste<Ref<GameObject>> Importer(const Ptr<Regles::CarteDef>& d);

        /// Le sous-dossier (objet vide) « Cases » ou « Décors » de la racine, créé au besoin.
        Ref<Transform> Dossier(const string& nom);

        static string NomMarqueur(int i, Regles::TypeCase t, const string& nom);
    };
}
