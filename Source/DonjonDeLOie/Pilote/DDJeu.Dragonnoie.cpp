// Donjon de l'Oie — le Dragonnoie à l'écran : il se tient au bord de sa case (les pions restent visibles), prend ses
// grandes envolées en arc au-dessus de l'île (la caméra le suit), et arrache leur or aux vaincus.
#include "DDJeu.h"

#include "../Interface/Hud.h"
#include "../Interface/UIK.h"
#include "../Modeles/Catalogue.h"
#include "../Monde/Animateur.h"
#include "../Monde/CameraRig.h"
#include "../Monde/Etincelles.h"
#include "../Monde/IleModele.h"
#include "../Monde/PionVue.h"
#include "../Monde/RigVue.h"
#include "../Monde/Sons.h"

#include <cfloat>

namespace DonjonDeLOie::Pilote
{
    using namespace DonjonDeLOie::Regles;
    using namespace DonjonDeLOie::Monde;
    using namespace DonjonDeLOie::Modeles;
    using namespace DonjonDeLOie::Interface;

    /// Où il se tient sur une case : à 2,3 m du centre, du côté le plus plat et le plus loin de la mer, face à la case.
    Vector3 DDJeu::PositionOie(int c, Quaternion& regard) const
    {
        auto p = modele->Cases[c];
        auto k = partie->Plateau[c];
        auto suiv = k->Suivants.Count() > 0 ? modele->Cases[k->Suivants[0]] : p + Vector3::forward;
        auto prec = k->Precedents.Count() > 0 ? modele->Cases[k->Precedents[0]] : p - Vector3::forward;
        auto t = suiv - prec;
        t.y = 0.0f;
        t = t.sqrMagnitude() > 1e-4f ? t.normalized() : Vector3::forward;
        auto n = Vector3(-t.z, 0.0f, t.x);
        Vector3 meilleur = p;
        float score = FLT_MAX;
        for (float cote : {1.0f, -1.0f})
        {
            auto q = p + n * (2.3f * cote);
            float h = modele->HauteurEn(q.x, q.z);
            float s = Mathf::Abs(h - p.y) + (modele->Ellipse(q.x, q.z) > 0.95f ? 10.0f : 0.0f) + (Vector3::Dot(n * cote, cam->transform()->forward()) > 0.3f ? 0.5f : 0.0f);
            if (s < score)
            {
                score = s;
                meilleur = Vector3(q.x, Mathf::Max(h, p.y - 0.4f), q.z);
            }
        }
        auto d = p - meilleur;
        d.y = 0.0f;
        regard = Quaternion::LookRotation(d.sqrMagnitude() > 1e-4f ? d.normalized() : Vector3::back);
        return meilleur;
    }

