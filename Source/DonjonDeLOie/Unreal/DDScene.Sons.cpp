// Donjon de l'Oie — la scène : les sons et la musique. Le jeu calcule ses échantillons (Monde/Sons.cpp) ; chaque son
// ponctuel (AudioSource.PlayOneShot) est joué par une onde procédurale à part, la musique (AudioSource.Play en boucle)
// par une onde qu'on réalimente à chaque image.
#include "DDScene.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundWaveProcedural.h"

const TArray<uint8>& ADDScene::EchantillonsDe(const Moteur::AudioClip& Clip)
{
    const int32 Id = Clip.GetInstanceID();
    if (const TArray<uint8>* Deja = Echantillons.Find(Id)) return *Deja;
    TArray<uint8>& Pcm = Echantillons.Add(Id);
    const Moteur::Liste<float>& D = Clip.Donnees();
    Pcm.SetNumZeroed(D.Count() * 2);
    int16* E = reinterpret_cast<int16*>(Pcm.GetData());
    for (int32 i = 0; i < D.Count(); i++) E[i] = static_cast<int16>(FMath::Clamp(FMath::RoundToInt(D[i] * 32767.0f), -32768, 32767));
    return Pcm;
}

void ADDScene::JouerPonctuel(const Moteur::AudioClip& Clip, float Volume, float Hauteur)
{
    const TArray<uint8>& Pcm = EchantillonsDe(Clip);
    if (Pcm.Num() == 0 || Volume <= 0.0001f) return;
    USoundWaveProcedural* Onde = NewObject<USoundWaveProcedural>(this);
    Onde->SetSampleRate(static_cast<uint32>(FMath::Max(1, Clip.frequency)), false);
    Onde->NumChannels = FMath::Max(1, Clip.channels);
    Onde->Duration = Clip.length();
    Onde->SoundGroup = SOUNDGROUP_Default;
    Onde->bLooping = false;
    Onde->QueueAudio(Pcm.GetData(), Pcm.Num());
    UAudioComponent* Son = UGameplayStatics::SpawnSound2D(this, Onde, Volume, Hauteur);
    if (Son == nullptr) return;                       // pas de périphérique audio
    Ponctuels.Add(Son);
    FinsPonctuels.Add(FPlatformTime::Seconds() + Clip.length() / FMath::Max(0.05f, Hauteur) + 0.25);
}

