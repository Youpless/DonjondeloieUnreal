// Donjon de l'Oie — Liste<T> : la List<T> de C#, avec les mêmes noms et le même comportement (Remove retire la première
// occurrence, Find renvoie la valeur par défaut, Sort trie avec l'algorithme de .NET : à égalité, l'ordre obtenu est
// exactement celui du jeu C#, donc les parties simulées donnent les mêmes résultats). Sert aussi pour les tableaux C#.
#pragma once

#include "Commun.h"

#include <algorithm>
#include <initializer_list>

namespace Moteur
{
    /// Comparaison à trois issues (CompareTo de C#) : -1, 0 ou 1.
    template <class T>
    inline int Comparer(const T& a, const T& b)
    {
        return a < b ? -1 : (b < a ? 1 : 0);
    }

    namespace TriNet
    {
        // Le tri introspectif de .NET (ArraySortHelper<T>.IntrospectiveSort), transcrit tel quel.
        template <class T, class C>
        inline void EchangerSiPlusGrand(T* k, C& cmp, int a, int b)
        {
            if (a != b && cmp(k[a], k[b]) > 0) std::swap(k[a], k[b]);
        }
        template <class T>
        inline void Echanger(T* k, int i, int j)
        {
            if (i != j) std::swap(k[i], k[j]);
        }
        template <class T, class C>
        void Insertion(T* k, int lo, int hi, C& cmp)
        {
            for (int i = lo; i < hi; i++)
            {
                int j = i;
                T t = k[i + 1];
                while (j >= lo && cmp(t, k[j]) < 0)
                {
                    k[j + 1] = k[j];
                    j--;
                }
                k[j + 1] = t;
            }
        }
        template <class T, class C>
        void Descendre(T* k, int i, int n, int lo, C& cmp)
        {
            T d = k[lo + i - 1];
            while (i <= n / 2)
            {
                int enfant = 2 * i;
                if (enfant < n && cmp(k[lo + enfant - 1], k[lo + enfant]) < 0) enfant++;
                if (!(cmp(d, k[lo + enfant - 1]) < 0)) break;
                k[lo + i - 1] = k[lo + enfant - 1];
                i = enfant;
            }
            k[lo + i - 1] = d;
        }
        template <class T, class C>
        void Tas(T* k, int lo, int hi, C& cmp)
        {
            int n = hi - lo + 1;
            for (int i = n / 2; i >= 1; i--) Descendre(k, i, n, lo, cmp);
            for (int i = n; i > 1; i--)
            {
                Echanger(k, lo, lo + i - 1);
                Descendre(k, 1, i - 1, lo, cmp);
            }
        }
        template <class T, class C>
        int Partition(T* k, int lo, int hi, C& cmp)
        {
            int milieu = lo + ((hi - lo) >> 1);
            EchangerSiPlusGrand(k, cmp, lo, milieu);
            EchangerSiPlusGrand(k, cmp, lo, hi);
            EchangerSiPlusGrand(k, cmp, milieu, hi);
            T pivot = k[milieu];
            Echanger(k, milieu, hi - 1);
            int gauche = lo, droite = hi - 1;
            while (gauche < droite)
            {
                while (cmp(k[++gauche], pivot) < 0) {}
                while (cmp(pivot, k[--droite]) < 0) {}
                if (gauche >= droite) break;
                Echanger(k, gauche, droite);
            }
            Echanger(k, gauche, hi - 1);
            return gauche;
        }
        template <class T, class C>
        void Intro(T* k, int lo, int hi, int profondeur, C& cmp)
        {
            while (hi > lo)
            {
                int taille = hi - lo + 1;
                if (taille <= 16)
                {
                    if (taille == 1) return;
                    if (taille == 2)
                    {
                        EchangerSiPlusGrand(k, cmp, lo, hi);
                        return;
                    }
                    if (taille == 3)
                    {
                        EchangerSiPlusGrand(k, cmp, lo, hi - 1);
                        EchangerSiPlusGrand(k, cmp, lo, hi);
                        EchangerSiPlusGrand(k, cmp, hi - 1, hi);
                        return;
                    }
                    Insertion(k, lo, hi, cmp);
                    return;
                }
                if (profondeur == 0)
                {
                    Tas(k, lo, hi, cmp);
                    return;
                }
                profondeur--;
                int p = Partition(k, lo, hi, cmp);
                Intro(k, p + 1, hi, profondeur, cmp);
                hi = p - 1;
            }
        }
        inline int Log2Plancher(int n)
        {
            int r = 0;
            while (n >= 1)
            {
                r++;
                n /= 2;
            }
            return r;
        }
        template <class T, class C>
        void Trier(T* k, int index, int longueur, int capacite, C cmp)
        {
            if (longueur < 2) return;
            Intro(k, index, longueur + index - 1, 2 * Log2Plancher(capacite), cmp);
        }
    }

