// Donjon de l'Oie — l'affichage en jeu : fiches des joueurs, bannière du tour, journal, grandes annonces,
// textes flottants au-dessus des pions et étiquettes de noms.
#pragma once

#include "../Moteur/Moteur.h"
#include "../Regles/Partie.h"

// Les classes des autres dossiers : déclarées ici, leurs en-têtes sont inclus par Hud.cpp.
namespace DonjonDeLOie::Monde
{
    class CameraRig;
}

namespace DonjonDeLOie::Interface
{
    /// (Les membres « Canvas » et « Camera » portent le nom d'un type du moteur : dans la classe, le type s'écrit
    /// Moteur::Canvas, Moteur::Camera.)
    class Hud : public MonoBehaviour
    {
        DD_TYPE_OBJET(Hud, MonoBehaviour)
    public:
        Ref<RectTransform> Racine;           // couvre l'écran
        Ref<Moteur::Canvas> Canvas;
        Ref<Monde::CameraRig> Camera;
        std::function<void()> MontrerOie;                // clic sur l'encart du Dragonnoie
        std::function<Ref<Transform>(int)> PionDe;       // joueur → transform du pion
        std::function<float(int)> HauteurPion;

        static Ref<Hud> Creer(const Ref<RectTransform>& racine, const Ref<Moteur::Canvas>& canvas);

        void Visible(bool v);

        /// Le mini-jeu de l'évadé occupe l'écran : fiches, bannière, chronique et encarts s'effacent le temps de la recherche.
        void ModeMiniJeu(bool oui);

        // ------------------------------------------------------------------ fiches des joueurs

        void Installer(const Ptr<Regles::Partie>& p);

        void Rafraichir();

        // ------------------------------------------------------------------ bannière, journal, annonces

        void Banniere(const string& texte, Color couleur);

        /// (« s » par valeur : il est raccourci sur place, comme en C#.)
        void Journal(string s, Color c);

        void Annonce(const string& texte, Color c);

        // ------------------------------------------------------------------ textes flottants

        void Flottant(Vector3 monde, const string& texte, Color c, int taille = 30);

    private:
        // (des classes en C# : tenues par Ptr)
        struct Fiche
        {
            Ref<RectTransform> R;
            Ref<Image> Halo, Bande;
            Ref<Text> Nom, Classe, Niveau, PV, Or, Atk, Def, Etats;
            Ref<Image> BarrePV;
            Liste<Ref<Image>> XP;
            int Joueur = 0;
        };

        struct TexteFlottant
        {
            Ref<Text> T;
            Vector3 Monde;
            float Vie = 0.0f, Duree = 0.0f;
            Vector2 Decalage;
        };

        Ptr<Regles::Partie> partie;
        Liste<Ptr<Fiche>> fiches;
        Ref<RectTransform> banniere;
        Ref<Text> banniereTexte;
        Ref<Image> banniereCouleur;
        Ref<Button> oie;
        Ref<Text> oieTexte;
        Ref<RectTransform> nuit;                         // la prochaine nuit, sous l'encart du Dragonnoie
        Ref<Text> nuitTexte;
        bool miniJeu = false;                            // le mini-jeu de l'évadé : seuls restent les annonces et les textes flottants
        Ref<RectTransform> journal;
        Liste<Ref<Text>> lignes;
        Liste<string> messages;
        Ref<RectTransform> annonce;
        Ref<Text> annonceTexte;
        Ref<CanvasGroup> annonceGroupe;
        float annonceT = 99.0f;
        Ref<RectTransform> calqueMonde;
        Liste<Ptr<TexteFlottant>> flottants;
        Dico<int, Ref<Text>> etiquettes;

        void Batir();

        Ptr<Fiche> CreerFiche(const Ptr<Regles::Joueur>& j, int rang);

        Vector2 VersCanvas(Vector3 ecran);

        void Update() override;
    };
}
