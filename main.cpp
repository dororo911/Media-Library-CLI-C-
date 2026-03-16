#include <iostream>
#include <filesystem>

#include "users.h"
#include "menus.h"

using namespace std;
namespace fs = std::filesystem;

int main()
{
    cout << "==============================================" << endl;
    cout << "  SYSTEME DE GESTION DE BIBLIOTHEQUE V2.0" << endl;
    cout << "==============================================" << endl;
    cout << "Dossier de travail: " << fs::current_path() << endl;

    GestionUtilisateurs gestionUsers;

    while (true)
    {
        cout << "\n=== ACCUEIL ===" << endl;

        auto user = login(gestionUsers);
        string role = user->getRole();

        if (role == "Client")
            montrerMenuClient(user, gestionUsers);
        else if (role == "Admin")
            montrerMenuAdmin(user, gestionUsers);
        else
            montrerMenuSuperAdmin(user, gestionUsers);

        cout << "\n>> Retour a l'ecran d'accueil..." << endl;
    }

    return 0;
}
