/* client.c - fonctions metier du module Clients. */
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"

/* Verifie qu'un tableau contient une chaine terminee et non vide. */
static int chaineNonVide(const char *s, size_t taille)
{
    size_t i;
    int contenu = 0;

    if (s == NULL)
        return 0;

    for (i = 0; i < taille && s[i] != '\0'; i++) {
        if (!isspace((unsigned char)s[i]))
            contenu = 1;
    }

    return i < taille && contenu;
}

/* Compare deux emails sans distinguer les majuscules des minuscules. */
static int emailsEgaux(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0') {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return 0;
        a++;
        b++;
    }

    return *a == '\0' && *b == '\0';
}

static int emailValide(const char *email)
{
    const char *arobase;

    if (!chaineNonVide(email, TAILLE_EMAIL))
        return 0;

    arobase = strchr(email, '@');
    return arobase != NULL && arobase != email && arobase[1] != '\0';
}

/* Le telephone et le code postal doivent contenir exactement des chiffres. */
static int chiffresExacts(const char *s, size_t taille, size_t longueur)
{
    size_t i;

    for (i = 0; i < longueur; i++) {
        if (i >= taille || s[i] < '0' || s[i] > '9')
            return 0;
    }

    return longueur < taille && s[longueur] == '\0';
}

static int clientValide(const Client *client)
{
    return client != NULL
        && chaineNonVide(client->nom, sizeof client->nom)
        && chaineNonVide(client->prenom, sizeof client->prenom)
        && emailValide(client->email)
        && chiffresExacts(client->telephone, sizeof client->telephone, 10)
        && chaineNonVide(client->adresse.rue, sizeof client->adresse.rue)
        && chaineNonVide(client->adresse.ville, sizeof client->adresse.ville)
        && chiffresExacts(client->adresse.codePostal,
                          sizeof client->adresse.codePostal, 5)
        && chaineNonVide(client->adresse.pays, sizeof client->adresse.pays);
}

/* Les emails restent reserves meme si le client est desactive. */
static int emailDejaUtilise(const Boutique *b, const char *email)
{
    return emailExiste(b, email, 0);
}

static CodeRetour agrandirRepertoireClients(Boutique *b)
{
    int nouvelleCapacite;
    Client *nouveauxClients;

    if (b->capaciteClients == 0) {
        nouvelleCapacite = CAPACITE_INITIALE;
    } else {
        if (b->capaciteClients > INT_MAX / 2)
            return ERR_MEMOIRE;
        nouvelleCapacite = b->capaciteClients * 2;
    }

    if ((size_t)nouvelleCapacite > SIZE_MAX / sizeof *nouveauxClients)
        return ERR_MEMOIRE;

    nouveauxClients = realloc(b->clients,
                              (size_t)nouvelleCapacite * sizeof *nouveauxClients);
    if (nouveauxClients == NULL)
        return ERR_MEMOIRE;

    b->clients = nouveauxClients;
    b->capaciteClients = nouvelleCapacite;
    return OK;
}

CodeRetour ajouterClient(Boutique *b, const Client *client, int *idAttribue)
{
    Client nouveauClient;
    CodeRetour resultat;

    if (b == NULL || idAttribue == NULL || !clientValide(client))
        return ERR_INVALIDE;

    if (b->nbClients < 0 || b->capaciteClients < 0
        || b->nbClients > b->capaciteClients
        || (b->nbClients > 0 && b->clients == NULL)
        || b->idClientSuivant <= 0 || b->idClientSuivant == INT_MAX)
        return ERR_INVALIDE;

    if (emailDejaUtilise(b, client->email))
        return ERR_DOUBLON;

    if (b->nbClients == b->capaciteClients) {
        resultat = agrandirRepertoireClients(b);
        if (resultat != OK)
            return resultat;
    }

    nouveauClient = *client;
    nouveauClient.idClient = b->idClientSuivant;
    nouveauClient.pointsFidelite = 0;
    nouveauClient.nbCommandes = 0;
    nouveauClient.capaciteCommandes = 0;
    nouveauClient.commandes = NULL;
    nouveauClient.actif = 1;

    b->clients[b->nbClients] = nouveauClient;
    b->nbClients++;
    b->idClientSuivant++;
    *idAttribue = nouveauClient.idClient;

    return OK;
}

int trouverClient(const Boutique *b, int idClient)
{
    int i;

    if (b == NULL || b->clients == NULL || b->nbClients <= 0 || idClient <= 0)
        return -1;

    for (i = 0; i < b->nbClients; i++) {
        if (b->clients[i].idClient == idClient)
            return i;
    }

    return -1;
}

