/* client.h - clients : CRUD, adresses de livraison, points de fidelite */
#ifndef CLIENT_H
#define CLIENT_H

#include "types.h"

/* Ajoute un client valide et renvoie son identifiant attribue par la boutique. */
CodeRetour ajouterClient(Boutique *b, const Client *client, int *idAttribue);

/* Saisit les informations et appelle ajouterClient. */
void menuAjouterClient(Boutique *b);

#endif /* CLIENT_H */
