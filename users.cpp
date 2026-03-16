#include "users.h"
#include "utils.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <functional>
#include <algorithm>
#include <cstdlib>
#include <ctime>

using namespace std;

// ===== Helpers (dates ISO: YYYY-MM-DD) =====
static string todayISO_simple()
{
    time_t now = time(nullptr);
    tm lt{};
    localtime_s(&lt, &now);
    char buf[11]; // "YYYY-MM-DD" + '\0'
    strftime(buf, sizeof(buf), "%Y-%m-%d", &lt);
    return string(buf);
}

static bool isValidISODateBasic(const string &iso)
{
    // Validation minimale: YYYY-MM-DD
    if (iso.size() != 10) return false;
    if (iso[4] != '-' || iso[7] != '-') return false;
    for (size_t i = 0; i < iso.size(); ++i)
    {
        if (i == 4 || i == 7) continue;
        if (iso[i] < '0' || iso[i] > '9') return false;
    }
    return true;
}

// ===== Utilisateur =====

string Utilisateur::hashPassword(const string &password)
{
    hash<string> hasher;
    size_t hashValue = hasher(password);

    stringstream ss;
    ss << hex << setw(16) << setfill('0') << hashValue;
    return ss.str();
}

Utilisateur::Utilisateur(const string &user, const string &pass, const string &r)
    : username(user), passwordHash(hashPassword(pass)), role(r)
{
    // Par defaut: un client n'a pas d'abonnement actif.
    if (role != "Client")
    {
        aboActif = true;
        aboPending = false;
        aboExpiration = "2099-01-01";
    }
}

Utilisateur::Utilisateur(const string &user,
                         const string &hash,
                         const string &r,
                         bool /*isHash*/,
                         bool aboActif_,
                         bool aboPending_,
                         const string &aboExpiration_)
    : username(user), passwordHash(hash), role(r), aboActif(aboActif_), aboPending(aboPending_), aboExpiration(aboExpiration_)
{
    // Les admins ont toujours les droits d'emprunt.
    if (role != "Client")
    {
        aboActif = true;
        aboPending = false;
        if (aboExpiration.empty()) aboExpiration = "2099-01-01";
    }
}

string Utilisateur::getUsername() const { return username; }
string Utilisateur::getPasswordHash() const { return passwordHash; }
string Utilisateur::getRole() const { return role; }

bool Utilisateur::isAboActif() const { return aboActif; }
bool Utilisateur::isAboPending() const { return aboPending; }
string Utilisateur::getAboExpiration() const { return aboExpiration; }

void Utilisateur::setAboActif(bool v)
{
    if (role == "Client") aboActif = v;
}

void Utilisateur::setAboPending(bool v)
{
    if (role == "Client") aboPending = v;
}

void Utilisateur::setAboExpiration(const string &iso)
{
    if (role == "Client") aboExpiration = iso;
}

void Utilisateur::setPassword(const string &pass)
{
    passwordHash = hashPassword(pass);
}

bool Utilisateur::abonnementValideAujourdHui() const
{
    // Admin / SuperAdmin: autorise.
    if (role != "Client") return true;

    if (!aboActif) return false;
    if (!isValidISODateBasic(aboExpiration)) return false;

    const string today = todayISO_simple();
    // IMPORTANT: marche car format ISO "YYYY-MM-DD".
    return today <= aboExpiration;
}

bool Utilisateur::checkPassword(const string &pass) const
{
    return passwordHash == hashPassword(pass);
}

// ===== GestionUtilisateurs =====

GestionUtilisateurs::GestionUtilisateurs()
{
    chargerUtilisateurs();
}

void GestionUtilisateurs::chargerUtilisateurs()
{
    comptes.clear();

    ifstream fichier(fichierUtilisateurs);
    if (!fichier)
    {
        // Comptes par defaut
        comptes = {
            Utilisateur("client", "123", "Client"),
            Utilisateur("admin", "456", "Admin"),
            Utilisateur("superadmin", "789", "SuperAdmin")};
        sauvegarderUtilisateurs();
        cout << ">> Fichier utilisateurs cree avec comptes par defaut" << endl;
        return;
    }

    string ligne;
    int count = 0;
    while (getline(fichier, ligne))
    {
        if (ligne.empty()) continue;

        vector<string> champs;
        stringstream ss(ligne);
        string item;
        while (getline(ss, item, ';')) champs.push_back(item);

        if (champs.size() < 3) continue;

        const string username = champs[0];
        const string passwordHash = champs[1];
        const string role = champs[2];

        // Formats supportes:
        //   ancien: username;hash;role
        //   nouveau: username;hash;role;abo_actif;abo_pending;abo_expiration
        bool aboActif = false;
        bool aboPending = false;
        string aboExp;

        if (champs.size() >= 6)
        {
            aboActif = (champs[3] == "1");
            aboPending = (champs[4] == "1");
            aboExp = champs[5];
        }
        else
        {
            // ancien format: pour Client => inactif, pour Admin => actif.
            if (role != "Client")
            {
                aboActif = true;
                aboPending = false;
                aboExp = "2099-01-01";
            }
        }

        comptes.push_back(Utilisateur(username, passwordHash, role, true, aboActif, aboPending, aboExp));
        count++;
    }

    if (count > 0) cout << ">> " << count << " utilisateurs charges" << endl;
}

