// Donjon de l'Oie — le petit moteur sur lequel tourne le jeu : objets, hiérarchie, composants, cycle de vie, temps.
// Il reprend les noms et le comportement de la partie d'Unity dont le jeu se sert (GameObject, Transform,
// MonoBehaviour…), si bien que le code du jeu se lit comme sa version Unity. Unreal n'en voit que le résultat : la
// couche Unreal/ fait avancer le moteur à chaque image et affiche ce qu'il contient (maillages, lampes, caméra,
// interface, sons).
//
// Les objets du moteur se tiennent par Ref<T> : une référence partagée qui, comme dans Unity, vaut null une fois
// l'objet détruit (« if (go == nullptr) »), même si quelqu'un la garde encore.
#pragma once

#include "../Noyau/Jeu.h"
#include "Maths.h"

#include <type_traits>

namespace Moteur
{
    class Object;
    class GameObject;
    class Component;
    class Transform;
    class RectTransform;
    class MonoBehaviour;
    class Coroutine;
    class IPointerEnterHandler;
    class IPointerExitHandler;
    class IPointerClickHandler;
    class IPointerDownHandler;
    class Instruction;
    /// Une coroutine : une fonction qui contient co_yield (l'« IEnumerator » des coroutines d'Unity). Voir Coroutines.h.
    using Tache = Gen<Instruction>;

    // ------------------------------------------------------------------ types (GetComponent<T> sans RTTI)

    /// Le type d'un objet du moteur : son nom et celui de sa classe mère. Chaque classe le déclare avec DD_TYPE_OBJET.
    struct TypeObjet
    {
        const char* Nom;
        const TypeObjet* Parent;
        bool Herite(const TypeObjet* t) const
        {
            for (const TypeObjet* p = this; p != nullptr; p = p->Parent)
                if (p == t) return true;
            return false;
        }
    };

