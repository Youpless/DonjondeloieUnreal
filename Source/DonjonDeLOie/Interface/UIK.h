// Donjon de l'Oie — boîte à outils d'interface (uGUI construit en code) : panneaux de bois et de parchemin,
// textes ombrés, boutons, icônes en pixel art, barres. Tous les sprites sont dessinés au lancement.
#pragma once

#include "../Moteur/Moteur.h"
#include "../Regles/Joueur.h"

namespace DonjonDeLOie::Interface
{
    enum class Style { Bois, Parchemin, Sombre, Bouton, BoutonOr, BoutonRouge, Carte, Puce, Verre };

    /// Une classe statique en C# : rien que des membres statiques.
    /// (Ses fonctions Image, Rect et Texte masquent, dans sa portée, les types Moteur::Image et Moteur::Rect et l'espace
    /// de noms Moteur::Texte : UIK.cpp les qualifie, comme le C# écrit « UnityEngine.UI.Image ».)
    struct UIK
    {
        static inline Ref<Font> Police;

        static constexpr Color Or = Color(0.93f, 0.76f, 0.36f);
        static constexpr Color OrSombre = Color(0.62f, 0.45f, 0.18f);
        static constexpr Color Encre = Color(0.17f, 0.11f, 0.07f);
        static constexpr Color Creme = Color(0.97f, 0.93f, 0.84f);
        static constexpr Color BoisFonce = Color(0.13f, 0.095f, 0.07f, 0.94f);
        static constexpr Color Vert = Color(0.46f, 0.82f, 0.42f);
        static constexpr Color Rouge = Color(0.93f, 0.38f, 0.32f);
        static constexpr Color Bleu = Color(0.45f, 0.7f, 0.98f);
        static constexpr Color Violet = Color(0.76f, 0.55f, 0.98f);
        static constexpr Color Gris = Color(0.62f, 0.6f, 0.56f);

        static void Initialiser();

        // ------------------------------------------------------------------ sprites dessinés

        static Ref<Sprite> Icone(const string& id);

        /// Oublie les icônes dont le nom commence ainsi (les portraits des évadés d'une partie terminée) : leurs
        /// textures sont libérées ; elles seraient redessinées si on les redemandait.
        static void OublierIcones(const string& prefixe);

        static Ref<Sprite> Plein() { return plein; }
        static Ref<Sprite> Lueur() { return lueur; }
        static Ref<Sprite> Blanc() { return blanc; }
        static Ref<Sprite> DegradeVertical() { return degrade; }

        // ------------------------------------------------------------------ placement

        static Ref<RectTransform> Rect(const Ref<Transform>& parent, const string& nom);

        /// Ancre en un point (ax, ay de 0 à 1), décalage et taille en pixels de référence (1920 × 1080).
        static Ref<RectTransform> Placer(const Ref<RectTransform>& r, float ax, float ay, float x, float y, float w, float h, float px = 0.5f, float py = 0.5f);

        /// Remplit le parent avec des marges.
        static Ref<RectTransform> Remplir(const Ref<RectTransform>& r, float g = 0.0f, float b = 0.0f, float d = 0.0f, float h = 0.0f);

        // ------------------------------------------------------------------ éléments

        static Ref<Moteur::Image> Image(const Ref<Transform>& parent, const string& nom, const Ref<Sprite>& s, Color c);

        /// Panneau stylé : fond + liseré. Rend le RectTransform du panneau.
        static Ref<RectTransform> Panneau(const Ref<Transform>& parent, const string& nom, Style st);

        static Ref<Text> Texte(const Ref<Transform>& parent, const string& nom, const string& s, int taille, Color c, TextAnchor ancre = TextAnchor::MiddleCenter, bool gras = false, bool ombre = true);

        /// « clic » peut être vide (nullptr) : le bouton ne fait alors que son bruit de clic.
        static Ref<Button> Bouton(const Ref<Transform>& parent, const string& nom, const string& libelle, Style st, std::function<void()> clic, int taille = 24);

        static Ref<Text> Libelle(const Ref<Button>& b);

        static Ref<Moteur::Image> IconeImage(const Ref<Transform>& parent, const string& id, float taille);

        /// Barre de jauge : fond sombre + remplissage (Image en mode « Filled »).
        static Ref<Moteur::Image> Barre(const Ref<Transform>& parent, const string& nom, Color c, Ref<RectTransform>& cadre);

        static string Hex(Color c);

        static string Colorer(const string& s, Color c);

        static void Vider(const Ref<Transform>& t);

        /// L'icône d'une quête selon son genre ; celle d'un avis de recherche est le portrait de l'évadé.
        static string IconeQuete(const Ptr<Regles::Quete>& q);

        static string IconeObjet(const Ptr<Regles::DefObjet>& o);

    private:
        static inline Ref<Sprite> plein, contour, lueur, degrade, blanc;
        static inline Dico<string, Ref<Sprite>> icones;

        /// Inscrit auprès du moteur la remise à zéro des variables statiques ci-dessus (Police, sprites, cache des
        /// icônes), comme au lancement d'une partie dans Unity ; défini dans UIK.cpp.
        static const bool statiquesInscrits;

        // ------------------------------------------------------------------ sprites dessinés

        static Ref<Sprite> Arrondi(int n, int rayon, float epaisseur);
        static Ref<Sprite> Radial(int n);
        static Ref<Sprite> Degrade(int w, int h);
    };
}
