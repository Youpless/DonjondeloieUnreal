// Donjon de l'Oie — les outils de texte (les méthodes de string de C#). Les textes sont en UTF-8 : les minuscules et
// majuscules savent traiter les lettres accentuées du français (É → é, Œ → œ…). Une chaîne vide tient lieu du « null »
// de C# (string.IsNullOrEmpty).
#pragma once

#include "Liste.h"

namespace Moteur
{
    namespace Texte
    {
        /// Un nombre en texte, comme ToString() en C# (les réels avec une virgule, à la française).
        string De(int n);
        string De(long long n);
        string De(float f);
        string De(double d);
        /// Deux chiffres hexadécimaux en minuscules (ToString("x2")).
        string Hex2(int octet);
        /// Au moins deux chiffres (ToString("00")).
        string Deux(int n);

        bool Vide(const string& s);                                  // string.IsNullOrEmpty
        bool Contient(const string& s, const string& morceau);
        bool CommencePar(const string& s, const string& debut);
        bool FinitPar(const string& s, const string& fin);
        /// Position (en octets) de la première occurrence, ou -1.
        int Position(const string& s, const string& morceau);
        /// Sous-chaîne à partir d'une position en octets (Substring) ; « longueur » < 0 : jusqu'au bout.
        string Sous(const string& s, int debut, int longueur = -1);
        string Minuscules(const string& s);                          // ToLowerInvariant
        string Majuscules(const string& s);                          // ToUpperInvariant
        bool EgauxSansCasse(const string& a, const string& b);       // StringComparison.OrdinalIgnoreCase
        string Remplacer(const string& s, const string& quoi, const string& par);   // Replace
        string Rogner(const string& s);                              // Trim
        string RognerFin(const string& s);                           // TrimEnd
        Liste<string> Decouper(const string& s, char separateur);    // Split
        string Joindre(const string& separateur, const Liste<string>& morceaux);   // string.Join
        /// Nombre de caractères (et non d'octets) : la Length de C# pour les textes du jeu.
        int Longueur(const string& s);
        /// Les « unites » premiers caractères, comptés comme Longueur (Substring(0, n) de C# sur un texte du jeu).
        /// Un caractère hors du plan de base (deux unités) qui serait coupé en deux s'arrête avant.
        string Debut(const string& s, int unites);
        /// Lire un entier (int.TryParse, culture invariante) ; faux si ce n'en est pas un.
        bool LireEntier(const string& s, int& valeur);
        /// « a ?? b » pour les textes : a s'il n'est pas vide, sinon b.
        inline string Ou(const string& a, const string& b) { return a.empty() ? b : a; }
    }
}