int emailExiste(const Boutique *b, const char *email, int idIgnore)
{
    int i;

    if (b == NULL || email == NULL || b->clients == NULL || b->nbClients <= 0)
        return 0;

    for (i = 0; i < b->nbClients; i++) {
        if (b->clients[i].idClient != idIgnore
            && emailsEgaux(b->clients[i].email, email))
            return 1;
    }

    return 0;
}

CodeRetour modifierClient(Boutique *b, int idClient, const Client *infos)
{
    int indice;
    Client *client;

    if (b == NULL || infos == NULL)
        return ERR_INVALIDE;

    indice = trouverClient(b, idClient);
    if (indice < 0 || !b->clients[indice].actif)
        return ERR_INTROUVABLE;

    if (!clientValide(infos))
        return ERR_INVALIDE;

    if (emailExiste(b, infos->email, idClient))
        return ERR_DOUBLON;

    client = &b->clients[indice];
    memcpy(client->nom, infos->nom, sizeof client->nom);
    memcpy(client->prenom, infos->prenom, sizeof client->prenom);
    memcpy(client->email, infos->email, sizeof client->email);
    memcpy(client->telephone, infos->telephone, sizeof client->telephone);
    client->adresse = infos->adresse;

    return OK;
}

CodeRetour supprimerClient(Boutique *b, int idClient)
{
    int indice;
    int i;
    Client *client;

    if (b == NULL)
        return ERR_INVALIDE;

    indice = trouverClient(b, idClient);
    if (indice < 0 || !b->clients[indice].actif)
        return ERR_INTROUVABLE;

    client = &b->clients[indice];
    if (client->nbCommandes < 0
        || client->nbCommandes > client->capaciteCommandes
        || (client->nbCommandes > 0 && client->commandes == NULL))
        return ERR_INVALIDE;

    for (i = 0; i < client->nbCommandes; i++) {
        if (client->commandes[i].statut != ANNULEE)
            return ERR_INTERDIT;
    }

    client->actif = 0;
    return OK;
}

float chiffreAffairesClient(const Client *client)
{
    float chiffreAffaires = 0.0f;
    int i;

    if (client == NULL || client->nbCommandes <= 0
        || client->nbCommandes > client->capaciteCommandes
        || client->commandes == NULL)
        return 0.0f;

    for (i = 0; i < client->nbCommandes; i++) {
        if (client->commandes[i].statut != ANNULEE)
            chiffreAffaires += client->commandes[i].montantHT;
    }

    return chiffreAffaires;
}

void afficherListeClients(const Boutique *b)
{
    int i;
    int affiches = 0;

    if (b == NULL) {
        puts("Erreur : boutique indisponible.");
        return;
    }

    if (b->nbClients < 0 || b->nbClients > b->capaciteClients
        || (b->nbClients > 0 && b->clients == NULL)) {
        puts("Erreur : repertoire clients incoherent.");
        return;
    }

    puts("\n--- Liste des clients ---");
    puts("ID | Client | Email | Commandes | CA HT");

    for (i = 0; i < b->nbClients; i++) {
        const Client *client = &b->clients[i];

        if (client->actif) {
            printf("%d | %s %s | %s | %d | %.2f EUR\n",
                   client->idClient, client->nom, client->prenom, client->email,
                   client->nbCommandes, chiffreAffairesClient(client));
            affiches++;
        }
    }

    if (affiches == 0)
        puts("Aucun client actif.");
}

/* Lit une ligne complete; refuse les valeurs trop longues au lieu de les couper. */
static int lireChamp(const char *invite, char *destination, size_t capacite)
{
    size_t longueur;
    int caractere;

    fputs(invite, stdout);
    if (fgets(destination, (int)capacite, stdin) == NULL)
        return 0;

    longueur = strlen(destination);
    if (longueur > 0 && destination[longueur - 1] == '\n') {
        destination[longueur - 1] = '\0';
        longueur--;
    } else {
        caractere = getchar();
        if (caractere == '\r')
            caractere = getchar();
        if (caractere != '\n' && caractere != EOF) {
            do {
                caractere = getchar();
            } while (caractere != '\n' && caractere != EOF);
            return 0;
        }
    }

    if (longueur > 0 && destination[longueur - 1] == '\r')
        destination[longueur - 1] = '\0';

    return 1;
}

static int lireIdClient(int *idClient)
{
    char saisie[32];
    char *fin;
    long valeur;

    if (!lireChamp("Identifiant client : ", saisie, sizeof saisie))
        return 0;

    errno = 0;
    valeur = strtol(saisie, &fin, 10);
    while (isspace((unsigned char)*fin))
        fin++;

    if (errno != 0 || fin == saisie || *fin != '\0'
        || valeur <= 0 || valeur > INT_MAX)
        return 0;

    *idClient = (int)valeur;
    return 1;
}

