// Donjon de l'Oie — le moteur : objets, hiérarchie, composants, cycle de vie, coroutines, destructions.
// L'ordre des appels est celui du moteur des essais de la version C# (tests hors Unity), pour qu'une même partie
// se déroule à l'identique dans les deux versions.
#include "Objets.h"

#include "Coroutines.h"
#include "Entrees.h"
#include "Rendu.h"

#include <chrono>
#include <cstdio>

namespace Moteur
{
    namespace
    {
        struct EtatMoteur
        {
            Liste<Ref<GameObject>> objets;
            Liste<Ref<Component>> aDemarrer;
            Liste<Ptr<Coroutine>> coroutines;
            Liste<Ref<Object>> aDetruire;
            std::vector<std::weak_ptr<Object>> tous;       // tout objet créé (pour la réinitialisation)
            size_t tailleCompactee = 64;
            int prochainId = 1;
            std::vector<void (*)()> reinitialisations;
        };

        EtatMoteur& E()
        {
            static EtatMoteur e;
            return e;
        }

        float Nz(float f) { return std::fabs(f) < 1e-9f ? 1e-9f : f; }
    }

    void ErreurObjetNul(const char* type)
    {
        string m = string("Donjon de l'Oie : accès à un objet nul ou détruit (") + type + ")";
        ErreurFatale(m.c_str());
    }

    // ------------------------------------------------------------------ Object

    Object::Object() : idInstance_(Boucle::NouvelId()) {}
    Object::~Object() = default;

    void Object::Destroy(const Ref<Object>& o) { Boucle::Detruire(o); }
    void Object::Destroy(const Ref<Object>& o, float) { Boucle::Detruire(o); }
    void Object::DestroyImmediate(const Ref<Object>& o) { Boucle::DetruireTout(o); }

    // ------------------------------------------------------------------ Component, Behaviour

    Ref<Transform> Component::transform() const
    {
        auto g = go_.lock();
        return g != nullptr ? g->transform() : Ref<Transform>();
    }

    // (comme le moteur des essais C# : « go != null » au sens d'Unity, un GameObject détruit ne donne plus son nom)
    string Component::name() const
    {
        auto g = go_.lock();
        return g != nullptr && !g->EstDetruit() ? g->name() : string();
    }

    void Component::name(const string& n)
    {
        auto g = go_.lock();
        if (g != nullptr && !g->EstDetruit()) g->name(n);
    }

    string Component::tag() const
    {
        auto g = go_.lock();
        return g != nullptr ? g->tag : string();
    }

    void Component::tag(const string& t)
    {
        if (auto g = go_.lock()) g->tag = t;
    }

    bool Behaviour::isActiveAndEnabled() const
    {
        auto g = go_.lock();
        return enabled && g != nullptr && g->activeInHierarchy();
    }

    // ------------------------------------------------------------------ GameObject

    GameObject::GameObject(const string& n, const TypeObjet* type)
    {
        nomObjet_ = n;
        DD_VERIFIE(type == RectTransform::TypeStatique() || type == Transform::TypeStatique(), "new GameObject(nomObjet_, type) : seul typeof(RectTransform) est pris en charge");
        avecRect = type == RectTransform::TypeStatique();
    }

    void GameObject::ApresCreation()
    {
        Ref<Transform> t;
        if (avecRect) t = Ref<Transform>(std::make_shared<RectTransform>());
        else t = Ref<Transform>(std::make_shared<Transform>());
        Boucle::Inscrire(t);
        t->go_ = std::static_pointer_cast<GameObject>(shared_from_this());
        transf = t;
        composants.Add(t);
        E().objets.Add(Soi<GameObject>());
    }

    void GameObject::Lier(const Ref<Component>& c)
    {
        c->go_ = std::static_pointer_cast<GameObject>(shared_from_this());
        composants.Add(c);
    }

    bool GameObject::activeInHierarchy() const
    {
        if (!actif || detruit_) return false;
        for (auto t = transf->parent(); t != nullptr; t = t->parent())
        {
            auto g = t->gameObject();
            if (g == nullptr) break;
            if (!g->actif || g->detruit_) return false;
        }
        return true;
    }

    Ref<GameObject> GameObject::CreatePrimitive(PrimitiveType t)
    {
        static const char* noms[] = {"Cube", "Sphere", "Quad", "Plane"};
        auto g = Neuf<GameObject>(string(noms[static_cast<int>(t)]));
        g->AddComponent<MeshFilter>();
        g->AddComponent<MeshRenderer>();
        return g;
    }

