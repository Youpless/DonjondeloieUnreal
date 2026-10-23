// Donjon de l'Oie — l'arrêt sur erreur de programmation. Dans Unreal, la couche Unreal branche RapportErreurFatale sur
// le journal (Output Log) ; hors Unreal, le message va sur la sortie d'erreur.
#include "Commun.h"

#include <cstdio>
#include <cstdlib>

namespace Moteur
{
    void (*RapportErreurFatale)(const char* message) = nullptr;

    void ErreurFatale(const char* message)
    {
        if (RapportErreurFatale != nullptr) RapportErreurFatale(message);
        std::fprintf(stderr, "[ERREUR FATALE] %s\n", message);
        std::abort();
    }
}
