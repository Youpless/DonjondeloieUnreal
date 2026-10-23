// Donjon de l'Oie — les outils de texte (UTF-8).
#include "Texte.h"

#include <cstdio>
#include <cstdlib>

namespace Moteur
{
    namespace Texte
    {
        namespace
        {
            /// Lit un caractère UTF-8 à partir de « i » (avance i) ; un octet invalide est rendu tel quel.
            uint32_t Lire(const string& s, size_t& i)
            {
                unsigned char c = static_cast<unsigned char>(s[i]);
                if (c < 0x80)
                {
                    i++;
                    return c;
                }
                int n = (c >= 0xF0) ? 3 : (c >= 0xE0) ? 2 : (c >= 0xC0) ? 1 : 0;
                if (n == 0 || i + static_cast<size_t>(n) >= s.size())
                {
                    i++;            // octet invalide ou séquence tronquée : rendu tel quel
                    return c;
                }
                uint32_t cp = c & (0x3Fu >> n);
                for (int k = 1; k <= n; k++) cp = (cp << 6) | (static_cast<unsigned char>(s[i + static_cast<size_t>(k)]) & 0x3Fu);
                i += static_cast<size_t>(n) + 1;
                return cp;
            }

            void Ecrire(string& o, uint32_t cp)
            {
                if (cp < 0x80) o += static_cast<char>(cp);
                else if (cp < 0x800)
                {
                    o += static_cast<char>(0xC0 | (cp >> 6));
                    o += static_cast<char>(0x80 | (cp & 0x3F));
                }
                else if (cp < 0x10000)
                {
                    o += static_cast<char>(0xE0 | (cp >> 12));
                    o += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                    o += static_cast<char>(0x80 | (cp & 0x3F));
                }
                else
                {
                    o += static_cast<char>(0xF0 | (cp >> 18));
                    o += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
                    o += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                    o += static_cast<char>(0x80 | (cp & 0x3F));
                }
            }

            uint32_t VersMinuscule(uint32_t c)
            {
                if (c >= 'A' && c <= 'Z') return c + 32;
                if (c >= 0xC0 && c <= 0xDE && c != 0xD7) return c + 0x20;
                if (c == 0x152) return 0x153;
                if (c == 0x178) return 0xFF;
                if ((c >= 0x100 && c <= 0x137) || (c >= 0x14A && c <= 0x177)) return (c % 2 == 0) ? c + 1 : c;
                if ((c >= 0x139 && c <= 0x148) || (c >= 0x179 && c <= 0x17E)) return (c % 2 == 1) ? c + 1 : c;
                return c;
            }

            uint32_t VersMajuscule(uint32_t c)
            {
                if (c >= 'a' && c <= 'z') return c - 32;
                if (c >= 0xE0 && c <= 0xFE && c != 0xF7) return c - 0x20;
                if (c == 0xFF) return 0x178;
                if (c == 0x153) return 0x152;
                if ((c >= 0x100 && c <= 0x137) || (c >= 0x14A && c <= 0x177)) return (c % 2 == 1) ? c - 1 : c;
                if ((c >= 0x139 && c <= 0x148) || (c >= 0x179 && c <= 0x17E)) return (c % 2 == 0) ? c - 1 : c;
                return c;
            }

            template <class F>
            string Transformer(const string& s, F f)
            {
                string o;
                o.reserve(s.size());
                size_t i = 0;
                while (i < s.size()) Ecrire(o, f(Lire(s, i)));
                return o;
            }

            bool Blanc(uint32_t c)
            {
                return c == ' ' || (c >= 0x09 && c <= 0x0D) || c == 0x85 || c == 0xA0 || c == 0x1680 || (c >= 0x2000 && c <= 0x200A) ||
                       c == 0x2028 || c == 0x2029 || c == 0x202F || c == 0x205F || c == 0x3000;
            }
        }

        string De(int n) { return std::to_string(n); }
        string De(long long n) { return std::to_string(n); }

        string De(double d)
        {
            char b[64];
            std::snprintf(b, sizeof b, "%.15g", d);
            string s = b;
            for (auto& c : s)
                if (c == '.') c = ',';
            return s;
        }

        string De(float f)
        {
            char b[64];
            std::snprintf(b, sizeof b, "%.7g", static_cast<double>(f));
            string s = b;
            for (auto& c : s)
                if (c == '.') c = ',';
            return s;
        }

        string Hex2(int octet)
        {
            const char* h = "0123456789abcdef";
            string s;
            s += h[(octet >> 4) & 15];
            s += h[octet & 15];
            return s;
        }

