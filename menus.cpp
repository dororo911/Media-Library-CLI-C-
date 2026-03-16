#include "menus.h"
#include "utils.h"

#include <iostream>
#include <algorithm>
#include <cctype>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

static shared_ptr<Utilisateur> refreshUser(shared_ptr<Utilisateur> sessionUser, GestionUtilisateurs &gestionUsers)
{
    if (!sessionUser) return nullptr;
    auto u = gestionUsers.getUtilisateur(sessionUser->getUsername());
    return u ? u : sessionUser;
}

static void printAboStatus(const Utilisateur &u)
{
    if (u.getRole() != "Client")
    {
        cout << "Etat abonnement: (non requis pour " << u.getRole() << ")" << endl;
        return;
    }

    if (u.isAboActif())
        cout << "Etat abonnement: ACTIF (expire le " << u.getAboExpiration() << ")" << endl;
    else if (u.isAboPending())
        cout << "Etat abonnement: EN ATTENTE de validation" << endl;
    else
        cout << "Etat abonnement: INACTIF" << endl;
}

static void menuValiderAbonnements(GestionUtilisateurs &gestionUsers)
{
    cout << "\n=== VALIDATION DES ABONNEMENTS ===" << endl;
    gestionUsers.listerDemandesAbonnement();

    cout << "\n1. Valider une demande" << endl;
    cout << "2. Refuser une demande" << endl;
    cout << "0. Retour" << endl;
    cout << "Choix: ";

    int ch = 0;
    if (!(cin >> ch))
    {
        cin.clear();
        viderBuffer();
        cout << ">> Choix invalide!" << endl;
        return;
    }
    viderBuffer();

    if (ch == 0) return;

    string username;
    cout << "Username client: ";
    getline(cin, username);

    if (ch == 1)
    {
        string exp;
        cout << "Date d'expiration (YYYY-MM-DD, ex: 2026-02-01): ";
        getline(cin, exp);
        gestionUsers.validerAbonnement(username, exp);
    }
    else if (ch == 2)
    {
        gestionUsers.refuserAbonnement(username);
    }
    else
    {
        cout << ">> Choix invalide!" << endl;
    }
}

void menuAjouter(Bibliotheque &biblio)
{
    int choixType, nPage = 0, duree = 0, quantite = 1;
    double tailleMo = 0.0;
    string titre, auteur, format, qualite, pub;

    cout << "\n--- AJOUT DE MEDIA ---" << endl;
    cout << "1. Livre\n2. Video\n3. Audio\n4. Ebook\n5. AudioBook\nChoix : ";
    cin >> choixType;
    viderBuffer();

    int id = biblio.genererId();
    cout << "ID genere automatiquement : " << id << endl;

    cout << "Titre : ";
    getline(cin, titre);

    switch (choixType)
    {
    case 1:
        cout << "Auteur : ";
        getline(cin, auteur);
        cout << "Nombre de pages : ";
        cin >> nPage;
        cout << "Quantite (exemplaires) : ";
        cin >> quantite;
        biblio.ajouterMedia(make_shared<Livre>(id, titre, quantite, auteur, nPage));
        break;

    case 2:
        cout << "Duree (min) : ";
        cin >> duree;
        viderBuffer();
        cout << "Qualite : ";
        getline(cin, qualite);
        biblio.ajouterMedia(make_shared<Video>(id, titre, true, duree, qualite));
        break;

    case 3:
        viderBuffer();
        cout << "Publicateur : ";
        getline(cin, pub);
        cout << "Duree (min) : ";
        cin >> duree;
        biblio.ajouterMedia(make_shared<Audio>(id, titre, true, pub, duree));
        break;

    case 4:
        viderBuffer();
        cout << "Auteur : ";
        getline(cin, auteur);
        cout << "Nombre de pages : ";
        cin >> nPage;
        cout << "Taille (Mo) : ";
        cin >> tailleMo;
        viderBuffer();
        cout << "Format : ";
        getline(cin, format);
        biblio.ajouterMedia(make_shared<Ebook>(id, titre, auteur, nPage, tailleMo, format));
        break;

    case 5:
        viderBuffer();
        cout << "Auteur : ";
        getline(cin, auteur);
        cout << "Nombre de pages : ";
        cin >> nPage;
        viderBuffer();
        cout << "Narrateur : ";
        getline(cin, pub);
        cout << "Duree Audio (min) : ";
        cin >> duree;
        biblio.ajouterMedia(make_shared<AudioBook>(id, titre, true, auteur, nPage, pub, duree));
        break;

    default:
        cout << "Type invalide." << endl;
        return;
    }

    cout << ">> Media ajoute avec succes." << endl;
}

