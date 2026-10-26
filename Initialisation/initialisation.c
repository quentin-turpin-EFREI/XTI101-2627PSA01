/* initialisation.c - jeu de donnees de demonstration charge en memoire. */
#include <stdlib.h>
#include <string.h>

#include "initialisation.h"
#include "client.h"

static void remplirClient(Client *client,
                          const char *nom,
                          const char *prenom,
                          const char *email,
                          const char *telephone,
                          const char *rue,
                          const char *ville,
                          const char *codePostal,
                          const char *pays)
{
    strcpy(client->nom, nom);
    strcpy(client->prenom, prenom);
    strcpy(client->email, email);
    strcpy(client->telephone, telephone);
    strcpy(client->adresse.rue, rue);
    strcpy(client->adresse.ville, ville);
    strcpy(client->adresse.codePostal, codePostal);
    strcpy(client->adresse.pays, pays);
}

static CodeRetour ajouterClientDemo(Boutique *boutique,
                                    const Client *infos,
                                    const Commande *commandes,
                                    int nbCommandes)
{
    Commande *copiesCommandes = NULL;
    int idAttribue;
    CodeRetour resultat;

    if (nbCommandes > 0) {
        copiesCommandes = malloc((size_t)nbCommandes * sizeof *copiesCommandes);
        if (copiesCommandes == NULL)
            return ERR_MEMOIRE;
        memcpy(copiesCommandes, commandes,
               (size_t)nbCommandes * sizeof *copiesCommandes);
    }

    resultat = ajouterClient(boutique, infos, &idAttribue);
    if (resultat != OK) {
        free(copiesCommandes);
        return resultat;
    }

    boutique->clients[boutique->nbClients - 1].commandes = copiesCommandes;
    boutique->clients[boutique->nbClients - 1].nbCommandes = nbCommandes;
    boutique->clients[boutique->nbClients - 1].capaciteCommandes = nbCommandes;
    return OK;
}

CodeRetour initialiserDonneesTest(Boutique *boutique)
{
    Boutique donnees = {0};
    Client client = {0};
    Commande commandesAline[] = {
        {1001, EN_COURS, 149.90f, NULL, 0, 0},
        {1002, ANNULEE, 39.90f, NULL, 0, 0}
    };
    Commande commandeKarim[] = {
        {1003, LIVREE, 89.50f, NULL, 0, 0}
    };
    Commande commandeSophie[] = {
        {1004, ANNULEE, 30.00f, NULL, 0, 0}
    };
    CodeRetour resultat;

    if (boutique == NULL)
        return ERR_INVALIDE;

    donnees.idClientSuivant = 1;
    donnees.numeroSuivant = 1005;
    strcpy(donnees.nom, "ShopFlow Demo");

    remplirClient(&client, "Martin", "Aline", "aline.martin@example.fr",
                  "0611111111", "10 rue Victor Hugo", "Paris", "75001",
                  "France");
    resultat = ajouterClientDemo(&donnees, &client, commandesAline, 2);
    if (resultat != OK)
        goto erreur;

    memset(&client, 0, sizeof client);
    remplirClient(&client, "Bernard", "Karim", "karim.bernard@example.fr",
                  "0622222222", "24 avenue des Fleurs", "Lyon", "69001",
                  "France");
    resultat = ajouterClientDemo(&donnees, &client, commandeKarim, 1);
    if (resultat != OK)
        goto erreur;

    memset(&client, 0, sizeof client);
    remplirClient(&client, "Leroy", "Sophie", "sophie.leroy@example.fr",
                  "0633333333", "8 place du Marche", "Nantes", "44000",
                  "France");
    resultat = ajouterClientDemo(&donnees, &client, commandeSophie, 1);
    if (resultat != OK)
        goto erreur;

    *boutique = donnees;
    return OK;

erreur:
    detruireBoutique(&donnees);
    return resultat;
}

void detruireBoutique(Boutique *boutique)
{
    int i;

    if (boutique == NULL)
        return;

    for (i = 0; i < boutique->nbClients; i++)
        free(boutique->clients[i].commandes);

    free(boutique->clients);
    free(boutique->produits);
    memset(boutique, 0, sizeof *boutique);
}
