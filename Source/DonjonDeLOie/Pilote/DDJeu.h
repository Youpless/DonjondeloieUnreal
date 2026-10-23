// Donjon de l'Oie — le chef d'orchestre : il bâtit le monde et l'interface, mène les écrans (titre, salon, partie,
// victoire) et fait jouer la partie en traduisant chaque signal des règles en animation, son et affichage.
//
// Une seule classe, partielle en C# : cet en-tête déclare les membres des sept fichiers, dans leur ordre, et chaque
// fichier C# donne son .cpp (DDJeu.cpp, DDJeu.Cartes.cpp, DDJeu.Combat.cpp, DDJeu.Dragonnoie.cpp, DDJeu.Evade.cpp,
// DDJeu.Nuit.cpp, DDJeu.Signaux.cpp).
#pragma once

#include "../Moteur/Moteur.h"
#include "../Regles/IA.h"
#include "../Regles/Partie.h"
#include "../Voxel/VoxelModel.h"

// Les classes des autres dossiers : déclarées ici, leurs en-têtes sont inclus par les .cpp.
namespace DonjonDeLOie::Monde
{
    class CameraRig;
    class IleModele;
    class IleVue;
    class PionVue;
    class RigVue;
    class Animateur;
    class Etincelles;
    class Diorama;
}

namespace DonjonDeLOie::Interface
{
    class Hud;
    class FenetreQuestion;
    class FenetreCombat;
    class EcranTitre;
    class MenuPause;
    class EcranSalon;
    class EcranRegles;
    class EcranVictoire;
    class EcranEvade;
}

namespace DonjonDeLOie::Cartes
{
    class DDCarte;
}

namespace DonjonDeLOie::Pilote
{
    class DDJeu : public MonoBehaviour
    {
        DD_TYPE_OBJET(DDJeu, MonoBehaviour)
    public:
        // Carte jouée par défaut. Vide : la carte d'origine (ou celle de l'éditeur de cartes présent dans la scène).
        Ref<Cartes::DDCarte> Carte;

        /// Pour les essais : la partie en cours (nullptr hors partie). La version C# n'a pas d'accesseur : son essai
        /// d'ensemble lit le champ privé « partie » par réflexion.
        const Ptr<Regles::Partie>& PartieEnCours() const { return partie; }

    private:
        // ================================================================== DDJeu.cpp

        enum class Etat { Chargement, Titre, Salon, Partie, Victoire };
        Etat etat = Etat::Chargement;

        Ref<Transform> monde, transitoires;
        Ref<Monde::CameraRig> cam;
        Ptr<Monde::IleModele> modele;
        Ref<Monde::IleVue> ile;
        Ref<Monde::Etincelles> etincelles, fumees;
        Ref<Canvas> canvas;
        Ref<RectTransform> ui;
        Ref<Interface::Hud> hud;
        Ptr<Interface::FenetreQuestion> fq;
        Ptr<Interface::FenetreCombat> fc;
        Ptr<Interface::EcranTitre> titre;
        Ptr<Interface::EcranSalon> salon;
        Ptr<Interface::EcranRegles> regles;
        Ptr<Interface::MenuPause> pause;
        Ptr<Interface::EcranVictoire> victoire;
        Ref<RectTransform> boutonsJeu;
        Ref<Text> chargement;

        Ptr<Regles::Partie> partie;
        Ptr<Regles::IA> ia;
        Liste<Ptr<Regles::ReglageJoueur>> derniersReglages;
        Dico<int, Ref<Monde::PionVue>> pions;
        Ptr<Coroutine> boucle;
        bool botsRapides = false;

        void Awake() override;
        /// IEnumerator Start() de C#.
        Tache StartCoroutineDeDemarrage() override;
        void BoutonsJeu();

        // ------------------------------------------------------------------ écrans

        void AllerTitre();
        void AllerSalon();
        void Quitter();
        void Lancer(const Liste<Ptr<Regles::ReglageJoueur>>& reglages);
        void Nettoyer();
        void Pause(bool oui);
        void Update() override;
        void CliquerCase(const Ptr<Regles::SQuestion>& q);

        // ------------------------------------------------------------------ outils

        float V() const;
        Tache Attendre(float s);
        /// Range les pions d'une case sur leurs places (anime ceux qui bougent).
        void RangerCase(int c, bool animer, int sauf = -1);
        Vector3 Place(int joueur) const;
        Vector3 Tete(int joueur) const;
        void MajMarqueurs(const Ptr<Regles::Joueur>& j);
        int Humains() const;

        // ================================================================== DDJeu.Cartes.cpp
        // Les cartes jouables : celle de l'éditeur de cartes de la scène, celle choisie sur DDJeu, la carte d'origine, et
        // toutes celles du dossier Resources/Cartes. Le salon propose de choisir ; l'île est reconstruite au besoin.

