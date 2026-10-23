// Donjon de l'Oie — les types mathématiques du jeu : vecteurs, quaternions, matrices, couleurs, Mathf. Mêmes noms et
// mêmes conventions que dans la version Unity : repère à main gauche, +y vers le haut, +z vers l'avant, unités en
// mètres, angles d'Euler appliqués dans l'ordre Z, X, Y. La couche Unreal convertit à l'affichage (Z vers le haut,
// centimètres) : le reste du code calcule exactement comme le jeu d'origine.
#pragma once

#include "../Noyau/AleaNet.h"
#include "../Noyau/Commun.h"

#include <cmath>
#include <cstdint>

namespace Moteur
{
    struct Vector3;
    struct Vector4;
    struct Color32;

    /// Math.Sin et Math.Cos de .NET. Le jeu C# tournait ses essais sous Mono, qui les calcule avec les instructions
    /// x87 fsin / fcos (quelques bits de différence avec la bibliothèque C près des multiples de π/2) : les essais de
    /// comparaison (DD_TRIGO_X87, Linux x86-64) les imitent ; le jeu (Unreal) prend les fonctions standard.
#if defined(DD_TRIGO_X87) && (defined(__x86_64__) || defined(__i386__)) && !defined(_MSC_VER)
    inline double SinNet(double x)
    {
        double r;
        __asm__("fsin" : "=t"(r) : "0"(x));
        return r;
    }
    inline double CosNet(double x)
    {
        double r;
        __asm__("fcos" : "=t"(r) : "0"(x));
        return r;
    }
#else
    inline double SinNet(double x) { return std::sin(x); }
    inline double CosNet(double x) { return std::cos(x); }
#endif

    struct Mathf
    {
        static constexpr float Pi = 3.14159265358979323846f;
        static constexpr float Deg2Rad = Pi / 180.0f;
        static constexpr float Rad2Deg = 180.0f / Pi;
        static constexpr float Infinity = HUGE_VALF;

