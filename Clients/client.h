/* client.h - clients : CRUD, adresses de livraison, points de fidelite */
#ifndef CLIENT_H
#define CLIENT_H

#include "types.h"

/* Ajoute un client valide et renvoie son identifiant attribue par la boutique. */
CodeRetour ajouterClient(Boutique *b, const Client *client, int *idAttribue);

/* Renvoie l'indice du client (desactives compris), ou -1 s'il est absent. */
int trouverClient(const Boutique *b, int idClient);
/* Les emails des clients desactives restent reserves. */
int emailExiste(const Boutique *b, const char *email, int idIgnore);

/* Regles metier et acces aux donnees (client.c). */
CodeRetour modifierClient(Boutique *b, int idClient, const Client *infos);
CodeRetour supprimerClient(Boutique *b, int idClient);
float chiffreAffairesClient(const Client *client);

/* Affichage des donnees clients (client_affichage.c). */
void afficherListeClients(const Boutique *b);

/* Fiche interactive et formulaires (client_saisie.c). */
void afficherFicheClient(const Boutique *b);
void menuAjouterClient(Boutique *b);
void menuModifierClient(Boutique *b);
void menuSupprimerClient(Boutique *b);

#endif /* CLIENT_H */
