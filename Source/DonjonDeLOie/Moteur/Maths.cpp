// Donjon de l'Oie — les calculs des types mathématiques (quaternions, matrices, couleurs, bruit, hasard visuel).
#include "Maths.h"

#include <chrono>

namespace Moteur
{
    namespace
    {
        const double PiDouble = 3.14159265358979323846;

        float Gradient(int x, int y, float dx, float dy)
        {
            uint32_t h = static_cast<uint32_t>(x) * 374761393u + static_cast<uint32_t>(y) * 668265263u;
            h = (h ^ (h >> 13)) * 1274126177u;
            h ^= h >> 16;
            double ang = (h & 1023u) / 1024.0 * PiDouble * 2;
            return static_cast<float>(CosNet(ang) * dx + SinNet(ang) * dy);
        }
    }

    float Mathf::PerlinNoise(float x, float y)
    {
        int xi = static_cast<int>(std::floor(static_cast<double>(x))), yi = static_cast<int>(std::floor(static_cast<double>(y)));
        float xf = x - static_cast<float>(xi), yf = y - static_cast<float>(yi);
        float u = xf * xf * (3 - 2 * xf), v = yf * yf * (3 - 2 * yf);
        float a = Gradient(xi, yi, xf, yf), b = Gradient(xi + 1, yi, xf - 1, yf), c = Gradient(xi, yi + 1, xf, yf - 1),
              d = Gradient(xi + 1, yi + 1, xf - 1, yf - 1);
        float r = a + (b - a) * u + (c - a) * v + (a - b - c + d) * u * v;
        return Clamp01(0.5f + r * 0.7f);
    }

    // ------------------------------------------------------------------ quaternions

    Quaternion Quaternion::AngleAxis(float angle, Vector3 axe)
    {
        axe = axe.normalized();
        double h = angle * PiDouble / 360.0;
        float s = static_cast<float>(SinNet(h));
        return {axe.x * s, axe.y * s, axe.z * s, static_cast<float>(CosNet(h))};
    }

    Quaternion Quaternion::Euler(float ex, float ey, float ez)
    {
        return AngleAxis(ey, Vector3::up) * AngleAxis(ex, Vector3::right) * AngleAxis(ez, Vector3::forward);
    }

    Vector3 Quaternion::operator*(const Vector3& v) const
    {
        float x2 = x * 2, y2 = y * 2, z2 = z * 2;
        float xx = x * x2, yy = y * y2, zz = z * z2, xy = x * y2, xz = x * z2, yz = y * z2;
        float wx = w * x2, wy = w * y2, wz = w * z2;
        return {(1 - (yy + zz)) * v.x + (xy - wz) * v.y + (xz + wy) * v.z,
                (xy + wz) * v.x + (1 - (xx + zz)) * v.y + (yz - wx) * v.z,
                (xz - wy) * v.x + (yz + wx) * v.y + (1 - (xx + yy)) * v.z};
    }

    Quaternion Quaternion::FromToRotation(Vector3 a, Vector3 b)
    {
        a = a.normalized();
        b = b.normalized();
        float d = Vector3::Dot(a, b);
        if (d > 0.99999f) return identity;
        if (d < -0.99999f)
        {
            Vector3 axe = Vector3::Cross(Vector3::right, a);
            if (axe.sqrMagnitude() < 1e-6f) axe = Vector3::Cross(Vector3::up, a);
            return AngleAxis(180.0f, axe);
        }
        Vector3 c = Vector3::Cross(a, b);
        Quaternion q(c.x, c.y, c.z, 1 + d);
        float m = static_cast<float>(std::sqrt(static_cast<double>(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w)));
        return {q.x / m, q.y / m, q.z / m, q.w / m};
    }

