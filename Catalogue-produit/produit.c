/* produit.c - catalogue produits, gestion du stock et saisies du module */
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "produit.h"

/* Refuse les chaines vides, blanches ou sans terminaison dans le tableau. */
static int chaineValide(const char *texte, size_t taille)
{
    size_t i;
    int nonVide = 0;
    if (texte == NULL)
        return 0;
    for (i = 0; i < taille; i++) {
        if (texte[i] == '\0')
            return nonVide;
        if (!isspace((unsigned char)texte[i]))
            nonVide = 1;
    }
    return 0;
}

static int catalogueValide(const Boutique *b)
{
    return b != NULL && b->nbProduits >= 0
        && b->capaciteProduits >= b->nbProduits
        && (b->capaciteProduits == 0 || b->produits != NULL);
}

int trouverProduit(const Boutique *b, const char *reference)
{
    int i;
    if (!catalogueValide(b) || !chaineValide(reference, TAILLE_REFERENCE))
        return -1;
    /* Les references des produits supprimes restent reservees. */
    for (i = 0; i < b->nbProduits; i++)
        if (strcmp(b->produits[i].reference, reference) == 0)
            return i;
    return -1;
}

static CodeRetour agrandirCatalogue(Boutique *b)
{
    int capacite;
    Produit *nouveaux;
    if (b->capaciteProduits > INT_MAX / 2)
        return ERR_MEMOIRE;
    capacite = b->capaciteProduits == 0 ? CAPACITE_INITIALE
                                      : b->capaciteProduits * 2;
    if ((size_t)capacite > SIZE_MAX / sizeof *nouveaux)
        return ERR_MEMOIRE;
    nouveaux = realloc(b->produits, (size_t)capacite * sizeof *nouveaux);
    if (nouveaux == NULL)
        return ERR_MEMOIRE;
    b->produits = nouveaux;
    b->capaciteProduits = capacite;
    return OK;
}

CodeRetour ajouterProduit(Boutique *b, const Produit *produit)
{
    CodeRetour resultat;
    Produit copie;
    if (!catalogueValide(b) || produit == NULL
        || !chaineValide(produit->reference, TAILLE_REFERENCE)
        || !chaineValide(produit->designation, TAILLE_DESIGNATION)
        || !chaineValide(produit->categorie, TAILLE_CATEGORIE)
        || !isfinite(produit->prixUnitaireHT) || produit->prixUnitaireHT <= 0
        || produit->quantiteStock < 0 || produit->seuilAlerte < 0)
        return ERR_INVALIDE;
    if (trouverProduit(b, produit->reference) >= 0)
        return ERR_DOUBLON;
    /* Copier avant realloc, au cas ou produit appartient deja au tableau. */
    copie = *produit;
    copie.actif = 1;
    if (b->nbProduits == b->capaciteProduits) {
        resultat = agrandirCatalogue(b);
        if (resultat != OK)
            return resultat;
    }
    b->produits[b->nbProduits++] = copie;
    return OK;
}

CodeRetour modifierProduit(Boutique *b, const char *reference,
                          float prix, int stock, int seuil)
{
    int indice;
    if (!catalogueValide(b) || !chaineValide(reference, TAILLE_REFERENCE)
        || !isfinite(prix) || prix <= 0 || stock < 0 || seuil < 0)
        return ERR_INVALIDE;
    indice = trouverProduit(b, reference);
    if (indice < 0)
        return ERR_INTROUVABLE;
    if (!b->produits[indice].actif)
        return ERR_INTERDIT;
    b->produits[indice].prixUnitaireHT = prix;
    b->produits[indice].quantiteStock = stock;
    b->produits[indice].seuilAlerte = seuil;
    return OK;
}

CodeRetour supprimerProduit(Boutique *b, const char *reference)
{
    int indice, i, j, k;
    if (!catalogueValide(b) || !chaineValide(reference, TAILLE_REFERENCE))
        return ERR_INVALIDE;
    indice = trouverProduit(b, reference);
    if (indice < 0)
        return ERR_INTROUVABLE;
    if (!b->produits[indice].actif)
        return ERR_INTERDIT;
    for (i = 0; i < b->nbClients; i++) {
        const Client *client = &b->clients[i];
        for (j = 0; j < client->nbCommandes; j++) {
            const Commande *commande = &client->commandes[j];
            if (strcmp(commande->statut, "ANNULEE") == 0)
                continue;
            /* Une commande livree bloque elle aussi la suppression. */
            for (k = 0; k < commande->nbLignes; k++)
                if (strcmp(commande->lignes[k].refProduit, reference) == 0)
                    return ERR_INTERDIT;
        }
    }
    b->produits[indice].actif = 0;
    return OK;
}

