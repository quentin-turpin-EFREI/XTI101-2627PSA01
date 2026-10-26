/* commande.c - commandes : creation, gestion des lignes, calcul des montants */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commande.h"
int creerCommande(Boutique *boutique, int idClient, int *numeroCommande)
{
    int indiceClient;
    int nouvelleCapacite;
    Client *client;
    Commande *commandesAgrandi;
    Commande nouvelleCommande;

    if (boutique == NULL || numeroCommande == NULL) {
        return ERR_INVALIDE;
    }

    indiceClient = trouverClient(boutique, idClient);
    if (indiceClient < 0) {
        return ERR_INTROUVABLE;
    }

    client = &boutique->clients[indiceClient];

    if (client->nbCommandes >= client->capaciteCommandes) {
        nouvelleCapacite = (client->capaciteCommandes == 0)
            ? CAPACITE_INITIALE
            : client->capaciteCommandes * 2;

        commandesAgrandi = realloc(
            client->commandes,
            nouvelleCapacite * sizeof(*commandesAgrandi)
        );

        if (commandesAgrandi == NULL) {
            return ERR_MEMOIRE;
        }

        client->commandes = commandesAgrandi;
        client->capaciteCommandes = nouvelleCapacite;
    }

    nouvelleCommande.numero = boutique->numeroSuivant;
    boutique->numeroSuivant++;
    nouvelleCommande.statut = EN_COURS;
    nouvelleCommande.montantHT = 0.0f;
    nouvelleCommande.lignes = NULL;
    nouvelleCommande.nbLignes = 0;
    nouvelleCommande.capaciteLignes = 0;

    client->commandes[client->nbCommandes] = nouvelleCommande;
    client->nbCommandes++;

    *numeroCommande = nouvelleCommande.numero;
    return OK;
}
void recalculerMontantsCommande(Commande *commande)
{
    int i;
    float total = 0.0f;

    if (commande == NULL) {
        return;
    }

    if (commande->lignes != NULL) {
        for (i = 0; i < commande->nbLignes; i++) {
            total += commande->lignes[i].prixUnitaireHT
                   * commande->lignes[i].quantite;
        }
    }

    commande->montantHT = total;
}