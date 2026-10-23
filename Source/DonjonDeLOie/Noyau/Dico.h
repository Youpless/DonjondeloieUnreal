// Donjon de l'Oie — Dico<K, V> et Ensemble<T> : le Dictionary et le HashSet de C#, parcourus dans le même ordre que
// .NET (ordre d'ajout ; une place libérée par Remove est reprise par l'ajout suivant, la dernière libérée d'abord).
// Le jeu en dépend : repeindre un modèle voxel tire une couleur au hasard pour chaque voxel, dans l'ordre du parcours.
#pragma once

#include "Liste.h"

#include <unordered_map>

namespace Moteur
{
    /// Le hachage des clés : std::hash, ou la méthode Hache() d'une structure du jeu.
    template <class K, class = void>
    struct HacheurDico
    {
        size_t operator()(const K& k) const { return std::hash<K>{}(k); }
    };
    template <class K>
    struct HacheurDico<K, std::void_t<decltype(std::declval<const K&>().Hache())>>
    {
        size_t operator()(const K& k) const { return static_cast<size_t>(k.Hache()); }
    };

    template <class K, class V>
    class Dico
    {
    public:
        struct Entree
        {
            K Key{};
            V Value{};
        };

    private:
        struct Fente
        {
            Entree e;
            int suivantLibre = -1;
            bool utilise = false;
        };

    public:

        int Count() const { return static_cast<int>(entrees.size()) - nbLibres; }

        bool ContainsKey(const K& k) const { return index.find(k) != index.end(); }
        bool TryGetValue(const K& k, V& sortie) const
        {
            auto it = index.find(k);
            if (it == index.end())
            {
                sortie = V{};
                return false;
            }
            sortie = entrees[static_cast<size_t>(it->second)].e.Value;
            return true;
        }
        /// Un pointeur vers la valeur, ou nullptr (pour lire sans copier).
        V* Chercher(const K& k)
        {
            auto it = index.find(k);
            return it == index.end() ? nullptr : &entrees[static_cast<size_t>(it->second)].e.Value;
        }
        const V* Chercher(const K& k) const
        {
            auto it = index.find(k);
            return it == index.end() ? nullptr : &entrees[static_cast<size_t>(it->second)].e.Value;
        }

        /// L'indexeur de C# : en écriture, ajoute ou remplace ; en lecture, la clé doit exister (comme en C#).
        V& operator[](const K& k)
        {
            auto it = index.find(k);
            if (it != index.end()) return entrees[static_cast<size_t>(it->second)].e.Value;
            return entrees[static_cast<size_t>(Inserer(k, V{}))].e.Value;
        }
        const V& At(const K& k) const
        {
            auto it = index.find(k);
            DD_VERIFIE(it != index.end(), "Dico : clé absente");
            return entrees[static_cast<size_t>(it->second)].e.Value;
        }
        void Add(const K& k, V v)
        {
            DD_VERIFIE(!ContainsKey(k), "Dico.Add : la clé existe déjà");
            Inserer(k, std::move(v));
        }
        bool Remove(const K& k)
        {
            auto it = index.find(k);
            if (it == index.end()) return false;
            int i = it->second;
            index.erase(it);
            auto& s = entrees[static_cast<size_t>(i)];
            s.utilise = false;
            s.e = Entree{};
            s.suivantLibre = libre;
            libre = i;
            nbLibres++;
            return true;
        }
        void Clear()
        {
            entrees.clear();
            index.clear();
            libre = -1;
            nbLibres = 0;
        }

        /// Les clés et les valeurs, dans l'ordre du parcours.
        Liste<K> Keys() const
        {
            Liste<K> r;
            for (auto& s : entrees)
                if (s.utilise) r.Add(s.e.Key);
            return r;
        }
        Liste<V> Values() const
        {
            Liste<V> r;
            for (auto& s : entrees)
                if (s.utilise) r.Add(s.e.Value);
            return r;
        }

        template <bool Constant>
        class IterateurBase
        {
        public:
            using Fente = std::conditional_t<Constant, const typename Dico::Fente, typename Dico::Fente>;
            using Element = std::conditional_t<Constant, const Entree, Entree>;
            IterateurBase(Fente* p, Fente* f) : pos(p), fin(f) { Sauter(); }
            Element& operator*() const { return pos->e; }
            Element* operator->() const { return &pos->e; }
            IterateurBase& operator++()
            {
                ++pos;
                Sauter();
                return *this;
            }
            bool operator!=(const IterateurBase& o) const { return pos != o.pos; }
            bool operator==(const IterateurBase& o) const { return pos == o.pos; }

        private:
            void Sauter()
            {
                while (pos != fin && !pos->utilise) ++pos;
            }
            Fente* pos;
            Fente* fin;
        };
        using iterator = IterateurBase<false>;
        using const_iterator = IterateurBase<true>;

        iterator begin() { return iterator(entrees.data(), entrees.data() + entrees.size()); }
        iterator end() { return iterator(entrees.data() + entrees.size(), entrees.data() + entrees.size()); }
        const_iterator begin() const { return const_iterator(entrees.data(), entrees.data() + entrees.size()); }
        const_iterator end() const { return const_iterator(entrees.data() + entrees.size(), entrees.data() + entrees.size()); }

        /// Une copie des paires, dans l'ordre (« new List<KeyValuePair<K, V>>(dico) »).
        Liste<Entree> Paires() const
        {
            Liste<Entree> r;
            for (auto& s : entrees)
                if (s.utilise) r.Add(s.e);
            return r;
        }

    private:
        int Inserer(const K& k, V v)
        {
            int i;
            if (nbLibres > 0)
            {
                i = libre;
                libre = entrees[static_cast<size_t>(i)].suivantLibre;
                nbLibres--;
            }
            else
            {
                i = static_cast<int>(entrees.size());
                entrees.push_back(Fente{});
            }
            auto& s = entrees[static_cast<size_t>(i)];
            s.e.Key = k;
            s.e.Value = std::move(v);
            s.utilise = true;
            s.suivantLibre = -1;
            index[k] = i;
            return i;
        }

        std::vector<Fente> entrees;
        std::unordered_map<K, int, HacheurDico<K>> index;
        int libre = -1;
        int nbLibres = 0;
    };

    /// Le HashSet de C# (même ordre de parcours que .NET).
    template <class T>
    class Ensemble
    {
    public:
        Ensemble() = default;
        Ensemble(std::initializer_list<T> l)
        {
            for (auto& x : l) Add(x);
        }
        int Count() const { return d.Count(); }
        bool Contains(const T& x) const { return d.ContainsKey(x); }
        /// Vrai si l'élément a été ajouté (faux s'il y était déjà).
        bool Add(const T& x)
        {
            if (d.ContainsKey(x)) return false;
            d.Add(x, 0);
            return true;
        }
        bool Remove(const T& x) { return d.Remove(x); }
        void Clear() { d.Clear(); }
        Liste<T> Elements() const { return d.Keys(); }

        class iterator
        {
        public:
            explicit iterator(typename Dico<T, char>::const_iterator i) : it(i) {}
            const T& operator*() const { return (*it).Key; }
            iterator& operator++()
            {
                ++it;
                return *this;
            }
            bool operator!=(const iterator& o) const { return it != o.it; }

        private:
            typename Dico<T, char>::const_iterator it;
        };
        iterator begin() const { return iterator(d.begin()); }
        iterator end() const { return iterator(d.end()); }

    private:
        Dico<T, char> d;
    };
}
