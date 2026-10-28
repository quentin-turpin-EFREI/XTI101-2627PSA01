/* client_saisie.c - saisie et menus des actions clients. */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"
#include "commande.h"

/* Helper local partage par la fiche et les formulaires de saisie. */
static int lireChamp(const char *invite, char *destination, size_t capacite);

static int saisirAdresse(Adresse *adresse)
{
    return adresse != NULL
        && lireChamp("Rue : ", adresse->rue, sizeof adresse->rue)
        && lireChamp("Ville : ", adresse->ville, sizeof adresse->ville)
        && lireChamp("Code postal (5 chiffres) : ", adresse->codePostal,
                     sizeof adresse->codePostal)
        && lireChamp("Pays : ", adresse->pays, sizeof adresse->pays);
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
    printf("Points de fidelite : %d\n", client->pointsFidelite);
    printf("Commandes : %d | Chiffre d'affaires : %.2f EUR HT\n",
           client->nbCommandes, chiffreAffairesClient(client));
    afficherHistoriqueCommandes(client);
}

/* Lit une ligne complete et refuse les champs trop longs. */
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
            /* Vide le reste de la ligne pour ne pas polluer la saisie suivante. */
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

/* Le menu collecte les champs; ajouterClient applique ensuite les regles metier. */
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
        || !saisirAdresse(&client.adresse)) {
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
        || !saisirAdresse(&infos.adresse)) {
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