        string Deux(int n)
        {
            if (n >= 0 && n < 10) return "0" + std::to_string(n);
            return std::to_string(n);
        }

        bool Vide(const string& s) { return s.empty(); }
        bool Contient(const string& s, const string& morceau) { return s.find(morceau) != string::npos; }
        bool CommencePar(const string& s, const string& debut) { return s.size() >= debut.size() && s.compare(0, debut.size(), debut) == 0; }
        bool FinitPar(const string& s, const string& fin) { return s.size() >= fin.size() && s.compare(s.size() - fin.size(), fin.size(), fin) == 0; }

        int Position(const string& s, const string& morceau)
        {
            size_t p = s.find(morceau);
            return p == string::npos ? -1 : static_cast<int>(p);
        }

        string Sous(const string& s, int debut, int longueur)
        {
            if (debut < 0) debut = 0;
            if (static_cast<size_t>(debut) >= s.size()) return string();
            if (longueur < 0) return s.substr(static_cast<size_t>(debut));
            return s.substr(static_cast<size_t>(debut), static_cast<size_t>(longueur));
        }

        string Minuscules(const string& s) { return Transformer(s, VersMinuscule); }
        string Majuscules(const string& s) { return Transformer(s, VersMajuscule); }
        bool EgauxSansCasse(const string& a, const string& b) { return Majuscules(a) == Majuscules(b); }

        string Remplacer(const string& s, const string& quoi, const string& par)
        {
            if (quoi.empty()) return s;
            string o;
            size_t i = 0;
            for (;;)
            {
                size_t p = s.find(quoi, i);
                if (p == string::npos)
                {
                    o.append(s, i, string::npos);
                    return o;
                }
                o.append(s, i, p - i);
                o += par;
                i = p + quoi.size();
            }
        }

        string Rogner(const string& s)
        {
            // les blancs ASCII et Unicode des deux bouts
            size_t i = 0, debut = s.size();
            while (i < s.size())
            {
                size_t avant = i;
                if (!Blanc(Lire(s, i)))
                {
                    debut = avant;
                    break;
                }
            }
            if (debut >= s.size()) return string();
            return RognerFin(s.substr(debut));
        }

        string RognerFin(const string& s)
        {
            size_t fin = 0, i = 0;
            while (i < s.size())
            {
                uint32_t c = Lire(s, i);
                if (!Blanc(c)) fin = i;
            }
            return s.substr(0, fin);
        }

        Liste<string> Decouper(const string& s, char separateur)
        {
            Liste<string> l;
            size_t debut = 0;
            for (;;)
            {
                size_t p = s.find(separateur, debut);
                if (p == string::npos)
                {
                    l.Add(s.substr(debut));
                    return l;
                }
                l.Add(s.substr(debut, p - debut));
                debut = p + 1;
            }
        }

        string Joindre(const string& separateur, const Liste<string>& morceaux)
        {
            string o;
            for (int i = 0; i < morceaux.Count(); i++)
            {
                if (i > 0) o += separateur;
                o += morceaux[i];
            }
            return o;
        }

        int Longueur(const string& s)
        {
            int n = 0;
            size_t i = 0;
            while (i < s.size())
            {
                uint32_t c = Lire(s, i);
                n += c >= 0x10000 ? 2 : 1;      // comme C# : deux unités UTF-16 au-delà du plan de base
            }
            return n;
        }

        string Debut(const string& s, int unites)
        {
            int n = 0;
            size_t i = 0;
            while (i < s.size())
            {
                size_t avant = i;
                uint32_t c = Lire(s, i);
                n += c >= 0x10000 ? 2 : 1;
                if (n > unites) return s.substr(0, avant);
            }
            return s;
        }

        bool LireEntier(const string& s, int& valeur)
        {
            string t = Rogner(s);
            if (t.empty()) return false;
            size_t i = 0;
            bool negatif = false;
            if (t[0] == '-' || t[0] == '+')
            {
                negatif = t[0] == '-';
                i = 1;
            }
            if (i >= t.size()) return false;
            long long v = 0;
            for (; i < t.size(); i++)
            {
                if (t[i] < '0' || t[i] > '9') return false;
                v = v * 10 + (t[i] - '0');
                if (v > 2147483648LL) return false;
            }
            if (negatif) v = -v;
            if (v > 2147483647LL || v < -2147483648LL) return false;
            valeur = static_cast<int>(v);
            return true;
        }
    }
}
