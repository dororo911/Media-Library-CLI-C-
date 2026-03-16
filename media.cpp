#include "media.h"

#include <algorithm>
#include <fstream>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

// ===== Media =====
Media::Media(int id, const string &titre, bool dispo) : id(id), titre(titre), dispo(dispo) {}

int Media::getId() const { return id; }
const string &Media::getTitre() const { return titre; }
bool Media::isDispo() const { return dispo; }

void Media::emprunter()
{
    if (dispo)
    {
        dispo = false;
        cout << ">> Succes: '" << titre << "' a ete emprunte." << endl;
    }
    else
    {
        cout << ">> Erreur: '" << titre << "' n'est pas disponible." << endl;
    }
}

void Media::retourner()
{
    dispo = true;
    cout << ">> Info: '" << titre << "' a ete retourne." << endl;
}

int Media::getDureeMinutes() const { return 0; }

ostream &operator<<(ostream &os, const Media &m)
{
    m.afficher(os);
    return os;
}

// ===== Telechargeable =====
Telechargeable::Telechargeable(double tailleMo, const string &format) : tailleMo(tailleMo), format(format) {}

void Telechargeable::afficherTelechargement(ostream &os) const
{
    os << " [Fichier: " << format << " | " << tailleMo << " Mo]";
}

double Telechargeable::getTailleMo() const { return tailleMo; }
const string &Telechargeable::getFormat() const { return format; }

// ===== Livre =====
Livre::Livre(int id, const string &titre, int quantite, const string &auteur, int nPage)
    : Media(id, titre, quantite > 0), auteur(auteur), nPage(nPage), quantite(max(0, quantite)) {}

void Livre::emprunter()
{
    if (quantite > 0)
    {
        quantite--;
        dispo = (quantite > 0);
        cout << ">> Succes: '" << titre << "' a ete emprunte. (restant: " << quantite << ")" << endl;
    }
    else
    {
        dispo = false;
        cout << ">> Erreur: '" << titre << "' n'est pas disponible (quantite = 0)." << endl;
    }
}

void Livre::retourner()
{
    quantite++;
    dispo = true;
    cout << ">> Info: '" << titre << "' a ete retourne. (quantite: " << quantite << ")" << endl;
}

void Livre::afficher(ostream &os) const
{
    os << "[Livre] ID:" << id << " | " << titre
       << " | Auteur: " << auteur << " | " << nPage << "p"
       << " | Qte: " << quantite
       << " | Dispo: " << (dispo ? "Oui" : "Non");
}

string Livre::getType() const { return "Livre"; }
const string &Livre::getAuteur() const { return auteur; }
int Livre::getNpage() const { return nPage; }
int Livre::getQuantite() const { return quantite; }

// ===== Video =====
Video::Video(int id, const string &titre, bool dispo, int duree, const string &qualite)
    : Media(id, titre, dispo), duree(duree), qualite(qualite) {}

void Video::afficher(ostream &os) const
{
    os << "[Video] ID:" << id << " | " << titre
       << " | Duree: " << duree << "min | " << qualite
       << " | Dispo: " << (dispo ? "Oui" : "Non");
}

int Video::getDureeMinutes() const { return duree; }
string Video::getType() const { return "Video"; }
const string &Video::getQualite() const { return qualite; }

// ===== Audio =====
Audio::Audio(int id, const string &titre, bool dispo, const string &publicateur, int duree)
    : Media(id, titre, dispo), publicateur(publicateur), duree(duree) {}

void Audio::afficher(ostream &os) const
{
    os << "[Audio] ID:" << id << " | " << titre
       << " | Pub: " << publicateur << " | " << duree << "min"
       << " | Dispo: " << (dispo ? "Oui" : "Non");
}

int Audio::getDureeMinutes() const { return duree; }
string Audio::getType() const { return "Audio"; }
const string &Audio::getPublicateur() const { return publicateur; }

// ===== Ebook =====
Ebook::Ebook(int id, const string &titre, const string &auteur, int nPage, double tailleMo, const string &format)
    : Media(id, titre, true),
      Livre(id, titre, 1, auteur, nPage),
      Telechargeable(tailleMo, format) {}

void Ebook::afficher(ostream &os) const
{
    Livre::afficher(os);
    Telechargeable::afficherTelechargement(os);
}

string Ebook::getType() const { return "Ebook"; }

// ===== AudioBook =====
AudioBook::AudioBook(int id, const string &titre, bool dispo, const string &auteur, int nPage, const string &publicateur, int duree)
    : Media(id, titre, dispo),
      Livre(id, titre, dispo ? 1 : 0, auteur, nPage),
      Audio(id, titre, dispo, publicateur, duree) {}

