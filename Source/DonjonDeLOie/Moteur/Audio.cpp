// Donjon de l'Oie — les sons.
#include "Audio.h"

#include <cmath>

namespace Moteur
{
    // (paramètres renommés : Visual Studio refuse qu'un paramètre d'une fonction statique porte le nom d'un membre, C4458)
    Ref<AudioClip> AudioClip::Create(const string& nom, int lengthSamples, int nbCanaux, int frequenceHz, bool)
    {
        DD_VERIFIE(lengthSamples > 0, "AudioClip vide");
        auto c = Neuf<AudioClip>();
        c->name(nom);
        c->samples = lengthSamples;
        c->channels = nbCanaux;
        c->frequency = frequenceHz;
        c->donnees = Liste<float>(lengthSamples * nbCanaux);
        return c;
    }

    bool AudioClip::SetData(const Liste<float>& d, int decalage)
    {
        for (float f : d) DD_VERIFIE(std::isfinite(f), "AudioClip : NaN ou infini");
        int n = std::min(d.Count(), donnees.Count() - decalage);
        for (int i = 0; i < n; i++) donnees[decalage + i] = d[i];
        return true;
    }

    void AudioSource::Play()
    {
        joue = clip.get() != nullptr;
        versionLecture++;
    }

    void AudioSource::Stop()
    {
        joue = false;
        versionLecture++;
    }

    void AudioSource::Pause()
    {
        joue = false;
        versionLecture++;
    }

    void AudioSource::PlayOneShot(const Ref<AudioClip>& c, float v)
    {
        DD_VERIFIE(c.get() != nullptr, "AudioSource.PlayOneShot : clip nul");
        if (ponctuels.Count() < 64) ponctuels.Add(SonPonctuel{c, v, pitch});
    }

    Liste<AudioSource::SonPonctuel> AudioSource::PrendreSonsPonctuels()
    {
        auto l = ponctuels;
        ponctuels.Clear();
        return l;
    }
}