CodeRetour retirerStock(Boutique *b, const char *reference, int quantite)
{
    int indice;
    if (!catalogueValide(b) || !chaineValide(reference, TAILLE_REFERENCE)
        || quantite <= 0)
        return ERR_INVALIDE;
    indice = trouverProduit(b, reference);
    if (indice < 0)
        return ERR_INTROUVABLE;
    if (!b->produits[indice].actif || b->produits[indice].quantiteStock < quantite)
        return ERR_INTERDIT;
    b->produits[indice].quantiteStock -= quantite;
    return OK;
}

CodeRetour remettreStock(Boutique *b, const char *reference, int quantite)
{
    int indice;
    if (!catalogueValide(b) || !chaineValide(reference, TAILLE_REFERENCE)
        || quantite <= 0)
        return ERR_INVALIDE;
    indice = trouverProduit(b, reference);
    if (indice < 0)
        return ERR_INTROUVABLE;
    if (b->produits[indice].quantiteStock > INT_MAX - quantite)
        return ERR_INVALIDE;
    /* La restitution reste possible sur un produit desactive. */
    b->produits[indice].quantiteStock += quantite;
    return OK;
}

float valeurStock(const Boutique *b)
{
    int i;
    float total = 0;
    if (!catalogueValide(b))
        return 0;
    for (i = 0; i < b->nbProduits; i++)
        if (b->produits[i].actif)
            total += b->produits[i].prixUnitaireHT * b->produits[i].quantiteStock;
    return total;
}

void afficherProduit(const Produit *produit)
{
    if (produit == NULL)
        return;
    printf("%-11s | %-59s | %-29s | %10.2f | %6d | %6d | %12.2f\n",
           produit->reference, produit->designation, produit->categorie,
           produit->prixUnitaireHT, produit->quantiteStock, produit->seuilAlerte,
           (double)produit->prixUnitaireHT * produit->quantiteStock);
}

static void afficherEntete(void)
{
    printf("Reference   | Designation                                                 | "
           "Categorie                     |    Prix HT |  Stock |  Seuil | Valeur HT\n");
}

void afficherCatalogue(const Boutique *b)
{
    int i, nombre = 0;
    if (!catalogueValide(b))
        return;
    afficherEntete();
    for (i = 0; i < b->nbProduits; i++)
        if (b->produits[i].actif) {
            afficherProduit(&b->produits[i]);
            nombre++;
        }
    printf("%d produit(s) actif(s). Valeur totale du stock HT : %.2f EUR\n",
           nombre, valeurStock(b));
}

void afficherAlertesStock(const Boutique *b)
{
    int i, nombre = 0;
    if (!catalogueValide(b))
        return;
    afficherEntete();
    for (i = 0; i < b->nbProduits; i++) {
        const Produit *p = &b->produits[i];
        if (p->actif && p->quantiteStock <= p->seuilAlerte) {
            afficherProduit(p);
            nombre++;
        }
    }
    printf("%d produit(s) a reapprovisionner.\n", nombre);
}

/* Lectures locales tant que le module utils n'est pas implemente.
 * EOF annule l'operation ; une ligne trop longue est rejetee en entier. */
static int lireTexte(const char *invite, char *destination, int taille)
{
    for (;;) {
        int c;
        printf("%s", invite);
        fflush(stdout);
        if (fgets(destination, taille, stdin) == NULL)
            return 0;
        if (strchr(destination, '\n') == NULL) {
            c = getchar();
            if (c != '\n' && c != EOF) {
                while ((c = getchar()) != '\n' && c != EOF)
                    ;
                printf("Saisie trop longue (maximum %d caracteres).\n", taille - 1);
                continue;
            }
        }
        destination[strcspn(destination, "\r\n")] = '\0';
        if (chaineValide(destination, (size_t)taille))
            return 1;
        printf("La saisie ne doit pas etre vide.\n");
    }
}

static int finNombreValide(const char *fin)
{
    while (isspace((unsigned char)*fin))
        fin++;
    return *fin == '\0';
}