    Ref<GameObject> GameObject::Find(const string& n)
    {
        for (auto& g : E().objets)
            if (g->nomObjet_ == n && !g->detruit_) return g;
        return Ref<GameObject>();
    }

    // ------------------------------------------------------------------ Transform

    void Transform::CopierDe(Transform& t)
    {
        localPosition = t.localPosition;
        localRotation = t.localRotation;
        localScale = t.localScale;
        parentT = t.parentT;
        auto moi = Soi<Transform>();
        if (auto p = parentT.lock())
            for (int i = 0; i < p->enfants.Count(); i++)
                if (p->enfants[i].get() == &t) p->enfants[i] = moi;
        enfants.AddRange(t.enfants);
        for (auto& e : enfants) e->parentT = moi.Partage();
    }

    bool Transform::IsChildOf(const Ref<Transform>& p) const
    {
        for (const Transform* t = this; t != nullptr;)
        {
            if (t == p.get()) return true;
            auto q = t->parentT.lock();
            t = q.get();
        }
        return false;
    }

    Ref<Transform> Transform::root() const
    {
        auto t = const_cast<Transform*>(this)->Soi<Transform>();
        while (auto p = t->parent()) t = p;
        return t;
    }

    void Transform::SetParent(const Ref<Transform>& p, bool garderMonde)
    {
        Vector3 pos = position();
        Quaternion rot = rotation();
        auto moi = Soi<Transform>();
        if (auto ancien = parentT.lock()) ancien->enfants.Remove(moi);
        parentT = p.Partage();
        if (p.get() != nullptr) p->enfants.Add(moi);
        if (garderMonde)
        {
            position(pos);
            rotation(rot);
        }
    }

    int Transform::GetSiblingIndex() const
    {
        auto p = parentT.lock();
        if (p == nullptr) return 0;
        for (int i = 0; i < p->enfants.Count(); i++)
            if (p->enfants[i].get() == this) return i;
        return -1;
    }

    void Transform::SetSiblingIndex(int i)
    {
        auto p = parentT.lock();
        if (p == nullptr) return;
        auto moi = Soi<Transform>();
        p->enfants.Remove(moi);
        p->enfants.Insert(std::max(0, std::min(i, p->enfants.Count())), moi);
    }

    void Transform::SetAsFirstSibling()
    {
        auto p = parentT.lock();
        if (p == nullptr) return;
        auto moi = Soi<Transform>();
        p->enfants.Remove(moi);
        p->enfants.Insert(0, moi);
    }

    void Transform::SetAsLastSibling()
    {
        auto p = parentT.lock();
        if (p == nullptr) return;
        auto moi = Soi<Transform>();
        p->enfants.Remove(moi);
        p->enfants.Add(moi);
    }

    Ref<Transform> Transform::Find(const string& n) const
    {
        for (auto& e : enfants)
            if (e->name() == n) return e;
        return Ref<Transform>();
    }

    Vector3 Transform::TransformPoint(Vector3 v) const
    {
        Vector3 x = localPosition + localRotation * Vector3::Scale(localScale, v);
        auto p = parentT.lock();
        return p != nullptr ? p->TransformPoint(x) : x;
    }

    Vector3 Transform::InverseTransformPoint(Vector3 w) const
    {
        auto p = parentT.lock();
        Vector3 l = p != nullptr ? p->InverseTransformPoint(w) : w;
        Vector3 v = Quaternion::Inverse(localRotation) * (l - localPosition);
        return Vector3(v.x / Nz(localScale.x), v.y / Nz(localScale.y), v.z / Nz(localScale.z));
    }

    Vector3 Transform::position() const
    {
        auto p = parentT.lock();
        return p != nullptr ? p->TransformPoint(localPosition) : localPosition;
    }

    void Transform::position(Vector3 v)
    {
        auto p = parentT.lock();
        localPosition = p != nullptr ? p->InverseTransformPoint(v) : v;
    }

    Quaternion Transform::rotation() const
    {
        auto p = parentT.lock();
        return p != nullptr ? p->rotation() * localRotation : localRotation;
    }

    void Transform::rotation(Quaternion q)
    {
        auto p = parentT.lock();
        localRotation = p != nullptr ? Quaternion::Inverse(p->rotation()) * q : q;
    }

    Vector3 Transform::lossyScale() const
    {
        auto p = parentT.lock();
        Vector3 s = p != nullptr ? p->lossyScale() : Vector3::one;
        return Vector3::Scale(s, localScale);
    }

