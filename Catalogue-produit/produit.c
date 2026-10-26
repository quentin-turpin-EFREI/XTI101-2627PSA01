/* produit.c - ajout et affichage des produits */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "produit.h"

/* Cherche une reference dans le tableau. Renvoie -1 si elle n'existe pas. */
int trouverProduit(const Boutique *boutique, const char *reference)
{
    int i;

    for (i = 0; i < boutique->nbProduits; i++) {
        /* strcmp renvoie 0 quand les deux textes sont identiques. */
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

    /* Un texte est vide si son premier caractere est '\0'. */
    if (produit->reference[0] == '\0' || produit->designation[0] == '\0'
        || produit->categorie[0] == '\0') {
        printf("Les textes du produit ne doivent pas etre vides.\n");
        return 0;
    }

    if (produit->prixUnitaireHT <= 0) {
        printf("Le prix doit etre positif.\n");
        return 0;
    }

    if (produit->quantiteStock < 0 || produit->seuilAlerte < 0) {
        printf("Le stock et le seuil ne doivent pas etre negatifs.\n");
        return 0;
    }

    if (trouverProduit(boutique, produit->reference) != -1) {
        printf("Cette reference existe deja.\n");
        return 0;
    }

    /* On reserve de la place si le tableau est plein. */
    if (boutique->nbProduits == boutique->capaciteProduits) {
        if (boutique->capaciteProduits == 0) {
            nouvelleCapacite = CAPACITE_INITIALE;
            nouveauTableau = malloc(nouvelleCapacite * sizeof(Produit));
        } else {
            nouvelleCapacite = boutique->capaciteProduits * 2;
            nouveauTableau = realloc(boutique->produits,
                                     nouvelleCapacite * sizeof(Produit));
        }

        /* Le pointeur temporaire permet de garder l'ancien tableau
         * si realloc echoue. */
        if (nouveauTableau == NULL) {
            printf("Erreur : memoire insuffisante.\n");
            return 0;
        }
        boutique->produits = nouveauTableau;
        boutique->capaciteProduits = nouvelleCapacite;
    }

    /* *produit permet de copier la structure dans la prochaine case libre. */
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
