// Donjon de l'Oie — les règles du jeu, en six pages, et l'écran de victoire.
#include "EcranRegles.h"

#include "UIK.h"
#include "../Monde/IleModele.h"
#include "../Voxel/Pal.h"

namespace DonjonDeLOie::Interface
{
    using namespace DonjonDeLOie::Regles;
    using namespace DonjonDeLOie::Voxel;

    const Liste<string>& EcranRegles::Titres()
    {
        static const Liste<string> titres = {"Le but", "Un tour", "Combats", "Les cases", "Nuit et quêtes", "Objets", "Les héros"};
        return titres;
    }

    Liste<string> EcranRegles::Pages()
    {
        Liste<string> o;
        o.Add(string("<b>Premier au niveau 10, premier couronné</b> (le niveau à atteindre se règle dans le salon)")
            + (Donnees::Dragonnoie() != nullptr ? " — <b>à moins qu'un aventurier ne terrasse le Dragonnoie</b>." : ".") + "\n\n"
            + "Jusqu'à six aventuriers parcourent l'île de Valcreux, case par case, sur une grande boucle qui traverse le Bois des Murmures, "
            + "le Marais Putride, les Cryptes d'Ombrelune et les Pics de Braise. Trois raccourcis coupent la boucle : plus courts, "
            + "mais plus dangereux — le dernier passe par l'antre d'Ignathor, le dragon rouge.\n\n"
            + "On gagne de l'<b>expérience</b> (" + Texte::De(Donnees::XPParNiveau()) + " XP font un niveau) en battant des monstres, en accomplissant des quêtes, en "
            + "remportant des duels et à chaque passage par Valcreux (+1 XP et +" + Texte::De(Donnees::SoldeVille()) + " or).\n\n"
            + "On peut aussi <b>perdre des niveaux</b> : en perdant un duel contre un autre joueur, sur une case maudite, par une rune "
            + "d'oubli ou par un parchemin de malédiction. Rien n'est jamais joué !\n\n"
            + "Les places vides sont tenues par des bots, et plusieurs humains peuvent jouer sur le même ordinateur : on se passe la souris."
            + TexteDragonnoie());
        o.Add(string("<b>À votre tour</b>, avant de bouger, vous pouvez utiliser autant d'objets de votre sac que vous voulez : boire une potion, ")
            + "poser un piège, lire un parchemin. Puis vous faites l'une de ces deux choses :\n\n"
            + "<b>• Lancer le dé</b> (1D6) et avancer d'autant de cases. Aux embranchements, vous choisissez votre chemin. "
            + "Si vous passez sur un piège ou dans une embuscade, votre course s'arrête là.\n\n"
            + "<b>• Tendre une embuscade</b> : vous restez sur place, et votre case plus une case voisine de votre choix deviennent un "
            + "piège vivant jusqu'à votre prochain tour. Le premier joueur qui y entre est arrêté et doit vous affronter en duel, "
            + "et vous avez +2 grâce à la surprise.\n\n"
            + "<b>Arrivé sur une case</b>, si d'autres aventuriers s'y trouvent, vous pouvez en défier un. Puis la case fait son effet.\n\n"
            + "<b>K.O.</b> : à 0 PV, vous retournez à Valcreux avec la moitié de vos PV, et vous perdez la moitié de votre or.");
        o.Add(string("<b>Duel entre joueurs</b> : chacun lance son <b>attaque + 1D3</b>. Le plus haut l'emporte (en cas d'égalité, on relance ; ")
            + "à la troisième égalité, le défenseur tient bon). Le perdant <b>perd un niveau</b> — deux si l'écart atteint 5 — et le "
            + "vainqueur lui prend 5 or par point d'écart et gagne 1 XP. Le perdant est ensuite protégé des défis jusqu'à la fin de son prochain tour.\n\n"
            + "<b>Contre un monstre</b> : votre <b>attaque + 1D6</b> contre sa <b>force + 1D6</b> ; l'égalité vous sourit. Victoire : son or et son "
            + "expérience. Défaite : il vous inflige ses dégâts, moins votre armure. Vous pouvez toujours fuir (vous reculez d'une case).\n\n"
            + "<b>Votre attaque</b> = celle de votre classe + votre arme. Votre <b>défense</b> (classe + armure) réduit les dégâts des monstres et de la foudre.\n\n"
            + "<b>Ignathor</b>, le dragon rouge, garde son antre au cœur de l'île : force 12, dégâts 7, mais 80 or et 8 XP pour qui le terrasse.");
        string cases;
        for (int i = 0; i < Donnees::NomsTypesCase.Length(); i++)
        {
            Color c = Monde::IleModele::CouleurType(static_cast<TypeCase>(i));
            cases += UIK::Colorer("■ ", c) + "<b>" + Donnees::NomsTypesCase[i] + "</b> — " + (i == 0 ? "La cité de départ : boutique complète, soins gratuits et quêtes. +" + Texte::De(Donnees::SoldeVille()) + " or et +" + Texte::De(Donnees::XPVille()) + " XP de renommée à chaque passage." : Donnees::AidesTypesCase[i]) + "\n";
        }
        cases += "\n<b>Quêtes</b> à Valcreux, dans les tavernes ou auprès d'un ermite : voir l'onglet « Nuit et quêtes ».";
        o.Add(cases);
        o.Add(TexteNuit() + TexteQuetes());
        string obj;
        obj += "<b>Armes</b> : ";
        Lister(obj, Regles::TypeObjet::Arme); obj += "\n<b>Armures</b> : "; Lister(obj, Regles::TypeObjet::Armure);
        obj += "\n<b>Accessoires</b> (deux au plus) : "; Lister(obj, Regles::TypeObjet::Accessoire);
        obj += "\n<b>Potions</b> : "; Lister(obj, Regles::TypeObjet::Potion);
        obj += "\n<b>Pièges</b> (sur votre case ou une case voisine, trois au plus ; invisibles pour les autres) : "; Lister(obj, Regles::TypeObjet::Piege);
        obj += "\n<b>Parchemins</b> (sur n'importe quel joueur) : "; Lister(obj, Regles::TypeObjet::Parchemin);
        obj += "\n\nLe sac contient " + Texte::De(Donnees::TailleSac()) + " objets. L'<b>amulette de garde</b> bloque la prochaine malédiction, rune ou sort, puis se brise. "
            + "Les marchands changent d'étal tous les trois tours ; Valcreux vend de tout, y compris les pièces légendaires.";
        o.Add(obj);
        string h;
        for (auto& d : Donnees::Classes)
            h += "<b>" + d->Nom + "</b>  (attaque " + Texte::De(d->Attaque) + ", défense " + Texte::De(d->Defense) + ", " + Texte::De(d->Vie) + " PV)\n" + d->Passif + " <i>" + d->Description + "</i>\n\n";
        o.Add(h);
        return o;
    }