void AudioBook::afficher(ostream &os) const
{
    os << "[AudioBook] ID:" << id << " | " << titre
       << " | Auteur: " << Livre::getAuteur()
       << " | Voix: " << Audio::getPublicateur()
       << " | Duree: " << Audio::getDureeMinutes() << "min"
       << " | Dispo: " << (dispo ? "Oui" : "Non");
}

string AudioBook::getType() const { return "AudioBook"; }

// ===== Bibliotheque =====
void Bibliotheque::ajouterMedia(shared_ptr<Media> media)
{
    catalogue.push_back(media);
}

void Bibliotheque::supprimerMedia(int id)
{
    size_t before = catalogue.size();
    catalogue.erase(remove_if(catalogue.begin(), catalogue.end(),
                              [id](const shared_ptr<Media> &media)
                              { return media->getId() == id; }),
                    catalogue.end());

    if (catalogue.size() < before)
        cout << ">> Media ID " << id << " supprime." << endl;
    else
        cout << ">> ID introuvable." << endl;
}

void Bibliotheque::rechercherParTitre(const string &motCle)
{
    cout << "\n--- Resultats Recherche : " << motCle << " ---" << endl;
    bool trouve = false;

    for (const auto &media : catalogue)
    {
        if (media->getTitre().find(motCle) != string::npos)
        {
            cout << *media << endl;
            trouve = true;
        }
    }
    if (!trouve) cout << "Aucun resultat." << endl;
}

void Bibliotheque::changerStatut(int id, bool emprunt)
{
    auto it = find_if(catalogue.begin(), catalogue.end(),
                      [id](const shared_ptr<Media> &media)
                      { return media->getId() == id; });

    if (it != catalogue.end())
    {
        if (emprunt) (*it)->emprunter();
        else (*it)->retourner();
    }
    else
    {
        cout << ">> Media introuvable." << endl;
    }
}

void Bibliotheque::afficherTout()
{
    cout << "\n--- CATALOGUE COMPLET (" << catalogue.size() << " medias) ---" << endl;

    sort(catalogue.begin(), catalogue.end(),
         [](const shared_ptr<Media> &a, const shared_ptr<Media> &b)
         { return a->getId() < b->getId(); });

    for (const auto &media : catalogue)
        cout << *media << endl;
}

void Bibliotheque::afficherStatistiques()
{
    int totalMedia = (int)catalogue.size();
    int totalDuree = 0;
    int nbLivres = 0;
    int nbDispo = 0;

    for (const auto &media : catalogue)
    {
        totalDuree += media->getDureeMinutes();
        string type = media->getType();

        if (type == "Livre" || type == "Ebook" || type == "AudioBook")
            nbLivres++;
        if (media->isDispo())
            nbDispo++;
    }

    cout << "\n--- STATISTIQUES ---" << endl;
    cout << "Nombre total de medias : " << totalMedia << endl;
    cout << "Medias disponibles : " << nbDispo << endl;
    cout << "Duree totale (Audio/Video) : " << totalDuree << " min" << endl;
    cout << "Nombre de livres (Papier/Ebook/AudioBook) : " << nbLivres << endl;
}

// ✅ FIXED save format (no duplicate fields)
void Bibliotheque::sauvegarderDansFichier()
{
    string nomFichier = "bibliotheque.txt";
    ofstream f(nomFichier);

    if (!f)
    {
        cerr << ">> ERREUR: Impossible d'ouvrir le fichier pour ecriture!" << endl;
        return;
    }

    for (const auto &media : catalogue)
    {
        string type = media->getType();
        // Pour les livres papier, on sauvegarde la QUANTITE (dispo = quantite > 0)
        if (type == "Livre")
        {
            auto livre = dynamic_pointer_cast<Livre>(media);
            f << type << ";" << media->getId() << ";" << media->getTitre() << ";" << livre->getQuantite();
            f << ";" << livre->getAuteur() << ";" << livre->getNpage();
        }
        else if (type == "Video")
        {
            f << type << ";" << media->getId() << ";" << media->getTitre() << ";" << (media->isDispo() ? "1" : "0");
            auto v = dynamic_pointer_cast<Video>(media);
            f << ";" << v->getDureeMinutes() << ";" << v->getQualite();
        }
        else if (type == "Audio")
        {
            f << type << ";" << media->getId() << ";" << media->getTitre() << ";" << (media->isDispo() ? "1" : "0");
            auto a = dynamic_pointer_cast<Audio>(media);
            f << ";" << a->getPublicateur() << ";" << a->getDureeMinutes();
        }
        else if (type == "Ebook")
        {
            f << type << ";" << media->getId() << ";" << media->getTitre() << ";" << (media->isDispo() ? "1" : "0");
            auto e = dynamic_pointer_cast<Ebook>(media);
            f << ";" << e->getAuteur() << ";" << e->getNpage()
              << ";" << e->getTailleMo() << ";" << e->getFormat();
        }
        else if (type == "AudioBook")
        {
            f << type << ";" << media->getId() << ";" << media->getTitre() << ";" << (media->isDispo() ? "1" : "0");
            auto ab = dynamic_pointer_cast<AudioBook>(media);
            f << ";" << ab->getAuteur() << ";" << ab->getNpage()
              << ";" << ab->getPublicateur() << ";" << ab->getDureeMinutes();
        }

        f << "\n";
    }

    cout << ">> Catalogue sauvegarde: " << catalogue.size() << " medias" << endl;
}

