// Donjon de l'Oie — sons et musique synthétisés au lancement (aucun fichier audio à importer).
// Bruitages : dé, pas, or, coup, niveau, malus, piège, sort, clic, victoire, monstre. Musique : une boucle au luth
// (cordes pincées, algorithme de Karplus-Strong) en ré dorien, sur un bourdon doux.
#include "Sons.h"

namespace DonjonDeLOie::Monde
{
    Ref<Sons> Sons::Installer(const Ref<GameObject>& hote)
    {
        instance = hote->AddComponent<Sons>();
        instance->effets = hote->AddComponent<AudioSource>();
        instance->effets->playOnAwake = false;
        instance->musique = hote->AddComponent<AudioSource>();
        instance->musique->loop = true;
        instance->musique->playOnAwake = false;
        instance->Generer();
        return instance;
    }

    void Sons::Jouer(Son s, float volume)
    {
        if (instance == nullptr) return;
        Ref<AudioClip> c;
        if (instance->clips.TryGetValue(s, c) && c != nullptr)
        {
            instance->effets->pitch = 1.0f;
            instance->effets->PlayOneShot(c, volume * VolumeEffets);
        }
    }

    void Sons::Musique(bool oui)
    {
        if (instance == nullptr || instance->musique->clip == nullptr) return;
        instance->musique->volume = VolumeMusique;
        if (oui && !instance->musique->isPlaying()) instance->musique->Play();
        if (!oui) instance->musique->Stop();
    }

    void Sons::ReglerVolumes(float volEffets, float volMusique)
    {
        VolumeEffets = volEffets; VolumeMusique = volMusique;
        if (instance != nullptr) instance->musique->volume = volMusique;
    }

    Ref<AudioClip> Sons::Clip(const string& nom, const Liste<float>& d)
    {
        auto c = AudioClip::Create(nom, d.Count(), 1, F, false);
        c->SetData(d, 0);
        return c;
    }

    float Sons::Bruit() { return static_cast<float>(r.NextDouble() * 2 - 1); }

    float Sons::Env(float t, float att, float dec) { return t < att ? t / att : Mathf::Exp(-(t - att) / dec); }

    Liste<float> Sons::Tonalite(float duree, const std::function<float(float)>& f)
    {
        Liste<float> d(static_cast<int>(duree * F));
        for (int i = 0; i < d.Count(); i++) d[i] = f(static_cast<float>(i) / static_cast<float>(F));
        return d;
    }

    float Sons::Sin(float hz, float t) { return Mathf::Sin(2.0f * Mathf::Pi * hz * t); }

    Liste<float> Sons::Corde(float hz, float duree, float amort, float brillance)
    {
        int n = static_cast<int>(duree * F);
        Liste<float> d(n);
        int p = Mathf::Max(2, static_cast<int>(F / hz));
        Liste<float> buf(p);
        for (int i = 0; i < p; i++) buf[i] = Bruit() * (1.0f - brillance * 0.5f);
        int k = 0;
        for (int i = 0; i < n; i++)
        {
            float a = buf[k], b = buf[(k + 1) % p];
            float v = (a + b) * 0.5f * amort;
            buf[k] = v;
            d[i] = a;
            k = (k + 1) % p;
        }
        return d;
    }

