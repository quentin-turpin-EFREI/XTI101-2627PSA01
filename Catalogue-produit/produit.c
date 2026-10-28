/* produit.c - V1 : ajout et affichage du catalogue */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "produit.h"

/* Renvoie la position du produit, ou -1 si la reference n'existe pas. */
int trouverProduit(const Boutique *boutique, const char *reference)
{
    int i;
    for (i = 0; i < boutique->nbProduits; i++) {
        if (strcmp(boutique->produits[i].reference, reference) == 0) {
            return i;
        }
    }
    return -1;
}

CodeRetour ajouterProduit(Boutique *boutique, const Produit *produit)
{
    Produit *nouveauTableau;
    int capacite;
    if (produit->reference[0] == '\0' || produit->designation[0] == '\0'
        || produit->categorie[0] == '\0' || !(produit->prixUnitaireHT > 0.0f)
        || !isfinite(produit->prixUnitaireHT)
        || produit->quantiteStock < 0 || produit->seuilAlerte < 0) {
        return ERR_INVALIDE;
    }
    if (trouverProduit(boutique, produit->reference) != -1) {
        return ERR_DOUBLON;
    }
    /* On agrandit le tableau seulement quand il est plein. */
    if (boutique->nbProduits == boutique->capaciteProduits) {
        if (boutique->capaciteProduits == 0) {
            capacite = CAPACITE_INITIALE;
            nouveauTableau = malloc(capacite * sizeof(Produit));
        } else {
            capacite = boutique->capaciteProduits * 2;
            nouveauTableau = realloc(boutique->produits, capacite * sizeof(Produit));
        }
        /* Si l'allocation echoue, on conserve l'ancien tableau. */
        if (nouveauTableau == NULL) {
            return ERR_MEMOIRE;
        }
        boutique->produits = nouveauTableau;
        boutique->capaciteProduits = capacite;
    }
    boutique->produits[boutique->nbProduits] = *produit;
    boutique->produits[boutique->nbProduits].actif = 1;
    boutique->nbProduits++;
    return OK;
}

void afficherCatalogue(const Boutique *boutique)
{
    int i, nombre = 0;
    float valeurStock = 0;
    printf("\n--- Catalogue produits ---\n");
    for (i = 0; i < boutique->nbProduits; i++) {
        if (boutique->produits[i].actif == 1) {
            printf("%s - %s (%s)\n", boutique->produits[i].reference,
                   boutique->produits[i].designation, boutique->produits[i].categorie);
            printf("Prix HT : %.2f EUR | Stock : %d | Seuil : %d\n",
                   boutique->produits[i].prixUnitaireHT,
                   boutique->produits[i].quantiteStock, boutique->produits[i].seuilAlerte);
            valeurStock += boutique->produits[i].prixUnitaireHT
                           * boutique->produits[i].quantiteStock;
            nombre++;
        }
    }
    if (nombre == 0) {
        printf("Aucun produit actif.\n");
    }
    printf("Valeur du stock : %.2f EUR HT\n", valeurStock);
}
