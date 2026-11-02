#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "produit.h"

/* Renvoie la position du produit, ou -1 si la reference n'existe pas. */
int trouverProduit(Boutique *boutique, const char *reference)
{
    int i;
    for (i = 0; i < boutique->nbProduits; i++)
    {
        if (strcmp(boutique->produits[i].reference, reference) == 0)
        {
            return i;
        }
    }
    return -1;
}

CodeRetour ajouterProduit(Boutique *boutique, const Produit *produit)
{
    Produit *nouveauTableau;
    int capacite;
    if (strlen(produit->reference) == 0 || strlen(produit->designation) == 0 || strlen(produit->categorie) == 0 || produit->prixUnitaireHT <= 0 || !isfinite(produit->prixUnitaireHT) || produit->quantiteStock < 0 || produit->seuilAlerte < 0)
    {
        return ERR_INVALIDE;
    }
    if (trouverProduit(boutique, produit->reference) != -1)
    {
        return ERR_DOUBLON;
    }
    /* On agrandit le tableau seulement quand il est plein. */
    if (boutique->nbProduits == boutique->capaciteProduits)
    {
        if (boutique->capaciteProduits == 0)
        {
            capacite = CAPACITE_INITIALE;
            nouveauTableau = malloc(capacite * sizeof(Produit));
        }
        else
        {
            capacite = boutique->capaciteProduits * 2;
            nouveauTableau = realloc(boutique->produits, capacite * sizeof(Produit));
        }
        /* Si l'allocation echoue, on conserve l'ancien tableau. */
        if (nouveauTableau == NULL)
        {
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

CodeRetour modifierProduit(Boutique *boutique, const char *reference,
                          float prix, int stock, int seuil)
{
    int indice;
    indice = trouverProduit(boutique, reference);
    if (indice == -1 || boutique->produits[indice].actif == 0)
    {
        return ERR_INTROUVABLE;
    }
    if (prix <= 0 || !isfinite(prix) || stock < 0 || seuil < 0)
    {
        return ERR_INVALIDE;
    }
    boutique->produits[indice].prixUnitaireHT = prix;
    boutique->produits[indice].quantiteStock = stock;
    boutique->produits[indice].seuilAlerte = seuil;
    return OK;
}

CodeRetour supprimerProduit(Boutique *boutique, const char *reference)
{
    int indice;
    int i, j, k;
    Client *client;
    Commande *commande;

    indice = trouverProduit(boutique, reference);
    if (indice == -1 || boutique->produits[indice].actif == 0)
    {
        return ERR_INTROUVABLE;
    }
    /* Verifie les lignes de toutes les commandes non annulees. */
    for (i = 0; i < boutique->nbClients; i++)
    {
        client = &boutique->clients[i];
        for (j = 0; j < client->nbCommandes; j++)
        {
            commande = &client->commandes[j];
            if (commande->statut != ANNULEE)
            {
                for (k = 0; k < commande->nbLignes; k++)
                {
                    if (strcmp(commande->lignes[k].refProduit, reference) == 0)
                    {
                        return ERR_INTERDIT;
                    }
                }
            }
        }
    }
    /* Le produit reste dans le tableau pour conserver son historique. */
    boutique->produits[indice].actif = 0;
    return OK;
}

/* Affichage commun au catalogue et a la recherche. */
static void afficherProduit(Produit *produit)
{
    printf("%s - %s (%s)\n", produit->reference, produit->designation, produit->categorie);
    printf("Prix HT : %.2f EUR | Stock : %d | Seuil : %d\n",
           produit->prixUnitaireHT, produit->quantiteStock, produit->seuilAlerte);
}

void afficherCatalogue(Boutique *boutique)
{
    int i;
    int nombre;
    float valeurStock;

    nombre = 0;
    valeurStock = 0;
    printf("\n--- Catalogue produits ---\n");
    for (i = 0; i < boutique->nbProduits; i++)
    {
        if (boutique->produits[i].actif == 1)
        {
            afficherProduit(&boutique->produits[i]);
            valeurStock += boutique->produits[i].prixUnitaireHT * boutique->produits[i].quantiteStock;
            nombre++;
        }
    }
    if (nombre == 0)
    {
        printf("Aucun produit actif.\n");
    }
    printf("Valeur du stock : %.2f EUR HT\n", valeurStock);
}

void rechercherProduits(Boutique *boutique, const char *recherche)
{
    int i;
    int nombre;
    Produit *produit;

    nombre = 0;
    if (strlen(recherche) == 0)
    {
        printf("La recherche ne doit pas etre vide.\n");
        return;
    }
    for (i = 0; i < boutique->nbProduits; i++)
    {
        produit = &boutique->produits[i];
        /* strcmp compare la reference ; strstr cherche un mot dans le texte. */
        if (produit->actif == 1 && (strcmp(produit->reference, recherche) == 0
            || strstr(produit->designation, recherche) != NULL))
        {
            afficherProduit(produit);
            nombre++;
        }
    }
    if (nombre == 0)
    {
        printf("Aucun produit correspondant.\n");
    }
}