    template <class T>
    class Liste
    {
    public:
        Liste() = default;
        /// Un tableau C# « new T[n] » : n valeurs par défaut.
        explicit Liste(int n) : v(static_cast<size_t>(n < 0 ? 0 : n)) {}
        Liste(int n, const T& valeur) : v(static_cast<size_t>(n < 0 ? 0 : n), valeur) {}
        Liste(std::initializer_list<T> l) : v(l) {}
        Liste(std::vector<T> o) : v(std::move(o)) {}

        int Count() const { return static_cast<int>(v.size()); }
        int Length() const { return static_cast<int>(v.size()); }
        bool Vide() const { return v.empty(); }

        T& operator[](int i)
        {
            DD_VERIFIE(i >= 0 && i < Count(), "Liste : indice hors limites");
            return v[static_cast<size_t>(i)];
        }
        const T& operator[](int i) const
        {
            DD_VERIFIE(i >= 0 && i < Count(), "Liste : indice hors limites");
            return v[static_cast<size_t>(i)];
        }

        void Add(const T& x) { v.push_back(x); }
        void Add(T&& x) { v.push_back(std::move(x)); }
        void AddRange(const Liste& o)
        {
            std::vector<T> copie = o.v;     // AddRange(soi-même) est permis en C#
            v.insert(v.end(), copie.begin(), copie.end());
        }
        void Insert(int i, T x)
        {
            DD_VERIFIE(i >= 0 && i <= Count(), "Liste.Insert : indice hors limites");
            v.insert(v.begin() + i, std::move(x));
        }
        bool Remove(const T& x)
        {
            auto it = std::find(v.begin(), v.end(), x);
            if (it == v.end()) return false;
            v.erase(it);
            return true;
        }
        void RemoveAt(int i)
        {
            DD_VERIFIE(i >= 0 && i < Count(), "Liste.RemoveAt : indice hors limites");
            v.erase(v.begin() + i);
        }
        void RemoveRange(int i, int n)
        {
            DD_VERIFIE(i >= 0 && n >= 0 && i + n <= Count(), "Liste.RemoveRange : hors limites");
            v.erase(v.begin() + i, v.begin() + i + n);
        }
        template <class P>
        int RemoveAll(P p)
        {
            // comme .NET : l'ordre des restants est gardé
            auto it = std::remove_if(v.begin(), v.end(), [&](const T& x) { return static_cast<bool>(p(x)); });
            int n = static_cast<int>(v.end() - it);
            v.erase(it, v.end());
            return n;
        }
        void Clear() { v.clear(); }

        bool Contains(const T& x) const { return std::find(v.begin(), v.end(), x) != v.end(); }
        int IndexOf(const T& x) const
        {
            auto it = std::find(v.begin(), v.end(), x);
            return it == v.end() ? -1 : static_cast<int>(it - v.begin());
        }
        int LastIndexOf(const T& x) const
        {
            for (int i = Count() - 1; i >= 0; i--)
                if (v[static_cast<size_t>(i)] == x) return i;
            return -1;
        }
        template <class P>
        T Find(P p) const
        {
            for (const auto& x : v)
                if (p(x)) return x;
            return T{};
        }
        template <class P>
        T FindLast(P p) const
        {
            for (int i = Count() - 1; i >= 0; i--)
                if (p(v[static_cast<size_t>(i)])) return v[static_cast<size_t>(i)];
            return T{};
        }
        template <class P>
        int FindIndex(P p) const
        {
            for (int i = 0; i < Count(); i++)
                if (p(v[static_cast<size_t>(i)])) return i;
            return -1;
        }
        template <class P>
        Liste FindAll(P p) const
        {
            Liste r;
            for (const auto& x : v)
                if (p(x)) r.v.push_back(x);
            return r;
        }
        template <class P>
        bool Exists(P p) const
        {
            for (const auto& x : v)
                if (p(x)) return true;
            return false;
        }
        template <class P>
        bool TrueForAll(P p) const
        {
            for (const auto& x : v)
                if (!p(x)) return false;
            return true;
        }
        template <class F>
        void ForEach(F f) const
        {
            for (const auto& x : v) f(x);
        }
        template <class U, class F>
        Liste<U> ConvertAll(F f) const
        {
            Liste<U> r;
            for (const auto& x : v) r.Add(f(x));
            return r;
        }

