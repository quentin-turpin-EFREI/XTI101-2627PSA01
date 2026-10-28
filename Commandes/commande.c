/* commande.c - commandes : creation, gestion des lignes, calcul des montants */
#include <stdio.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "commande.h"
#include "client.h"

CodeRetour creerCommande(Boutique *boutique, int idClient,
                         int *numeroCommande)
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
    if (indiceClient < 0 || !boutique->clients[indiceClient].actif) {
        return ERR_INTROUVABLE;
    }

    client = &boutique->clients[indiceClient];

    if (client->nbCommandes < 0 || client->capaciteCommandes < 0
        || client->nbCommandes > client->capaciteCommandes
        || (client->nbCommandes > 0 && client->commandes == NULL)) {
        return ERR_INVALIDE;
    }

    if (client->nbCommandes >= client->capaciteCommandes) {
        if (client->capaciteCommandes == 0) {
            nouvelleCapacite = CAPACITE_INITIALE;
        } else {
            if (client->capaciteCommandes > INT_MAX / 2)
                return ERR_MEMOIRE;
            nouvelleCapacite = client->capaciteCommandes * 2;
        }

        if ((size_t)nouvelleCapacite > SIZE_MAX / sizeof *commandesAgrandi)
            return ERR_MEMOIRE;

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

    if (commande == NULL || commande->nbLignes < 0
        || commande->nbLignes > commande->capaciteLignes) {
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

static const char *nomStatutCommande(StatutCommande statut)
{
    switch (statut) {
        case EN_ATTENTE:
            return "En attente";
        case EN_COURS:
            return "En cours";
        case LIVREE:
            return "Livree";
        case ANNULEE:
            return "Annulee";
        default:
            return "Inconnu";
    }
}

void afficherHistoriqueCommandes(const Client *client)
{
    int i;

    puts("\n--- Historique des commandes ---");
    if (client == NULL || client->nbCommandes < 0
        || client->nbCommandes > client->capaciteCommandes
        || (client->nbCommandes > 0 && client->commandes == NULL)) {
        puts("Erreur : historique des commandes incoherent.");
        return;
    }

    if (client->nbCommandes == 0) {
        puts("Aucune commande.");
        return;
    }

    for (i = 0; i < client->nbCommandes; i++) {
        const Commande *commande = &client->commandes[i];

        printf("%d | %s | %.2f EUR HT\n", commande->numero,
               nomStatutCommande(commande->statut), commande->montantHT);
    }
}
