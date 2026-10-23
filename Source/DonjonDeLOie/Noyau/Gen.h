// Donjon de l'Oie — les énumérateurs paresseux de C# (« yield return ») en C++20.
// Une fonction qui renvoie Gen<T> et contient co_yield ne s'exécute qu'au fil de l'énumération, exactement comme une
// méthode C# qui renvoie IEnumerable<T> : rien ne tourne à l'appel, chaque pas de la boucle avance jusqu'au co_yield
// suivant. On l'énumère avec une boucle « for (auto& s : F()) » ou avec MoveNext() / Current().
#pragma once

#include <coroutine>
#include <iterator>
#include <utility>

namespace Moteur
{
    template <class T>
    class Gen
    {
    public:
        struct promise_type
        {
            T Valeur{};

            Gen get_return_object() noexcept { return Gen(std::coroutine_handle<promise_type>::from_promise(*this)); }
            std::suspend_always initial_suspend() noexcept { return {}; }
            std::suspend_always final_suspend() noexcept { return {}; }
            std::suspend_always yield_value(T v) noexcept
            {
                Valeur = std::move(v);
                return {};
            }
            void return_void() noexcept {}
            void unhandled_exception() noexcept {}      // les exceptions sont désactivées (Unreal) : jamais appelé
        };
        using Poignee = std::coroutine_handle<promise_type>;

        Gen() noexcept = default;
        explicit Gen(Poignee p) noexcept : h(p) {}
        Gen(Gen&& o) noexcept : h(std::exchange(o.h, {})) {}
        Gen& operator=(Gen&& o) noexcept
        {
            if (this != &o)
            {
                Liberer();
                h = std::exchange(o.h, {});
            }
            return *this;
        }
        Gen(const Gen&) = delete;
        Gen& operator=(const Gen&) = delete;
        ~Gen() { Liberer(); }

        /// Avance jusqu'au co_yield suivant ; faux quand la fonction est terminée.
        bool MoveNext()
        {
            if (!h || h.done()) return false;
            h.resume();
            return !h.done();
        }
        T& Current() { return h.promise().Valeur; }
        /// Vrai si c'est bien une coroutine (un Gen construit par défaut n'en est pas une).
        bool Valide() const noexcept { return static_cast<bool>(h); }

        class iterator
        {
        public:
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            iterator() noexcept = default;
            explicit iterator(Gen* g) noexcept : gen(g) {}
            T& operator*() const { return gen->Current(); }
            iterator& operator++()
            {
                if (!gen->MoveNext()) gen = nullptr;
                return *this;
            }
            void operator++(int) { ++*this; }
            friend bool operator==(const iterator& i, std::default_sentinel_t) noexcept { return i.gen == nullptr; }
            friend bool operator!=(const iterator& i, std::default_sentinel_t s) noexcept { return !(i == s); }

        private:
            Gen* gen = nullptr;
        };

        iterator begin()
        {
            if (!MoveNext()) return iterator();
            return iterator(this);
        }
        std::default_sentinel_t end() noexcept { return {}; }

    private:
        void Liberer()
        {
            if (h)
            {
                h.destroy();
                h = {};
            }
        }
        Poignee h{};
    };
}
