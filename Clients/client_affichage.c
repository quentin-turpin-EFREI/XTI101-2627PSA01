/* client_affichage.c - affichage des listes du module Clients. */
#include <stdio.h>

#include "client.h"

void afficherListeClients(const Boutique *b)
{
    int i;
    int nombreAffiche = 0;

    puts("\n--- Liste des clients ---");
    if (b == NULL || b->nbClients < 0 || b->nbClients > b->capaciteClients
        || (b->nbClients > 0 && b->clients == NULL)) {
        puts("Erreur : repertoire clients incoherent.");
        return;
    }

    /* Les clients desactives restent en memoire, mais sont masques ici. */
    for (i = 0; i < b->nbClients; i++) {
        const Client *client = &b->clients[i];

        if (client->actif) {
            printf("%d | %s %s | %s | %d | %.2f EUR\n",
                   client->idClient, client->nom, client->prenom,
                   client->email, client->nbCommandes,
                   chiffreAffairesClient(client));
            nombreAffiche++;
        }
    }

    if (nombreAffiche == 0)
        puts("Aucun client actif.");
}