void menuAjouterClient(Boutique *b)
{
    Client client = {0};
    int idAttribue;
    CodeRetour resultat;

    if (b == NULL) {
        puts("Erreur : boutique indisponible.");
        return;
    }

    puts("\n--- Creation d'un client ---");
    if (!lireChamp("Nom : ", client.nom, sizeof client.nom)
        || !lireChamp("Prenom : ", client.prenom, sizeof client.prenom)
        || !lireChamp("Email : ", client.email, sizeof client.email)
        || !lireChamp("Telephone (10 chiffres) : ", client.telephone,
                      sizeof client.telephone)
        || !lireChamp("Rue : ", client.adresse.rue, sizeof client.adresse.rue)
        || !lireChamp("Ville : ", client.adresse.ville,
                      sizeof client.adresse.ville)
        || !lireChamp("Code postal (5 chiffres) : ",
                      client.adresse.codePostal,
                      sizeof client.adresse.codePostal)
        || !lireChamp("Pays : ", client.adresse.pays,
                      sizeof client.adresse.pays)) {
        puts("Saisie interrompue ou trop longue; client non cree.");
        return;
    }

    resultat = ajouterClient(b, &client, &idAttribue);
    switch (resultat) {
        case OK:
            printf("Client cree avec l'identifiant %d.\n", idAttribue);
            break;
        case ERR_DOUBLON:
            puts("Erreur : un client utilise deja cette adresse email.");
            break;
        case ERR_MEMOIRE:
            puts("Erreur : memoire insuffisante pour ajouter le client.");
            break;
        case ERR_INVALIDE:
            puts("Erreur : informations invalides. Verifiez les champs saisis.");
            break;
        default:
            puts("Erreur inattendue pendant la creation du client.");
            break;
    }
}

void menuModifierClient(Boutique *b)
{
    Client infos = {0};
    int idClient;
    CodeRetour resultat;

    if (b == NULL || !lireIdClient(&idClient)) {
        puts("Identifiant client invalide.");
        return;
    }

    puts("--- Nouvelles coordonnees et adresse de livraison ---");
    if (!lireChamp("Nom : ", infos.nom, sizeof infos.nom)
        || !lireChamp("Prenom : ", infos.prenom, sizeof infos.prenom)
        || !lireChamp("Email : ", infos.email, sizeof infos.email)
        || !lireChamp("Telephone (10 chiffres) : ", infos.telephone,
                      sizeof infos.telephone)
        || !lireChamp("Rue : ", infos.adresse.rue, sizeof infos.adresse.rue)
        || !lireChamp("Ville : ", infos.adresse.ville,
                      sizeof infos.adresse.ville)
        || !lireChamp("Code postal (5 chiffres) : ",
                      infos.adresse.codePostal,
                      sizeof infos.adresse.codePostal)
        || !lireChamp("Pays : ", infos.adresse.pays,
                      sizeof infos.adresse.pays)) {
        puts("Saisie interrompue ou trop longue; client non modifie.");
        return;
    }

    resultat = modifierClient(b, idClient, &infos);
    switch (resultat) {
        case OK:
            puts("Coordonnees et adresse du client mises a jour.");
            break;
        case ERR_DOUBLON:
            puts("Erreur : un autre client utilise deja cette adresse email.");
            break;
        case ERR_INTROUVABLE:
            puts("Erreur : client introuvable ou desactive.");
            break;
        case ERR_INVALIDE:
            puts("Erreur : informations invalides. Verifiez les champs saisis.");
            break;
        default:
            puts("Erreur inattendue pendant la modification du client.");
            break;
    }
}

void menuSupprimerClient(Boutique *b)
{
    int idClient;
    CodeRetour resultat;

    if (b == NULL || !lireIdClient(&idClient)) {
        puts("Identifiant client invalide.");
        return;
    }

    resultat = supprimerClient(b, idClient);
    switch (resultat) {
        case OK:
            puts("Client supprime (suppression logique).");
            break;
        case ERR_INTROUVABLE:
            puts("Erreur : client introuvable ou deja desactive.");
            break;
        case ERR_INTERDIT:
            puts("Suppression refusee : ce client a une commande non annulee.");
            break;
        case ERR_INVALIDE:
            puts("Erreur : donnees client incoherentes.");
            break;
        default:
            puts("Erreur inattendue pendant la suppression du client.");
            break;
    }
}