void GestionUtilisateurs::sauvegarderUtilisateurs()
{
    ofstream fichier(fichierUtilisateurs);
    for (const auto &user : comptes)
    {
        fichier << user.getUsername() << ";"
                << user.getPasswordHash() << ";"
                << user.getRole() << ";"
                << (user.isAboActif() ? "1" : "0") << ";"
                << (user.isAboPending() ? "1" : "0") << ";"
                << user.getAboExpiration() << "\n";
    }
}

bool GestionUtilisateurs::usernameExiste(const string &username)
{
    for (const auto &user : comptes)
        if (user.getUsername() == username) return true;
    return false;
}

bool GestionUtilisateurs::ajouterUtilisateur(const string &username, const string &password, const string &role)
{
    if (usernameExiste(username))
    {
        cout << ">> Erreur: Ce nom d'utilisateur existe deja!" << endl;
        return false;
    }

    if (role != "Client" && role != "Admin" && role != "SuperAdmin")
    {
        cout << ">> Erreur: Role invalide! Utilisez: Client, Admin ou SuperAdmin" << endl;
        return false;
    }

    if (password.length() < 3)
    {
        cout << ">> Erreur: Mot de passe trop court (minimum 3 caracteres)!" << endl;
        return false;
    }

    comptes.push_back(Utilisateur(username, password, role));
    sauvegarderUtilisateurs();
    cout << ">> Succes: Utilisateur '" << username << "' ajoute avec role '" << role << "'" << endl;
    return true;
}

bool GestionUtilisateurs::supprimerUtilisateur(const string &username)
{
    int nbSuperAdmin = 0;
    for (const auto &user : comptes)
        if (user.getRole() == "SuperAdmin") nbSuperAdmin++;

    for (auto it = comptes.begin(); it != comptes.end(); ++it)
    {
        if (it->getUsername() == username)
        {
            if (it->getRole() == "SuperAdmin" && nbSuperAdmin <= 1)
            {
                cout << ">> Erreur: Impossible de supprimer le dernier SuperAdmin!" << endl;
                return false;
            }
            comptes.erase(it);
            sauvegarderUtilisateurs();
            cout << ">> Succes: Utilisateur '" << username << "' supprime" << endl;
            return true;
        }
    }

    cout << ">> Erreur: Utilisateur non trouve!" << endl;
    return false;
}

void GestionUtilisateurs::listerUtilisateurs()
{
    cout << "\n=== LISTE DES UTILISATEURS (" << comptes.size() << ") ===" << endl;
    cout << "=============================================" << endl;
    for (const auto &user : comptes)
    {
        cout << "Username: " << user.getUsername()
             << " | Role: " << user.getRole();
        if (user.getRole() == "Client")
        {
            cout << " | Abo: " << (user.isAboActif() ? "ACTIF" : (user.isAboPending() ? "EN ATTENTE" : "INACTIF"))
                 << " | Exp: " << user.getAboExpiration();
        }
        cout << " | Hash: " << user.getPasswordHash().substr(0, 8) << "..." << endl;
    }
    cout << "=============================================" << endl;
}

bool GestionUtilisateurs::changerMotDePasse(const string &username, const string &nouveauPassword)
{
    if (nouveauPassword.length() < 3)
    {
        cout << ">> Erreur: Mot de passe trop court (minimum 3 caracteres)!" << endl;
        return false;
    }

    for (auto &user : comptes)
    {
        if (user.getUsername() == username)
        {
            user.setPassword(nouveauPassword);
            sauvegarderUtilisateurs();
            cout << ">> Mot de passe change pour '" << username << "'" << endl;
            return true;
        }
    }
    cout << ">> Erreur: Utilisateur non trouve!" << endl;
    return false;
}

// ===== Abonnement =====

void GestionUtilisateurs::listerDemandesAbonnement()
{
    cout << "\n--- DEMANDES D'ABONNEMENT EN ATTENTE ---" << endl;
    bool found = false;
    for (const auto &u : comptes)
    {
        if (u.getRole() == "Client" && u.isAboPending())
        {
            cout << "- " << u.getUsername() << endl;
            found = true;
        }
    }
    if (!found) cout << "Aucune demande en attente." << endl;
}

bool GestionUtilisateurs::demanderAbonnement(const string &username)
{
    for (auto &u : comptes)
    {
        if (u.getUsername() == username)
        {
            if (u.getRole() != "Client")
            {
                cout << ">> Info: Les admins n'ont pas besoin d'abonnement." << endl;
                return false;
            }
            if (u.isAboActif())
            {
                cout << ">> Info: Abonnement deja actif (expire le " << u.getAboExpiration() << ")" << endl;
                return false;
            }
            u.setAboPending(true);
            sauvegarderUtilisateurs();
            cout << ">> Demande envoyee. En attente de validation par un Admin." << endl;
            return true;
        }
    }
    cout << ">> Erreur: Utilisateur non trouve!" << endl;
    return false;
}

