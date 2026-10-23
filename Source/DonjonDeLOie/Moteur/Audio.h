// Donjon de l'Oie — les sons : des échantillons calculés par le jeu (AudioClip) et des sources qui les jouent
// (AudioSource). Le moteur note ce qu'on demande ; la couche Unreal le fait entendre.
#pragma once

#include "Objets.h"

namespace Moteur
{
    class AudioClip : public Object
    {
        DD_TYPE_OBJET(AudioClip, Object)
    public:
        int samples = 0, channels = 1, frequency = 44100;
        float length() const { return frequency > 0 ? static_cast<float>(samples) / static_cast<float>(frequency) : 0.0f; }
        static Ref<AudioClip> Create(const string& nom, int lengthSamples, int nbCanaux, int frequenceHz, bool stream);
        bool SetData(const Liste<float>& d, int decalage);

        // --- pour la couche d'affichage
        const Liste<float>& Donnees() const { return donnees; }

    private:
        Liste<float> donnees;
    };

    class AudioSource : public Behaviour
    {
        DD_TYPE_OBJET(AudioSource, Behaviour)
    public:
        Ref<AudioClip> clip;
        bool loop = false, playOnAwake = true, mute = false;
        float volume = 1.0f, pitch = 1.0f, spatialBlend = 0.0f;
        bool isPlaying() const { return joue; }
        void Play();
        void Stop();
        void Pause();
        void PlayOneShot(const Ref<AudioClip>& c) { PlayOneShot(c, 1.0f); }
        void PlayOneShot(const Ref<AudioClip>& c, float v);

        // --- pour la couche d'affichage : les sons ponctuels demandés depuis la dernière lecture, et l'état de la
        // lecture continue (clip, boucle, volume).
        struct SonPonctuel
        {
            Ref<AudioClip> clip;
            float volume;
            float pitch;
        };
        Liste<SonPonctuel> PrendreSonsPonctuels();
        int VersionLecture() const { return versionLecture; }

    private:
        bool joue = false;
        int versionLecture = 0;
        Liste<SonPonctuel> ponctuels;
    };

    class AudioListener : public Behaviour
    {
        DD_TYPE_OBJET(AudioListener, Behaviour)
    public:
        static inline float volume = 1.0f;
    };
}
