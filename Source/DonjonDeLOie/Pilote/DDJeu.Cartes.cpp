// Donjon de l'Oie — les cartes jouables : celle de l'éditeur de cartes de la scène, celle choisie sur DDJeu, la carte
// d'origine, et toutes celles du dossier Resources/Cartes. Le salon propose de choisir ; l'île est reconstruite au besoin.
#include "DDJeu.h"

#include "../Cartes/DDCarte.h"
#include "../Cartes/DDEditeurCarte.h"
#include "../Monde/CameraRig.h"
#include "../Monde/IleModele.h"
#include "../Monde/IleVue.h"

namespace DonjonDeLOie::Pilote
{
    using namespace DonjonDeLOie::Regles;
    using namespace DonjonDeLOie::Monde;
    using namespace DonjonDeLOie::Cartes;

    void DDJeu::PreparerCartes()
    {
        cartes.Clear();
        // 1. la carte en cours d'édition dans cette scène
        auto ed = FindAnyObjectByType<DDEditeurCarte>();
        if (ed != nullptr)
        {
            Ajouter(ed->NomCarte + " (éditeur)", ed->Exporter(), ed->NiveauConseille);
            ed->gameObject()->SetActive(false);         // les marqueurs ne s'affichent pas en jeu
        }
        // 2. la carte choisie sur ce script
        if (Carte != nullptr) Ajouter(Texte::Vide(Carte->Carte->Nom) ? Carte->name() : Carte->Carte->Nom, Carte->Carte, Carte->NiveauConseille);
        // 3. la carte d'origine
        auto origine = Nouveau<CarteJouable>();
        origine->Nom = "Valcreux (carte d'origine)";
        origine->Def = nullptr;
        cartes.Add(origine);
        // 4. les cartes ajoutées au jeu
        for (auto& c : DDCarte::Toutes())
        {
            if (c == Carte) continue;
            Ajouter(Texte::Vide(c->Carte->Nom) ? c->name() : c->Carte->Nom, c->Carte, c->NiveauConseille);
        }
        carteParDefaut = 0;
    }

    void DDJeu::Ajouter(const string& nom, const Ptr<CarteDef>& def, int niveau)
    {
        Liste<string> erreurs;
        Liste<string> conseils;
        Regles::Plateau::Verifier(*def, erreurs, conseils);
        if (erreurs.Count() > 0)
        {
            Debug::LogError("Donjon de l'Oie : la carte « " + nom + " » n'est pas jouable :\n• " + Texte::Joindre("\n• ", erreurs));
            return;
        }
        for (auto& c : conseils) Debug::LogWarning("Carte « " + nom + " » : " + c);
        auto cj = Nouveau<CarteJouable>();
        cj->Nom = nom;
        cj->Def = def;
        cj->Niveau = Mathf::Clamp(niveau, 3, 10);
        cartes.Add(cj);
    }

    Liste<string> DDJeu::NomsCartes() const
    {
        Liste<string> l;
        for (auto& c : cartes) l.Add(c->Nom);
        return l;
    }

    Liste<int> DDJeu::NiveauxConseilles() const
    {
        Liste<int> l;
        for (auto& c : cartes) l.Add(c->Niveau);
        return l;
    }

    /// (Re)construit l'île d'une carte de la liste.
    void DDJeu::ConstruireIle(int index)
    {
        index = Mathf::Clamp(index, 0, cartes.Count() - 1);
        auto c = cartes[index];
        if (ile != nullptr) Destroy(ile->gameObject());
        auto plateau = c->Def == nullptr ? Regles::Plateau::Standard() : Regles::Plateau::Depuis(*c->Def);
        modele = Nouveau<IleModele>(plateau, c->Def);
        ile = IleVue::Construire(modele, monde);
        carteConstruite = index;
        cam->CentreCarte = Vector3(modele->X0 + modele->LargeurM() / 2.0f, 0.0f, modele->Z0 + modele->ProfondeurM() / 2.0f);
        cam->TailleCarte = Mathf::Max(modele->LargeurM(), modele->ProfondeurM()) - 24.0f;   // la terre, sans la bande de mer de la grille
    }
}
