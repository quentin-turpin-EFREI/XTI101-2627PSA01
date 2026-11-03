/* Calcul et affichage des statistiques de la boutique. */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "statistique.h"

typedef struct {
    const Produit *produit;
    long long quantite;
} VenteProduit;

typedef struct {
    const Client *client;
    double chiffreAffaires;
} VenteClient;

typedef struct {
    char nom[TAILLE_CATEGORIE];
    long long quantite;
    double montantBrutHT;
} VenteCategorie;

static int commandeValidePourCA(const Commande *commande,
                                int annee, int mois)
{
    return commande->statut != ANNULEE
        && (annee == 0 || commande->dateCommande.annee == annee)
        && (mois == 0 || commande->dateCommande.mois == mois);
}

static const Produit *produitParReference(const Boutique *boutique,
                                          const char *reference)
{
    int i;

    for (i = 0; i < boutique->nbProduits; i++)
        if (strcmp(boutique->produits[i].reference, reference) == 0)
            return &boutique->produits[i];
    return NULL;
}

static void calculerCA(const Boutique *boutique, int annee, int mois,
                       double *totalHT, double *totalTTC)
{
    int i, j;

    *totalHT = 0.0;
    *totalTTC = 0.0;
    if (boutique == NULL)
        return;

    for (i = 0; i < boutique->nbClients; i++) {
        const Client *client = &boutique->clients[i];
        for (j = 0; j < client->nbCommandes; j++) {
            const Commande *commande = &client->commandes[j];
            if (commandeValidePourCA(commande, annee, mois)) {
                *totalHT += commande->montantHT;
                *totalTTC += commande->montantTTC;
            }
        }
    }
}

void afficherChiffreAffaires(const Boutique *boutique)
{
    double totalHT, totalTTC;

    calculerCA(boutique, 0, 0, &totalHT, &totalTTC);
    puts("\n--- Chiffre d'affaires global (hors commandes annulees) ---");
    printf("Total HT  : %.2f EUR\nTotal TTC : %.2f EUR\n",
           totalHT, totalTTC);
}

void afficherChiffreAffairesPeriode(const Boutique *boutique,
                                    int annee, int mois)
{
    double totalHT, totalTTC;

    if (boutique == NULL || annee < 2000 || annee > 2100
        || mois < 0 || mois > 12) {
        puts("Periode invalide.");
        return;
    }
    calculerCA(boutique, annee, mois, &totalHT, &totalTTC);
    if (mois == 0)
        printf("\n--- Chiffre d'affaires de %d (hors commandes annulees) ---\n",
               annee);
    else
        printf("\n--- Chiffre d'affaires %02d/%d (hors commandes annulees) ---\n",
               mois, annee);
    printf("Total HT  : %.2f EUR\nTotal TTC : %.2f EUR\n",
           totalHT, totalTTC);
}

static long long quantiteVendue(const Boutique *boutique,
                                const char *reference)
{
    int i, j, k;
    long long total = 0;

    for (i = 0; i < boutique->nbClients; i++) {
        const Client *client = &boutique->clients[i];
        for (j = 0; j < client->nbCommandes; j++) {
            const Commande *commande = &client->commandes[j];
            if (commande->statut == ANNULEE)
                continue;
            for (k = 0; k < commande->nbLignes; k++)
                if (strcmp(commande->lignes[k].refProduit, reference) == 0)
                    total += commande->lignes[k].quantite;
        }
    }
    return total;
}

void afficherPalmaresProduits(const Boutique *boutique, int limite)
{
    VenteProduit *ventes;
    int i, j, nombre;

    if (boutique == NULL || boutique->nbProduits < 0) {
        puts("Catalogue indisponible.");
        return;
    }
    nombre = boutique->nbProduits;
    ventes = nombre == 0 ? NULL : malloc((size_t)nombre * sizeof *ventes);
    if (nombre > 0 && ventes == NULL) {
        puts("Memoire insuffisante pour calculer le palmares produits.");
        return;
    }
    for (i = 0; i < nombre; i++) {
        ventes[i].produit = &boutique->produits[i];
        ventes[i].quantite = quantiteVendue(boutique,
                                             boutique->produits[i].reference);
    }
    for (i = 0; i < nombre; i++) {
        int meilleur = i;
        for (j = i + 1; j < nombre; j++)
            if (ventes[j].quantite > ventes[meilleur].quantite)
                meilleur = j;
        if (meilleur != i) {
            VenteProduit temporaire = ventes[i];
            ventes[i] = ventes[meilleur];
            ventes[meilleur] = temporaire;
        }
    }
    if (limite <= 0 || limite > nombre)
        limite = nombre;
    puts("\n--- Palmares produits (quantites non annulees) ---");
    if (limite == 0)
        puts("Aucune vente.");
    for (i = 0; i < limite; i++)
        printf("%d. %s - %s : %lld vendu(s)\n", i + 1,
               ventes[i].produit->reference, ventes[i].produit->designation,
               ventes[i].quantite);
    free(ventes);
}