    Quaternion Quaternion::LookRotation(const Vector3& avant, const Vector3& haut)
    {
        Vector3 f = avant.normalized();
        if (f.sqrMagnitude() < 1e-9f) return identity;
        Vector3 r = Vector3::Cross(haut, f).normalized();
        if (r.sqrMagnitude() < 1e-9f) r = Vector3::right;
        Vector3 u = Vector3::Cross(f, r);
        float m00 = r.x, m01 = u.x, m02 = f.x, m10 = r.y, m11 = u.y, m12 = f.y, m20 = r.z, m21 = u.z, m22 = f.z;
        float tr = m00 + m11 + m22;
        if (tr > 0)
        {
            float s = static_cast<float>(std::sqrt(static_cast<double>(tr + 1))) * 2;
            return {(m21 - m12) / s, (m02 - m20) / s, (m10 - m01) / s, 0.25f * s};
        }
        if (m00 > m11 && m00 > m22)
        {
            float s = static_cast<float>(std::sqrt(static_cast<double>(1 + m00 - m11 - m22))) * 2;
            return {0.25f * s, (m01 + m10) / s, (m02 + m20) / s, (m21 - m12) / s};
        }
        if (m11 > m22)
        {
            float s = static_cast<float>(std::sqrt(static_cast<double>(1 + m11 - m00 - m22))) * 2;
            return {(m01 + m10) / s, 0.25f * s, (m12 + m21) / s, (m02 - m20) / s};
        }
        float s = static_cast<float>(std::sqrt(static_cast<double>(1 + m22 - m00 - m11))) * 2;
        return {(m02 + m20) / s, (m12 + m21) / s, 0.25f * s, (m10 - m01) / s};
    }

