// Donjon de l'Oie — la racine d'une carte dans la scène de l'éditeur : ses réglages, sa case de départ, et la conversion
// des marqueurs (enfants DDCaseMarqueur et DDDecorMarqueur) en une carte de jeu (CarteDef).
// En jeu, DDJeu lit la carte de cette racine s'il en trouve une dans la scène, puis la masque.
#include "DDEditeurCarte.h"

namespace DonjonDeLOie::Cartes
{
    Liste<Ref<DDCaseMarqueur>> DDEditeurCarte::Cases() const
    {
        Liste<Ref<DDCaseMarqueur>> l = GetComponentsInChildren<DDCaseMarqueur>(true);
        return l;
    }

    /// La carte de jeu décrite par les marqueurs. Les positions sont celles de la scène (en mètres, sur le plan horizontal).
    Ptr<Regles::CarteDef> DDEditeurCarte::Exporter() const
    {
        auto cases = Cases();
        auto d = Nouveau<Regles::CarteDef>();
        d->Nom = NomCarte; d->Marge = Marge; d->DecorAutomatique = DecorAutomatique; d->DensiteDecor = DensiteDecor;
        d->LieuxAutomatiques = LieuxAutomatiques; d->Graine = Graine;
        Dico<Ref<DDCaseMarqueur>, int> index;
        for (int i = 0; i < cases.Count(); i++) index[cases[i]] = i;
        for (int i = 0; i < cases.Count(); i++)
        {
            auto m = cases[i];
            auto p = m->transform()->position();
            auto k = Nouveau<Regles::CaseCarte>();
            k->Nom = m->NomLieu; k->Type = m->Type; k->Zone = m->Zone; k->X = p.x; k->Z = p.z; k->Voie = m->Voie;
            k->Mode = m->Mode; k->Lieu = m->Lieu;
            for (auto& s : m->Suivants)
            {
                int j = 0;
                if (s != nullptr && index.TryGetValue(s, j) && !k->Suivants.Contains(j)) k->Suivants.Add(j);
            }
            for (auto& e : m->Evenements)
                if (!Texte::Vide(e)) k->Evenements.Add(Texte::Rogner(e));
            d->Cases.Add(k);
        }
        int dep = 0;
        d->Depart = Depart != nullptr && index.TryGetValue(Depart, dep) ? dep : 0;
        for (auto& dm : GetComponentsInChildren<DDDecorMarqueur>(true))
        {
            auto p = dm->transform()->position();
            auto dp = Nouveau<Regles::DecorPose>();
            dp->Id = dm->Id; dp->Graine = dm->Graine; dp->X = p.x; dp->Z = p.z; dp->RotY = dm->transform()->eulerAngles().y; dp->Echelle = dm->transform()->localScale.x;
            d->Decors.Add(dp);
        }
        return d;
    }

    /// Crée les marqueurs d'une carte de jeu sous cette racine (cases puis décors). Rend les objets créés,
    /// pour que l'éditeur puisse les inscrire dans l'historique d'annulation.
    Liste<Ref<GameObject>> DDEditeurCarte::Importer(const Ptr<Regles::CarteDef>& d)
    {
        Liste<Ref<GameObject>> crees;
        NomCarte = Texte::Vide(d->Nom) ? NomCarte : d->Nom;
        Marge = d->Marge; DecorAutomatique = d->DecorAutomatique; DensiteDecor = d->DensiteDecor;
        LieuxAutomatiques = d->LieuxAutomatiques; Graine = d->Graine;
        auto cases = Dossier("Cases");
        Liste<Ref<DDCaseMarqueur>> marqueurs;
        for (int i = 0; i < d->Cases.Count(); i++)
        {
            auto k = d->Cases[i];
            auto go = Neuf<GameObject>(NomMarqueur(i, k->Type, k->Nom));
            go->transform()->SetParent(cases, false);
            go->transform()->position(Vector3(k->X, 0.0f, k->Z));
            auto m = go->AddComponent<DDCaseMarqueur>();
            m->Type = k->Type; m->Zone = k->Zone; m->NomLieu = k->Nom; m->Voie = k->Voie; m->Mode = k->Mode;
            m->Evenements = k->Evenements; m->Lieu = k->Lieu;
            marqueurs.Add(m); crees.Add(go);
        }
        for (int i = 0; i < d->Cases.Count(); i++)
            for (int j : d->Cases[i]->Suivants)
                if (j >= 0 && j < marqueurs.Count()) marqueurs[i]->Suivants.Add(marqueurs[j]);
        if (d->Depart >= 0 && d->Depart < marqueurs.Count()) Depart = marqueurs[d->Depart];
        if (d->Decors.Count() > 0)
        {
            auto decors = Dossier("Décors");
            for (auto& dp : d->Decors)
            {
                auto go = Neuf<GameObject>("Décor " + dp->Id);
                go->transform()->SetParent(decors, false);
                go->transform()->position(Vector3(dp->X, 0.0f, dp->Z));
                go->transform()->rotation(Quaternion::Euler(0.0f, dp->RotY, 0.0f));
                go->transform()->localScale = Vector3::one * (dp->Echelle <= 0.0f ? 1.0f : dp->Echelle);
                auto m = go->AddComponent<DDDecorMarqueur>();
                m->Id = dp->Id; m->Graine = dp->Graine;
                crees.Add(go);
            }
        }
        return crees;
    }

    /// Le sous-dossier (objet vide) « Cases » ou « Décors » de la racine, créé au besoin.
    Ref<Transform> DDEditeurCarte::Dossier(const string& nom)
    {
        auto t = transform()->Find(nom);
        if (t != nullptr) return t;
        auto go = Neuf<GameObject>(nom);
        go->transform()->SetParent(transform(), false);
        return go->transform();
    }

    string DDEditeurCarte::NomMarqueur(int i, Regles::TypeCase t, const string& nom)
    {
        return "Case " + Texte::Deux(i + 1) + " · " + (Texte::Vide(nom) ? Regles::Donnees::NomsTypesCase[static_cast<int>(t)] : nom);
    }
}