void menuGestionUtilisateurs(GestionUtilisateurs &gestionUsers)
{
    int choix = -1;

    while (choix != 0)
    {
        cout << "\n=== GESTION DES UTILISATEURS ===" << endl;
        cout << "1. Ajouter un nouvel utilisateur" << endl;
        cout << "2. Supprimer un utilisateur" << endl;
        cout << "3. Lister tous les utilisateurs" << endl;
        cout << "4. Changer mot de passe utilisateur" << endl;
        cout << "0. Retour au menu principal" << endl;
        cout << "Choix: ";

        if (!(cin >> choix))
        {
            cin.clear();
            viderBuffer();
            cout << ">> Choix invalide!" << endl;
            choix = -1;
            continue;
        }
        viderBuffer();

        switch (choix)
        {
        case 1:
        {
            string user, pass, role;
            cout << "\n--- AJOUT D'UTILISATEUR ---" << endl;
            cout << "Nouvel username: ";
            getline(cin, user);
            cout << "Mot de passe: ";
            getline(cin, pass);
            cout << "Role (Client/Admin/SuperAdmin): ";
            getline(cin, role);

            transform(role.begin(), role.end(), role.begin(), ::tolower);
            if (role == "client") role = "Client";
            else if (role == "admin") role = "Admin";
            else if (role == "superadmin") role = "SuperAdmin";

            gestionUsers.ajouterUtilisateur(user, pass, role);
            break;
        }
        case 2:
        {
            string user;
            cout << "\n--- SUPPRESSION D'UTILISATEUR ---" << endl;
            cout << "Username a supprimer: ";
            getline(cin, user);
            gestionUsers.supprimerUtilisateur(user);
            break;
        }
        case 3:
            gestionUsers.listerUtilisateurs();
            break;

        case 4:
        {
            string user, newPass;
            cout << "\n--- CHANGEMENT DE MOT DE PASSE ---" << endl;
            cout << "Username: ";
            getline(cin, user);
            cout << "Nouveau mot de passe: ";
            getline(cin, newPass);
            gestionUsers.changerMotDePasse(user, newPass);
            break;
        }

        case 0:
            cout << ">> Retour au menu principal..." << endl;
            break;

        default:
            cout << ">> Choix invalide!" << endl;
        }
    }
}

void montrerMenuClient(shared_ptr<Utilisateur> sessionUser, GestionUtilisateurs &gestionUsers)
{
    Bibliotheque biblio;
    int choix = -1;

    cout << "\n===================================" << endl;
    cout << "  BIBLIOTHEQUE MULTIMEDIA (CLIENT)" << endl;
    cout << "===================================" << endl;

    biblio.chargerDepuisFichier();

    while (choix != 0)
    {
        auto u = refreshUser(sessionUser, gestionUsers);
        cout << "\n--- MENU CLIENT ---" << endl;
        printAboStatus(*u);

        cout << "1. Afficher tout le catalogue" << endl;
        cout << "2. Rechercher un media" << endl;
        cout << "3. Emprunter un media (abonnement actif)" << endl;
        cout << "4. Retourner un media" << endl;
        cout << "5. Demander abonnement (apres paiement)" << endl;
        cout << "0. Deconnexion" << endl;
        cout << "Votre choix : ";

        if (!(cin >> choix))
        {
            cin.clear();
            viderBuffer();
            cout << ">> Choix invalide!" << endl;
            choix = -1;
            continue;
        }
        viderBuffer();

        switch (choix)
        {
        case 1:
            biblio.afficherTout();
            break;
        case 2:
        {
            string motCle;
            cout << "Mot du titre : ";
            getline(cin, motCle);
            biblio.rechercherParTitre(motCle);
            break;
        }
        case 3:
        {
            // Regle: un client ne peut emprunter que si abonnement actif et non expire.
            if (!u->abonnementValideAujourdHui())
            {
                cout << "\n>> Acces refuse: abonnement inactif ou expire." << endl;
                if (u->isAboPending())
                    cout << ">> Votre demande est EN ATTENTE de validation." << endl;
                else
                    cout << ">> Faites une demande: menu 5 (apres paiement)." << endl;
                break;
            }

            int id;
            cout << "ID du media a emprunter : ";
            cin >> id;
            viderBuffer();
            biblio.changerStatut(id, true);
            break;
        }
        case 4:
        {
            int id;
            cout << "ID du media a retourner : ";
            cin >> id;
            viderBuffer();
            biblio.changerStatut(id, false);
            break;
        }
        case 5:
        {
            gestionUsers.demanderAbonnement(u->getUsername());
            break;
        }
        case 0:
            biblio.sauvegarderDansFichier();
            cout << "\n>> Deconnexion..." << endl;
            break;
        default:
            cout << ">> Choix invalide!" << endl;
        }
    }
}

