// Donjon de l'Oie — la caméra : suit le joueur actif en vue plongeante, cadre les combats, montre tout le plateau.
// Molette : zoom · clic droit glissé (ou flèches gauche / droite) : tourner · clic milieu glissé : déplacer · Espace : vue d'ensemble.
#include "CameraRig.h"

#include "../Pilote/DDInput.h"

namespace DonjonDeLOie::Monde
{
    Ref<CameraRig> CameraRig::Creer(const Ref<Transform>& parent)
    {
        auto go = Neuf<GameObject>("Caméra");
        go->transform()->SetParent(parent, false);
        auto cam = go->AddComponent<Camera>();
        cam->fieldOfView = ChampNormal;
        cam->nearClipPlane = 0.2f;
        cam->farClipPlane = 600.0f;
        cam->allowHDR = true;
        cam->clearFlags = CameraClearFlags::SolidColor;
        go->tag = "MainCamera";
        go->AddComponent<AudioListener>();
        auto r = go->AddComponent<CameraRig>();
        r->Cam = cam;
        return r;
    }

    void CameraRig::Suivre(const Ref<Transform>& t, float distance)
    {
        suivi = t; decalageLibre = Vector3::zero; vueEnsemble_ = false;
        distanceVoulue = distance; tangageVoulu = 50.0f;
    }

    void CameraRig::Regarder(Vector3 p, float distance, float tangage)
    {
        suivi = nullptr; decalageLibre = Vector3::zero; vueEnsemble_ = false;
        cibleVoulue = p; distanceVoulue = distance; tangageVoulu = tangage;
    }

    /// Cadre deux points (duel, combat contre un monstre), vue un peu plus rasante.
    void CameraRig::Cadrer(Vector3 a, Vector3 b)
    {
        suivi = nullptr; decalageLibre = Vector3::zero; vueEnsemble_ = false;
        cibleVoulue = (a + b) * 0.5f + Vector3::up * 0.6f;
        distanceVoulue = Mathf::Clamp(Vector3::Distance(a, b) * 1.6f + 5.5f, 6.5f, 12.0f);
        tangageVoulu = 32.0f;
        // tourner pour voir les deux de profil
        auto d = b - a; d.y = 0.0f;
        if (d.sqrMagnitude() > 0.01f)
        {
            float cap = Mathf::Atan2(d.x, d.z) * Mathf::Rad2Deg - 90.0f;
            // choisir le côté le plus proche du lacet actuel
            if (Mathf::Abs(Mathf::DeltaAngle(lacetVoulu, cap)) > 90.0f) cap += 180.0f;
            lacetVoulu = cap;
        }
    }

    /// Place la caméra exactement, champ compris (une mise en scène image par image : le zoom du mini-jeu de l'évadé).
    void CameraRig::Fixer(Vector3 cible, float distance, float lacet, float tangage, float champ)
    {
        suivi = nullptr; decalageLibre = Vector3::zero; vueEnsemble_ = false;
        Cible = cibleVoulue = cible; Distance = distanceVoulue = distance;
        Lacet = lacetVoulu = lacet; Tangage = tangageVoulu = tangage;
        Cam->fieldOfView = champVoulu = champ;
        Appliquer();
    }

    /// Le champ de vision redevient l'habituel, tout de suite.
    void CameraRig::RetablirChamp() { Cam->fieldOfView = champVoulu = ChampNormal; }

    /// Fait tourner lentement la caméra (écran titre).
    void CameraRig::Orbiter(float degresParSeconde) { lacetVoulu += degresParSeconde * Time::unscaledDeltaTime; }

    void CameraRig::BasculerEnsemble()
    {
        if (vueEnsemble_) { vueEnsemble_ = false; distanceVoulue = distanceAvantEnsemble; tangageVoulu = 50.0f; }
        else { distanceAvantEnsemble = distanceVoulue; vueEnsemble_ = true; distanceVoulue = TailleCarte * 0.97f; tangageVoulu = 56.0f; }
    }

    void CameraRig::Placer()
    {
        Cible = cibleVoulue; Distance = distanceVoulue; Lacet = lacetVoulu; Tangage = tangageVoulu;
        Appliquer();
    }

    void CameraRig::LateUpdate()
    {
        if (CommandesActives)
        {
            float molette = Pilote::DDInput::Molette();
            if (Mathf::Abs(molette) > 0.01f)
            {
                distanceVoulue = Mathf::Clamp(distanceVoulue * (1.0f - molette * 0.12f), 5.0f, Mathf::Max(75.0f, TailleCarte * 1.15f));
                if (vueEnsemble_ && distanceVoulue < TailleCarte * 0.5f) vueEnsemble_ = false;
            }
            if (Pilote::DDInput::BoutonDroitMaintenu()) lacetVoulu += Pilote::DDInput::DeplacementSouris().x * 0.25f;
            if (Pilote::DDInput::Touche(Pilote::KeyCodeDD::Gauche)) lacetVoulu += 70.0f * Time::unscaledDeltaTime;
            if (Pilote::DDInput::Touche(Pilote::KeyCodeDD::Droite)) lacetVoulu -= 70.0f * Time::unscaledDeltaTime;
            if (Pilote::DDInput::BoutonMilieuMaintenu())
            {
                auto dm = Pilote::DDInput::DeplacementSouris() * (Distance * 0.0016f);
                auto q = Quaternion::Euler(0.0f, Lacet, 0.0f);
                decalageLibre -= q * Vector3(dm.x, 0.0f, dm.y);
            }
            if (Pilote::DDInput::ToucheAppuyee(Pilote::KeyCodeDD::Espace)) BasculerEnsemble();
        }
        if (vueEnsemble_) cibleVoulue = CentreCarte;
        else if (suivi != nullptr) cibleVoulue = suivi->position() + Vector3::up * 0.6f;
        float dt = Mathf::Min(Time::unscaledDeltaTime, 0.05f);
        Cible = Vector3::Lerp(Cible, cibleVoulue + decalageLibre, 1.0f - Mathf::Exp(-dt * 4.5f));
        Distance = Mathf::Lerp(Distance, distanceVoulue, 1.0f - Mathf::Exp(-dt * 3.5f));
        Lacet = Mathf::LerpAngle(Lacet, lacetVoulu, 1.0f - Mathf::Exp(-dt * 4.0f));
        Tangage = Mathf::Lerp(Tangage, tangageVoulu, 1.0f - Mathf::Exp(-dt * 3.0f));
        if (Mathf::Abs(Cam->fieldOfView - champVoulu) > 0.01f) Cam->fieldOfView = Mathf::Lerp(Cam->fieldOfView, champVoulu, 1.0f - Mathf::Exp(-dt * 3.5f));
        Appliquer();
    }

    void CameraRig::Appliquer()
    {
        auto q = Quaternion::Euler(Tangage, Lacet, 0.0f);
        transform()->position(Cible - q * Vector3::forward * Distance);
        transform()->rotation(q);
    }

    /// Position écran d'un point du monde (pour les bulles et les étiquettes). z < 0 : derrière la caméra.
    Vector3 CameraRig::Ecran(Vector3 p) const { return Cam->WorldToScreenPoint(p); }
}
