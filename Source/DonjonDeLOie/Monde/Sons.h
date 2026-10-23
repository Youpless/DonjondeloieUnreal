// Donjon de l'Oie — sons et musique synthétisés au lancement (aucun fichier audio à importer).
// Bruitages : dé, pas, or, coup, niveau, malus, piège, sort, clic, victoire, monstre. Musique : une boucle au luth
// (cordes pincées, algorithme de Karplus-Strong) en ré dorien, sur un bourdon doux.
#pragma once

#include "../Moteur/Moteur.h"

namespace DonjonDeLOie::Monde
{
    enum class Son { De, Pas, Or, Coup, Niveau, Malus, Piege, Sort, Clic, Victoire, Monstre, Soin, Page };

    class Sons : public MonoBehaviour
    {
        DD_TYPE_OBJET(Sons, MonoBehaviour)
    public:
        static inline float VolumeEffets = 0.8f, VolumeMusique = 0.35f;

        static Ref<Sons> Installer(const Ref<GameObject>& hote);

        static void Jouer(Son s, float volume = 1.0f);

        static void Musique(bool oui);

        // (volEffets, volMusique : les paramètres « effets » et « musique » du C#, renommés pour ne pas masquer les champs.)
        static void ReglerVolumes(float volEffets, float volMusique);

    private:
        static constexpr int F = 44100;
        static inline Ref<Sons> instance;
        Ref<AudioSource> effets, musique;
        Dico<Son, Ref<AudioClip>> clips;
        AleaNet r{5};                                   // System.Random r = new System.Random(5)

        static Ref<AudioClip> Clip(const string& nom, const Liste<float>& d);

        float Bruit();

        static float Env(float t, float att, float dec);

        Liste<float> Tonalite(float duree, const std::function<float(float)>& f);

        static float Sin(float hz, float t);

        Liste<float> Corde(float hz, float duree, float amort = 0.996f, float brillance = 0.5f);

        void Generer();

        Liste<float> Arpege(const Liste<float>& notes, float ecart, float duree, float vol);

        /// Boucle de 32 mesures à 3/4, ~84 battements par minute, ré dorien : basse, arpèges, mélodie simple.
        Liste<float> Luth();

        static void Ajouter(Liste<float>& d, const Liste<float>& s, float t0, float vol);

        /// Remise des variables statiques à leur valeur de départ (Boucle::Reinitialiser) : inscrite dans le .cpp.
        static const bool remiseAZero;
    };
}
