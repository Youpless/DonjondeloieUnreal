// Donjon de l'Oie — lecture de la souris et du clavier, avec le nouveau système d'entrées d'Unity 6 (par défaut)
// ou l'ancien gestionnaire, selon le réglage « Active Input Handling » du projet.
// (Portage : la branche ENABLE_INPUT_SYSTEM seule ; le moteur imite Mouse.current et Keyboard.current.)
#include "DDInput.h"

namespace DonjonDeLOie::Pilote
{
    const bool DDInput::remiseAZero = Boucle::AuReinitialiser([] {
        image = -1;
        dernierePos = Vector2::zero;
        delta = Vector2::zero;
    });

    void DDInput::MettreAJour()
    {
        if (image == Time::frameCount) return;
        image = Time::frameCount;
        auto p = PositionSouris();
        delta = p - dernierePos;
        dernierePos = p;
    }

    Vector2 DDInput::PositionSouris()
    {
        return Mouse::current() != nullptr ? Mouse::current()->position.ReadValue() : Vector2::zero;
    }

    Vector2 DDInput::DeplacementSouris()
    {
        return Mouse::current() != nullptr ? Mouse::current()->delta.ReadValue() : Vector2::zero;
    }

    /// Crans de molette depuis l'image précédente (positif : vers l'avant).
    float DDInput::Molette()
    {
        if (Mouse::current() == nullptr) return 0.0f;
        float v = Mouse::current()->scroll.ReadValue().y;
        return Mathf::Abs(v) > 5.0f ? v / 120.0f : v;
    }

    bool DDInput::BoutonDroitMaintenu()
    {
        MettreAJour();
        return Mouse::current() != nullptr && Mouse::current()->rightButton.isPressed;
    }

    bool DDInput::BoutonMilieuMaintenu()
    {
        MettreAJour();
        return Mouse::current() != nullptr && Mouse::current()->middleButton.isPressed;
    }

    bool DDInput::ClicGauche()
    {
        return Mouse::current() != nullptr && Mouse::current()->leftButton.wasPressedThisFrame;
    }

    bool DDInput::Touche(KeyCodeDD k)
    {
        auto kb = Keyboard::current();
        if (kb == nullptr) return false;
        auto c = Controle(kb, k);
        return c != nullptr && c->isPressed;
    }

    bool DDInput::ToucheAppuyee(KeyCodeDD k)
    {
        auto kb = Keyboard::current();
        if (kb == nullptr) return false;
        auto c = Controle(kb, k);
        return c != nullptr && c->wasPressedThisFrame;
    }

    KeyControl* DDInput::Controle(Keyboard* kb, KeyCodeDD k)
    {
        switch (k)
        {
            case KeyCodeDD::Gauche: return &kb->leftArrowKey;
            case KeyCodeDD::Droite: return &kb->rightArrowKey;
            case KeyCodeDD::Espace: return &kb->spaceKey;
            case KeyCodeDD::Echap: return &kb->escapeKey;
            case KeyCodeDD::Entree: return &kb->enterKey;
            case KeyCodeDD::F1: return &kb->f1Key;
            case KeyCodeDD::Tab: return &kb->tabKey;
        }
        return nullptr;
    }

    // (La fonction Legacy(KeyCodeDD) de la version C# n'existe que sans ENABLE_INPUT_SYSTEM : pas de portage.)
}