    /// Le Dragonnoie, avec les valeurs du catalogue (vide s'il est désactivé).
    string EcranRegles::TexteDragonnoie()
    {
        auto m = Donnees::Dragonnoie();
        if (m == nullptr) return "";
        int n = std::max(1, Donnees::Reglages->ToursEntreEnvols);
        return "\n\n<b>Le Dragonnoie</b>, mi-oie, mi-dragon, se pose au hasard sur l'île. Au début d'un tour de table sur " + Texte::De(n)
            + ", il prend une grande envolée vers une autre case et <b>défie tous ceux qui s'y trouvent</b>. Si vous <b>passez</b> sur sa case, "
            + "vous choisissez : l'attaquer (votre course s'arrête là) ou continuer votre route. Si vous vous <b>arrêtez</b> sur sa case, il faut l'affronter. "
            + "Combat comme contre un monstre (force " + Texte::De(m->Force) + ") : vaincu, il vous arrache <b>tout votre or</b> (jamais de niveau) ; "
            + "vainqueur, <b>vous gagnez la partie</b>.";
    }

    /// La nuit et ses événements, selon les réglages du catalogue.
    string EcranRegles::TexteNuit()
    {
        auto r = Donnees::Reglages;
        if (!r->NuitsActives) return "<b>La nuit</b> ne tombe jamais dans cette partie (réglage du catalogue).\n\n";
        string sb;
        int n = std::max(1, r->ToursEntreNuits);
        sb += "<b>La nuit tombe</b> à la fin de chaque " + (n == 1 ? string() : Texte::De(n) + "e ") + "tour de table (un encart sous la chronique dit "
            + "quand) ; un seul événement l'occupe, tiré parmi ceux qui peuvent avoir lieu :\n";
        if (r->NuitVoleur) sb += "  • <b>Le voleur</b> : un aventurier se fait voler la moitié de son or.\n";
        if (r->NuitTrouDeVer) sb += "  • <b>Le trou de ver</b> : deux aventuriers sur deux cases différentes échangent leur place.\n";
        if (r->NuitBonneNuit) sb += "  • <b>La bonne nuit</b> : un aventurier dort bien et gagne un niveau (jamais celui de la victoire).\n";
        if (r->NuitArene) sb += string("  • <b>L'arène du diable</b> : tous s'y affrontent en duels, dans un ordre tiré au sort ; le vainqueur reste et affronte ")
            + "le suivant. Chaque défaite coûte un niveau ; le dernier vainqueur gagne un niveau (même celui de la victoire) et " + Texte::De(r->OrArene) + " or.\n";
        sb += "\n";
        return sb;
    }