    void Transform::Rotate(float x, float y, float z, Space s)
    {
        Quaternion q = Quaternion::Euler(x, y, z);
        if (s == Space::World) rotation(q * rotation());
        else localRotation = localRotation * q;
    }

    void Transform::LookAt(Vector3 p)
    {
        Vector3 d = p - position();
        if (d.sqrMagnitude() > 1e-8f) rotation(Quaternion::LookRotation(d));
    }

    Matrix4x4 Transform::localToWorldMatrix() const
    {
        Matrix4x4 m = Matrix4x4::TRS(localPosition, localRotation, localScale);
        auto p = parentT.lock();
        return p != nullptr ? p->localToWorldMatrix() * m : m;
    }

    // ------------------------------------------------------------------ RectTransform

    void RectTransform::offsetMin(Vector2 v)
    {
        Vector2 om = offsetMax();
        sizeDelta = om - v;
        anchoredPosition = v + Vector2::Scale(sizeDelta, pivot);
    }

    void RectTransform::offsetMax(Vector2 v)
    {
        Vector2 om = offsetMin();
        sizeDelta = v - om;
        anchoredPosition = om + Vector2::Scale(sizeDelta, pivot);
    }

    Rect RectTransform::rect() const
    {
        if (RectCalcule.width != 0.0f || RectCalcule.height != 0.0f) return RectCalcule;
        return Rect(-pivot.x * sizeDelta.x, -pivot.y * sizeDelta.y, sizeDelta.x, sizeDelta.y);
    }

    void RectTransform::SetSizeWithCurrentAnchors(Axis a, float s)
    {
        if (a == Axis::Horizontal) sizeDelta = Vector2(s, sizeDelta.y);
        else sizeDelta = Vector2(sizeDelta.x, s);
    }

    // ------------------------------------------------------------------ MonoBehaviour

    Ptr<Coroutine> MonoBehaviour::StartCoroutine(Tache t) { return Boucle::Demarrer(Soi<MonoBehaviour>(), std::move(t)); }
    void MonoBehaviour::StopCoroutine(const Ptr<Coroutine>& c) { Boucle::Arreter(c); }
    void MonoBehaviour::StopAllCoroutines() { Boucle::ArreterTout(this); }
    void MonoBehaviour::print(const string& s) { Debug::Log(s); }
    Tache MonoBehaviour::StartCoroutineDeDemarrage() { return Tache(); }

    // ------------------------------------------------------------------ Debug, Application, Resources, Environnement

    Liste<string>& Debug::Erreurs()
    {
        static Liste<string> l;
        return l;
    }

    namespace
    {
        void Ecrire(int niveau, const string& s)
        {
            if (Debug::Sortie != nullptr)
            {
                Debug::Sortie(niveau, s);
                return;
            }
            const char* prefixe = niveau == 0 ? "[log] " : (niveau == 1 ? "[avertissement] " : "[ERREUR] ");
            std::printf("%s%s\n", prefixe, s.c_str());
        }
    }

    void Debug::Log(const string& s)
    {
        if (!Muet) Ecrire(0, s);
    }
    void Debug::LogWarning(const string& s) { Ecrire(1, s); }
    void Debug::LogError(const string& s)
    {
        Erreurs().Add(s);
        Ecrire(2, s);
    }
    void Debug::Assert(bool c, const string& m)
    {
        if (!c) LogError("Assert : " + m);
    }

    void Application::Quit() { QuitterDemande = true; }

    Dico<string, Liste<Ref<Object>>>& Resources::Factices()
    {
        static Dico<string, Liste<Ref<Object>>> d;
        return d;
    }

    int Environnement::TickCount()
    {
        if (TickFixe) return ValeurTick;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        return static_cast<int>(static_cast<uint32_t>(static_cast<uint64_t>(ms)));
    }

    // ------------------------------------------------------------------ la boucle

    Liste<Ref<GameObject>>& Boucle::Objets() { return E().objets; }
    int Boucle::NombreCoroutines() { return E().coroutines.Count(); }
    int Boucle::NouvelId() { return E().prochainId++; }

    void Boucle::Inscrire(const Ref<Object>& o)
    {
        auto& e = E();
        e.tous.push_back(o.Partage());
        if (e.tous.size() > 2 * e.tailleCompactee)
        {
            size_t j = 0;
            for (size_t i = 0; i < e.tous.size(); i++)
                if (!e.tous[i].expired()) e.tous[j++] = e.tous[i];
            e.tous.resize(j);
            e.tailleCompactee = std::max<size_t>(64, j);
        }
    }

