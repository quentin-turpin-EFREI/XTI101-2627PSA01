/* produit.c - catalogue produits : CRUD, stock et affichage. */
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "produit.h"

/* Verifie que le texte contient autre chose que des espaces. */
static int textePresent(const char *texte)
{
    int i;

    if (texte == NULL) {
        return 0;
    }
    for (i = 0; texte[i] != '\0'; i++) {
        if (texte[i] != ' ' && texte[i] != '\t'
            && texte[i] != '\n' && texte[i] != '\r'
            && texte[i] != '\v' && texte[i] != '\f') {
            return 1;
        }
    }
    return 0;
}

/* malloc cree le tableau ; realloc l'agrandit quand il est plein. */
static CodeRetour agrandirCatalogue(Boutique *boutique)
{
    int capacite;
    Produit *nouveauTableau;

    if (boutique->capaciteProduits == 0) {
        capacite = CAPACITE_INITIALE;
        nouveauTableau = malloc(capacite * sizeof(Produit));
    } else {
        /* Evite de depasser la plus grande valeur possible d'un int. */
        if (boutique->capaciteProduits > INT_MAX / 2) {
            return ERR_MEMOIRE;
        }
        capacite = boutique->capaciteProduits * 2;
        nouveauTableau = realloc(boutique->produits, capacite * sizeof(Produit));
    }

    /* On garde l'ancien tableau si l'allocation echoue. */
    if (nouveauTableau == NULL) {
        return ERR_MEMOIRE;
    }
    boutique->produits = nouveauTableau;
    boutique->capaciteProduits = capacite;
    return OK;
}

int trouverProduit(const Boutique *boutique, const char *reference)
{
    int i;

    if (boutique == NULL || reference == NULL || reference[0] == '\0') {
        return -1;
    }
    for (i = 0; i < boutique->nbProduits; i++) {
        if (strcmp(boutique->produits[i].reference, reference) == 0) {
            return i;
        }
    }
    return -1;
}

CodeRetour ajouterProduit(Boutique *boutique, const Produit *produit)
{
    CodeRetour resultat;
    Produit copie;

    if (boutique == NULL || produit == NULL) {
        return ERR_INVALIDE;
    }
    if (textePresent(produit->reference) == 0
        || textePresent(produit->designation) == 0
        || textePresent(produit->categorie) == 0) {
        return ERR_INVALIDE;
    }
    if (!(produit->prixUnitaireHT > 0) || produit->quantiteStock < 0
        || produit->seuilAlerte < 0) {
        return ERR_INVALIDE;
    }
    if (boutique->nbProduits < 0 || boutique->capaciteProduits < 0
        || boutique->nbProduits > boutique->capaciteProduits
        || (boutique->capaciteProduits > 0 && boutique->produits == NULL)) {
        return ERR_INVALIDE;
    }
    if (trouverProduit(boutique, produit->reference) != -1) {
        return ERR_DOUBLON;
    }

    /* Copie avant realloc, car realloc peut deplacer le tableau. */
    copie = *produit;
    copie.actif = 1;
    if (boutique->nbProduits == boutique->capaciteProduits) {
        resultat = agrandirCatalogue(boutique);
        if (resultat != OK) {
            return resultat;
        }
    }
    boutique->produits[boutique->nbProduits] = copie;
    boutique->nbProduits++;
    return OK;
}

CodeRetour modifierProduit(Boutique *boutique, const char *reference,
                           float prix, int stock, int seuil)
{
    int indice;

    if (boutique == NULL || reference == NULL || !(prix > 0.0f) ||
        stock < 0 || seuil < 0) {
        return ERR_INVALIDE;
    }
    indice = trouverProduit(boutique, reference);
    if (indice < 0 || boutique->produits[indice].actif == 0) {
        return ERR_INTROUVABLE;
    }
    boutique->produits[indice].prixUnitaireHT = prix;
    boutique->produits[indice].quantiteStock = stock;
    boutique->produits[indice].seuilAlerte = seuil;
    return OK;
}