void Bibliotheque::chargerDepuisFichier()
{
    string nomFichier = "bibliotheque.txt";
    ifstream f(nomFichier);

    if (!f)
    {
        cout << ">> Info: Catalogue vide. Fichier '" << nomFichier << "' non trouve." << endl;
        return;
    }

    int count = 0;
    string ligne;

    while (getline(f, ligne))
    {
        if (ligne.empty()) continue;

        vector<string> champs;
        size_t pos = 0;

        while ((pos = ligne.find(';')) != string::npos)
        {
            champs.push_back(ligne.substr(0, pos));
            ligne.erase(0, pos + 1);
        }
        champs.push_back(ligne);

        if (champs.size() < 4) continue;

        string type = champs[0];
        int id = stoi(champs[1]);
        string titre = champs[2];
        bool dispo = (champs[3] == "1");

        // Pour Livre: le champ[3] represente la quantite (exemplaires).
        // Par compatibilite, un ancien fichier avec 0/1 fonctionne aussi.
        if (type == "Livre" && champs.size() >= 6)
        {
            int qte = 0;
            try { qte = stoi(champs[3]); } catch (...) { qte = (dispo ? 1 : 0); }
            ajouterMedia(make_shared<Livre>(id, titre, qte, champs[4], stoi(champs[5])));
            count++;
        }
        else if (type == "Video" && champs.size() >= 6)
        {
            ajouterMedia(make_shared<Video>(id, titre, dispo, stoi(champs[4]), champs[5]));
            count++;
        }
        else if (type == "Audio" && champs.size() >= 6)
        {
            ajouterMedia(make_shared<Audio>(id, titre, dispo, champs[4], stoi(champs[5])));
            count++;
        }
        else if (type == "Ebook" && champs.size() >= 8)
        {
            ajouterMedia(make_shared<Ebook>(id, titre, champs[4], stoi(champs[5]), stod(champs[6]), champs[7]));
            count++;
        }
        else if (type == "AudioBook" && champs.size() >= 8)
        {
            ajouterMedia(make_shared<AudioBook>(id, titre, dispo, champs[4], stoi(champs[5]), champs[6], stoi(champs[7])));
            count++;
        }
    }
recalculerProchainId();

    if (count > 0)
        cout << ">> " << count << " medias charges depuis " << nomFichier << endl;
}

void Bibliotheque::verifierFichier()
{
    string nomFichier = "bibliotheque.txt";
    string cheminComplet = (fs::current_path() / nomFichier).string();

    cout << "\n--- VERIFICATION FICHIER ---" << endl;
    cout << "Emplacement: " << cheminComplet << endl;

    if (!fs::exists(cheminComplet))
    {
        cout << ">> Le fichier n'existe pas!" << endl;
        return;
    }

    ifstream f(nomFichier);
    if (!f)
    {
        cout << ">> Impossible d'ouvrir le fichier en lecture!" << endl;
        return;
    }

    auto size = fs::file_size(cheminComplet);
    cout << "Taille: " << size << " octets" << endl;

    if (size == 0)
    {
        cout << ">> Fichier vide!" << endl;
        return;
    }

    cout << "\n--- CONTENU DU FICHIER ---" << endl;
    string ligne;
    int numLigne = 0;
    while (getline(f, ligne))
    {
        numLigne++;
        cout << numLigne << ": " << ligne << endl;
    }
    cout << "--- FIN DU FICHIER ---" << endl;
    cout << "Total: " << numLigne << " lignes" << endl;
}
int Bibliotheque::genererId()
{
    return prochainId++;  // retourne puis incrémente
}

void Bibliotheque::recalculerProchainId()
{
    int maxId = 0;
    for (const auto &m : catalogue)
    {
        if (m->getId() > maxId) maxId = m->getId();
    }
    prochainId = maxId + 1;
}