bool GestionUtilisateurs::validerAbonnement(const string &username, const string &expirationISO)
{
    if (!isValidISODateBasic(expirationISO))
    {
        cout << ">> Erreur: Date invalide. Utilisez le format YYYY-MM-DD (ex: 2026-02-01)." << endl;
        return false;
    }

    for (auto &u : comptes)
    {
        if (u.getUsername() == username)
        {
            if (u.getRole() != "Client")
            {
                cout << ">> Erreur: Seuls les Clients ont un abonnement." << endl;
                return false;
            }
            u.setAboActif(true);
            u.setAboPending(false);
            u.setAboExpiration(expirationISO);
            sauvegarderUtilisateurs();
            cout << ">> Abonnement valide pour '" << username << "' jusqu'au " << expirationISO << endl;
            return true;
        }
    }
    cout << ">> Erreur: Utilisateur non trouve!" << endl;
    return false;
}

bool GestionUtilisateurs::refuserAbonnement(const string &username)
{
    for (auto &u : comptes)
    {
        if (u.getUsername() == username)
        {
            if (u.getRole() != "Client")
            {
                cout << ">> Erreur: Seuls les Clients ont un abonnement." << endl;
                return false;
            }
            u.setAboActif(false);
            u.setAboPending(false);
            u.setAboExpiration("");
            sauvegarderUtilisateurs();
            cout << ">> Demande refusee pour '" << username << "'" << endl;
            return true;
        }
    }
    cout << ">> Erreur: Utilisateur non trouve!" << endl;
    return false;
}

shared_ptr<Utilisateur> GestionUtilisateurs::getUtilisateur(const string &username)
{
    for (const auto &u : comptes)
    {
        if (u.getUsername() == username)
            return make_shared<Utilisateur>(u);
    }
    return nullptr;
}

shared_ptr<Utilisateur> GestionUtilisateurs::authentifier(const string &username, const string &password)
{
    for (const auto &user : comptes)
    {
        if (user.getUsername() == username && user.checkPassword(password))
            return make_shared<Utilisateur>(user);
    }
    return nullptr;
}

// ===== Login =====
shared_ptr<Utilisateur> login(GestionUtilisateurs &gestionUsers)
{
    while (true)
    {
        cout << "\n=== SYSTEME D'AUTHENTIFICATION ===" << endl;
        cout << "1. Se connecter" << endl;
        cout << "2. Creer un nouveau compte (Client seulement)" << endl;
        cout << "3. Quitter le programme" << endl;
        cout << "==================================" << endl;
        cout << "Choix: ";

        int choix;
        if (!(cin >> choix))
        {
            cin.clear();
            viderBuffer();
            cout << ">> Choix invalide!" << endl;
            continue;
        }
        viderBuffer();

        if (choix == 1)
        {
            string user, pass;
            cout << "\n--- CONNEXION ---" << endl;
            cout << "Username: ";
            getline(cin, user);
            cout << "Password: ";
            getline(cin, pass);

            auto userAuth = gestionUsers.authentifier(user, pass);
            if (userAuth)
            {
                cout << "\n>> Connexion reussie!" << endl;
                cout << ">> Bienvenue " << userAuth->getUsername()
                     << " (Role: " << userAuth->getRole() << ")" << endl;
                return userAuth;
            }
            cout << "\n>> Nom d'utilisateur ou mot de passe incorrect!" << endl;
        }
        else if (choix == 2)
        {
            cout << "\n=== CREATION DE COMPTE CLIENT ===" << endl;
            string user, pass, confPass;

            cout << "Nouveau username: ";
            getline(cin, user);

            if (gestionUsers.usernameExiste(user))
            {
                cout << ">> Erreur: Ce nom d'utilisateur existe deja!" << endl;
                continue;
            }

            if (user.length() < 3)
            {
                cout << ">> Erreur: Username trop court (minimum 3 caracteres)!" << endl;
                continue;
            }

            cout << "Nouveau mot de passe: ";
            getline(cin, pass);
            cout << "Confirmer le mot de passe: ";
            getline(cin, confPass);

            if (pass != confPass)
            {
                cout << ">> Erreur: Les mots de passe ne correspondent pas!" << endl;
                continue;
            }

            if (pass.length() < 3)
            {
                cout << ">> Erreur: Le mot de passe doit contenir au moins 3 caracteres!" << endl;
                continue;
            }

            if (gestionUsers.ajouterUtilisateur(user, pass, "Client"))
                cout << "\n>> Compte cree avec succes! Vous pouvez maintenant vous connecter." << endl;
        }
        else if (choix == 3)
        {
            cout << "Au revoir!" << endl;
            exit(0);
        }
        else
        {
            cout << ">> Choix invalide!" << endl;
        }
    }
}
