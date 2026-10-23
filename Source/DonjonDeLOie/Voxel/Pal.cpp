// Donjon de l'Oie — la palette : mélanges, nuances, variation par position.
#include "Pal.h"

namespace DonjonDeLOie::Voxel
{
    Color32 Pal::Mix(Color32 a, Color32 b, float t)
    {
        t = Mathf::Clamp01(t);
        return Color32(static_cast<uint8_t>(static_cast<float>(a.r) + static_cast<float>(b.r - a.r) * t),
                       static_cast<uint8_t>(static_cast<float>(a.g) + static_cast<float>(b.g - a.g) * t),
                       static_cast<uint8_t>(static_cast<float>(a.b) + static_cast<float>(b.b - a.b) * t), 255);
    }

    Color32 Pal::Shade(Color32 c, float f)
    {
        return Color32(static_cast<uint8_t>(Mathf::Clamp(static_cast<float>(c.r) * f, 0.0f, 255.0f)), static_cast<uint8_t>(Mathf::Clamp(static_cast<float>(c.g) * f, 0.0f, 255.0f)),
                       static_cast<uint8_t>(Mathf::Clamp(static_cast<float>(c.b) * f, 0.0f, 255.0f)), c.a);
    }

    float Pal::Hash01(int x, int y, int z)
    {
        uint32_t h = (static_cast<uint32_t>(x) * 73856093u) ^ (static_cast<uint32_t>(y) * 19349663u) ^ (static_cast<uint32_t>(z) * 83492791u);
        h ^= h >> 13;
        h *= 0x5bd1e995u;
        h ^= h >> 15;
        return static_cast<float>(h & 0xffffffu) / static_cast<float>(0x1000000);
    }
}
