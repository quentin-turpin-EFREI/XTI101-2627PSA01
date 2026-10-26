/* client.c - fonctions metier du module Clients. */
#include <ctype.h>
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
    int i;

    for (i = 0; i < b->nbClients; i++) {
        if (emailsEgaux(b->clients[i].email, email))
            return 1;
    }

    return 0;
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

static int lireChamp(const char *invite, char *destination, size_t capacite);

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

/* Recherche parmi tous les clients, y compris les clients desactives. */
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

    if (b == NULL || email == NULL || b->clients == NULL || b->nbClients < 0)
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
    int position;
    Client *client;
    Client copieInfos;

    if (b == NULL || !clientValide(infos))
        return ERR_INVALIDE;

    position = trouverClient(b, idClient);
    if (position < 0 || !b->clients[position].actif)
        return ERR_INTROUVABLE;

    if (emailExiste(b, infos->email, idClient))
        return ERR_DOUBLON;

    copieInfos = *infos;
    client = &b->clients[position];
    memcpy(client->nom, copieInfos.nom, sizeof client->nom);
    memcpy(client->prenom, copieInfos.prenom, sizeof client->prenom);
    memcpy(client->email, copieInfos.email, sizeof client->email);
    memcpy(client->telephone, copieInfos.telephone, sizeof client->telephone);
    client->adresse = copieInfos.adresse;
    return OK;
}

CodeRetour supprimerClient(Boutique *b, int idClient)
{
    int position;
    int i;
    Client *client;

    if (b == NULL)
        return ERR_INVALIDE;

    position = trouverClient(b, idClient);
    if (position < 0 || !b->clients[position].actif)
        return ERR_INTROUVABLE;

    client = &b->clients[position];
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
    float total = 0.0f;
    int i;

    if (client == NULL || client->nbCommandes <= 0
        || client->nbCommandes > client->capaciteCommandes
        || client->commandes == NULL)
        return total;

    for (i = 0; i < client->nbCommandes; i++) {
        if (client->commandes[i].statut != ANNULEE)
            total += client->commandes[i].montantHT;
    }

    return total;
}

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

void afficherFicheClient(const Boutique *b)
{
    char saisie[32];
    char *fin;
    long id;
    int position;
    const Client *client;

    if (b == NULL || !lireChamp("Identifiant du client : ", saisie,
                                sizeof saisie))
        return;

    id = strtol(saisie, &fin, 10);
    if (*saisie == '\0' || *fin != '\0' || id <= 0 || id > INT_MAX) {
        puts("Identifiant invalide.");
        return;
    }

    position = trouverClient(b, (int)id);
    if (position < 0 || !b->clients[position].actif) {
        puts("Client introuvable.");
        return;
    }

    client = &b->clients[position];
    printf("%d | %s %s | %s | %s\n",
           client->idClient, client->nom, client->prenom,
           client->email, client->telephone);
    printf("%s, %s %s, %s\n", client->adresse.rue,
           client->adresse.codePostal, client->adresse.ville,
           client->adresse.pays);
    printf("Commandes : %d | Chiffre d'affaires : %.2f EUR\n",
           client->nbCommandes, chiffreAffairesClient(client));
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

static int lireIdentifiant(int *id)
{
    char saisie[32];
    char *fin;
    long valeur;

    if (!lireChamp("Identifiant du client : ", saisie, sizeof saisie))
        return 0;

    valeur = strtol(saisie, &fin, 10);
    if (*saisie == '\0' || *fin != '\0' || valeur <= 0 || valeur > INT_MAX)
        return 0;

    *id = (int)valeur;
    return 1;
}

void menuModifierClient(Boutique *b)
{
    Client infos = {0};
    int id;
    CodeRetour resultat;

    if (b == NULL || !lireIdentifiant(&id)) {
        puts("Identifiant invalide; modification annulee.");
        return;
    }

    if (!lireChamp("Nom : ", infos.nom, sizeof infos.nom)
        || !lireChamp("Prenom : ", infos.prenom, sizeof infos.prenom)
        || !lireChamp("Email : ", infos.email, sizeof infos.email)
        || !lireChamp("Telephone (10 chiffres) : ", infos.telephone,
                      sizeof infos.telephone)
        || !lireChamp("Rue : ", infos.adresse.rue,
                      sizeof infos.adresse.rue)
        || !lireChamp("Ville : ", infos.adresse.ville,
                      sizeof infos.adresse.ville)
        || !lireChamp("Code postal (5 chiffres) : ",
                      infos.adresse.codePostal,
                      sizeof infos.adresse.codePostal)
        || !lireChamp("Pays : ", infos.adresse.pays,
                      sizeof infos.adresse.pays)) {
        puts("Saisie interrompue ou trop longue; modification annulee.");
        return;
    }

    resultat = modifierClient(b, id, &infos);
    switch (resultat) {
        case OK:
            puts("Coordonnees et adresse du client mises a jour.");
            break;
        case ERR_DOUBLON:
            puts("Modification refusee : cette adresse email est deja utilisee.");
            break;
        case ERR_INTROUVABLE:
            puts("Client introuvable.");
            break;
        default:
            puts("Informations invalides; modification refusee.");
            break;
    }
}

void menuSupprimerClient(Boutique *b)
{
    int id;
    CodeRetour resultat;

    if (b == NULL || !lireIdentifiant(&id)) {
        puts("Identifiant invalide; suppression annulee.");
        return;
    }

    resultat = supprimerClient(b, id);
    switch (resultat) {
        case OK:
            puts("Client supprime (suppression logique).");
            break;
        case ERR_INTERDIT:
            puts("Suppression refusee : ce client a une commande non annulee.");
            break;
        case ERR_INTROUVABLE:
            puts("Client introuvable.");
            break;
        default:
            puts("Suppression impossible.");
            break;
    }
}