/* On parcourt les clients, leurs commandes, puis les lignes de commande. */
static int produitDansCommandeActive(const Boutique *boutique,
                                     const char *reference)
{
    int i, j, k;
    const Client *client;
    const Commande *commande;

    for (i = 0; i < boutique->nbClients; i++) {
        client = &boutique->clients[i];
        for (j = 0; j < client->nbCommandes; j++) {
            commande = &client->commandes[j];
            if (commande->statut != ANNULEE) {
                for (k = 0; k < commande->nbLignes; k++) {
                    if (strcmp(commande->lignes[k].refProduit, reference) == 0) {
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

CodeRetour supprimerProduit(Boutique *boutique, const char *reference)
{
    int indice;

    if (boutique == NULL || reference == NULL) {
        return ERR_INVALIDE;
    }
    indice = trouverProduit(boutique, reference);
    if (indice < 0 || boutique->produits[indice].actif == 0) {
        return ERR_INTROUVABLE;
    }
    if (boutique->nbClients < 0 ||
        (boutique->nbClients > 0 && boutique->clients == NULL)) {
        return ERR_INVALIDE;
    }
    if (produitDansCommandeActive(boutique, reference)) {
        return ERR_INTERDIT;
    }
    boutique->produits[indice].actif = 0;
    return OK;
}

float valeurStock(const Boutique *boutique)
{
    float total = 0.0f;
    int i;

    if (boutique == NULL) {
        return 0.0f;
    }
    for (i = 0; i < boutique->nbProduits; i++) {
        const Produit *produit = &boutique->produits[i];
        if (produit->actif) {
            total += produit->prixUnitaireHT * produit->quantiteStock;
        }
    }
    return total;
}

CodeRetour retirerStock(Boutique *boutique, const char *reference, int quantite)
{
    int indice;

    if (boutique == NULL || reference == NULL || quantite <= 0) {
        return ERR_INVALIDE;
    }
    indice = trouverProduit(boutique, reference);
    if (indice < 0 || boutique->produits[indice].actif == 0) {
        return ERR_INTROUVABLE;
    }
    if (boutique->produits[indice].quantiteStock < quantite) {
        return ERR_INTERDIT;
    }
    boutique->produits[indice].quantiteStock -= quantite;
    return OK;
}

CodeRetour remettreStock(Boutique *boutique, const char *reference, int quantite)
{
    int indice;

    if (boutique == NULL || reference == NULL || quantite <= 0) {
        return ERR_INVALIDE;
    }
    indice = trouverProduit(boutique, reference);
    if (indice < 0) {
        return ERR_INTROUVABLE;
    }
    if (boutique->produits[indice].quantiteStock > INT_MAX - quantite) {
        return ERR_INVALIDE;
    }
    boutique->produits[indice].quantiteStock += quantite;
    return OK;
}

void afficherProduit(const Produit *produit)
{
    if (produit != NULL)
        printf("%-11s | %-30s | %-20s | %8.2f EUR | %5d | %5d\n",
               produit->reference, produit->designation, produit->categorie,
               produit->prixUnitaireHT, produit->quantiteStock,
               produit->seuilAlerte);
}

void afficherCatalogue(const Boutique *boutique)
{
    int i;
    int affiches = 0;

    puts("\n--- Catalogue produits ---");
    puts("Reference   | Designation                    | Categorie            | Prix HT  | Stock | Seuil");
    if (boutique != NULL) {
        for (i = 0; i < boutique->nbProduits; i++) {
            if (boutique->produits[i].actif) {
                afficherProduit(&boutique->produits[i]);
                affiches++;
            }
        }
    }
    if (affiches == 0) {
        puts("Aucun produit actif.");
    }
    printf("Valeur du stock : %.2f EUR HT\n", valeurStock(boutique));
}

void afficherAlertesStock(const Boutique *boutique)
{
    int i;
    int alertes = 0;

    puts("\n--- Alertes de stock ---");
    if (boutique != NULL) {
        for (i = 0; i < boutique->nbProduits; i++) {
            const Produit *produit = &boutique->produits[i];
            if (produit->actif &&
                produit->quantiteStock <= produit->seuilAlerte) {
                afficherProduit(produit);
                alertes++;
            }
        }
    }
    if (alertes == 0) {
        puts("Aucune alerte de stock.");
            }
}

static int lireLigne(const char *invite, char *buffer, size_t taille)
{
    size_t longueur;
    int c;

    printf("%s", invite);
    if (fgets(buffer, (int)taille, stdin) == NULL) {
        return 0;
    }
    longueur = strlen(buffer);
    if (longueur > 0 && buffer[longueur - 1] == '\n') {
        buffer[longueur - 1] = '\0';
    } else {
        while ((c = getchar()) != '\n' && c != EOF) {
            /* Vide les caracteres restants de la ligne. */
        }
    }
    return 1;
}

static int lireEntier(const char *invite, int *valeur)
{
    char saisie[64];
    char reste;

    if (lireLigne(invite, saisie, sizeof saisie) == 0) {
        return 0;
    }
    /* %c detecte un caractere en trop : "12abc" doit etre refuse. */
    if (sscanf(saisie, " %d %c", valeur, &reste) != 1) {
        return 0;
    }
    return 1;
}

static int lirePrix(const char *invite, float *valeur)
{
    char saisie[64];
    char reste;

    if (lireLigne(invite, saisie, sizeof saisie) == 0) {
        return 0;
    }
    if (sscanf(saisie, " %f %c", valeur, &reste) != 1) {
        return 0;
    }
    return 1;
}

void menuAjouterProduit(Boutique *boutique)
{
    Produit produit = {0};
    CodeRetour resultat;

    if (boutique == NULL ||
        !lireLigne("Reference : ", produit.reference, sizeof produit.reference) ||
        !lireLigne("Designation : ", produit.designation, sizeof produit.designation) ||
        !lireLigne("Categorie : ", produit.categorie, sizeof produit.categorie) ||
        !lirePrix("Prix unitaire HT : ", &produit.prixUnitaireHT) ||
        !lireEntier("Quantite en stock : ", &produit.quantiteStock) ||
        !lireEntier("Seuil d'alerte : ", &produit.seuilAlerte)) {
        puts("Saisie invalide ou interrompue.");
        return;
    }
    resultat = ajouterProduit(boutique, &produit);
    if (resultat == OK) {
        printf("Produit ajoute.\n");
    } else if (resultat == ERR_DOUBLON) {
        printf("Cette reference existe deja.\n");
    } else if (resultat == ERR_MEMOIRE) {
        printf("Memoire insuffisante.\n");
    } else {
        printf("Produit invalide.\n");
    }
}

void menuModifierProduit(Boutique *boutique)
{
    char reference[TAILLE_REFERENCE];
    float prix;
    int stock;
    int seuil;
    CodeRetour resultat;

    if (boutique == NULL || !lireLigne("Reference du produit : ", reference,
                                       sizeof reference) ||
        !lirePrix("Nouveau prix HT : ", &prix) ||
        !lireEntier("Nouveau stock : ", &stock) ||
        !lireEntier("Nouveau seuil : ", &seuil)) {
        puts("Saisie invalide ou interrompue.");
        return;
    }
    resultat = modifierProduit(boutique, reference, prix, stock, seuil);
    if (resultat == OK) {
        printf("Produit mis a jour.\n");
    } else if (resultat == ERR_INTROUVABLE) {
        printf("Produit actif introuvable.\n");
    } else {
        printf("Valeurs invalides.\n");
    }
}

void menuSupprimerProduit(Boutique *boutique)
{
    char reference[TAILLE_REFERENCE];
    CodeRetour resultat;

    if (boutique == NULL || !lireLigne("Reference du produit : ", reference,
                                       sizeof reference)) {
        puts("Saisie invalide ou interrompue.");
        return;
    }
    resultat = supprimerProduit(boutique, reference);
    if (resultat == OK) {
        printf("Produit supprime (suppression logique).\n");
    } else if (resultat == ERR_INTERDIT) {
        printf("Suppression refusee : une commande le reference.\n");
    } else if (resultat == ERR_INTROUVABLE) {
        printf("Produit actif introuvable.\n");
    } else {
        printf("Donnees invalides.\n");
    }
}

/* Copie les deux textes en minuscules, puis cherche le mot avec strstr.
 * Ces textes sont limites a TAILLE_DESIGNATION caracteres, '\0' compris. */
static int contientSansCasse(const char *texte, const char *mot)
{
    char texteMinuscule[TAILLE_DESIGNATION];
    char motMinuscule[TAILLE_DESIGNATION];
    int i;

    strcpy(texteMinuscule, texte);
    strcpy(motMinuscule, mot);
    /* unsigned char permet a tolower de recevoir une valeur valide. */
    for (i = 0; texteMinuscule[i] != '\0'; i++) {
        texteMinuscule[i] = tolower((unsigned char)texteMinuscule[i]);
    }
    for (i = 0; motMinuscule[i] != '\0'; i++) {
        motMinuscule[i] = tolower((unsigned char)motMinuscule[i]);
    }
    if (strstr(texteMinuscule, motMinuscule) != NULL) {
        return 1;
    }
    return 0;
}

void menuRechercherProduit(const Boutique *boutique)
{
    char choix[16];
    char recherche[TAILLE_DESIGNATION];
    int i;
    int trouves = 0;

    if (boutique == NULL || !lireLigne("1. Reference exacte  2. Mot-cle : ",
                                       choix, sizeof choix)) {
        return;
    }
    if (strcmp(choix, "1") == 0) {
        int indice;
        if (!lireLigne("Reference : ", recherche, sizeof recherche)) {
            return;
        }
        indice = trouverProduit(boutique, recherche);
        if (indice >= 0 && boutique->produits[indice].actif) {
            afficherProduit(&boutique->produits[indice]);
        } else {
            puts("Produit actif introuvable.");
        }
        return;
    }
    if (strcmp(choix, "2") != 0 ||
        !lireLigne("Mot-cle : ", recherche, sizeof recherche) ||
        !textePresent(recherche)) {
        puts("Choix ou mot-cle invalide.");
        return;
    }
    for (i = 0; i < boutique->nbProduits; i++) {
        const Produit *produit = &boutique->produits[i];
        if (produit->actif &&
            contientSansCasse(produit->designation, recherche)) {
            afficherProduit(produit);
            trouves++;
        }
    }
    if (trouves == 0) {
        puts("Aucun produit correspondant.");
    }
}
