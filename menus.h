#ifndef MENUS_H
#define MENUS_H

#include "users.h"
#include "media.h"

void menuAjouter(Bibliotheque &biblio);
void menuGestionUtilisateurs(GestionUtilisateurs &gestionUsers);

void montrerMenuClient(std::shared_ptr<Utilisateur> sessionUser, GestionUtilisateurs &gestionUsers);
void montrerMenuAdmin(std::shared_ptr<Utilisateur> sessionUser, GestionUtilisateurs &gestionUsers);
void montrerMenuSuperAdmin(std::shared_ptr<Utilisateur> sessionUser, GestionUtilisateurs &gestionUsers);

#endif