        void Reverse() { std::reverse(v.begin(), v.end()); }
        Liste GetRange(int i, int n) const
        {
            DD_VERIFIE(i >= 0 && n >= 0 && i + n <= Count(), "Liste.GetRange : hors limites");
            return Liste(std::vector<T>(v.begin() + i, v.begin() + i + n));
        }
        /// Une copie (ToArray, ou « new List<T>(liste) »).
        Liste ToArray() const { return *this; }

        /// Tri par la comparaison par défaut (comme List.Sort()).
        void Sort()
        {
            TriNet::Trier(v.data(), 0, Count(), Count(), [](const T& a, const T& b) { return Comparer(a, b); });
        }
        /// Tri par une comparaison qui renvoie un int (négatif, 0, positif), comme List.Sort(Comparison).
        template <class C>
        void Sort(C comparaison)
        {
            TriNet::Trier(v.data(), 0, Count(), Count(), comparaison);
        }

        T& First() { return (*this)[0]; }
        T& Last() { return (*this)[Count() - 1]; }
        const T& First() const { return (*this)[0]; }
        const T& Last() const { return (*this)[Count() - 1]; }

        typename std::vector<T>::iterator begin() { return v.begin(); }
        typename std::vector<T>::iterator end() { return v.end(); }
        typename std::vector<T>::const_iterator begin() const { return v.begin(); }
        typename std::vector<T>::const_iterator end() const { return v.end(); }

        std::vector<T>& Vecteur() { return v; }
        const std::vector<T>& Vecteur() const { return v; }
        T* Donnees() { return v.data(); }
        const T* Donnees() const { return v.data(); }

        bool operator==(const Liste& o) const { return v == o.v; }

    private:
        std::vector<T> v;
    };

    /// Les tableaux de booléens (std::vector<bool> ne donne pas de bool&) : un octet par valeur.
    template <>
    class Liste<bool>
    {
    public:
        Liste() = default;
        explicit Liste(int n) : v(static_cast<size_t>(n < 0 ? 0 : n), 0) {}
        Liste(int n, bool valeur) : v(static_cast<size_t>(n < 0 ? 0 : n), valeur ? 1 : 0) {}
        int Count() const { return static_cast<int>(v.size()); }
        int Length() const { return static_cast<int>(v.size()); }
        bool& operator[](int i)
        {
            DD_VERIFIE(i >= 0 && i < Count(), "Liste<bool> : indice hors limites");
            return v[static_cast<size_t>(i)].b;
        }
        bool operator[](int i) const
        {
            DD_VERIFIE(i >= 0 && i < Count(), "Liste<bool> : indice hors limites");
            return v[static_cast<size_t>(i)].b;
        }
        void Add(bool x) { v.push_back(Case{x}); }
        void Clear() { v.clear(); }
        bool Contains(bool x) const
        {
            for (auto& c : v)
                if (c.b == x) return true;
            return false;
        }
        int IndexOf(bool x) const
        {
            for (int i = 0; i < Count(); i++)
                if (v[static_cast<size_t>(i)].b == x) return i;
            return -1;
        }

    private:
        struct Case
        {
            bool b = false;
            Case() = default;
            Case(bool x) : b(x) {}
            Case(int x) : b(x != 0) {}
        };
        std::vector<Case> v;
    };
}
