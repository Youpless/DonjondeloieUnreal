// Donjon de l'Oie — le socle commun du code du jeu, indépendant d'Unreal : pointeurs partagés (les « références » de
// C#), entiers, et quelques outils. Tout le jeu (règles, modèles voxel, monde, interface) est écrit en C++ standard ;
// seul le dossier Unreal/ parle à Unreal Engine.
//
// Règles d'écriture, pour que le code compile partout (Unreal désactive les exceptions et le RTTI) :
//  - pas de throw/try, pas de dynamic_cast ni de typeid ;
//  - pas de nom qui soit aussi une macro d'Unreal (PI, check, verify, ensure, TEXT, IN, OUT…) ;
//  - les tailles sont des int (Nb()), jamais de comparaison entre signé et non signé.
#pragma once

// Visual Studio (Unreal) : ce code n'inclut pas les en-têtes d'Unreal, qui désactivent d'ordinaire ces avertissements de
// niveau 4 (paramètres inutilisés, conversions entre nombres, expressions constantes…). Les masquages de noms
// (4456 à 4459) sont vérifiés ici par gcc et clang ; MSVC compte en plus comme masquage un nom de variable locale
// identique à une fonction de sa bibliothèque C (y0, y1…), d'où leur désactivation.
#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning(disable : 4100 4127 4189 4244 4245 4267 4305 4324 4456 4457 4458 4459 4505 4702 4706 4834 4996 5054)
// Unreal compile avec /fp:fast, qui permet à Visual Studio de réordonner les calculs à virgule et de fusionner
// multiplications et additions : le jeu, lui, calcule comme C# (IEEE 754 strict), pour que l'île, le décor, les tirages et
// les bots donnent exactement les mêmes résultats que la version Unity. (Ces réglages valent pour la suite du fichier.)
#pragma float_control(precise, on)
#pragma fp_contract(off)
#endif

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Moteur
{
    /// Une référence partagée : l'équivalent d'une référence C# (l'objet vit tant que quelqu'un le tient).
    template <class T>
    using Ptr = std::shared_ptr<T>;

    template <class T, class... A>
    inline Ptr<T> Nouveau(A&&... a)
    {
        return std::make_shared<T>(std::forward<A>(a)...);
    }

    using std::string;

    /// La taille d'un conteneur en int (comme Count en C#).
    template <class C>
    inline int Nb(const C& c)
    {
        return static_cast<int>(c.size());
    }

    /// Un ensemble de fonctions sans argument (un « UnityEvent », un delegate multicast de C#).
    class Evenement
    {
    public:
        void AddListener(std::function<void()> f)
        {
            if (f) l.push_back(std::move(f));
        }
        void RemoveAllListeners() { l.clear(); }
        void Invoke() const
        {
            auto copie = l;     // une action peut modifier la liste pendant l'appel
            for (auto& f : copie) f();
        }
        bool Vide() const { return l.empty(); }

    private:
        std::vector<std::function<void()>> l;
    };

    /// Arrêt net avec un message (erreur de programmation : indice hors limites, etc.). Dans Unreal, écrit dans le
    /// journal (Output Log) avant de s'arrêter ; hors Unreal, sur la sortie d'erreur.
    [[noreturn]] void ErreurFatale(const char* message);
    /// Branché par la couche Unreal pour écrire le message dans le journal avant l'arrêt.
    extern void (*RapportErreurFatale)(const char* message);

#define DD_VERIFIE(condition, message)                          \
    do                                                          \
    {                                                           \
        if (!(condition)) ::Moteur::ErreurFatale(message);      \
    } while (0)
}