void ADDScene::ReproduireSons()
{
    // les sons ponctuels finis
    const double Maintenant = FPlatformTime::Seconds();
    for (int32 i = Ponctuels.Num() - 1; i >= 0; i--)
    {
        UAudioComponent* Son = Ponctuels[i].Get();
        if (IsValid(Son) && Maintenant < FinsPonctuels[i]) continue;
        if (IsValid(Son)) Son->Stop();               // détruit avec lui (bAutoDestroy)
        Ponctuels.RemoveAtSwap(i);
        FinsPonctuels.RemoveAtSwap(i);
    }

    TSet<int32> Vues;
    const Moteur::Liste<Moteur::Ref<Moteur::GameObject>>& Objets = Moteur::Boucle::Objets();
    for (int32 i = 0; i < Objets.Count(); i++)
    {
        const Moteur::Ref<Moteur::GameObject>& Go = Objets[i];
        if (Go == nullptr) continue;
        for (const Moteur::Ref<Moteur::Component>& C : Go->Composants())
        {
            const Moteur::Ref<Moteur::AudioSource> Source = Moteur::Comme<Moteur::AudioSource>(C);
            if (Source == nullptr) continue;
            const int32 Id = Source->GetInstanceID();
            Vues.Add(Id);
            const bool Active = Go->activeInHierarchy() && Source->enabled;
            const float VolumeSource = Source->mute ? 0.0f : Source->volume * Moteur::AudioListener::volume;

            for (const Moteur::AudioSource::SonPonctuel& S : Source->PrendreSonsPonctuels())
                if (Active && S.clip != nullptr) JouerPonctuel(*S.clip, S.volume * VolumeSource, S.pitch * Source->pitch);

            // la lecture continue (la musique)
            FDDSonContinu& Etat = EtatsContinus.FindOrAdd(Id);
            const bool Joue = Active && Source->isPlaying() && Source->clip != nullptr;
            const int32 IdClip = Joue ? Source->clip->GetInstanceID() : -1;
            if (Etat.Version != Source->VersionLecture() || Etat.Clip != IdClip)
            {
                Etat.Version = Source->VersionLecture();
                Etat.Clip = IdClip;
                Etat.Volume = -1.0f;
                if (UAudioComponent* Ancien = Continus.FindRef(Id).Get()) Ancien->Stop();
                OndesContinues.Remove(Id);
                if (Joue)
                {
                    const TArray<uint8>& Pcm = EchantillonsDe(*Source->clip);
                    USoundWaveProcedural* Onde = NewObject<USoundWaveProcedural>(this);
                    Onde->SetSampleRate(static_cast<uint32>(FMath::Max(1, Source->clip->frequency)), false);
                    Onde->NumChannels = FMath::Max(1, Source->clip->channels);
                    Onde->Duration = Source->loop ? INDEFINITELY_LOOPING_DURATION : Source->clip->length();
                    Onde->SoundGroup = SOUNDGROUP_Music;
                    Onde->bLooping = Source->loop;
                    if (Pcm.Num() > 0)
                    {
                        Onde->QueueAudio(Pcm.GetData(), Pcm.Num());
                        if (Source->loop) Onde->QueueAudio(Pcm.GetData(), Pcm.Num());
                    }
                    Etat.OctetsClip = Pcm.Num();
                    Etat.Boucle = Source->loop;
                    TObjectPtr<UAudioComponent>& Composant = Continus.FindOrAdd(Id);
                    if (Composant == nullptr)
                    {
                        Composant = NewObject<UAudioComponent>(this, NAME_None, RF_Transient);
                        Composant->bAutoActivate = false;
                        Composant->bAutoDestroy = false;
                        Composant->bAllowSpatialization = false;
                        Composant->bIsUISound = true;
                        Composant->SetupAttachment(Racine);
                        Composant->RegisterComponent();
                    }
                    OndesContinues.Add(Id, Onde);
                    Composant->SetSound(Onde);
                    Composant->SetVolumeMultiplier(FMath::Max(0.0001f, VolumeSource));
                    Composant->SetPitchMultiplier(FMath::Max(0.05f, Source->pitch));
                    Composant->Play();
                    Etat.Volume = VolumeSource;
                }
            }
            if (!Joue) continue;
            if (UAudioComponent* Composant = Continus.FindRef(Id).Get())
                if (Etat.Volume != VolumeSource)
                {
                    Composant->SetVolumeMultiplier(FMath::Max(0.0001f, VolumeSource));
                    Etat.Volume = VolumeSource;
                }
            // en boucle : toujours au moins un tour d'avance dans la file
            if (Etat.Boucle && Etat.OctetsClip > 0)
                if (USoundWaveProcedural* Onde = OndesContinues.FindRef(Id).Get())
                    if (Onde->GetAvailableAudioByteCount() < Etat.OctetsClip)
                    {
                        const TArray<uint8>& Pcm = EchantillonsDe(*Source->clip);
                        Onde->QueueAudio(Pcm.GetData(), Pcm.Num());
                    }
        }
    }

    // les sources que le jeu a détruites
    for (auto It = EtatsContinus.CreateIterator(); It; ++It)
    {
        if (Vues.Contains(It.Key())) continue;
        if (UAudioComponent* Composant = Continus.FindRef(It.Key()).Get()) Composant->DestroyComponent();
        Continus.Remove(It.Key());
        OndesContinues.Remove(It.Key());
        It.RemoveCurrent();
    }
}

void ADDScene::ArreterSons()
{
    for (auto& P : Continus)
        if (IsValid(P.Value)) P.Value->Stop();
    for (auto& Son : Ponctuels)
        if (IsValid(Son)) Son->Stop();
    Ponctuels.Empty();
    FinsPonctuels.Empty();
    OndesContinues.Empty();
    EtatsContinus.Empty();
    Echantillons.Empty();
}
