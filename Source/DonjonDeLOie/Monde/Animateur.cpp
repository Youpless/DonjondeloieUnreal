// Donjon de l'Oie — animation procédurale des rigs voxel : respiration, marche, battements d'ailes, gestes (attaque,
// coup reçu, victoire, K.O.). Chaque pièce est retrouvée par son nom ; celles qui manquent sont ignorées.
#include "Animateur.h"

namespace DonjonDeLOie::Monde
{
    Ref<Animateur> Animateur::Sur(const Ref<RigVue>& rv)
    {
        auto a = rv->gameObject()->AddComponent<Animateur>();
        a->Rig = rv;
        a->Graine = Random::value() * 10.0f;
        a->Prendre();
        return a;
    }

    void Animateur::Prendre()
    {
        corps = Rig->Piece("corps"); tete = Rig->Piece("tete"); brasG = Rig->Piece("brasG"); brasD = Rig->Piece("brasD");
        jambeG = Rig->Piece("jambeG"); jambeD = Rig->Piece("jambeD"); arme = Rig->Piece("arme"); cape = Rig->Piece("cape");
        machoire = Rig->Piece("machoire"); aileG = Rig->Piece("aileG"); aileD = Rig->Piece("aileD"); queue = Rig->Piece("queue");
        avG = Rig->Piece("pattesAvG"); avD = Rig->Piece("pattesAvD"); arG = Rig->Piece("pattesArG"); arD = Rig->Piece("pattesArD");
        feuillage = Rig->Piece("feuillage"); bouclier = Rig->Piece("bouclier");
        echelle = Mathf::Clamp(Rig->Hauteur / 1.3f, 0.4f, 3.0f);
        AilesActives = aileG != nullptr && aileD != nullptr;
    }

    void Animateur::Jouer(Geste g, float duree)
    {
        geste = g; tGeste = 0.0f;
        koTenu = g == Geste::Ko;
        switch (g)
        {
            case Geste::Attaque: dureeGeste = 0.75f; break;
            case Geste::Touche: dureeGeste = 0.5f; break;
            case Geste::Victoire: dureeGeste = 1.6f; break;
            case Geste::Ko: dureeGeste = 0.6f; break;
            case Geste::Joie: dureeGeste = 0.9f; break;
            case Geste::Sort: dureeGeste = 0.9f; break;
            case Geste::Lancer: dureeGeste = 0.5f; break;
            default: dureeGeste = 0.0f; break;
        }
        if (duree > 0.0f) dureeGeste = duree;
    }

    void Animateur::Relever()
    {
        koTenu = false;
        geste = Geste::Aucun;
    }

    Quaternion Animateur::E(float x, float y, float z) { return Quaternion::Euler(x, y, z); }

    void Animateur::Poser(const Ref<Transform>& p, Quaternion rot, Vector3 decalage)
    {
        if (p == nullptr) return;
        p->localRotation = Rig->Repos.At(p->name()) * rot;
        p->localPosition = Rig->PositionsRepos.At(p->name()) + decalage;
    }

