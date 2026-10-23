// Donjon de l'Oie — le marché de Valcreux en diorama, pour le mini-jeu de l'évadé : un socle de rue pavée, quatre
// maisons, trois étals et leurs marchands, une vingtaine d'habitants (dont deux ou trois sosies de l'évadé) et l'évadé
// lui-même, caché derrière des caisses, des tonneaux ou des sacs : seule sa tête dépasse. Il se bâtit une pièce par
// image pendant que la caméra zoome, puis surgit pièce après pièce (la vie du marché : Diorama.Vie.cpp).
// Repère : origine au centre du sol, la rue le long de x, les maisons au fond (z < 0) ; la caméra le regarde de
// l'avant-droite (LacetVue), d'en haut (TangageVue).
//
// Les définitions sont réparties en deux fichiers, comme dans la version C# : Diorama.cpp (l'agencement, la
// construction, la foule) et Diorama.Vie.cpp (la vie du marché : il surgit pièce après pièce, les passants vont et
// viennent, les marchands haranguent, l'évadé guette en tournant la tête ; un clic sur une tête dit qui est là. À la
// fin, l'évadé est démasqué (il bondit hors de sa cachette) ou s'enfuit en courant hors du diorama, qui se replie).
#pragma once

#include "../Modeles/HerosModeles.h"
#include "../Moteur/Moteur.h"
#include "../Voxel/VoxelModel.h"
#include "Animateur.h"
#include "RigVue.h"

namespace DonjonDeLOie::Monde
{
    class Diorama : public MonoBehaviour
    {
        DD_TYPE_OBJET(Diorama, MonoBehaviour)
    public:
        static constexpr float LacetVue = 28.0f, TangageVue = 38.0f;
        /// La direction de la caméra, à l'horizontale, dans le repère du diorama.
        static const Vector3 VersCamera;          // (sin, 0, cos) de LacetVue : calculée dans Diorama.cpp

        struct Habitant
        {
            Ref<RigVue> Rig; Ref<Animateur> Anim; Ptr<Modeles::Allure> Allure;
            bool Evade = false, Sosie = false, Cache = false;
            bool Marche = false; Vector3 A, B; float Vitesse = 0.0f, Phase = 0.0f;    // un passant va et vient de A à B
            float Lacet = 0.0f;                                                     // orientation au repos (repère du diorama)
            int Cachette = -1;
            /// Le milieu de la tête, dans le monde.
            Vector3 Tete() const;
        };

        Liste<Ptr<Habitant>> Foule;
        Ptr<Habitant> Evade() const { return evade_; }
        bool Pret() const { return pret_; }
        bool EnJeu = false;                                  // la recherche a commencé (les essais hors Unity le guettent)
        static Ref<Diorama> Actif() { return actif_; }
        const Liste<Color32>& Portrait() const { return portrait_; }

        static Ref<Diorama> Creer(const Ref<Transform>& parent, Vector3 position, float lacet, int graine, const Ptr<Modeles::Allure>& evade);

        /// La caméra qui cadre le diorama : orientation fixe (LacetVue, TangageVue sur son repère) ; distance et cible
        /// cherchées pour que l'emprise tienne, perspective comprise, dans la fenêtre libre de l'écran : à droite de l'avis
        /// de recherche (« marge » : part de la largeur), sous le sablier (« haut ») et au-dessus de la consigne (« bas »).
        static void Cadrage(const Ref<Transform>& d, float champ, float aspect, float marge, float haut, float bas,
                            Vector3& cible, float& distance, float& lacet);

        // ------------------------------------------------------------------ la construction

        /// Bâtit le diorama, une pièce par image (« parImage » faux : tout d'un coup). Tout reste caché jusqu'à Apparaitre().
        Tache Batir(bool parImage);

        // ------------------------------------------------------------------ apparaître, disparaître (Diorama.Vie.cpp)

        /// Le diorama surgit : le socle bondit, les maisons et les étals poussent, les habitants tombent du ciel.
        /// « pop » est appelé au surgissement de chaque pièce (sa position dans le monde), pour un son ou un nuage ;
        /// « vitesse » accélère le tout (les bots rapides).
        Tache Apparaitre(std::function<void(Vector3)> pop, float vitesse = 1.0f);

        /// Tout en place d'un coup (essais hors Unity, ou partie qui se termine pendant l'animation).
        void Montrer();

        /// Le diorama se replie sur lui-même en tournant, puis s'éteint.
        Tache Disparaitre(float duree);

        // ------------------------------------------------------------------ le clic (Diorama.Vie.cpp)

        /// L'habitant dont la tête est sous ce point de l'écran (le plus proche de la caméra), ou null.
        Ptr<Habitant> Toucher(Vector2 souris, const Ref<Camera>& cam) const;

        /// Un habitant au hasard dont on voit le visage (la loupe d'un bot qui cherche), sauf l'évadé.
        Ptr<Habitant> Badaud();

        /// Un sosie de l'évadé (ou n'importe qui), pour l'erreur d'un bot.
        Ptr<Habitant> UnSosie();

        // ------------------------------------------------------------------ la fin (Diorama.Vie.cpp)

        /// Démasqué : l'évadé bondit par-dessus sa cachette et atterrit devant, penaud.
        Tache Attrape();

        /// Raté : l'évadé saute dans la rue, file vers le bord le plus proche et saute dans le vide.
        Tache Fuite();

