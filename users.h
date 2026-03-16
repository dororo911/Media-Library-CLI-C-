#ifndef USERS_H
#define USERS_H

#include <string>
#include <vector>
#include <memory>

class Utilisateur
{
private:
    std::string username;
    std::string passwordHash;
    std::string role; // "Client", "Admin", "SuperAdmin"

    // Abonnement (pour Client)
    bool aboActif = false;
    bool aboPending = false;
    std::string aboExpiration; // format ISO: YYYY-MM-DD

    static std::string hashPassword(const std::string &password);

public:
    Utilisateur(const std::string &user, const std::string &pass, const std::string &r);
    Utilisateur(const std::string &user,
                const std::string &hash,
                const std::string &r,
                bool isHash,
                bool aboActif = false,
                bool aboPending = false,
                const std::string &aboExpiration = "");

    std::string getUsername() const;
    std::string getPasswordHash() const;
    std::string getRole() const;

    void setPassword(const std::string &pass);

    bool isAboActif() const;
    bool isAboPending() const;
    std::string getAboExpiration() const;

    void setAboActif(bool v);
    void setAboPending(bool v);
    void setAboExpiration(const std::string &iso);

    // True si l'utilisateur peut emprunter aujourd'hui.
    bool abonnementValideAujourdHui() const;

    bool checkPassword(const std::string &pass) const;
};

class GestionUtilisateurs
{
private:
    std::vector<Utilisateur> comptes;
    std::string fichierUtilisateurs = "utilisateurs.txt";

public:
    GestionUtilisateurs();

    void chargerUtilisateurs();
    void sauvegarderUtilisateurs();

    bool usernameExiste(const std::string &username);
    bool ajouterUtilisateur(const std::string &username, const std::string &password, const std::string &role);
    bool supprimerUtilisateur(const std::string &username);
    bool changerMotDePasse(const std::string &username, const std::string &nouveauPassword);

    void listerUtilisateurs();

    // Abonnement
    void listerDemandesAbonnement();
    bool demanderAbonnement(const std::string &username);
    bool validerAbonnement(const std::string &username, const std::string &expirationISO);
    bool refuserAbonnement(const std::string &username);

    std::shared_ptr<Utilisateur> getUtilisateur(const std::string &username);

    std::shared_ptr<Utilisateur> authentifier(const std::string &username, const std::string &password);
};

// Login menu
std::shared_ptr<Utilisateur> login(GestionUtilisateurs &gestionUsers);

#endif