    bool Boucle::AuReinitialiser(void (*f)())
    {
        E().reinitialisations.push_back(f);
        return true;
    }

    void Boucle::Naissance(const Ref<Component>& c)
    {
        auto mb = Comme<MonoBehaviour>(c);
        if (mb.get() == nullptr) return;
        mb->Awake();
        mb->OnEnable();
        E().aDemarrer.Add(c);
    }

    Ptr<Coroutine> Boucle::Demarrer(const Ref<MonoBehaviour>& hote, Tache t)
    {
        auto co = Nouveau<Coroutine>();
        co->hote = hote;
        co->pile.Add(Nouveau<Tache>(std::move(t)));
        E().coroutines.Add(co);
        Avancer(co);           // comme Unity : le corps tourne jusqu'au premier co_yield
        return co;
    }

    void Boucle::Arreter(const Ptr<Coroutine>& c)
    {
        if (c != nullptr) c->fini = true;
    }

    void Boucle::ArreterTout(const MonoBehaviour* hote)
    {
        for (auto& c : E().coroutines)
            if (c->hote.get() == hote) c->fini = true;
    }

    void Boucle::Avancer(const Ptr<Coroutine>& co)
    {
        int garde = 0;
        while (!co->fini && co->pile.Count() > 0 && garde++ < 10000)
        {
            if (co->attend != nullptr)
            {
                if (!co->attend->fini) return;
                co->attend = nullptr;
            }
            if (co->attenteJusqua >= 0.0f)
            {
                if (Time::time < co->attenteJusqua) return;
                co->attenteJusqua = -1.0f;
            }
            if (co->condition != nullptr)
            {
                if (co->condition()) return;
                co->condition = nullptr;
            }
            Ptr<Tache> e = co->pile[co->pile.Count() - 1];
            bool suite = e->MoveNext();
            if (!suite)
            {
                co->pile.RemoveAt(co->pile.Count() - 1);
                continue;
            }
            Instruction y = std::move(e->Current());
            e->Current() = Instruction();
            switch (y.sorte)
            {
                case Instruction::Sorte::ImageSuivante:
                    return;
                case Instruction::Sorte::Secondes:
                    co->attenteJusqua = Time::time + y.secondes;
                    return;
                case Instruction::Sorte::AttendreCoroutine:
                    co->attend = y.coroutine;
                    if (co->attend == nullptr) return;          // (StartCoroutine a rendu null : comme « yield return null »)
                    continue;
                case Instruction::Sorte::Condition:
                    co->condition = y.attendreEncore;
                    return;
                case Instruction::Sorte::SousTache:
                    if (y.sousTache != nullptr && y.sousTache->Valide()) co->pile.Add(y.sousTache);
                    continue;
            }
            return;
        }
        if (co->pile.Count() == 0) co->fini = true;
    }

    void Boucle::DetruireTout(const Ref<Object>& o)
    {
        if (o.get() == nullptr || o->EstDetruit()) return;
        auto& l = E().aDetruire;
        for (int i = 0; i < l.Count(); i++)
            if (l[i].get() == o.get())
            {
                l.RemoveAt(i);
                break;
            }
        Effacer(o);
    }

    void Boucle::Detruire(const Ref<Object>& o)
    {
        if (o.get() == nullptr || o->EstDetruit()) return;
        auto& l = E().aDetruire;
        for (auto& x : l)
            if (x.get() == o.get()) return;
        l.Add(o);
    }

    void Boucle::Effacer(const Ref<Object>& o)
    {
        if (o->detruit_) return;
        o->detruit_ = true;
        auto& e = E();
        auto g = Comme<GameObject>(o);
        if (g.get() != nullptr)
        {
            auto copie = g->composants;
            for (auto& c : copie)
            {
                if (auto mb = Comme<MonoBehaviour>(c)) mb->OnDestroy();
                c->detruit_ = true;
            }
            auto enfants = g->transf->enfants;
            for (auto& t : enfants)
                if (auto eg = t->gameObject()) Effacer(eg);
            if (auto p = g->transf->parentT.lock()) p->enfants.Remove(g->transf);
            for (int i = 0; i < e.objets.Count(); i++)
                if (e.objets[i].get() == g.get())
                {
                    e.objets.RemoveAt(i);
                    break;
                }
            for (auto& co : e.coroutines)
                if (co->hote.get() != nullptr && co->hote->go_.lock().get() == g.get()) co->fini = true;
        }
        auto comp = Comme<Component>(o);
        if (comp.get() != nullptr)
        {
            if (auto mb = Comme<MonoBehaviour>(comp)) mb->OnDestroy();
            if (auto cg = comp->go_.lock())
                for (int i = 0; i < cg->composants.Count(); i++)
                    if (cg->composants[i].get() == comp.get())
                    {
                        cg->composants.RemoveAt(i);
                        break;
                    }
            for (auto& co : e.coroutines)
                if (co->hote.get() == comp.get()) co->fini = true;
        }
    }

