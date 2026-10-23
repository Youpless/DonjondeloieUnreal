// Donjon de l'Oie — System.Random de .NET (algorithme soustractif de Knuth, d'après « Numerical Recipes in C »),
// transcrit de la source de référence de .NET.
#include "AleaNet.h"

#include <chrono>
#include <climits>
#include <cstdint>
#include <cstdlib>

namespace Moteur
{
    namespace
    {
        const int MBIG = INT_MAX;
        const int MSEED = 161803398;
    }

    AleaNet::AleaNet(int graine) { Initialiser(graine); }

    AleaNet::AleaNet()
    {
        // Environment.TickCount : millisecondes écoulées, sur 32 bits
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        Initialiser(static_cast<int>(static_cast<uint32_t>(ms & 0x7fffffff)));
    }

    void AleaNet::Initialiser(int graine)
    {
        int soustraction = (graine == INT_MIN) ? INT_MAX : std::abs(graine);
        int mj = MSEED - soustraction;
        t[55] = mj;
        int mk = 1;
        for (int i = 1; i < 55; i++)
        {
            int ii = (21 * i) % 55;
            t[ii] = mk;
            mk = mj - mk;
            if (mk < 0) mk += MBIG;
            mj = t[ii];
        }
        for (int k = 1; k < 5; k++)
        {
            for (int i = 1; i < 56; i++)
            {
                t[i] -= t[1 + (i + 30) % 55];
                if (t[i] < 0) t[i] += MBIG;
            }
        }
        suivant = 0;
        suivantP = 21;
    }

    int AleaNet::Tirage()
    {
        int a = suivant, b = suivantP;
        if (++a >= 56) a = 1;
        if (++b >= 56) b = 1;
        int r = t[a] - t[b];
        if (r == MBIG) r--;
        if (r < 0) r += MBIG;
        t[a] = r;
        suivant = a;
        suivantP = b;
        return r;
    }

    double AleaNet::Echantillon() { return Tirage() * (1.0 / MBIG); }

    double AleaNet::EchantillonLarge()
    {
        int r = Tirage();
        bool negatif = (Tirage() % 2 == 0);
        if (negatif) r = -r;
        double d = r;
        d += (INT_MAX - 1);
        d /= 2.0 * static_cast<double>(static_cast<uint32_t>(INT_MAX)) - 1;
        return d;
    }

    int AleaNet::Next() { return Tirage(); }

    int AleaNet::Next(int max) { return static_cast<int>(Echantillon() * max); }

    int AleaNet::Next(int min, int max)
    {
        long long etendue = static_cast<long long>(max) - min;
        if (etendue <= static_cast<long long>(INT_MAX)) return static_cast<int>(Echantillon() * static_cast<double>(etendue)) + min;
        return static_cast<int>(static_cast<long long>(EchantillonLarge() * static_cast<double>(etendue)) + min);
    }

    double AleaNet::NextDouble() { return Echantillon(); }
}