    private:
        /// Une pièce qui surgira : 0 bondit, 1 pousse de bas en haut, 2 tombe du ciel.
        struct Piece
        {
            Ref<Transform> T; Vector3 Echelle, Position; float Delai = 0.0f; int Genre = 0;
        };

        // les propriétés { get; private set; } du C# : Evade, Pret, Actif, Portrait
        Ptr<Habitant> evade_;
        bool pret_ = false;
        static inline Ref<Diorama> actif_;
        Liste<Color32> portrait_;

        Liste<Ptr<Piece>> pieces;
        Liste<Ref<Mesh>> propres;      // maillages des habitants, faits pour ce diorama seul
        Ptr<AleaNet> r;
        Ptr<Modeles::Allure> allure;

        // ------------------------------------------------------------------ l'agencement (mètres, repère du diorama)

        static constexpr float MaisonsX[] = {-3.84f, -1.28f, 1.28f, 3.84f};
        static constexpr float MaisonsZ = -2.9f;
        static constexpr float EtalsX[] = {-3.55f, 0.0f, 3.55f};
        static constexpr float EtalsZ = -1.0f;
        /// Les cachettes : où se tient celui qui se cache, l'abri posé entre lui et la caméra, la demi-épaisseur de l'abri.
        static constexpr Vector3 Cachettes[] = {Vector3(-2.05f, 0.0f, -1.25f), Vector3(1.6f, 0.0f, -1.25f), Vector3(-4.3f, 0.0f, 2.5f), Vector3(0.05f, 0.0f, 2.5f)};
        static constexpr const char* Abris[] = {"caisses", "tonneaux", "sacs", "caisses"};
        static constexpr float DemiAbris[] = {0.3f, 0.3f, 0.28f, 0.3f};
        static constexpr float Recul = 0.3f;                                   // de l'abri au milieu du corps caché
        /// Celui qui se cache plie les genoux, selon l'abri et l'angle sous lequel la caméra voit la cachette : on voit
        /// son visage, à peu près du nez aux cheveux, et presque rien d'autre.
        static constexpr float Accroupis[] = {0.06f, 0.06f, 0.15f, 0.1f};
        static constexpr Vector3 Puits = Vector3(-1.9f, 0.0f, 3.0f), Charrette = Vector3(1.85f, 0.0f, 3.0f), Avis = Vector3(4.3f, 0.0f, 3.05f);
        static constexpr float Allee1 = 0.7f, Allee2 = 1.65f;                 // les allées des passants

        void OnDestroy() override;

        /// Ce qui doit tenir dans l'image (repère du diorama) : la rue, des façades au bord avant du socle, et les étals
        /// jusqu'à leurs auvents ; les toits des maisons et le bas de la plinthe peuvent déborder.
        static constexpr Vector3 Emprise[] =
        {
            Vector3(-5.3f, 0.0f, -1.9f), Vector3(5.3f, 0.0f, -1.9f), Vector3(-5.3f, 0.0f, 4.1f), Vector3(5.3f, 0.0f, 4.1f),
            Vector3(-5.3f, -0.45f, 4.1f), Vector3(5.3f, -0.45f, 4.1f),
            Vector3(-4.7f, 2.25f, -1.85f), Vector3(4.7f, 2.25f, -1.85f), Vector3(0.0f, 2.25f, -1.85f),
        };

        // ------------------------------------------------------------------ la construction

        void Ajouter(const Ref<RigVue>& rv, Vector3 pos, float lacet, float delai, int genre);

        void Propres(const Ref<RigVue>& rv);

        Ptr<Habitant> Placer(const Ptr<Modeles::Allure>& a, const Ptr<Voxel::VoxelRig>& rig, Vector3 pos, float lacet, float delai);

        float Hasard(float a, float b);

        /// Un visage qu'on ne confond pas avec celui de l'évadé : au moins trois pixels de différence sur l'avis, à partir
        /// de la ligne « depuis » (4 : du nez aux cheveux, ce qui dépasse d'une cachette).
        bool Distinct(const Ptr<Voxel::VoxelRig>& rig, int depuis);

        /// Une allure tirée au hasard, ou un sosie de l'évadé, et son modèle — toujours un visage distinct du sien (pour
        /// qui se cache : distinct dans ce qui dépasse de la cachette).
        Ptr<Modeles::Allure> Tirer(bool sosie, bool cache, Ptr<Voxel::VoxelRig>& rig);

        Ptr<Habitant> Gens(bool sosie, Vector3 pos, float lacet, float delai, bool cache = false);

        /// La foule : marchands, chalands, passants, badauds, quelqu'un qui lit l'avis, un ou deux autres cachés, et
        /// l'évadé. Chaque habitant bâti rend « vrai » (une image de pause possible).
        Gen<bool> Peupler(Ptr<Voxel::VoxelRig> rigEvade);

        // ------------------------------------------------------------------ Diorama.Vie.cpp

        static constexpr float DureePiece = 0.42f;
        bool vivant = false, fini = false;
        float temps = 0.0f, prochainGeste = 1.0f;

        static Vector3 PositionPassant(Vector3 a, Vector3 b, float phase);

        static float Rebond(float k);

        static void Poser(const Ptr<Piece>& p, float k);

        // la vie du marché
        void Update() override;

        /// Remise des variables statiques à leur valeur de départ (Boucle::Reinitialiser) : inscrite dans le .cpp
        /// (Actif redevient null).
        static const bool remiseAZero;
    };
}
