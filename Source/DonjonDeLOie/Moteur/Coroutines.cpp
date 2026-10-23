// Donjon de l'Oie — les instructions des coroutines.
#include "Coroutines.h"

namespace Moteur
{
    WaitForSecondsRealtime::WaitForSecondsRealtime(float secondes) : fin(Time::realtimeSinceStartup + secondes) {}

    Instruction::Instruction(const Ptr<Coroutine>& c) : sorte(Sorte::AttendreCoroutine), coroutine(c) {}

    Instruction::Instruction(WaitForSecondsRealtime w) : sorte(Sorte::Condition)
    {
        float fin = w.fin;
        attendreEncore = [fin] { return Time::realtimeSinceStartup < fin; };
    }

    Instruction::Instruction(WaitUntil w) : sorte(Sorte::Condition)
    {
        auto f = std::move(w.f);
        attendreEncore = [f] { return !f(); };
    }

    Instruction::Instruction(WaitWhile w) : sorte(Sorte::Condition), attendreEncore(std::move(w.f)) {}

    Instruction::Instruction(Tache&& t) : sorte(Sorte::SousTache), sousTache(Nouveau<Tache>(std::move(t))) {}
}