    string EcranRegles::TexteQuetes()
    {
        return string("<b>Les quêtes</b> — une seule à la fois ; en prendre une autre abandonne la première. À Valcreux, dans les tavernes ou auprès d'un ")
            + "ermite, jusqu'à quatre propositions : <b>un lieu</b> à atteindre (y passer suffit), <b>une chasse</b> ou des ennemis à vaincre, et deux "
            + "parmi <b>le guet-apens</b> (votre embuscade surprend un aventurier), <b>le défi</b> (provoquer en duel l'aventurier désigné, gagner ou perdre) "
            + "et <b>l'avis de recherche</b>.\n\n"
            + "<b>L'avis de recherche</b> se joue dans la ville qu'il nomme (Valcreux sur la carte d'origine) : dès que vous y êtes avec cette quête, "
            + "même ramené par un parchemin de rappel, le marché apparaît en diorama. L'évadé se cache "
            + "dans la foule, derrière des caisses, des tonneaux ou des sacs : seul le haut de sa tête dépasse. Comparez les visages à son portrait "
            + "(gare aux sosies !) et <b>cliquez sur lui</b> : vous avez " + Texte::De(Donnees::Reglages->DureeEvade) + " secondes, une erreur en coûte 3, la molette "
            + "zoome. Sinon, il s'enfuit et la quête est perdue.";
    }

    void EcranRegles::Lister(string& sb, Regles::TypeObjet t)
    {
        Liste<string> l;
        for (auto& o : Donnees::Objets) if (o->Type == t) l.Add(o->Nom + " (" + Texte::De(o->Prix) + " or" + (o->Bonus > 0 && (t == Regles::TypeObjet::Arme) ? ", +" + Texte::De(o->Bonus) : string()) + ")");
        sb += Texte::Joindre(", ", l) + ".";
    }

    EcranRegles::EcranRegles(const Ref<RectTransform>& racine, std::function<void()> fermer)
    {
        R = UIK::Remplir(UIK::Rect(racine, "Règles"));
        auto voile = UIK::Image(R, "Voile", UIK::Blanc(), Color(0, 0, 0, 0.5f));
        UIK::Remplir(voile->rectTransform());
        voile->raycastTarget = true;
        auto p = UIK::Placer(UIK::Panneau(R, "Panneau", Style::Parchemin), 0.5f, 0.5f, 0, 0, 1320, 860);
        auto bandeau = UIK::Placer(UIK::Panneau(p, "Titre", Style::Bois), 0.5f, 1.0f, 0, 28, 600, 66, 0.5f, 1.0f);
        titre = UIK::Texte(bandeau, "Texte", "Règles du jeu", 34, UIK::Or, TextAnchor::MiddleCenter, true);
        UIK::Remplir(titre->rectTransform());
        auto pages = Pages();
        for (int i = 0; i < Titres().Length(); i++)
        {
            int k = i;
            auto b = UIK::Bouton(p, "Onglet" + Texte::De(i), Titres()[i], Style::Bouton, [this, k, pages] { Page(k, pages); }, 20);
            UIK::Placer(Vers<RectTransform>(b->transform()), 0.5f, 1.0f, static_cast<float>(-540 + i * 180), -64, 170, 50, 0.5f, 1.0f);
            onglets.Add(b);
        }
        corps = UIK::Texte(p, "Corps", "", 21, UIK::Encre, TextAnchor::UpperLeft, false, false);
        UIK::Placer(corps->rectTransform(), 0.5f, 1.0f, 0, -136, 1210, 610, 0.5f, 1.0f);
        corps->lineSpacing = 1.12f;
        auto bf = UIK::Bouton(p, "Fermer", "Fermer", Style::BoutonOr, [fermer] { fermer(); }, 24);
        UIK::Placer(Vers<RectTransform>(bf->transform()), 0.5f, 0.0f, 0, 26, 260, 58, 0.5f, 0.0f);
        Page(0, pages);
        Visible(false);
    }

    void EcranRegles::Page(int i, const Liste<string>& pages)
    {
        corps->text(pages[i]);
        for (int k = 0; k < onglets.Count(); k++)
            onglets[k]->GetComponent<Image>()->color = k == i ? Color(0.92f, 0.7f, 0.3f) : Color(0.25f, 0.18f, 0.13f, 0.97f);
    }

    void EcranRegles::Visible(bool v) { R->gameObject()->SetActive(v); if (v) R->SetAsLastSibling(); }

