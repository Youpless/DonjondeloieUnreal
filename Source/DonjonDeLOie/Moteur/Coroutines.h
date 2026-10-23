// Donjon de l'Oie — les coroutines (les « IEnumerator » d'Unity) en C++20. Une méthode qui renvoie Tache et contient
// co_yield se lance avec StartCoroutine, et avance d'image en image comme dans Unity :
//
//     Tache Attendre(float s) { co_yield WaitForSeconds(s); }          // yield return new WaitForSeconds(s);
//     Tache Boucle()
//     {
//         co_yield nullptr;                                             // yield return null;      (image suivante)
//         co_yield StartCoroutine(Attendre(1.0f));                      // yield return StartCoroutine(…);
//         co_yield Attendre(0.5f);                                      // yield return Attendre(0.5f);  (imbriquée)
//         co_yield WaitUntil([this] { return pret; });                  // yield return new WaitUntil(() => pret);
//         co_return;                                                    // yield break;
//     }
//
// Les paramètres d'une coroutine se passent par valeur (ils sont copiés dans la coroutine, comme en C#).
#pragma once

#include "Objets.h"

namespace Moteur
{
    struct WaitForSeconds
    {
        float s;
        explicit WaitForSeconds(float secondes) : s(secondes) {}
    };
    struct WaitForEndOfFrame
    {
    };
    struct WaitForSecondsRealtime
    {
        float fin;
        explicit WaitForSecondsRealtime(float secondes);
    };
    struct WaitUntil
    {
        std::function<bool()> f;
        explicit WaitUntil(std::function<bool()> condition) : f(std::move(condition)) {}
    };
    struct WaitWhile
    {
        std::function<bool()> f;
        explicit WaitWhile(std::function<bool()> condition) : f(std::move(condition)) {}
    };

    /// Ce qu'une coroutine rend à chaque co_yield.
    class Instruction
    {
    public:
        enum class Sorte { ImageSuivante, Secondes, AttendreCoroutine, Condition, SousTache };

        Instruction() = default;
        Instruction(std::nullptr_t) {}
        Instruction(WaitForSeconds w) : sorte(Sorte::Secondes), secondes(w.s) {}
        Instruction(WaitForEndOfFrame) {}
        Instruction(const Ptr<Coroutine>& c);
        Instruction(WaitForSecondsRealtime w);
        Instruction(WaitUntil w);
        Instruction(WaitWhile w);
        Instruction(Tache&& t);

        Sorte sorte = Sorte::ImageSuivante;
        float secondes = 0.0f;
        Ptr<Coroutine> coroutine;
        std::function<bool()> attendreEncore;      // keepWaiting
        Ptr<Tache> sousTache;
    };

    /// Une coroutine lancée (rendue par StartCoroutine, pour l'arrêter ou l'attendre).
    class Coroutine
    {
    public:
        bool Finie() const { return fini; }

    private:
        Ref<MonoBehaviour> hote;
        Liste<Ptr<Tache>> pile;              // les coroutines imbriquées : la dernière est celle qui tourne
        float attenteJusqua = -1.0f;
        Ptr<Coroutine> attend;
        std::function<bool()> condition;
        bool fini = false;
        friend struct Boucle;
    };
}