    void Sons::Generer()
    {
        // Dé : cliquetis de petits chocs
        Liste<float> de(static_cast<int>(0.55f * F));
        for (int k = 0; k < 7; k++)
        {
            int debut = static_cast<int>((0.03f + static_cast<float>(k) * 0.065f + static_cast<float>(r.NextDouble()) * 0.02f) * F);
            float hz = 900.0f + static_cast<float>(r.NextDouble()) * 900.0f, amp = 0.5f * (1.0f - static_cast<float>(k) / 8.0f);
            for (int i = 0; i < 1600 && debut + i < de.Count(); i++)
            {
                float t = static_cast<float>(i) / static_cast<float>(F);
                de[debut + i] += (Sin(hz, t) * 0.6f + Bruit() * 0.4f) * amp * Mathf::Exp(-t * 90.0f);
            }
        }
        clips[Son::De] = Clip("de", de);
        clips[Son::Pas] = Clip("pas", Tonalite(0.12f, [this](float t) { return (Sin(95.0f, t) * 0.7f + Bruit() * 0.3f) * Env(t, 0.004f, 0.03f) * 0.55f; }));
        clips[Son::Or] = Clip("or", Tonalite(0.5f, [](float t) {
            return (Sin(1568.0f, t) * Env(t, 0.002f, 0.12f) + Sin(2093.0f, Mathf::Max(0.0f, t - 0.07f)) * (t > 0.07f ? Env(t - 0.07f, 0.002f, 0.18f) : 0.0f)) * 0.32f;
        }));
        clips[Son::Coup] = Clip("coup", Tonalite(0.32f, [this](float t) {
            return (Bruit() * Env(t, 0.002f, 0.05f) * 0.8f + Sin(70.0f + 60.0f * Mathf::Exp(-t * 20.0f), t) * Env(t, 0.003f, 0.09f)) * 0.7f;
        }));
        clips[Son::Niveau] = Clip("niveau", Arpege({523.25f, 659.25f, 783.99f, 1046.5f}, 0.09f, 0.9f, 0.3f));
        clips[Son::Victoire] = Clip("victoire", Arpege({392.0f, 523.25f, 659.25f, 783.99f, 1046.5f, 783.99f, 1046.5f}, 0.14f, 1.8f, 0.3f));
        clips[Son::Malus] = Clip("malus", Arpege({392.0f, 349.23f, 311.13f, 233.08f}, 0.12f, 0.8f, 0.26f));
        clips[Son::Piege] = Clip("piege", Tonalite(0.4f, [this](float t) {
            return (Bruit() * Env(t, 0.001f, 0.02f) + Sin(220.0f, t) * Sin(7.0f, t) * Env(t, 0.002f, 0.12f) * 0.6f + Sin(1200.0f, t) * Env(t, 0.001f, 0.03f) * 0.5f) * 0.6f;
        }));
        clips[Son::Sort] = Clip("sort", Tonalite(0.9f, [this](float t) {
            return (Sin(600.0f + 900.0f * t, t) * 0.25f + Sin(1200.0f + 1600.0f * t, t) * 0.15f + Bruit() * 0.15f) * Env(t, 0.15f, 0.25f) * 0.5f;
        }));
        clips[Son::Soin] = Clip("soin", Arpege({659.25f, 880.0f, 1318.5f}, 0.1f, 0.8f, 0.2f));
        clips[Son::Clic] = Clip("clic", Tonalite(0.05f, [](float t) { return Sin(1400.0f, t) * Env(t, 0.001f, 0.008f) * 0.35f; }));
        clips[Son::Page] = Clip("page", Tonalite(0.25f, [this](float t) { return Bruit() * Env(t, 0.03f, 0.06f) * 0.25f; }));
        // Monstre : grognement (bruit filtré modulé, grave)
        Liste<float> gr(static_cast<int>(0.8f * F));
        float lp = 0;
        for (int i = 0; i < gr.Count(); i++)
        {
            float t = static_cast<float>(i) / static_cast<float>(F);
            lp += (Bruit() - lp) * 0.04f;
            gr[i] = (lp * 3.0f * (0.6f + 0.4f * Sin(28.0f, t)) + Sin(55.0f + 15.0f * Sin(3.0f, t), t) * 0.4f) * Env(t, 0.06f, 0.3f) * 0.6f;
        }
        clips[Son::Monstre] = Clip("monstre", gr);
        musique->clip = Clip("musique", Luth());
    }

    Liste<float> Sons::Arpege(const Liste<float>& notes, float ecart, float duree, float vol)
    {
        Liste<float> d(static_cast<int>(duree * F));
        for (int k = 0; k < notes.Count(); k++)
        {
            int debut = static_cast<int>(static_cast<float>(k) * ecart * F);
            Liste<float> c = Corde(notes[k], duree - static_cast<float>(k) * ecart, 0.997f, 0.3f);
            for (int i = 0; i < c.Count() && debut + i < d.Count(); i++)
            {
                float t = static_cast<float>(i) / static_cast<float>(F);
                d[debut + i] += (c[i] * 0.6f + Sin(notes[k], t) * 0.4f * Mathf::Exp(-t * 4.0f)) * vol;
            }
        }
        return d;
    }