        class CarteJouable   // Def null : la carte d'origine
        {
        public:
            string Nom;
            Ptr<Regles::CarteDef> Def;
            int Niveau = 10;
        };

        Liste<Ptr<CarteJouable>> cartes;
        int carteParDefaut = 0, carteConstruite = -1;

        void PreparerCartes();
        void Ajouter(const string& nom, const Ptr<Regles::CarteDef>& def, int niveau);
        Liste<string> NomsCartes() const;
        Liste<int> NiveauxConseilles() const;
        /// (Re)construit l'île d'une carte de la liste.
        void ConstruireIle(int index);

        // ================================================================== DDJeu.Combat.cpp
        // Mise en scène des combats : le monstre surgit à côté du héros, les dés roulent, les coups portent.

        Ref<Monde::RigVue> monstreVue;
        Ref<Monde::Animateur> monstreAnim;
        bool monstrePersistant = false;

        /// Le monstre jaillit du sol à côté du héros (le dragon, lui, est déjà dans son antre, et le Dragonnoie sur sa case).
        Tache FaireSurgir(Ptr<Regles::DefMonstre> m, Ptr<Regles::Joueur> j);
        void EnleverMonstre(bool immediat);
        Tache Enfouir(Ref<GameObject> go);
        Tache Combat(Ptr<Regles::SCombat> c);

        // ================================================================== DDJeu.Dragonnoie.cpp
        // Le Dragonnoie à l'écran : il se tient au bord de sa case (les pions restent visibles), prend ses grandes envolées
        // en arc au-dessus de l'île (la caméra le suit), et arrache leur or aux vaincus.

        Ref<Monde::RigVue> oieVue;
        Ref<Monde::Animateur> oieAnim;

        /// Où il se tient sur une case : à 2,3 m du centre, du côté le plus plat et le plus loin de la mer, face à la case.
        Vector3 PositionOie(int c, Quaternion& regard) const;
        /// Une grande envolée (ou son arrivée sur l'île, De = -1) : un arc au-dessus de l'île, ailes battantes.
        Tache Envol(Ptr<Regles::SEnvol> s);
        /// Le Dragonnoie arrache sa bourse au vaincu : les pièces volent jusqu'à lui.
        Tache OrArrache(Ptr<Regles::SEffet> e);
        /// Le Dragonnoie a été retiré pendant la partie (catalogue appliqué en jeu, « Dragonnoie actif » décoché) : il quitte l'île.
        void VerifierOie();
        /// Clic sur l'encart du Dragonnoie : la caméra va le voir.
        void MontrerOie();

        // ================================================================== DDJeu.Evade.cpp
        // Le mini-jeu de l'évadé à l'écran : la caméra zoome sur la ville en vue presque isométrique (le champ se resserre
        // pendant qu'elle recule), la ville se floute (flou de profondeur d'URP), le marché surgit en diorama au-dessus des
        // toits, et l'on a trente secondes pour cliquer sur le visage de l'évadé, caché dans la foule.
        // Un bot cherche à sa place, loupe à la main : il réussit ou non selon la réponse de l'IA, tirée d'avance.

        /// Le champ de vision de la vue du diorama : étroit, presque une vue isométrique. La molette le resserre encore
        /// (jusqu'à ZoomMax fois), vers le point visé par la souris.
        static constexpr float ChampDiorama = 16.0f, ZoomMax = 2.6f;
        /// Le flou de la ville : net jusqu'à DebutFlou mètres derrière le point visé, flou complet à FinFlou.
        static constexpr float DebutFlou = 3.5f, FinFlou = 15.0f;
        Ref<Monde::Diorama> diorama;
        Ptr<Interface::EcranEvade> ecranEvade;
        bool evadeEnCours = false;

        Tache ChercherEvade(Ptr<Regles::SQuestion> q);
        /// Trente secondes (réglables) pour cliquer sur le bon visage ; une erreur en coûte trois. La molette zoome vers
        /// la souris (sans sortir du cadre de départ).
        /// « trouve » : le « bool[] trouve » de C# (un tableau d'une case), partagé avec l'appelant qui y lit le résultat.
        Tache RechercheHumaine(Ptr<Regles::Quete> qu, Ptr<Liste<bool>> trouve);
        /// Le zoom de la molette : le champ se resserre (ou s'élargit) et la cible glisse dans le plan de l'image pour que
        /// le point sous la souris reste sous la souris ; la vue ne sort jamais du cadre de départ (cible0, zoom 1).
        float ZoomerVers(Vector3 cible0, float zoom, float nouveau);
        /// Un bot cherche, loupe à la main : quelques visages, puis l'évadé (réussite) ou un sosie (raté).
        Tache RechercheBot(Ptr<Regles::Quete> qu, bool reussite);
        /// Mouvement de caméra image par image : la cible, la distance, le lacet, le tangage et le champ glissent ensemble ;
        /// la hauteur vue au point visé varie régulièrement (un « travelling compensé » : le décor se tasse quand le champ
        /// se resserre). Le flou de profondeur va de sa force actuelle à « flou », réglé sur la distance du moment.
        Tache Zoomer(Vector3 cible, float distance, float lacet, float tangage, float champ, float duree, float flou);
        /// Le lacet de la vue du diorama : la diagonale (45° + k·90°) la plus proche, sauf celle qui fait face au soleil
        /// (les visages seraient à contre-jour).
        static float LacetDiagonal(float lacet);
        /// Une quête d'avis de recherche ratée : un nuage de dépit au-dessus du héros.
        Tache QueteRatee(Ptr<Regles::SEffet> e);
        /// Fin du mini-jeu (ou partie interrompue) : plus de diorama, plus de flou, l'écran de jeu revient.
        void FinirEvade();