    Quaternion Quaternion::Slerp(const Quaternion& a, Quaternion b, float t)
    {
        t = Mathf::Clamp01(t);
        float d = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
        if (d < 0)
        {
            b = Quaternion(-b.x, -b.y, -b.z, -b.w);
        }
        Quaternion q(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
        float m = static_cast<float>(std::sqrt(static_cast<double>(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w)));
        return {q.x / m, q.y / m, q.z / m, q.w / m};
    }

    float Quaternion::Angle(const Quaternion& a, const Quaternion& b)
    {
        float d = Mathf::Min(std::fabs(a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w), 1.0f);
        return static_cast<float>(std::acos(static_cast<double>(d)) * 2 * 180 / PiDouble);
    }

    Vector3 Quaternion::eulerAngles() const
    {
        // inverse de q = qy * qx * qz ; les produits sont en float, comme en C#, puis passent en double
        double sx = 2.0f * (w * x - y * z);
        sx = sx < -1 ? -1 : (sx > 1 ? 1 : sx);
        double ax = std::asin(sx);
        double ay = std::atan2(static_cast<double>(2.0f * (w * y + x * z)), static_cast<double>(1.0f - 2.0f * (x * x + y * y)));
        double az = std::atan2(static_cast<double>(2.0f * (w * z + x * y)), static_cast<double>(1.0f - 2.0f * (x * x + z * z)));
        auto deg = [](double r) {
            float d = std::fmod(static_cast<float>(r * 180 / PiDouble), 360.0f);
            return d < 0 ? d + 360.0f : d;
        };
        return {deg(ax), deg(ay), deg(az)};
    }

    // ------------------------------------------------------------------ matrices

    Matrix4x4 Matrix4x4::TRS(const Vector3& p, const Quaternion& q, const Vector3& s)
    {
        Matrix4x4 m;
        Vector3 cx = q * Vector3(s.x, 0, 0), cy = q * Vector3(0, s.y, 0), cz = q * Vector3(0, 0, s.z);
        m.m00 = cx.x; m.m10 = cx.y; m.m20 = cx.z;
        m.m01 = cy.x; m.m11 = cy.y; m.m21 = cy.z;
        m.m02 = cz.x; m.m12 = cz.y; m.m22 = cz.z;
        m.m03 = p.x; m.m13 = p.y; m.m23 = p.z; m.m33 = 1;
        return m;
    }

    Matrix4x4 Matrix4x4::operator*(const Matrix4x4& b) const
    {
        const float A[16] = {m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33};
        const float B[16] = {b.m00, b.m01, b.m02, b.m03, b.m10, b.m11, b.m12, b.m13, b.m20, b.m21, b.m22, b.m23, b.m30, b.m31, b.m32, b.m33};
        float R[16];
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
            {
                float s = 0;
                for (int k = 0; k < 4; k++) s += A[i * 4 + k] * B[k * 4 + j];
                R[i * 4 + j] = s;
            }
        Matrix4x4 r;
        r.m00 = R[0]; r.m01 = R[1]; r.m02 = R[2]; r.m03 = R[3];
        r.m10 = R[4]; r.m11 = R[5]; r.m12 = R[6]; r.m13 = R[7];
        r.m20 = R[8]; r.m21 = R[9]; r.m22 = R[10]; r.m23 = R[11];
        r.m30 = R[12]; r.m31 = R[13]; r.m32 = R[14]; r.m33 = R[15];
        return r;
    }

    // ------------------------------------------------------------------ couleurs

    namespace
    {
        uint8_t Octet(float f) { return static_cast<uint8_t>(std::nearbyint(static_cast<double>(Mathf::Clamp01(f) * 255))); }
    }

    Color32::Color32(const Color& c) : r(Octet(c.r)), g(Octet(c.g)), b(Octet(c.b)), a(Octet(c.a)) {}

    Color Color::HSVToRGB(float h, float s, float v)
    {
        h = std::fmod(std::fmod(h, 1.0f) + 1.0f, 1.0f);
        float i = static_cast<float>(std::floor(static_cast<double>(h * 6))), f = h * 6 - i, p = v * (1 - s), q = v * (1 - f * s),
              t = v * (1 - (1 - f) * s);
        switch (static_cast<int>(i) % 6)
        {
            case 0: return {v, t, p};
            case 1: return {q, v, p};
            case 2: return {p, v, t};
            case 3: return {p, q, v};
            case 4: return {t, p, v};
            default: return {v, p, q};
        }
    }

    void Color::RGBToHSV(const Color& c, float& h, float& s, float& v)
    {
        float mx = Mathf::Max(c.r, Mathf::Max(c.g, c.b)), mn = Mathf::Min(c.r, Mathf::Min(c.g, c.b)), d = mx - mn;
        v = mx;
        s = mx > 0 ? d / mx : 0;
        if (d <= 0) h = 0;
        else if (mx == c.r) h = ((c.g - c.b) / d) / 6.0f;
        else if (mx == c.g) h = (2 + (c.b - c.r) / d) / 6.0f;
        else h = (4 + (c.r - c.g) / d) / 6.0f;
        if (h < 0) h += 1;
    }

    // ------------------------------------------------------------------ hasard visuel

    namespace
    {
        AleaNet& Generateur()
        {
            static AleaNet g(static_cast<int>(std::chrono::steady_clock::now().time_since_epoch().count() & 0x7fffffff));
            return g;
        }
    }

    void Random::InitState(int graine) { Generateur() = AleaNet(graine); }
    float Random::value() { return static_cast<float>(Generateur().NextDouble()); }
    float Random::Range(float a, float b) { return a + static_cast<float>(Generateur().NextDouble()) * (b - a); }
    int Random::Range(int a, int b) { return b <= a ? a : Generateur().Next(a, b); }
    // Comme le moteur des essais de la version C# (et non la répartition uniforme d'Unity) : les mêmes tirages, dans le
    // même ordre, donnent les mêmes étincelles.
    Vector3 Random::insideUnitSphere()
    {
        float x = Range(-1.0f, 1.0f);
        float y = Range(-1.0f, 1.0f);
        float z = Range(-1.0f, 1.0f);
        return Vector3(x, y, z) * 0.577f;
    }
    Vector2 Random::insideUnitCircle()
    {
        float x = Range(-1.0f, 1.0f);
        float y = Range(-1.0f, 1.0f);
        return Vector2(x, y) * 0.7f;
    }
    Vector3 Random::onUnitSphere()
    {
        Vector3 v = insideUnitSphere();
        return v.sqrMagnitude() < 1e-6f ? Vector3::up : v.normalized();
    }
    Quaternion Random::rotation()
    {
        float x = Range(0.0f, 360.0f);
        float y = Range(0.0f, 360.0f);
        float z = Range(0.0f, 360.0f);
        return Quaternion::Euler(x, y, z);
    }
}