    /// Boucle de 32 mesures à 3/4, ~84 battements par minute, ré dorien : basse, arpèges, mélodie simple.
    Liste<float> Sons::Luth()
    {
        const float bpm = 84.0f;
        float temps = 60.0f / bpm;
        int mesures = 16;
        float duree = static_cast<float>(mesures * 3) * temps;
        Liste<float> d(static_cast<int>(duree * F));
        // degrés de ré dorien
        const float gamme[] = {146.83f, 164.81f, 174.61f, 196.00f, 220.00f, 246.94f, 261.63f, 293.66f, 329.63f, 349.23f, 392.00f, 440.00f, 493.88f, 523.25f, 587.33f};
        const int nGamme = static_cast<int>(std::size(gamme));
        const int accords[4][3] = {{0, 2, 4}, {6, 1, 3}, {3, 5, 0}, {4, 6, 1}};   // Rém, Do, Sol, Lam
        const int suite[] = {0, 0, 1, 1, 2, 2, 3, 0, 0, 0, 1, 1, 2, 3, 0, 0};
        AleaNet rm(11);
        for (int m = 0; m < mesures; m++)
        {
            const int* ac = accords[suite[m]];
            float t0 = static_cast<float>(m * 3) * temps;
            // basse au premier temps
            Ajouter(d, Corde(gamme[ac[0]] / 2.0f, 3 * temps, 0.998f, 0.2f), t0, 0.33f);
            // arpège en croches
            for (int k = 0; k < 6; k++)
            {
                int deg = ac[k % 3] + (k >= 3 ? 7 : 0);
                deg = Mathf::Min(deg, nGamme - 1);
                Ajouter(d, Corde(gamme[deg], 1.2f, 0.995f, 0.4f), t0 + static_cast<float>(k) * temps * 0.5f, 0.14f);
            }
            // mélodie : une note par temps, sur l'accord ou en passage
            if (m % 4 != 3)
                for (int b = 0; b < 3; b++)
                {
                    if (rm.NextDouble() < 0.25) continue;
                    // C# : ac[rm.Next(3)] + 7 + (rm.NextDouble() < 0.3 ? 1 : 0) — deux tirages, de gauche à droite
                    int i3 = rm.Next(3);
                    int deg = ac[i3] + 7 + (rm.NextDouble() < 0.3 ? 1 : 0);
                    deg = Mathf::Min(deg, nGamme - 1);
                    Ajouter(d, Corde(gamme[deg] * 2.0f, 1.0f, 0.996f, 0.6f), t0 + static_cast<float>(b) * temps + 0.01f, 0.1f);
                }
        }
        // bourdon très doux et fondu pour une boucle sans clic
        for (int i = 0; i < d.Count(); i++)
        {
            float t = static_cast<float>(i) / static_cast<float>(F);
            d[i] += (Sin(73.42f, t) + Sin(110.0f, t) * 0.5f) * 0.035f;
            float bord = Mathf::Min(1.0f, Mathf::Min(t, duree - t) / 0.05f);
            d[i] = Mathf::Clamp(d[i] * bord, -0.95f, 0.95f);
        }
        return d;
    }

    void Sons::Ajouter(Liste<float>& d, const Liste<float>& s, float t0, float vol)
    {
        int debut = static_cast<int>(t0 * F);
        for (int i = 0; i < s.Count() && debut + i < d.Count(); i++) d[debut + i] += s[i] * vol;
    }

    // Remise à zéro des variables statiques (une partie relancée dans Unreal repart comme un lancement d'Unity).
    const bool Sons::remiseAZero = Boucle::AuReinitialiser([] {
        Sons::instance = nullptr;
        Sons::VolumeEffets = 0.8f;
        Sons::VolumeMusique = 0.35f;
    });
}