    /// Une grande envolée (ou son arrivée sur l'île, De = -1) : un arc au-dessus de l'île, ailes battantes.
    Tache DDJeu::Envol(Ptr<SEnvol> s)
    {
        auto m = partie->Dragonnoie();
        if (m == nullptr) co_return;
        Quaternion regardFin;
        auto arrivee = PositionOie(s->Vers, regardFin);
        Vector3 depart;
        bool premier = oieVue == nullptr;
        if (premier)
        {
            oieVue = RigVue::Creer(Catalogue::Monstre(m->Modele()), transitoires, "Dragonnoie", true, 2.6f);
            oieAnim = Animateur::Sur(oieVue);
            auto dehors = arrivee - modele->Centre;
            dehors.y = 0.0f;
            dehors = dehors.sqrMagnitude() > 1.0f ? dehors.normalized() : Vector3::back;
            depart = arrivee + dehors * 45.0f + Vector3::up * 28.0f;     // il arrive du large
            oieVue->transform()->position(depart);
        }
        else depart = oieVue->transform()->position();

        hud->Banniere(premier ? string("Le Dragonnoie arrive !") : "Tour " + Texte::De(partie->Tour) + "  ·  le Dragonnoie s'envole !", UIK::Or);
        Sons::Jouer(Son::Monstre, 1.0f);
        oieAnim->Vitesse = 1.0f;
        oieAnim->EnMarche = true;                                    // battements rapides
        if (!premier)
        {
            oieAnim->Jouer(Geste::Victoire, 0.5f);                    // il déploie ses ailes avant de bondir
            fumees->Fumee(depart, Color(0.85f, 0.82f, 0.75f, 0.6f), 16, 1.2f);
            co_yield Attendre(0.4f);
        }
        cam->Suivre(oieVue->transform(), 26.0f);
        auto plat = arrivee - depart;
        plat.y = 0.0f;
        float distance = plat.magnitude();
        float duree = Mathf::Clamp(1.4f + distance / 22.0f, 1.8f, 3.4f) / V();
        float hauteur = premier ? 0.0f : Mathf::Clamp(7.0f + distance * 0.3f, 9.0f, 22.0f);
        float t = 0.0f;
        while (t < duree)
        {
            t += Time::deltaTime;
            float k = Mathf::Clamp01(t / duree);
            float doux = premier ? 1.0f - (1.0f - k) * (1.0f - k) : Mathf::SmoothStep(0.0f, 1.0f, k);
            auto pos = Vector3::Lerp(depart, arrivee, doux) + Vector3::up * (4.0f * k * (1.0f - k) * hauteur);
            auto dir = pos - oieVue->transform()->position();
            dir.y = 0.0f;
            if (dir.sqrMagnitude() > 1e-5f)
                oieVue->transform()->rotation(Quaternion::Slerp(oieVue->transform()->rotation(), Quaternion::LookRotation(dir.normalized()), 1.0f - Mathf::Exp(-Time::deltaTime * 6.0f)));
            oieVue->transform()->position(pos);
            co_yield nullptr;
        }
        oieVue->transform()->position(arrivee);
        oieAnim->EnMarche = false;
        // l'atterrissage : poussière, cri, ailes déployées, face à la case
        fumees->Fumee(arrivee, Color(0.8f, 0.74f, 0.62f, 0.75f), 26, 1.6f);
        etincelles->Explosion(arrivee + Vector3::up * 0.3f, Color(1.0f, 0.9f, 0.7f), 18, 3.0f);
        Sons::Jouer(Son::Coup, 0.9f);
        float r0 = 0.0f;
        auto rot0 = oieVue->transform()->rotation();
        while (r0 < 0.35f)
        {
            r0 += Time::deltaTime;
            oieVue->transform()->rotation(Quaternion::Slerp(rot0, regardFin, r0 / 0.35f));
            co_yield nullptr;
        }
        oieVue->transform()->rotation(regardFin);
        oieAnim->Jouer(Geste::Victoire, 1.1f);
        auto milieu = (modele->Cases[s->Vers] + arrivee) * 0.5f;
        cam->Regarder(milieu + Vector3::up * 1.2f, 17.0f, 40.0f);
        hud->Rafraichir();
        co_yield Attendre(premier ? 1.0f : 0.7f);
    }

    /// Le Dragonnoie arrache sa bourse au vaincu : les pièces volent jusqu'à lui.
    Tache DDJeu::OrArrache(Ptr<SEffet> e)
    {
        if (oieVue == nullptr || !pions.ContainsKey(e->Joueur)) co_return;
        auto p = pions.At(e->Joueur);
        auto bec = oieVue->transform()->position() + Vector3::up * (oieVue->Hauteur * 0.85f);
        etincelles->Trait(Tete(p->Joueur), bec, Color(1.0f, 0.82f, 0.3f), 36);
        etincelles->Pieces(oieVue->transform()->position(), 18);
        Sons::Jouer(Son::Or, 1.0f);
        if (oieAnim != nullptr) oieAnim->Jouer(Geste::Joie);
        p->Anim->Jouer(Geste::Touche);
        co_yield Attendre(0.7f);
    }

    /// Le Dragonnoie a été retiré pendant la partie (catalogue appliqué en jeu, « Dragonnoie actif » décoché) : il quitte l'île.
    void DDJeu::VerifierOie()
    {
        if (oieVue == nullptr || partie == nullptr || partie->Dragonnoie() != nullptr) return;
        fumees->Fumee(oieVue->transform()->position(), Color(0.85f, 0.82f, 0.75f, 0.6f), 16, 1.2f);
        if (monstreVue == oieVue)
        {
            monstreVue = nullptr;
            monstreAnim = nullptr;
            monstrePersistant = false;
        }
        Destroy(oieVue->gameObject());
        oieVue = nullptr;
        oieAnim = nullptr;
    }

    /// Clic sur l'encart du Dragonnoie : la caméra va le voir.
    void DDJeu::MontrerOie()
    {
        if (oieVue == nullptr) return;
        cam->Regarder(oieVue->transform()->position() + Vector3::up * 1.2f, 15.0f, 38.0f);
    }
}