    /// À mettre en tête de chaque classe dérivée d'Object (composants du jeu compris) : DD_TYPE_OBJET(PionVue, MonoBehaviour)
#define DD_TYPE_OBJET(Classe, Mere)                                                                                     \
public:                                                                                                                 \
    static const ::Moteur::TypeObjet* TypeStatique()                                                                    \
    {                                                                                                                   \
        static const ::Moteur::TypeObjet typeStatique_{#Classe, Mere::TypeStatique()};                                  \
        return &typeStatique_;                                                                                          \
    }                                                                                                                   \
    const ::Moteur::TypeObjet* TypeDynamique() const override { return TypeStatique(); }                                \
                                                                                                                        \
private:

    /// typeof(T)
    template <class T>
    inline const TypeObjet* TypeDe()
    {
        return T::TypeStatique();
    }

    [[noreturn]] void ErreurObjetNul(const char* type);

    // ------------------------------------------------------------------ Ref<T>

    /// Une référence vers un objet du moteur. Comme dans Unity, un objet détruit est « null » : « r == nullptr » et
    /// « !r » sont vrais dès que Destroy l'a effacé. Deux références vers des objets bien réels se comparent par
    /// identité (comme les listes et dictionnaires de C#).
    template <class T>
    class Ref
    {
    public:
        Ref() = default;
        Ref(std::nullptr_t) {}
        explicit Ref(std::shared_ptr<T> p) : p_(std::move(p)) {}
        template <class U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
        Ref(const Ref<U>& o) : p_(o.Partage())
        {
        }

        T* operator->() const
        {
            if (p_ == nullptr) ErreurObjetNul(NomType());
            return p_.get();
        }
        T& operator*() const
        {
            if (p_ == nullptr) ErreurObjetNul(NomType());
            return *p_;
        }
        /// Vrai si l'objet existe et n'est pas détruit.
        explicit operator bool() const;
        T* get() const { return p_.get(); }
        const std::shared_ptr<T>& Partage() const { return p_; }
        /// Pour les dictionnaires : le hachage de l'adresse.
        size_t Hache() const { return std::hash<const void*>{}(static_cast<const void*>(p_.get())); }

    private:
        static const char* NomType();
        std::shared_ptr<T> p_;
    };

    template <class T>
    bool operator==(const Ref<T>& a, std::nullptr_t)
    {
        return !a;
    }
    template <class T, class U>
    bool operator==(const Ref<T>& a, const Ref<U>& b)
    {
        if (a.get() == nullptr || b.get() == nullptr) return !a && !b;      // comparé à null : un objet détruit l'est
        return static_cast<const void*>(static_cast<const Object*>(a.get())) == static_cast<const void*>(static_cast<const Object*>(b.get()));
    }

    /// « x as T » : la même référence vue comme un T, ou null si l'objet n'en est pas un.
    template <class T, class U>
    Ref<T> Comme(const Ref<U>& r);
    /// « (T)x » : la même référence vue comme un T ; arrêt net si l'objet n'en est pas un.
    template <class T, class U>
    Ref<T> Vers(const Ref<U>& r);
    /// « x is T »
    template <class T, class U>
    bool Est(const Ref<U>& r);

    // ------------------------------------------------------------------ énumérations

    enum class FindObjectsInactive { Exclude, Include };
    enum class FindObjectsSortMode { None, InstanceID };
    enum class HideFlags { None = 0, HideAndDontSave = 61, DontSave = 52 };
    enum class Space { World, Self };
    enum class PrimitiveType { Cube, Sphere, Quad, Plane };
    enum class SendMessageOptions { RequireReceiver, DontRequireReceiver };

    // ------------------------------------------------------------------ Object

    class Object : public std::enable_shared_from_this<Object>
    {
    public:
        static const TypeObjet* TypeStatique()
        {
            static const TypeObjet t{"Object", nullptr};
            return &t;
        }
        virtual const TypeObjet* TypeDynamique() const { return TypeStatique(); }
        bool EstUn(const TypeObjet* t) const { return TypeDynamique()->Herite(t); }

        Object();
        virtual ~Object();
        Object(const Object&) = delete;
        Object& operator=(const Object&) = delete;

        virtual string name() const { return nomObjet_; }
        virtual void name(const string& n) { nomObjet_ = n; }
        HideFlags hideFlags = HideFlags::None;
        int GetInstanceID() const { return idInstance_; }

        /// Détruit l'objet à la fin de l'image (un GameObject emporte ses composants et ses enfants).
        static void Destroy(const Ref<Object>& o);
        static void Destroy(const Ref<Object>& o, float delai);
        /// Détruit l'objet tout de suite.
        static void DestroyImmediate(const Ref<Object>& o);
        static void DontDestroyOnLoad(const Ref<Object>&) {}
        template <class T>
        static Liste<Ref<T>> FindObjectsByType(FindObjectsInactive i = FindObjectsInactive::Exclude, FindObjectsSortMode s = FindObjectsSortMode::None);
        template <class T>
        static Ref<T> FindAnyObjectByType();
        template <class T>
        static Ref<T> FindObjectOfType() { return FindAnyObjectByType<T>(); }
        template <class T>
        static Ref<T> Instantiate(const Ref<T>& o) { return o; }

        // --- usage interne du moteur
        bool EstDetruit() const { return detruit_; }
        /// Appelé juste après la construction, quand l'objet est déjà tenu par un pointeur partagé.
        virtual void ApresCreation() {}
        /// La référence de cet objet (il doit être tenu par une Ref).
        template <class T>
        Ref<T> Soi()
        {
            return Ref<T>(std::static_pointer_cast<T>(shared_from_this()));
        }

    protected:
        bool detruit_ = false;
        string nomObjet_;
        int idInstance_;
        friend struct Boucle;
    };

    template <class T>
    Ref<T>::operator bool() const
    {
        return p_ != nullptr && !static_cast<const Object*>(p_.get())->EstDetruit();
    }
    template <class T>
    const char* Ref<T>::NomType()
    {
        if constexpr (std::is_base_of_v<Object, T>) return T::TypeStatique()->Nom;
        else return "objet";
    }

    /// « new X(…) » pour un objet du moteur (GameObject, Mesh, Material, Texture2D…) : l'objet est créé, inscrit dans la
    /// scène, et rendu par une Ref. (Les autres classes du jeu se créent avec Nouveau<T>, qui rend un Ptr<T>.)
    template <class T, class... A>
    Ref<T> Neuf(A&&... a);

    // ------------------------------------------------------------------ Component, Behaviour

    class Component : public Object
    {
        DD_TYPE_OBJET(Component, Object)
    public:
        Ref<GameObject> gameObject() const { return Ref<GameObject>(go_.lock()); }
        Ref<Transform> transform() const;
        string name() const override;
        void name(const string& n) override;
        string tag() const;
        void tag(const string& t);
        template <class T>
        Ref<T> GetComponent() const;
        template <class T>
        Ref<T> GetComponentInChildren() const;
        template <class T>
        Liste<Ref<T>> GetComponentsInChildren(bool inactifs = false) const;
        template <class T>
        Ref<T> GetComponentInParent() const;
        template <class T>
        bool TryGetComponent(Ref<T>& c) const
        {
            c = GetComponent<T>();
            return c != nullptr;
        }

        // Les gestionnaires d'événements de pointeur (les interfaces IPointer…Handler de C#) : un composant qui en
        // implémente une la rend ici.
        virtual IPointerEnterHandler* GestionnaireEntree() { return nullptr; }
        virtual IPointerExitHandler* GestionnaireSortie() { return nullptr; }
        virtual IPointerClickHandler* GestionnaireClic() { return nullptr; }
        virtual IPointerDownHandler* GestionnaireAppui() { return nullptr; }

    protected:
        std::weak_ptr<GameObject> go_;      // (faible : c'est le GameObject qui tient ses composants)
        friend class GameObject;
        friend struct Boucle;
    };

    class Behaviour : public Component
    {
        DD_TYPE_OBJET(Behaviour, Component)
    public:
        bool enabled = true;
        bool isActiveAndEnabled() const;
    };

    // ------------------------------------------------------------------ GameObject

    class GameObject : public Object
    {
        DD_TYPE_OBJET(GameObject, Object)
    public:
        GameObject() : GameObject("GameObject") {}
        explicit GameObject(const string& n) { nomObjet_ = n; }
        /// new GameObject(nom, typeof(RectTransform)) : seul RectTransform est pris en charge ici.
        GameObject(const string& n, const TypeObjet* type);

        Ref<Transform> transform() const { return transf; }
        string tag = "Untagged";
        bool isStatic = false;
        int layer = 0;

        template <class T>
        Ref<T> AddComponent();
        template <class T>
        Ref<T> GetComponent() const;
        template <class T>
        Ref<T> GetComponentInChildren() const;
        template <class T>
        Liste<Ref<T>> GetComponentsInChildren(bool inactifs = false) const;

        void SetActive(bool v) { actif = v; }
        bool activeSelf() const { return actif; }
        bool activeInHierarchy() const;
        static Ref<GameObject> CreatePrimitive(PrimitiveType t);
        static Ref<GameObject> Find(const string& n);

        // --- usage interne
        const Liste<Ref<Component>>& Composants() const { return composants; }
        void ApresCreation() override;

    private:
        void Lier(const Ref<Component>& c);
        Liste<Ref<Component>> composants;
        Ref<Transform> transf;
        bool actif = true;
        bool avecRect = false;
        friend struct Boucle;
    };

    // ------------------------------------------------------------------ Transform, RectTransform

    class Transform : public Component
    {
        DD_TYPE_OBJET(Transform, Component)
    public:
        Vector3 localPosition;
        Quaternion localRotation = Quaternion::identity;
        Vector3 localScale = Vector3::one;

        Ref<Transform> parent() const { return Ref<Transform>(parentT.lock()); }
        void parent(const Ref<Transform>& p) { SetParent(p, true); }
        bool IsChildOf(const Ref<Transform>& p) const;
        Ref<Transform> root() const;
        void SetParent(const Ref<Transform>& p) { SetParent(p, true); }
        void SetParent(const Ref<Transform>& p, bool garderMonde);
        int childCount() const { return enfants.Count(); }
        int GetSiblingIndex() const;
        void SetSiblingIndex(int i);
        Ref<Transform> GetChild(int i) const { return enfants[i]; }
        /// « foreach (Transform e in t) » : une copie de la liste des enfants.
        Liste<Ref<Transform>> Enfants() const { return enfants; }
        void SetAsFirstSibling();
        void SetAsLastSibling();
        /// L'enfant direct de ce nom, ou null.
        Ref<Transform> Find(const string& n) const;

        Vector3 TransformPoint(Vector3 v) const;
        Vector3 InverseTransformPoint(Vector3 w) const;
        Vector3 TransformDirection(Vector3 d) const { return rotation() * d; }
        Vector3 InverseTransformDirection(Vector3 d) const { return Quaternion::Inverse(rotation()) * d; }

        Vector3 position() const;
        void position(Vector3 v);
        Quaternion rotation() const;
        void rotation(Quaternion q);
        Vector3 lossyScale() const;
        Vector3 eulerAngles() const { return rotation().eulerAngles(); }
        void eulerAngles(Vector3 e) { rotation(Quaternion::Euler(e)); }
        Vector3 localEulerAngles() const { return localRotation.eulerAngles(); }
        void localEulerAngles(Vector3 e) { localRotation = Quaternion::Euler(e); }
        Vector3 right() const { return rotation() * Vector3::right; }
        Vector3 up() const { return rotation() * Vector3::up; }
        Vector3 forward() const { return rotation() * Vector3::forward; }
        void forward(Vector3 v) { rotation(Quaternion::LookRotation(v)); }
        void Rotate(float x, float y, float z, Space s = Space::Self);
        void Rotate(Vector3 e, Space s = Space::Self) { Rotate(e.x, e.y, e.z, s); }
        void LookAt(Vector3 p);
        void LookAt(const Ref<Transform>& t) { LookAt(t->position()); }
        void Translate(Vector3 d) { position(position() + d); }
        Matrix4x4 localToWorldMatrix() const;

    protected:
        void CopierDe(Transform& t);
        std::weak_ptr<Transform> parentT;  // (faible : c'est le parent qui tient ses enfants)
        Liste<Ref<Transform>> enfants;
        friend class GameObject;
        friend struct Boucle;
    };

    class RectTransform : public Transform
    {
        DD_TYPE_OBJET(RectTransform, Transform)
    public:
        enum class Axis { Horizontal, Vertical };
        Vector2 anchorMin{0.5f, 0.5f}, anchorMax{0.5f, 0.5f}, pivot{0.5f, 0.5f}, anchoredPosition, sizeDelta;
        Vector2 offsetMin() const { return anchoredPosition - Vector2::Scale(sizeDelta, pivot); }
        void offsetMin(Vector2 v);
        Vector2 offsetMax() const { return anchoredPosition + Vector2::Scale(sizeDelta, Vector2(1.0f - pivot.x, 1.0f - pivot.y)); }
        void offsetMax(Vector2 v);
        /// Le rectangle dans le repère de l'élément (origine au pivot), d'après la taille calculée par la mise en page.
        Rect rect() const;
        void SetSizeWithCurrentAnchors(Axis a, float s);

        // --- usage interne (mise en page de l'interface, voir Dessin.h)
        Rect RectCalcule;              // dernier rectangle calculé, dans le repère local (origine au pivot)
        Vector2 OrigineEcran;          // le pivot en pixels d'écran (origine en bas à gauche)
        Vector2 EchelleEcran{1, 1};    // pixels d'écran par unité locale (échelles des parents comprises)
        int ImageMiseEnPage = -1;      // l'image où la mise en page l'a placé (-1 : jamais, ou caché)
    };

    // ------------------------------------------------------------------ MonoBehaviour

    class MonoBehaviour : public Behaviour
    {
        DD_TYPE_OBJET(MonoBehaviour, Behaviour)
    public:
        Ptr<Coroutine> StartCoroutine(Tache t);
        void StopCoroutine(const Ptr<Coroutine>& c);
        void StopAllCoroutines();
        bool useGUILayout = false;
        static void print(const string& s);

        // Le cycle de vie : le moteur appelle ces méthodes comme Unity (et comme le moteur des essais C#).
        virtual void Awake() {}
        virtual void OnEnable() {}
        virtual void Start() {}
        /// Pour un « IEnumerator Start() » de C# : le moteur lance la coroutine rendue (après Start()).
        virtual Tache StartCoroutineDeDemarrage();
        virtual void Update() {}
        virtual void LateUpdate() {}
        virtual void OnDestroy() {}
    };

    // ------------------------------------------------------------------ ScriptableObject

    class ScriptableObject : public Object
    {
        DD_TYPE_OBJET(ScriptableObject, Object)
    public:
        template <class T>
        static Ref<T> CreateInstance()
        {
            return Neuf<T>();
        }
    };

    // ------------------------------------------------------------------ Time, Debug, Application, Screen, Resources

    struct Time
    {
        static inline float time = 0.0f, deltaTime = 1.0f / 60.0f, unscaledDeltaTime = 1.0f / 60.0f, timeScale = 1.0f, unscaledTime = 0.0f,
                            realtimeSinceStartup = 0.0f, fixedDeltaTime = 0.02f, smoothDeltaTime = 1.0f / 60.0f;
        static inline int frameCount = 0;
        static double timeAsDouble() { return static_cast<double>(time); }
    };

    struct Debug
    {
        static Liste<string>& Erreurs();
        static inline bool Muet = false;
        /// Où écrire (la couche Unreal y branche son journal) ; par défaut la sortie standard.
        static inline void (*Sortie)(int niveau, const string& texte) = nullptr;
        static void Log(const string& s);
        static void LogWarning(const string& s);
        static void LogError(const string& s);
        static void DrawLine(Vector3, Vector3, Color, float = 0.0f) {}
        static void Assert(bool c, const string& m = "");
    };

    enum class RuntimePlatform { WindowsPlayer, OSXPlayer, LinuxPlayer, WindowsEditor };

    struct Application
    {
        static inline int targetFrameRate = -1;
        static inline bool isPlaying = true, isEditor = false, isBatchMode = false;
        static inline string dataPath, version = "1.0", persistentDataPath;
        static inline RuntimePlatform platform = RuntimePlatform::WindowsPlayer;
        static void Quit();
        static void OpenURL(const string&) {}
        /// Application.Quit() a été demandé (la couche Unreal ferme le jeu).
        static inline bool QuitterDemande = false;
    };

    struct Screen
    {
        static inline int width = 1920, height = 1080;
        static inline float dpi = 96.0f;
        static inline bool fullScreen = false;
    };

    struct Resources
    {
        /// Les objets « présents » dans Resources (chemin → objets) : la couche Unreal ou les essais les déposent ici.
        static Dico<string, Liste<Ref<Object>>>& Factices();
        template <class T>
        static Ref<T> GetBuiltinResource(const string& chemin)
        {
            auto o = Neuf<T>();
            o->name(chemin);
            return o;
        }
        template <class T>
        static Ref<T> Load(const string& chemin);
        template <class T>
        static Liste<Ref<T>> LoadAll(const string& dossier);
    };

    /// Environment.TickCount (le temps écoulé en millisecondes). Les essais le fixent pour rejouer une partie.
    struct Environnement
    {
        static int TickCount();
        static inline bool TickFixe = false;
        static inline int ValeurTick = 0;
    };

    // ------------------------------------------------------------------ la boucle du moteur

    /// Le moteur proprement dit : la liste des objets, le cycle de vie des MonoBehaviour, les coroutines, les
    /// destructions différées. Même ordre d'appel que le moteur des essais de la version C# (MoteurFactice).
    struct Boucle
    {
        /// Tous les GameObject vivants, dans l'ordre de création.
        static Liste<Ref<GameObject>>& Objets();
        /// Une image du jeu : temps, Start des nouveaux composants, Update, coroutines, LateUpdate, destructions.
        static void Image(float dt);
        static int NombreCoroutines();
        /// Fin de session : tous les objets sont détruits (OnDestroy compris), le temps est remis à zéro et les
        /// variables statiques du jeu sont réinitialisées (comme un nouveau lancement dans Unity).
        static void Reinitialiser();
        /// Inscrit une remise à zéro de variables statiques du jeu, appelée par Reinitialiser(). Rend true (pour
        /// s'écrire « static const bool x = Boucle::AuReinitialiser(…); »).
        static bool AuReinitialiser(void (*f)());

        // --- usage interne
        static void Inscrire(const Ref<Object>& o);
        static void Naissance(const Ref<Component>& c);
        static Ptr<Coroutine> Demarrer(const Ref<MonoBehaviour>& hote, Tache t);
        static void Arreter(const Ptr<Coroutine>& c);
        static void ArreterTout(const MonoBehaviour* hote);
        static void Detruire(const Ref<Object>& o);
        static void DetruireTout(const Ref<Object>& o);
        static int NouvelId();

    private:
        static void Avancer(const Ptr<Coroutine>& co);
        static void Effacer(const Ref<Object>& o);
        static bool Actif(const Ref<Component>& c);
    };

    // ------------------------------------------------------------------ définitions des modèles

    template <class T, class U>
    Ref<T> Comme(const Ref<U>& r)
    {
        if (r.get() == nullptr || !static_cast<const Object*>(r.get())->EstUn(T::TypeStatique())) return Ref<T>();
        return Ref<T>(std::static_pointer_cast<T>(r.Partage()));
    }
    template <class T, class U>
    Ref<T> Vers(const Ref<U>& r)
    {
        if (r.get() == nullptr) return Ref<T>();
        DD_VERIFIE(static_cast<const Object*>(r.get())->EstUn(T::TypeStatique()), "Donjon de l'Oie : conversion d'un objet vers un type qu'il n'a pas");
        return Ref<T>(std::static_pointer_cast<T>(r.Partage()));
    }
    template <class T, class U>
    bool Est(const Ref<U>& r)
    {
        return r.get() != nullptr && static_cast<const Object*>(r.get())->EstUn(T::TypeStatique());
    }

    template <class T, class... A>
    Ref<T> Neuf(A&&... a)
    {
        Ref<T> r(std::make_shared<T>(std::forward<A>(a)...));
        Boucle::Inscrire(r);
        r->ApresCreation();
        return r;
    }

    template <class T>
    Ref<T> GameObject::AddComponent()
    {
        static_assert(std::is_base_of_v<Component, T>, "AddComponent : T doit être un composant");
        if constexpr (std::is_same_v<T, RectTransform>)
        {
            if (auto rt = Comme<RectTransform>(transf)) return rt;
            auto rt = Ref<RectTransform>(std::make_shared<RectTransform>());
            Boucle::Inscrire(rt);
            rt->go_ = Soi<GameObject>().Partage();
            rt->CopierDe(*transf);
            for (int i = 0; i < composants.Count(); i++)
                if (composants[i].get() == static_cast<Component*>(transf.get())) composants[i] = rt;
            transf = rt;
            return rt;
        }
        else
        {
            auto c = Ref<T>(std::make_shared<T>());
            Boucle::Inscrire(c);
            Lier(c);
            Boucle::Naissance(c);
            return c;
        }
    }

    template <class T>
    Ref<T> GameObject::GetComponent() const
    {
        for (auto& c : composants)
            if (!c->EstDetruit() && c->EstUn(T::TypeStatique())) return Ref<T>(std::static_pointer_cast<T>(c.Partage()));
        return Ref<T>();
    }

    template <class T>
    Ref<T> GameObject::GetComponentInChildren() const
    {
        auto c = GetComponent<T>();
        if (c != nullptr) return c;
        for (int i = 0; i < transf->childCount(); i++)
        {
            auto r = transf->GetChild(i)->gameObject()->GetComponentInChildren<T>();
            if (r != nullptr) return r;
        }
        return Ref<T>();
    }

    namespace DetailObjets
    {
        template <class T>
        void Collecter(const GameObject& g, Liste<Ref<T>>& l)
        {
            for (auto& c : g.Composants())
                if (!c->EstDetruit() && c->EstUn(T::TypeStatique())) l.Add(Ref<T>(std::static_pointer_cast<T>(c.Partage())));
            auto t = g.transform();
            for (int i = 0; i < t->childCount(); i++) Collecter(*t->GetChild(i)->gameObject(), l);
        }
    }

    template <class T>
    Liste<Ref<T>> GameObject::GetComponentsInChildren(bool) const
    {
        Liste<Ref<T>> l;
        DetailObjets::Collecter(*this, l);
        return l;
    }

    template <class T>
    Ref<T> Component::GetComponent() const
    {
        auto g = go_.lock();
        return g != nullptr ? g->GetComponent<T>() : Ref<T>();
    }
    template <class T>
    Ref<T> Component::GetComponentInChildren() const
    {
        auto g = go_.lock();
        return g != nullptr ? g->GetComponentInChildren<T>() : Ref<T>();
    }
    template <class T>
    Liste<Ref<T>> Component::GetComponentsInChildren(bool inactifs) const
    {
        auto g = go_.lock();
        return g != nullptr ? g->GetComponentsInChildren<T>(inactifs) : Liste<Ref<T>>();
    }
    template <class T>
    Ref<T> Component::GetComponentInParent() const
    {
        for (auto t = transform(); t != nullptr; t = t->parent())
        {
            auto c = t->gameObject()->GetComponent<T>();
            if (c != nullptr) return c;
        }
        return Ref<T>();
    }

    template <class T>
    Liste<Ref<T>> Object::FindObjectsByType(FindObjectsInactive, FindObjectsSortMode)
    {
        Liste<Ref<T>> l;
        auto objets = Boucle::Objets();
        for (auto& g : objets)
            for (auto& c : g->Composants())
                if (!c->EstDetruit() && c->EstUn(T::TypeStatique())) l.Add(Ref<T>(std::static_pointer_cast<T>(c.Partage())));
        return l;
    }

    template <class T>
    Ref<T> Object::FindAnyObjectByType()
    {
        for (auto& g : Boucle::Objets())
            for (auto& c : g->Composants())
                if (!c->EstDetruit() && c->EstUn(T::TypeStatique())) return Ref<T>(std::static_pointer_cast<T>(c.Partage()));
        return Ref<T>();
    }

    template <class T>
    Ref<T> Resources::Load(const string& chemin)
    {
        if (auto l = Factices().Chercher(chemin))
            for (auto& o : *l)
                if (Est<T>(o)) return Vers<T>(o);
        return Ref<T>();
    }

    template <class T>
    Liste<Ref<T>> Resources::LoadAll(const string& dossier)
    {
        Liste<Ref<T>> r;
        for (auto& kv : Factices())
            if (kv.Key == dossier || Texte::CommencePar(kv.Key, dossier + "/"))
                for (auto& o : kv.Value)
                    if (Est<T>(o)) r.Add(Vers<T>(o));
        return r;
    }
}

// Les références comme clés de dictionnaire : égalité par identité (comme Dictionary<UnityEngine.Object, …> en C#).
namespace std
{
    template <class T>
    struct equal_to<Moteur::Ref<T>>
    {
        bool operator()(const Moteur::Ref<T>& a, const Moteur::Ref<T>& b) const { return a.get() == b.get(); }
    };
}
