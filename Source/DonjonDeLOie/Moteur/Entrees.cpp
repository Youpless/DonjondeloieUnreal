// Donjon de l'Oie — la souris et le clavier.
#include "Entrees.h"

namespace Moteur
{
    Mouse* Mouse::current()
    {
        static Mouse m;
        return &m;
    }

    Keyboard* Keyboard::current()
    {
        static Keyboard k;
        return &k;
    }

    namespace
    {
        void Effacer(ButtonControl& b)
        {
            b.wasPressedThisFrame = false;
            b.wasReleasedThisFrame = false;
        }
    }

    void Entrees::DebutImage()
    {
        auto m = Mouse::current();
        Effacer(m->leftButton);
        Effacer(m->rightButton);
        Effacer(m->middleButton);
        m->scroll.valeur = Vector2::zero;
        m->delta.valeur = Vector2::zero;
        auto k = Keyboard::current();
        for (KeyControl* c : {&k->leftArrowKey, &k->rightArrowKey, &k->upArrowKey, &k->downArrowKey, &k->spaceKey, &k->escapeKey, &k->enterKey, &k->f1Key, &k->tabKey,
                              &k->qKey, &k->eKey, &k->backspaceKey})
            Effacer(*c);
        k->CaracteresTapes.clear();
    }

    void Entrees::PositionSouris(Vector2 p)
    {
        auto m = Mouse::current();
        m->delta.valeur = m->delta.valeur + (p - m->position.valeur);
        m->position.valeur = p;
    }

    void Entrees::Molette(float crans)
    {
        auto m = Mouse::current();
        m->scroll.valeur = Vector2(0.0f, m->scroll.valeur.y + crans);
    }
}
