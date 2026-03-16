#ifndef MEDIA_H
#define MEDIA_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>

class Media
{
protected:
    int id;
    std::string titre;
    bool dispo;

public:
    Media(int id, const std::string &titre, bool dispo);
    virtual ~Media() = default;

    int getId() const;
    const std::string &getTitre() const;
    bool isDispo() const;

    virtual void emprunter();
    virtual void retourner();

    virtual void afficher(std::ostream &os) const = 0;
    virtual int getDureeMinutes() const;
    virtual std::string getType() const = 0;

    friend std::ostream &operator<<(std::ostream &os, const Media &m);
};

class Telechargeable
{
protected:
    double tailleMo;
    std::string format;

public:
    Telechargeable(double tailleMo, const std::string &format);
    virtual ~Telechargeable() = default;

    void afficherTelechargement(std::ostream &os) const;

    double getTailleMo() const;
    const std::string &getFormat() const;
};

class Livre : public virtual Media
{
protected:
    std::string auteur;
    int nPage;
    int quantite; // nombre d'exemplaires disponibles

public:
    // dispo est derive de quantite (dispo = quantite > 0)
    Livre(int id, const std::string &titre, int quantite, const std::string &auteur, int nPage);

    // Un livre papier peut exister en plusieurs exemplaires.
    void emprunter() override;
    void retourner() override;

    void afficher(std::ostream &os) const override;
    std::string getType() const override;

    const std::string &getAuteur() const;
    int getNpage() const;
    int getQuantite() const;
};

class Video : public virtual Media
{
private:
    int duree;
    std::string qualite;

public:
    Video(int id, const std::string &titre, bool dispo, int duree, const std::string &qualite);

    void afficher(std::ostream &os) const override;
    int getDureeMinutes() const override;
    std::string getType() const override;

    const std::string &getQualite() const;
};

class Audio : public virtual Media
{
protected:
    std::string publicateur;
    int duree;

public:
    Audio(int id, const std::string &titre, bool dispo, const std::string &publicateur, int duree);

    void afficher(std::ostream &os) const override;
    int getDureeMinutes() const override;
    std::string getType() const override;

    const std::string &getPublicateur() const;
};

class Ebook : public Livre, public Telechargeable
{
public:
    Ebook(int id, const std::string &titre, const std::string &auteur, int nPage, double tailleMo, const std::string &format);

    void afficher(std::ostream &os) const override;
    std::string getType() const override;
};

class AudioBook : public Livre, public Audio
{
public:
    AudioBook(int id, const std::string &titre, bool dispo, const std::string &auteur, int nPage, const std::string &publicateur, int duree);

    void afficher(std::ostream &os) const override;
    std::string getType() const override;
};

class Bibliotheque
{
private:
    std::vector<std::shared_ptr<Media>> catalogue;
    int prochainId = 1; // ✅ nouveau

public:
    int genererId();               // ✅ nouveau
    void recalculerProchainId();   // ✅ nouveau

    void ajouterMedia(std::shared_ptr<Media> media);
    void supprimerMedia(int id);
    void rechercherParTitre(const std::string &motCle);
    void changerStatut(int id, bool emprunt);
    void afficherTout();
    void afficherStatistiques();

    void sauvegarderDansFichier();
    void chargerDepuisFichier();
    void verifierFichier();
};

#endif