        static float Sin(float f) { return static_cast<float>(SinNet(static_cast<double>(f))); }
        static float Cos(float f) { return static_cast<float>(CosNet(static_cast<double>(f))); }
        static float Tan(float f) { return static_cast<float>(std::tan(static_cast<double>(f))); }
        static float Atan2(float y, float x) { return static_cast<float>(std::atan2(static_cast<double>(y), static_cast<double>(x))); }
        static float Atan(float f) { return static_cast<float>(std::atan(static_cast<double>(f))); }
        static float Acos(float f) { return static_cast<float>(std::acos(static_cast<double>(f))); }
        static float Asin(float f) { return static_cast<float>(std::asin(static_cast<double>(f))); }
        static float Sqrt(float f) { return static_cast<float>(std::sqrt(static_cast<double>(f))); }
        static float Abs(float f) { return std::fabs(f); }
        static int Abs(int f) { return f < 0 ? -f : f; }
        static float Pow(float a, float b) { return static_cast<float>(std::pow(static_cast<double>(a), static_cast<double>(b))); }
        static float Exp(float a) { return static_cast<float>(std::exp(static_cast<double>(a))); }
        static float Log(float a) { return static_cast<float>(std::log(static_cast<double>(a))); }
        static float Min(float a, float b) { return a < b ? a : b; }
        static float Max(float a, float b) { return a > b ? a : b; }
        static int Min(int a, int b) { return a < b ? a : b; }
        static int Max(int a, int b) { return a > b ? a : b; }
        static float Min(float a, float b, float c) { return Min(Min(a, b), c); }
        static float Max(float a, float b, float c) { return Max(Max(a, b), c); }
        static float Min(float a, float b, float c, float d) { return Min(Min(Min(a, b), c), d); }
        static float Max(float a, float b, float c, float d) { return Max(Max(Max(a, b), c), d); }
        static float Clamp(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }
        static int Clamp(int v, int a, int b) { return v < a ? a : (v > b ? b : v); }
        static float Clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }
        static float Lerp(float a, float b, float t) { return a + (b - a) * Clamp01(t); }
        static float LerpUnclamped(float a, float b, float t) { return a + (b - a) * t; }
        static float InverseLerp(float a, float b, float v) { return a != b ? Clamp01((v - a) / (b - a)) : 0.0f; }
        static float SmoothStep(float a, float b, float t)
        {
            t = Clamp01(t);
            t = t * t * (3.0f - 2.0f * t);
            return a + (b - a) * t;
        }
        static float MoveTowards(float c, float t, float d)
        {
            if (std::fabs(t - c) <= d) return t;
            return c + (t - c > 0.0f ? 1.0f : (t - c < 0.0f ? -1.0f : 0.0f)) * d;
        }
        static float Repeat(float t, float l) { return Clamp(t - static_cast<float>(std::floor(static_cast<double>(t / l))) * l, 0.0f, l); }
        static float PingPong(float t, float l)
        {
            t = Repeat(t, l * 2.0f);
            return l - std::fabs(t - l);
        }
        static float DeltaAngle(float a, float b)
        {
            float d = Repeat(b - a, 360.0f);
            if (d > 180.0f) d -= 360.0f;
            return d;
        }
        static float LerpAngle(float a, float b, float t) { return a + DeltaAngle(a, b) * Clamp01(t); }
        static int FloorToInt(float f) { return static_cast<int>(std::floor(static_cast<double>(f))); }
        static int CeilToInt(float f) { return static_cast<int>(std::ceil(static_cast<double>(f))); }
        /// Arrondi au plus proche, à égalité vers le pair (Math.Round de .NET).
        static int RoundToInt(float f) { return static_cast<int>(std::nearbyint(static_cast<double>(f))); }
        static float Floor(float f) { return static_cast<float>(std::floor(static_cast<double>(f))); }
        static float Ceil(float f) { return static_cast<float>(std::ceil(static_cast<double>(f))); }
        static float Round(float f) { return static_cast<float>(std::nearbyint(static_cast<double>(f))); }
        static float Sign(float f) { return f >= 0.0f ? 1.0f : -1.0f; }
        static bool Approximately(float a, float b) { return std::fabs(a - b) < 1e-6f; }
        /// Bruit de gradient simple dans [0, 1] (le même que la version Unity de ce jeu hors moteur).
        static float PerlinNoise(float x, float y);
    };

    /// Math.Round de .NET sur un double (égalité vers le pair), Math.Floor, Math.Sign.
    inline double ArrondiNet(double d) { return std::nearbyint(d); }
    inline int SigneNet(float f) { return f > 0.0f ? 1 : (f < 0.0f ? -1 : 0); }
    inline int SigneNet(int i) { return i > 0 ? 1 : (i < 0 ? -1 : 0); }

    struct Vector2
    {
        float x = 0, y = 0;
        constexpr Vector2() = default;
        constexpr Vector2(float x_, float y_) : x(x_), y(y_) {}
        Vector2(const Vector3& v);       // implicite, comme en C#

        static const Vector2 zero, one, up, right;
        Vector2 operator+(const Vector2& b) const { return {x + b.x, y + b.y}; }
        Vector2 operator-(const Vector2& b) const { return {x - b.x, y - b.y}; }
        Vector2 operator-() const { return {-x, -y}; }
        Vector2 operator*(float d) const { return {x * d, y * d}; }
        Vector2 operator/(float d) const { return {x / d, y / d}; }
        Vector2& operator+=(const Vector2& b) { x += b.x; y += b.y; return *this; }
        Vector2& operator-=(const Vector2& b) { x -= b.x; y -= b.y; return *this; }
        Vector2& operator*=(float d) { x *= d; y *= d; return *this; }
        bool operator==(const Vector2& b) const { return x == b.x && y == b.y; }
        bool operator!=(const Vector2& b) const { return !(*this == b); }
        float magnitude() const { return static_cast<float>(std::sqrt(static_cast<double>(x * x + y * y))); }
        float sqrMagnitude() const { return x * x + y * y; }
        Vector2 normalized() const
        {
            float m = magnitude();
            return m > 1e-6f ? *this / m : zero;
        }
        static Vector2 Lerp(const Vector2& a, const Vector2& b, float t) { return a + (b - a) * Mathf::Clamp01(t); }
        static float Distance(const Vector2& a, const Vector2& b) { return (a - b).magnitude(); }
        static Vector2 Scale(const Vector2& a, const Vector2& b) { return {a.x * b.x, a.y * b.y}; }
        static float Dot(const Vector2& a, const Vector2& b) { return a.x * b.x + a.y * b.y; }
    };
    inline Vector2 operator*(float d, const Vector2& a) { return {a.x * d, a.y * d}; }
    inline const Vector2 Vector2::zero{0, 0};
    inline const Vector2 Vector2::one{1, 1};
    inline const Vector2 Vector2::up{0, 1};
    inline const Vector2 Vector2::right{1, 0};

    struct Vector3
    {
        float x = 0, y = 0, z = 0;
        constexpr Vector3() = default;
        constexpr Vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
        constexpr Vector3(float x_, float y_) : x(x_), y(y_), z(0) {}
        constexpr Vector3(const Vector2& v) : x(v.x), y(v.y), z(0) {}      // implicite, comme en C#

        static const Vector3 zero, one, up, down, right, left, forward, back;

        float operator[](int i) const { return i == 0 ? x : (i == 1 ? y : z); }
        float& operator[](int i) { return i == 0 ? x : (i == 1 ? y : z); }
        Vector3 operator+(const Vector3& b) const { return {x + b.x, y + b.y, z + b.z}; }
        Vector3 operator-(const Vector3& b) const { return {x - b.x, y - b.y, z - b.z}; }
        Vector3 operator-() const { return {-x, -y, -z}; }
        Vector3 operator*(float d) const { return {x * d, y * d, z * d}; }
        Vector3 operator/(float d) const { return {x / d, y / d, z / d}; }
        Vector3& operator+=(const Vector3& b) { x += b.x; y += b.y; z += b.z; return *this; }
        Vector3& operator-=(const Vector3& b) { x -= b.x; y -= b.y; z -= b.z; return *this; }
        Vector3& operator*=(float d) { x *= d; y *= d; z *= d; return *this; }
        Vector3& operator/=(float d) { x /= d; y /= d; z /= d; return *this; }
        /// Égalité approchée, comme Unity (écart au carré < 1e-10).
        bool operator==(const Vector3& b) const { return (*this - b).sqrMagnitude() < 1e-10f; }
        bool operator!=(const Vector3& b) const { return !(*this == b); }
        float magnitude() const { return static_cast<float>(std::sqrt(static_cast<double>(x * x + y * y + z * z))); }
        float sqrMagnitude() const { return x * x + y * y + z * z; }
        Vector3 normalized() const
        {
            float m = magnitude();
            return m > 1e-6f ? *this / m : zero;
        }
        void Normalize() { *this = normalized(); }

        static float Dot(const Vector3& a, const Vector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
        static Vector3 Cross(const Vector3& a, const Vector3& b) { return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
        static Vector3 Lerp(const Vector3& a, const Vector3& b, float t) { return a + (b - a) * Mathf::Clamp01(t); }
        static Vector3 LerpUnclamped(const Vector3& a, const Vector3& b, float t) { return a + (b - a) * t; }
        static Vector3 Scale(const Vector3& a, const Vector3& b) { return {a.x * b.x, a.y * b.y, a.z * b.z}; }
        static float Distance(const Vector3& a, const Vector3& b) { return (a - b).magnitude(); }
        static Vector3 Normalize(const Vector3& v) { return v.normalized(); }
        static Vector3 Min(const Vector3& a, const Vector3& b) { return {Mathf::Min(a.x, b.x), Mathf::Min(a.y, b.y), Mathf::Min(a.z, b.z)}; }
        static Vector3 Max(const Vector3& a, const Vector3& b) { return {Mathf::Max(a.x, b.x), Mathf::Max(a.y, b.y), Mathf::Max(a.z, b.z)}; }
        static Vector3 MoveTowards(const Vector3& a, const Vector3& b, float d)
        {
            Vector3 v = b - a;
            float m = v.magnitude();
            return (m <= d || m < 1e-6f) ? b : a + v / m * d;
        }
        static Vector3 ProjectOnPlane(const Vector3& v, const Vector3& n)
        {
            float s = n.sqrMagnitude();
            if (s < 1e-9f) return v;
            return v - n * (Dot(v, n) / s);
        }
        static float Angle(const Vector3& a, const Vector3& b)
        {
            float d = Mathf::Clamp(Dot(a.normalized(), b.normalized()), -1.0f, 1.0f);
            return static_cast<float>(std::acos(static_cast<double>(d)) * 180.0 / 3.14159265358979323846);
        }
        static Vector3 Slerp(const Vector3& a, const Vector3& b, float t) { return Lerp(a, b, t); }
    };
    inline Vector3 operator*(float d, const Vector3& a) { return {a.x * d, a.y * d, a.z * d}; }
    inline const Vector3 Vector3::zero{0, 0, 0};
    inline const Vector3 Vector3::one{1, 1, 1};
    inline const Vector3 Vector3::up{0, 1, 0};
    inline const Vector3 Vector3::down{0, -1, 0};
    inline const Vector3 Vector3::right{1, 0, 0};
    inline const Vector3 Vector3::left{-1, 0, 0};
    inline const Vector3 Vector3::forward{0, 0, 1};
    inline const Vector3 Vector3::back{0, 0, -1};
    inline Vector2::Vector2(const Vector3& v) : x(v.x), y(v.y) {}

    struct Vector4
    {
        float x = 0, y = 0, z = 0, w = 0;
        constexpr Vector4() = default;
        constexpr Vector4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
        constexpr Vector4(const Vector3& v) : x(v.x), y(v.y), z(v.z), w(0) {}
        operator Vector3() const { return {x, y, z}; }
        static const Vector4 zero;
    };
    inline const Vector4 Vector4::zero{0, 0, 0, 0};

    struct Vector3Int
    {
        int x = 0, y = 0, z = 0;
        constexpr Vector3Int() = default;
        constexpr Vector3Int(int x_, int y_, int z_) : x(x_), y(y_), z(z_) {}
        bool operator==(const Vector3Int& o) const { return x == o.x && y == o.y && z == o.z; }
        bool operator!=(const Vector3Int& o) const { return !(*this == o); }
        size_t Hache() const { return static_cast<size_t>((static_cast<uint32_t>(x) * 73856093u) ^ (static_cast<uint32_t>(y) * 19349663u) ^ (static_cast<uint32_t>(z) * 83492791u)); }
    };

    struct Vector2Int
    {
        int x = 0, y = 0;
        constexpr Vector2Int() = default;
        constexpr Vector2Int(int x_, int y_) : x(x_), y(y_) {}
        bool operator==(const Vector2Int& o) const { return x == o.x && y == o.y; }
        bool operator!=(const Vector2Int& o) const { return !(*this == o); }
        size_t Hache() const { return static_cast<size_t>((static_cast<uint32_t>(x) * 73856093u) ^ (static_cast<uint32_t>(y) * 19349663u)); }
    };

    struct Quaternion
    {
        float x = 0, y = 0, z = 0, w = 1;
        constexpr Quaternion() = default;
        constexpr Quaternion(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
        static const Quaternion identity;

        static Quaternion AngleAxis(float angle, Vector3 axe);
        /// Ordre d'Unity : Z, puis X, puis Y (q = qy * qx * qz).
        static Quaternion Euler(float ex, float ey, float ez);
        static Quaternion Euler(const Vector3& e) { return Euler(e.x, e.y, e.z); }
        Quaternion operator*(const Quaternion& b) const
        {
            return {w * b.x + x * b.w + y * b.z - z * b.y,
                    w * b.y + y * b.w + z * b.x - x * b.z,
                    w * b.z + z * b.w + x * b.y - y * b.x,
                    w * b.w - x * b.x - y * b.y - z * b.z};
        }
        Vector3 operator*(const Vector3& v) const;
        static Quaternion FromToRotation(Vector3 a, Vector3 b);
        static Quaternion Inverse(const Quaternion& q) { return {-q.x, -q.y, -q.z, q.w}; }
        static Quaternion LookRotation(const Vector3& avant) { return LookRotation(avant, Vector3::up); }
        static Quaternion LookRotation(const Vector3& avant, const Vector3& haut);
        static Quaternion Slerp(const Quaternion& a, Quaternion b, float t);
        static Quaternion Lerp(const Quaternion& a, const Quaternion& b, float t) { return Slerp(a, b, t); }
        static float Angle(const Quaternion& a, const Quaternion& b);
        Vector3 eulerAngles() const;
        bool operator==(const Quaternion& o) const { return x == o.x && y == o.y && z == o.z && w == o.w; }
        bool operator!=(const Quaternion& o) const { return !(*this == o); }
    };
    inline const Quaternion Quaternion::identity{0, 0, 0, 1};

    struct Matrix4x4
    {
        float m00 = 0, m01 = 0, m02 = 0, m03 = 0, m10 = 0, m11 = 0, m12 = 0, m13 = 0;
        float m20 = 0, m21 = 0, m22 = 0, m23 = 0, m30 = 0, m31 = 0, m32 = 0, m33 = 0;
        static Matrix4x4 identity()
        {
            Matrix4x4 m;
            m.m00 = m.m11 = m.m22 = m.m33 = 1;
            return m;
        }
        static Matrix4x4 TRS(const Vector3& p, const Quaternion& q, const Vector3& s);
        Matrix4x4 operator*(const Matrix4x4& b) const;
        Vector3 MultiplyPoint3x4(const Vector3& v) const
        {
            return {m00 * v.x + m01 * v.y + m02 * v.z + m03, m10 * v.x + m11 * v.y + m12 * v.z + m13, m20 * v.x + m21 * v.y + m22 * v.z + m23};
        }
        Vector3 MultiplyPoint(const Vector3& v) const { return MultiplyPoint3x4(v); }
        Vector3 MultiplyVector(const Vector3& v) const
        {
            return {m00 * v.x + m01 * v.y + m02 * v.z, m10 * v.x + m11 * v.y + m12 * v.z, m20 * v.x + m21 * v.y + m22 * v.z};
        }
    };

    struct Color
    {
        float r = 0, g = 0, b = 0, a = 0;
        constexpr Color() = default;
        constexpr Color(float r_, float g_, float b_, float a_ = 1.0f) : r(r_), g(g_), b(b_), a(a_) {}
        Color(const Color32& c);         // implicite, comme en C#

        static const Color white, black, clear, red, green, blue, yellow, gray;
        Color operator*(float f) const { return {r * f, g * f, b * f, a * f}; }
        Color operator*(const Color& o) const { return {r * o.r, g * o.g, b * o.b, a * o.a}; }
        Color operator+(const Color& o) const { return {r + o.r, g + o.g, b + o.b, a + o.a}; }
        Color operator-(const Color& o) const { return {r - o.r, g - o.g, b - o.b, a - o.a}; }
        Color operator/(float f) const { return {r / f, g / f, b / f, a / f}; }
        static Color Lerp(const Color& x, const Color& y, float t)
        {
            t = Mathf::Clamp01(t);
            return {x.r + (y.r - x.r) * t, x.g + (y.g - x.g) * t, x.b + (y.b - x.b) * t, x.a + (y.a - x.a) * t};
        }
        operator Vector4() const { return {r, g, b, a}; }
        Color linear() const { return *this; }
        float maxColorComponent() const { return Mathf::Max(r, Mathf::Max(g, b)); }
        float grayscale() const { return 0.299f * r + 0.587f * g + 0.114f * b; }
        bool operator==(const Color& o) const
        {
            return std::fabs(r - o.r) < 1e-5f && std::fabs(g - o.g) < 1e-5f && std::fabs(b - o.b) < 1e-5f && std::fabs(a - o.a) < 1e-5f;
        }
        bool operator!=(const Color& o) const { return !(*this == o); }
        static Color HSVToRGB(float h, float s, float v);
        static void RGBToHSV(const Color& c, float& h, float& s, float& v);
    };
    inline Color operator*(float f, const Color& c) { return c * f; }
    inline const Color Color::white{1, 1, 1, 1};
    inline const Color Color::black{0, 0, 0, 1};
    inline const Color Color::clear{0, 0, 0, 0};
    inline const Color Color::red{1, 0, 0, 1};
    inline const Color Color::green{0, 1, 0, 1};
    inline const Color Color::blue{0, 0, 1, 1};
    inline const Color Color::yellow{1, 0.92f, 0.016f, 1};
    inline const Color Color::gray{0.5f, 0.5f, 0.5f, 1};

    struct Color32
    {
        uint8_t r = 0, g = 0, b = 0, a = 0;
        constexpr Color32() = default;
        constexpr Color32(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_) : r(r_), g(g_), b(b_), a(a_) {}
        /// Depuis une couleur réelle (implicite, comme en C#) : arrondi .NET de chaque composante × 255.
        Color32(const Color& c);
        static Color32 Lerp(const Color32& x, const Color32& y, float t) { return Color32(Color::Lerp(Color(x), Color(y), t)); }
        bool operator==(const Color32& o) const { return r == o.r && g == o.g && b == o.b && a == o.a; }
        bool operator!=(const Color32& o) const { return !(*this == o); }
    };
    inline Color::Color(const Color32& c) : r(c.r / 255.0f), g(c.g / 255.0f), b(c.b / 255.0f), a(c.a / 255.0f) {}

    struct Rect
    {
        float x = 0, y = 0, width = 0, height = 0;
        constexpr Rect() = default;
        constexpr Rect(float x_, float y_, float w, float h) : x(x_), y(y_), width(w), height(h) {}
        Vector2 center() const { return {x + width / 2.0f, y + height / 2.0f}; }
        Vector2 size() const { return {width, height}; }
        float xMin() const { return x; }
        float yMin() const { return y; }
        float xMax() const { return x + width; }
        float yMax() const { return y + height; }
        bool Contains(const Vector2& p) const { return p.x >= x && p.y >= y && p.x < x + width && p.y < y + height; }
    };

    struct Bounds
    {
        Vector3 center, size;
        constexpr Bounds() = default;
        constexpr Bounds(const Vector3& c, const Vector3& s) : center(c), size(s) {}
        Vector3 extents() const { return size * 0.5f; }
        Vector3 min() const { return center - extents(); }
        Vector3 max() const { return center + extents(); }
        void Encapsulate(const Vector3& p)
        {
            Vector3 mn = Vector3::Min(min(), p), mx = Vector3::Max(max(), p);
            center = (mn + mx) * 0.5f;
            size = mx - mn;
        }
        void Encapsulate(const Bounds& o)
        {
            Encapsulate(o.min());
            Encapsulate(o.max());
        }
    };

    struct Ray
    {
        Vector3 origin, direction;
        Ray() = default;
        Ray(const Vector3& o, const Vector3& d) : origin(o), direction(d.normalized()) {}
        Vector3 GetPoint(float t) const { return origin + direction * t; }
    };

    struct Plane
    {
        Vector3 normal;
        float distance = 0;
        Plane() = default;
        Plane(const Vector3& n, const Vector3& p) : normal(n.normalized()), distance(-Vector3::Dot(normal, p)) {}
        bool Raycast(const Ray& r, float& entree) const
        {
            float vd = Vector3::Dot(r.direction, normal), nd = -Vector3::Dot(r.origin, normal) - distance;
            if (std::fabs(vd) < 1e-6f)
            {
                entree = 0;
                return false;
            }
            entree = nd / vd;
            return entree > 0;
        }
    };

    /// Le hasard « visuel » (UnityEngine.Random) : étincelles, gestes, positions de feux d'artifice. Les règles et les
    /// modèles, eux, utilisent AleaNet avec leur propre graine.
    struct Random
    {
        static float value();
        static float Range(float a, float b);
        static int Range(int a, int b);
        static Vector3 insideUnitSphere();
        static Vector2 insideUnitCircle();
        static Vector3 onUnitSphere();
        static Quaternion rotation();
        static void InitState(int graine);
    };
}