    EcranVictoire::EcranVictoire(const Ref<RectTransform>& racine, const Ptr<Partie>& p, std::function<void()> rejouer, std::function<void()> menu)
    {
        R = UIK::Remplir(UIK::Rect(racine, "Victoire"));
        auto voile = UIK::Image(R, "Voile", UIK::Blanc(), Color(0, 0, 0, 0.45f));
        UIK::Remplir(voile->rectTransform());
        voile->raycastTarget = true;
        auto g = p->Joueurs[p->Gagnant];
        Color cg = Pal::Joueurs[g->Couleur];
        auto lueur = UIK::Image(R, "Lueur", UIK::Lueur(), Color(cg.r, cg.g, cg.b, 0.35f));
        UIK::Placer(lueur->rectTransform(), 0.5f, 1.0f, 0, -40, 1200, 300, 0.5f, 1.0f);
        auto t = UIK::Texte(R, "Titre", "VICTOIRE !", 96, UIK::Or, TextAnchor::MiddleCenter, true);
        UIK::Placer(t->rectTransform(), 0.5f, 1.0f, 0, -70, 1200, 120, 0.5f, 1.0f);
        auto ol = t->gameObject()->AddComponent<Outline>(); ol->effectColor = Color(0.25f, 0.1f, 0.02f); ol->effectDistance = Vector2(3, -3);
        string exploit = p->VaincuDragonnoie ? string("terrasse le Dragonnoie") : "atteint le niveau " + Texte::De(g->Niveau);
        auto n = UIK::Texte(R, "Gagnant", g->Nom + ", " + Texte::Minuscules(g->Def()->Nom) + ", " + exploit + " et devient la légende de Valcreux.", 32, cg, TextAnchor::MiddleCenter, true);
        UIK::Placer(n->rectTransform(), 0.5f, 1.0f, 0, -196, 1600, 50, 0.5f, 1.0f);
        n->gameObject()->AddComponent<Outline>()->effectColor = Color(0, 0, 0, 0.8f);
        auto tab = UIK::Placer(UIK::Panneau(R, "Tableau", Style::Bois), 0.5f, 0.5f, 0, -60, 1260, static_cast<float>(120 + p->Joueurs.Count() * 56));
        Liste<string> cols = {"Aventurier", "Niveau", "Monstres", "Duels", "Quêtes", "Or gagné", "Pièges", "K.O."};
        Liste<float> xs = {40, 420, 560, 700, 820, 940, 1070, 1180};
        for (int c = 0; c < cols.Length(); c++)
        {
            auto h = UIK::Texte(tab, "Col" + Texte::De(c), cols[c], 20, UIK::Or, c == 0 ? TextAnchor::MiddleLeft : TextAnchor::MiddleCenter, true);
            UIK::Placer(h->rectTransform(), 0, 1, xs[c], -40, c == 0 ? 360.0f : 120.0f, 30, c == 0 ? 0.0f : 0.5f, 0.5f);
        }
        Liste<Ptr<Joueur>> tri = p->Joueurs;
        tri.Sort([g](const Ptr<Joueur>& a, const Ptr<Joueur>& b) { return a == g ? -1 : b == g ? 1 : b->Niveau != a->Niveau ? Comparer(b->Niveau, a->Niveau) : Comparer(b->XP, a->XP); });
        for (int i = 0; i < tri.Count(); i++)
        {
            auto j = tri[i];
            float y = static_cast<float>(-92 - i * 56);
            Liste<string> v = {j->Nom + "  <size=16>(" + j->Def()->Nom + (j->Humain ? "" : ", bot") + ")</size>", Texte::De(j->Niveau), Texte::De(j->MonstresTues), Texte::De(j->DuelsGagnes) + " / " + Texte::De(j->DuelsGagnes + j->DuelsPerdus), Texte::De(j->QuetesFinies), Texte::De(j->OrGagne), Texte::De(j->PiegesDeclenches), Texte::De(j->KO)};
            for (int c = 0; c < v.Length(); c++)
            {
                auto tx = UIK::Texte(tab, "V" + Texte::De(i) + "_" + Texte::De(c), v[c], 22, c == 0 ? Color(Pal::Joueurs[j->Couleur]) : UIK::Creme, c == 0 ? TextAnchor::MiddleLeft : TextAnchor::MiddleCenter, c == 0);
                UIK::Placer(tx->rectTransform(), 0, 1, xs[c], y, c == 0 ? 360.0f : 120.0f, 40, c == 0 ? 0.0f : 0.5f, 0.5f);
            }
        }
        auto b1 = UIK::Bouton(R, "Rejouer", "Rejouer", Style::BoutonOr, [rejouer] { rejouer(); }, 28);
        UIK::Placer(Vers<RectTransform>(b1->transform()), 0.5f, 0.0f, -170, 60, 300, 70, 0.5f, 0.0f);
        auto b2 = UIK::Bouton(R, "Menu", "Menu principal", Style::Bouton, [menu] { menu(); }, 26);
        UIK::Placer(Vers<RectTransform>(b2->transform()), 0.5f, 0.0f, 170, 60, 300, 70, 0.5f, 0.0f);
    }

    void EcranVictoire::Detruire() { if (R != nullptr) Object::Destroy(R->gameObject()); }
}