static double chiffreAffairesClientPeriode(const Client *client)
{
    int i;
    double total = 0.0;

    for (i = 0; i < client->nbCommandes; i++)
        if (client->commandes[i].statut != ANNULEE)
            total += client->commandes[i].montantHT;
    return total;
}

void afficherPalmaresClients(const Boutique *boutique)
{
    VenteClient *ventes;
    int i, j, nombre;

    if (boutique == NULL || boutique->nbClients < 0) {
        puts("Repertoire clients indisponible.");
        return;
    }
    nombre = boutique->nbClients;
    ventes = nombre == 0 ? NULL : malloc((size_t)nombre * sizeof *ventes);
    if (nombre > 0 && ventes == NULL) {
        puts("Memoire insuffisante pour calculer le palmares clients.");
        return;
    }
    for (i = 0; i < nombre; i++) {
        ventes[i].client = &boutique->clients[i];
        ventes[i].chiffreAffaires = chiffreAffairesClientPeriode(
            &boutique->clients[i]);
    }
    for (i = 0; i < nombre; i++) {
        int meilleur = i;
        for (j = i + 1; j < nombre; j++)
            if (ventes[j].chiffreAffaires > ventes[meilleur].chiffreAffaires)
                meilleur = j;
        if (meilleur != i) {
            VenteClient temporaire = ventes[i];
            ventes[i] = ventes[meilleur];
            ventes[meilleur] = temporaire;
        }
    }
    puts("\n--- Palmares clients par chiffre d'affaires HT ---");
    for (i = 0; i < nombre; i++)
        printf("%d. %s %s (ID %d) : %.2f EUR HT\n", i + 1,
               ventes[i].client->prenom, ventes[i].client->nom,
               ventes[i].client->idClient, ventes[i].chiffreAffaires);
    if (nombre == 0)
        puts("Aucun client.");
    free(ventes);
}

void afficherStatistiquesParCategorie(const Boutique *boutique)
{
    VenteCategorie *categories;
    int i, j, k, nombre = 0;

    if (boutique == NULL || boutique->nbProduits <= 0) {
        puts("Aucune categorie a afficher.");
        return;
    }
    categories = calloc((size_t)boutique->nbProduits, sizeof *categories);
    if (categories == NULL) {
        puts("Memoire insuffisante pour calculer les categories.");
        return;
    }
    for (i = 0; i < boutique->nbProduits; i++) {
        for (j = 0; j < nombre; j++)
            if (strcmp(categories[j].nom, boutique->produits[i].categorie) == 0)
                break;
        if (j == nombre) {
            strncpy(categories[nombre].nom, boutique->produits[i].categorie,
                    TAILLE_CATEGORIE - 1);
            nombre++;
        }
    }
    for (i = 0; i < boutique->nbClients; i++) {
        const Client *client = &boutique->clients[i];
        for (j = 0; j < client->nbCommandes; j++) {
            const Commande *commande = &client->commandes[j];
            if (commande->statut == ANNULEE)
                continue;
            for (k = 0; k < commande->nbLignes; k++) {
                const Produit *produit = produitParReference(
                    boutique, commande->lignes[k].refProduit);
                int c;
                if (produit == NULL)
                    continue;
                for (c = 0; c < nombre; c++)
                    if (strcmp(categories[c].nom, produit->categorie) == 0)
                        break;
                if (c < nombre) {
                    categories[c].quantite += commande->lignes[k].quantite;
                    categories[c].montantBrutHT +=
                        commande->lignes[k].sousTotalHT;
                }
            }
        }
    }
    puts("\n--- Ventes par categorie (commandes non annulees) ---");
    for (i = 0; i < nombre; i++)
        printf("%s : %lld article(s), %.2f EUR HT brut\n",
               categories[i].nom, categories[i].quantite,
               categories[i].montantBrutHT);
    free(categories);
}
