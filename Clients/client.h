/* client.h - clients : CRUD, adresses de livraison, points de fidelite */
#ifndef CLIENT_H
#define CLIENT_H

#include "types.h"

/* Ajoute un client valide et renvoie son identifiant attribue par la boutique. */
CodeRetour ajouterClient(Boutique *b, const Client *client, int *idAttribue);

/* Recherche et unicite de l'email (les clients inactifs restent consideres). */
int trouverClient(const Boutique *b, int idClient);
int emailExiste(const Boutique *b, const char *email, int idIgnore);

/* Fonctions metier des actions client. */
CodeRetour modifierClient(Boutique *b, int idClient, const Client *infos);
CodeRetour supprimerClient(Boutique *b, int idClient);
float chiffreAffairesClient(const Client *client);
void afficherListeClients(const Boutique *b);

/* Saisit les informations et appelle ajouterClient. */
void menuAjouterClient(Boutique *b);
void menuModifierClient(Boutique *b);
void menuSupprimerClient(Boutique *b);

#endif /* CLIENT_H */