static int lireEntierPositif(const char *invite, int *valeur)
{
    char texte[80], *fin;
    long nombre;
    while (lireTexte(invite, texte, sizeof texte)) {
        errno = 0;
        nombre = strtol(texte, &fin, 10);
        if (fin != texte && errno == 0 && finNombreValide(fin)
            && nombre >= 0 && nombre <= INT_MAX) {
            *valeur = (int)nombre;
            return 1;
        }
        printf("Saisissez un entier entre 0 et %d.\n", INT_MAX);
    }
    return 0;
}

static int lirePrix(float *prix)
{
    char texte[80], *fin;
    float nombre;
    while (lireTexte("Prix unitaire HT : ", texte, sizeof texte)) {
        errno = 0;
        nombre = strtof(texte, &fin);
        if (fin != texte && errno == 0 && finNombreValide(fin)
            && isfinite(nombre) && nombre > 0) {
            *prix = nombre;
            return 1;
        }
        printf("Saisissez un prix strictement positif (exemple : 19.90).\n");
    }
    return 0;
}

static void afficherResultat(CodeRetour resultat)
{
    switch (resultat) {
        case OK: printf("Operation effectuee.\n"); break;
        case ERR_MEMOIRE: printf("Memoire insuffisante.\n"); break;
        case ERR_DOUBLON: printf("Cette reference existe deja, meme si desactivee.\n"); break;
        case ERR_INTROUVABLE: printf("Produit introuvable.\n"); break;
        case ERR_INVALIDE: printf("Donnees invalides.\n"); break;
        case ERR_INTERDIT:
            printf("Operation interdite : produit desactive ou utilise dans une commande non annulee.\n");
            break;
    }
}

void menuAjouterProduit(Boutique *b)
{
    Produit produit = {0};
    if (!lireTexte("Reference : ", produit.reference, sizeof produit.reference)
        || !lireTexte("Designation : ", produit.designation, sizeof produit.designation)
        || !lireTexte("Categorie : ", produit.categorie, sizeof produit.categorie)
        || !lirePrix(&produit.prixUnitaireHT)
        || !lireEntierPositif("Stock : ", &produit.quantiteStock)
        || !lireEntierPositif("Seuil d'alerte : ", &produit.seuilAlerte))
        return;
    afficherResultat(ajouterProduit(b, &produit));
}

void menuModifierProduit(Boutique *b)
{
    char reference[TAILLE_REFERENCE];
    float prix;
    int stock, seuil, indice;
    if (!lireTexte("Reference : ", reference, sizeof reference))
        return;
    indice = trouverProduit(b, reference);
    if (indice < 0 || !b->produits[indice].actif) {
        afficherResultat(indice < 0 ? ERR_INTROUVABLE : ERR_INTERDIT);
        return;
    }
    afficherProduit(&b->produits[indice]);
    if (!lirePrix(&prix) || !lireEntierPositif("Nouveau stock : ", &stock)
        || !lireEntierPositif("Nouveau seuil : ", &seuil))
        return;
    afficherResultat(modifierProduit(b, reference, prix, stock, seuil));
}

void menuSupprimerProduit(Boutique *b)
{
    char reference[TAILLE_REFERENCE];
    if (lireTexte("Reference : ", reference, sizeof reference))
        afficherResultat(supprimerProduit(b, reference));
}

static int contientMotCle(const char *texte, const char *mot)
{
    size_t i, j;
    for (i = 0; texte[i] != '\0'; i++) {
        for (j = 0; mot[j] != '\0' && texte[i + j] != '\0'; j++)
            if (tolower((unsigned char)texte[i + j]) != tolower((unsigned char)mot[j]))
                break;
        if (mot[j] == '\0')
            return 1;
    }
    return 0;
}

void menuRechercherProduit(const Boutique *b)
{
    char recherche[TAILLE_DESIGNATION];
    int choix, i, nombre = 0;
    if (!catalogueValide(b))
        return;
    do {
        if (!lireEntierPositif("1. Reference exacte / 2. Mot-cle de designation : ", &choix))
            return;
    } while (choix != 1 && choix != 2);
    if (!lireTexte("Recherche : ", recherche,
                   choix == 1 ? TAILLE_REFERENCE : TAILLE_DESIGNATION))
        return;
    afficherEntete();
    for (i = 0; i < b->nbProduits; i++) {
        const Produit *p = &b->produits[i];
        if (p->actif && ((choix == 1 && strcmp(p->reference, recherche) == 0)
            || (choix == 2 && contientMotCle(p->designation, recherche)))) {
            afficherProduit(p);
            nombre++;
        }
    }
    printf("%d produit(s) trouve(s).\n", nombre);
}