void montrerMenuAdmin(shared_ptr<Utilisateur> sessionUser, GestionUtilisateurs &gestionUsers)
{
    Bibliotheque biblio;
    int choix = -1;

    cout << "\n===================================" << endl;
    cout << "  BIBLIOTHEQUE MULTIMEDIA (ADMIN)" << endl;
    cout << "===================================" << endl;

    biblio.chargerDepuisFichier();

    while (choix != 0)
    {
        cout << "\n--- MENU ADMINISTRATEUR ---" << endl;
        cout << "1. Afficher tout le catalogue" << endl;
        cout << "2. Ajouter un media" << endl;
        cout << "3. Rechercher un media" << endl;
        cout << "4. Emprunter / Retourner" << endl;
        cout << "5. Supprimer un media" << endl;
        cout << "6. Voir les statistiques" << endl;
        cout << "7. Valider abonnements" << endl;
        cout << "0. Deconnexion" << endl;
        cout << "Votre choix : ";

        if (!(cin >> choix))
        {
            cin.clear();
            viderBuffer();
            cout << ">> Choix invalide!" << endl;
            choix = -1;
            continue;
        }

        switch (choix)
        {
        case 1:
            biblio.afficherTout();
            break;
        case 2:
            menuAjouter(biblio);
            break;
        case 3:
        {
            string motCle;
            cout << "Mot du titre : ";
            viderBuffer();
            getline(cin, motCle);
            biblio.rechercherParTitre(motCle);
            break;
        }
        case 4:
        {
            int id, action;
            cout << "ID du media : ";
            cin >> id;
            cout << "1. Emprunter\n2. Retourner\nChoix : ";
            cin >> action;
            biblio.changerStatut(id, (action == 1));
            break;
        }
        case 5:
        {
            int id;
            cout << "ID a supprimer : ";
            cin >> id;
            biblio.supprimerMedia(id);
            break;
        }
        case 6:
            biblio.afficherStatistiques();
            break;
        case 7:
            menuValiderAbonnements(gestionUsers);
            break;
        case 0:
            biblio.sauvegarderDansFichier();
            gestionUsers.sauvegarderUtilisateurs();
            cout << "\n>> Deconnexion..." << endl;
            break;
        default:
            cout << ">> Choix invalide!" << endl;
            viderBuffer();
        }
    }
}

void montrerMenuSuperAdmin(shared_ptr<Utilisateur> sessionUser, GestionUtilisateurs &gestionUsers)
{
    Bibliotheque biblio;
    int choix = -1;

    cout << "\n===================================" << endl;
    cout << "  BIBLIOTHEQUE MULTIMEDIA (SUPER ADMIN)" << endl;
    cout << "===================================" << endl;
    cout << "Repertoire courant: " << fs::current_path() << endl;

    biblio.chargerDepuisFichier();

    while (choix != 0)
    {
        cout << "\n--- MENU SUPER ADMINISTRATEUR ---" << endl;
        cout << "1. Afficher tout le catalogue" << endl;
        cout << "2. Ajouter un media" << endl;
        cout << "3. Rechercher un media" << endl;
        cout << "4. Emprunter / Retourner" << endl;
        cout << "5. Supprimer un media" << endl;
        cout << "6. Voir les statistiques" << endl;
        cout << "7. Verifier le fichier de sauvegarde" << endl;
        cout << "8. Gestion des utilisateurs" << endl;
        cout << "9. Valider abonnements" << endl;
        cout << "0. Deconnexion" << endl;
        cout << "Votre choix : ";

        if (!(cin >> choix))
        {
            cin.clear();
            viderBuffer();
            cout << ">> Choix invalide!" << endl;
            choix = -1;
            continue;
        }

        switch (choix)
        {
        case 1:
            biblio.afficherTout();
            break;
        case 2:
            menuAjouter(biblio);
            break;
        case 3:
        {
            string motCle;
            cout << "Mot du titre : ";
            viderBuffer();
            getline(cin, motCle);
            biblio.rechercherParTitre(motCle);
            break;
        }
        case 4:
        {
            int id, action;
            cout << "ID du media : ";
            cin >> id;
            cout << "1. Emprunter\n2. Retourner\nChoix : ";
            cin >> action;
            biblio.changerStatut(id, (action == 1));
            break;
        }
        case 5:
        {
            int id;
            cout << "ID a supprimer : ";
            cin >> id;
            biblio.supprimerMedia(id);
            break;
        }
        case 6:
            biblio.afficherStatistiques();
            break;
        case 7:
            biblio.verifierFichier();
            break;
        case 8:
            menuGestionUtilisateurs(gestionUsers);
            break;
        case 9:
            menuValiderAbonnements(gestionUsers);
            break;
        case 0:
            biblio.sauvegarderDansFichier();
            gestionUsers.sauvegarderUtilisateurs();
            cout << "\n>> Deconnexion..." << endl;
            break;
        default:
            cout << ">> Choix invalide!" << endl;
            viderBuffer();
        }
    }
}