        // ================================================================== DDJeu.Nuit.cpp
        // La nuit à l'écran : la lumière tombe (lune, lampes plus vives), puis l'événement de la nuit — le voleur, le trou
        // de ver, la bonne nuit ou l'arène du diable, qui flotte au-dessus de l'île et où les pions se battent l'un après
        // l'autre — et le jour se lève.

        static constexpr Color CouleurNuit = Color(0.6f, 0.7f, 1.0f);
        static inline Ptr<Voxel::VoxelRig> modeleArene;      // l'arène (≈ 80 000 voxels) : construite et maillée une seule fois, au chargement
        /// Inscrit auprès du moteur la remise à zéro de modeleArene (une variable statique qui change), comme au
        /// lancement d'une partie dans Unity ; défini dans DDJeu.Nuit.cpp.
        static const bool statiquesInscrits;
        Ref<Monde::RigVue> arene;
        Vector3 areneCentre;
        bool nuitEnCours = false;                 // la nuit se joue à vitesse normale, même avec les bots rapides
        Liste<int> areneOrdre;

        Tache Nuit(Ptr<Regles::SNuit> s);
        Tache Crepuscule(float de, float vers, float duree);
        /// Une partie gagnée en pleine nuit (le champion de l'arène atteint le niveau visé, un trou de ver mène au
        /// Dragonnoie) : chacun regagne sa case, l'arène s'enfonce et le jour se lève avant la fête.
        Tache AubeVictoire();
        /// Au chargement : le modèle de l'arène et ses maillages sont prêts d'avance (pas d'à-coup à la première nuit
        /// d'arène ; les nuits suivantes réutilisent le même modèle).
        void PreparerArene();
        /// Une partie interrompue en pleine nuit (retour au menu, nouvelle partie) : le jour revient d'un coup.
        void JourImmediat();

        // ------------------------------------------------------------------ le voleur, le trou de ver, la bonne nuit

        Tache VolNocturne(Ptr<Regles::SEffet> e);
        Tache TrouDeVer(Ptr<Regles::SEffet> e);
        Tache BonneNuit(Ptr<Regles::SEffet> e);

        // ------------------------------------------------------------------ l'arène du diable

        /// L'arène flotte au-dessus de l'île ; les pions y sont transportés en cercle autour du centre.
        Tache EntrerArene(Ptr<Regles::SEffet> e);
        /// La place d'attente d'un combattant : en arc de cercle derrière le centre (vu de la caméra), en deçà de la lave.
        Vector3 PlaceArene(int rang) const;
        /// Avant un duel de l'arène : les deux adversaires s'avancent au centre, la caméra les regarde d'assez haut pour
        /// passer au-dessus des cornes de l'entrée.
        Tache AvancerDuelArene(Ptr<Regles::SCombat> c);
        /// Après un duel de l'arène : le vaincu regagne sa place, le champion attend le suivant.
        Tache ApresDuelArene(Ptr<Regles::SCombat> c);
        Tache ChampionArene(Ptr<Regles::SEffet> e);
        /// Au lever du jour : chacun retrouve sa case, l'arène s'enfonce dans les ténèbres.
        Tache QuitterArene();

        // ================================================================== DDJeu.Signaux.cpp
        // Chaque signal des règles devient ce que voient et entendent les joueurs.

        Tache Boucle();
        static Color CouleurTon(Regles::Ton t);
        Tache Presenter(Ptr<Regles::Signal> s);
        Tache TourDebut(Ptr<Regles::STourDebut> s);
        Tache De(Ptr<Regles::SDe> s);
        Tache Disparaitre(Ref<GameObject> go, float duree);
        Tache Pas(Ptr<Regles::SPas> s);
        Tache Teleport(Ptr<Regles::STeleport> s);
        Tache Stat(Ptr<Regles::SStat> s);
        Tache Effet(Ptr<Regles::SEffet> e);

        // ------------------------------------------------------------------ questions

        Tache Question(Ptr<Regles::SQuestion> q);

        // ------------------------------------------------------------------ fin

        Tache Victoire();
    };
}