    void Animateur::LateUpdate()
    {
        if (Rig == nullptr) return;
        float dt = Time::deltaTime * Vitesse;
        t += dt;
        if (EnMarche) marche += dt * 9.0f; else marche = Mathf::MoveTowards(marche, Mathf::Round(marche / Mathf::Pi) * Mathf::Pi, dt * 6.0f);
        if (geste != Geste::Aucun)
        {
            tGeste += dt;
            if (tGeste >= dureeGeste && !koTenu) geste = Geste::Aucun;
        }
        float g = dureeGeste > 0.0f ? Mathf::Clamp01(tGeste / dureeGeste) : 0.0f;

        // ---- base : respiration et marche
        float souffle = Mathf::Sin(t * 2.1f + Graine);
        float pas = Mathf::Sin(marche);
        float amp = EnMarche ? 1.0f : 0.0f;
        Vector3 dCorps = Vector3(0.0f, (Flotte ? Mathf::Sin(t * 1.6f + Graine) * 0.06f : souffle * 0.008f) * echelle, 0.0f);
        Quaternion rCorps = E(EnMarche ? 6.0f : souffle * 0.6f, 0.0f, Flotte ? Mathf::Sin(t * 1.1f) * 3.0f : 0.0f);
        Quaternion rTete = E(Mathf::Sin(t * 0.9f + Graine) * 3.0f, Mathf::Sin(t * 0.47f + Graine * 2.0f) * 10.0f, 0.0f);
        Quaternion rBrasG = E(-pas * 28.0f * amp + souffle * 2.5f, 0.0f, -3.0f - souffle);
        Quaternion rBrasD = E(pas * 28.0f * amp + souffle * 2.5f, 0.0f, 3.0f + souffle);
        Quaternion rJambeG = E(pas * 32.0f * amp, 0.0f, 0.0f), rJambeD = E(-pas * 32.0f * amp, 0.0f, 0.0f);
        Quaternion rAv = E(pas * 26.0f * amp + souffle, 0.0f, 0.0f), rAr = E(-pas * 26.0f * amp - souffle, 0.0f, 0.0f);
        float battement = AilesActives ? (EnMarche ? 35.0f : 12.0f) * Mathf::Sin(t * (EnMarche ? 9.0f : 2.4f)) + (EnMarche ? 25.0f : 6.0f) : 0.0f;
        Quaternion rAileG = E(0.0f, 0.0f, -battement), rAileD = E(0.0f, 0.0f, battement);   // aile gauche (côté −x) : un angle négatif la lève
        Quaternion rQueue = E(0.0f, Mathf::Sin(t * 1.4f + Graine) * 14.0f, 0.0f);
        Quaternion rMach = E(Mathf::Max(0.0f, Mathf::Sin(t * 0.8f + Graine)) * 6.0f, 0.0f, 0.0f);
        Quaternion rCape = E(8.0f + Mathf::Sin(t * 2.7f + Graine) * 5.0f + amp * 18.0f, 0.0f, 0.0f);
        Quaternion rArme = Quaternion::identity;
        Vector3 dCorpsGeste = Vector3::zero;

        // ---- gestes
        switch (geste)
        {
            case Geste::Attaque:
            {
                // armer (0–40 %), frapper (40–60 %), revenir
                float arm = Mathf::SmoothStep(0.0f, 1.0f, g / 0.4f), frappe = Mathf::Clamp01((g - 0.4f) / 0.2f), retour = Mathf::Clamp01((g - 0.6f) / 0.4f);
                float bras = Mathf::Lerp(Mathf::Lerp(0.0f, -150.0f, arm), 30.0f, frappe);
                bras = Mathf::Lerp(bras, 0.0f, retour);
                rBrasD = E(bras, 0.0f, 6.0f);
                rBrasG = E(Mathf::Lerp(0.0f, -40.0f, arm) * (1.0f - retour), 0.0f, -6.0f);
                float avance = (frappe - retour) * 0.28f * echelle;
                dCorpsGeste = Vector3(0.0f, 0.0f, avance);
                rCorps = E(Mathf::Lerp(-8.0f * arm, 14.0f, frappe) * (1.0f - retour), Mathf::Lerp(-12.0f * arm, 10.0f, frappe) * (1.0f - retour), 0.0f);
                rMach = E(Mathf::Lerp(0.0f, 38.0f, Mathf::Max(arm * 0.6f, frappe)) * (1.0f - retour), 0.0f, 0.0f);
                rTete = rTete * E(Mathf::Lerp(-10.0f * arm, 18.0f, frappe) * (1.0f - retour), 0.0f, 0.0f);
                rAv = E(-30.0f * frappe * (1.0f - retour), 0.0f, 0.0f);
                if (AilesActives)
                {
                    rAileG = E(0.0f, 0.0f, -55.0f * arm * (1.0f - retour));
                    rAileD = E(0.0f, 0.0f, 55.0f * arm * (1.0f - retour));
                }
                break;
            }
            case Geste::Touche:
            {
                float k = Mathf::Sin(g * Mathf::Pi);
                dCorpsGeste = Vector3(0.0f, 0.0f, -0.18f * echelle * k);
                rCorps = E(-16.0f * k, 0.0f, 6.0f * k);
                rTete = rTete * E(-14.0f * k, 0.0f, 0.0f);
                rBrasG = E(-30.0f * k, 0.0f, -20.0f * k); rBrasD = E(-30.0f * k, 0.0f, 20.0f * k);
                break;
            }
            case Geste::Victoire:
            {
                float saut = Mathf::Abs(Mathf::Sin(g * Mathf::Pi * 2.0f));
                dCorpsGeste = Vector3(0.0f, saut * 0.22f * echelle, 0.0f);
                rBrasD = E(-165.0f, 0.0f, 12.0f); rBrasG = E(-165.0f * Mathf::Min(1.0f, g * 3.0f), 0.0f, -12.0f);
                rCorps = E(-6.0f, Mathf::Sin(g * Mathf::Pi * 4.0f) * 8.0f, 0.0f);
                if (AilesActives)
                {
                    rAileG = E(0.0f, 0.0f, -70.0f);
                    rAileD = E(0.0f, 0.0f, 70.0f);
                }
                break;
            }
            case Geste::Joie:
            {
                float saut = Mathf::Abs(Mathf::Sin(g * Mathf::Pi * 2.0f));
                dCorpsGeste = Vector3(0.0f, saut * 0.15f * echelle, 0.0f);
                rBrasD = E(-120.0f * saut, 0.0f, 10.0f); rBrasG = E(-120.0f * saut, 0.0f, -10.0f);
                break;
            }
            case Geste::Sort:
            {
                float k = Mathf::Sin(Mathf::Clamp01(g * 1.2f) * Mathf::Pi);
                rBrasD = E(-100.0f * k, -10.0f * k, 0.0f); rBrasG = E(-100.0f * k, 10.0f * k, 0.0f);
                rCorps = E(-6.0f * k, 0.0f, 0.0f);
                dCorpsGeste = Vector3(0.0f, 0.06f * k * echelle, 0.0f);
                break;
            }
            case Geste::Lancer:
            {
                float k = Mathf::Sin(g * Mathf::Pi);
                rBrasD = E(-70.0f * k, 0.0f, 10.0f);
                rCorps = E(8.0f * k, 0.0f, 0.0f);
                break;
            }
            case Geste::Ko:
            {
                float k = Mathf::SmoothStep(0.0f, 1.0f, g);
                rCorps = E(-82.0f * k, 0.0f, 10.0f * k);
                dCorpsGeste = Vector3(0.0f, -0.25f * echelle * k * (Rig->Hauteur > 0.0f ? 1.0f : 0.0f), -0.1f * k);
                rBrasD = E(-60.0f * k, 0.0f, 40.0f * k); rBrasG = E(-60.0f * k, 0.0f, -40.0f * k);
                rTete = E(-20.0f * k, 25.0f * k, 0.0f);
                break;
            }
            case Geste::Aucun: break;
        }

        Poser(corps, rCorps, dCorps + dCorpsGeste);
        Poser(tete, rTete, Vector3::zero);
        Poser(brasG, rBrasG, Vector3::zero);
        Poser(brasD, rBrasD, Vector3::zero);
        Poser(jambeG, rJambeG, Vector3::zero);
        Poser(jambeD, rJambeD, Vector3::zero);
        Poser(avG, rAv, Vector3::zero); Poser(avD, rAr, Vector3::zero);
        Poser(arG, rAr, Vector3::zero); Poser(arD, rAv, Vector3::zero);
        Poser(aileG, rAileG, Vector3::zero); Poser(aileD, rAileD, Vector3::zero);
        Poser(queue, rQueue, Vector3::zero);
        Poser(machoire, rMach, Vector3::zero);
        Poser(cape, rCape, Vector3::zero);
        Poser(arme, rArme, Vector3::zero);
        Poser(bouclier, Quaternion::identity, Vector3::zero);
        if (feuillage != nullptr)
            Poser(feuillage, E(Mathf::Sin(t * 0.9f + Graine) * 1.6f, 0.0f, Mathf::Sin(t * 0.7f + Graine * 1.7f) * 2.2f), Vector3::zero);
    }
}
