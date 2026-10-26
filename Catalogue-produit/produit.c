/* produit.c - ajout et affichage des produits */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "produit.h"

/* Cherche une reference dans le tableau. Renvoie -1 si elle n'existe pas. */
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

/* Ajoute un produit deja renseigne dans le catalogue. */
int ajouterProduit(Boutique *boutique, const Produit *produit)
{
    Produit *nouveauTableau;
    int nouvelleCapacite;

    if (produit->reference[0] == '\0' || produit->designation[0] == '\0'
        || produit->categorie[0] == '\0' || !isfinite(produit->prixUnitaireHT)
        || produit->prixUnitaireHT <= 0 || produit->quantiteStock < 0
        || produit->seuilAlerte < 0) {
        printf("Les informations du produit sont invalides.\n");
        return 0;
    }

    if (trouverProduit(boutique, produit->reference) != -1) {
        printf("Cette reference existe deja.\n");
        return 0;
    }

    /* Si le tableau est plein, on double sa capacite. */
    if (boutique->nbProduits == boutique->capaciteProduits) {
        if (boutique->capaciteProduits == 0) {
            nouvelleCapacite = CAPACITE_INITIALE;
        } else {
            nouvelleCapacite = boutique->capaciteProduits * 2;
        }

        nouveauTableau = realloc(boutique->produits,
                                 nouvelleCapacite * sizeof(Produit));
        if (nouveauTableau == NULL) {
            printf("Erreur : memoire insuffisante.\n");
            return 0;
        }
        boutique->produits = nouveauTableau;
        boutique->capaciteProduits = nouvelleCapacite;
    }

    boutique->produits[boutique->nbProduits] = *produit;
    boutique->produits[boutique->nbProduits].actif = 1;
    boutique->nbProduits++;
    return 1;
}

void afficherCatalogue(const Boutique *boutique)
{
    int i;
    float valeurTotale = 0;

    printf("\n--- Catalogue des produits ---\n");
    for (i = 0; i < boutique->nbProduits; i++) {
        if (boutique->produits[i].actif == 1) {
            printf("%s - %s (%s)\n",
                   boutique->produits[i].reference,
                   boutique->produits[i].designation,
                   boutique->produits[i].categorie);
            printf("Prix HT : %.2f EUR | Stock : %d | Seuil : %d\n",
                   boutique->produits[i].prixUnitaireHT,
                   boutique->produits[i].quantiteStock,
                   boutique->produits[i].seuilAlerte);

            valeurTotale += boutique->produits[i].prixUnitaireHT
                           * boutique->produits[i].quantiteStock;
        }
    }
    printf("Valeur totale du stock HT : %.2f EUR\n", valeurTotale);
}