    bool Boucle::Actif(const Ref<Component>& c)
    {
        if (c->detruit_) return false;
        auto g = c->go_.lock();
        if (g == nullptr || g->detruit_ || !g->activeInHierarchy()) return false;
        auto b = Comme<Behaviour>(c);
        return b.get() == nullptr || b->enabled;
    }

    void Boucle::Image(float dt)
    {
        auto& e = E();
        Time::unscaledDeltaTime = dt;
        Time::deltaTime = dt * Time::timeScale;
        Time::time += Time::deltaTime;
        Time::unscaledTime += dt;
        Time::realtimeSinceStartup += dt;
        Time::frameCount++;
        auto demarrer = e.aDemarrer;
        e.aDemarrer.Clear();
        for (auto& c : demarrer)
            if (!c->detruit_)
                if (auto mb = Comme<MonoBehaviour>(c))
                {
                    mb->Start();
                    Tache t = mb->StartCoroutineDeDemarrage();
                    if (t.Valide()) Demarrer(mb, std::move(t));
                }
        Liste<Ref<Component>> tous;
        auto objets = e.objets;
        for (auto& g : objets) tous.AddRange(g->composants);
        for (auto& c : tous)
            if (auto mb = Comme<MonoBehaviour>(c))
                if (Actif(c)) mb->Update();
        auto coroutines = e.coroutines;
        for (auto& co : coroutines)
        {
            if (co->fini) continue;
            auto h = co->hote;
            if (h.get() == nullptr || h->detruit_ || !h->go_.lock() || !h->go_.lock()->activeInHierarchy())
            {
                if (h.get() == nullptr || h->detruit_) co->fini = true;
                continue;
            }
            Avancer(co);
        }
        e.coroutines.RemoveAll([](const Ptr<Coroutine>& c) { return c->fini; });
        for (auto& c : tous)
            if (auto mb = Comme<MonoBehaviour>(c))
                if (Actif(c)) mb->LateUpdate();
        auto aDetruire = e.aDetruire;
        for (auto& o : aDetruire) Effacer(o);
        e.aDetruire.Clear();
    }

    void Boucle::Reinitialiser()
    {
        auto& e = E();
        // les GameObject d'abord (OnDestroy des composants, enfants compris), puis tout le reste
        auto objets = e.objets;
        for (auto& g : objets)
            if (!g->detruit_) Effacer(g);
        for (auto& w : e.tous)
            if (auto o = w.lock()) o->detruit_ = true;
        e.objets.Clear();
        e.aDemarrer.Clear();
        e.coroutines.Clear();
        e.aDetruire.Clear();
        e.tous.clear();
        e.tailleCompactee = 64;
        Time::time = 0.0f;
        Time::deltaTime = 1.0f / 60.0f;
        Time::unscaledDeltaTime = 1.0f / 60.0f;
        Time::timeScale = 1.0f;
        Time::unscaledTime = 0.0f;
        Time::realtimeSinceStartup = 0.0f;
        Time::smoothDeltaTime = 1.0f / 60.0f;
        Time::frameCount = 0;
        Application::QuitterDemande = false;
        RenderSettings::ambientMode = AmbientMode::Skybox;
        RenderSettings::ambientSkyColor = RenderSettings::ambientEquatorColor = RenderSettings::ambientGroundColor = RenderSettings::ambientLight = Color();
        RenderSettings::fogColor = Color();
        RenderSettings::ambientIntensity = 1.0f;
        RenderSettings::fog = false;
        RenderSettings::fogMode = FogMode::Linear;
        RenderSettings::fogStartDistance = 0.0f;
        RenderSettings::fogEndDistance = 300.0f;
        RenderSettings::fogDensity = 0.01f;
        RenderSettings::reflectionIntensity = 1.0f;
        RenderSettings::skybox = nullptr;
        RenderSettings::sun = nullptr;
        Resources::Factices().Clear();
        Debug::Erreurs().Clear();
        *Mouse::current() = Mouse();
        *Keyboard::current() = Keyboard();
        Random::InitState(1);
        for (auto f : e.reinitialisations) f();
    }
}
