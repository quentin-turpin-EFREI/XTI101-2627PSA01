/* client.c - regles metier et gestion des donnees clients. */
#include <ctype.h>
#include <limits.h>
#include <math.h>
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

/* Copie les coordonnees, mais initialise les donnees qui appartiennent
 * a la boutique (identifiant, commandes et statut). */
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

    /* Une commande non annulee conserve son lien vers ce client. */
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

    /* Les commandes annulees ne contribuent pas au chiffre d'affaires. */
    for (i = 0; i < client->nbCommandes; i++) {
        if (client->commandes[i].statut != ANNULEE)
            total += client->commandes[i].montantHT;
    }

    return total;
}

/* Convertit un montant TTC en points, apres arrondi au centime. */
static int pointsPourMontant(float montantTTC)
{
    long long montantCentimes;

    if (!isfinite(montantTTC) || montantTTC <= 0.0f)
        return 0;

    /* Au-dela, le nombre de points ne peut de toute facon pas tenir dans int. */
    if ((double)montantTTC >= (double)INT_MAX * 10.0)
        return INT_MAX;

    montantCentimes = (long long)((double)montantTTC * 100.0 + 0.5);
    return (int)(montantCentimes / 1000);
}

void ajouterPointsFidelite(Client *client, float montantTTC)
{
    int points;

    if (client == NULL)
        return;

    points = pointsPourMontant(montantTTC);
    if (client->pointsFidelite < 0)
        client->pointsFidelite = 0;
    if (points > INT_MAX - client->pointsFidelite)
        client->pointsFidelite = INT_MAX;
    else
        client->pointsFidelite += points;
}

void retirerPointsFidelite(Client *client, float montantTTC)
{
    int points;

    if (client == NULL)
        return;

    points = pointsPourMontant(montantTTC);
    if (client->pointsFidelite <= 0 || points >= client->pointsFidelite)
        client->pointsFidelite = 0;
    else
        client->pointsFidelite -= points;
}

void libererClient(Client *client)
{
    int i;

    if (client == NULL)
        return;

    if (client->commandes != NULL && client->nbCommandes >= 0
        && client->nbCommandes <= client->capaciteCommandes) {
        for (i = 0; i < client->nbCommandes; i++)
            free(client->commandes[i].lignes);
    }

    free(client->commandes);
    client->commandes = NULL;
    client->nbCommandes = 0;
    client->capaciteCommandes = 0;
}
