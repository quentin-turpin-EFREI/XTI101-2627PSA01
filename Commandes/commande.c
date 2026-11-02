/* commande.c - gestion metier des commandes et factures */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "commande.h"
#include "../Clients/client.h"

static Commande *chercherCommande(Boutique *b, int idClient, int numero)
{
    int ic, i;
    Client *client;
    if (b == NULL) return NULL;
    ic = trouverClient(b, idClient);
    if (ic < 0) return NULL;
    client = &b->clients[ic];
    for (i = 0; i < client->nbCommandes; i++)
        if (client->commandes[i].numero == numero)
            return &client->commandes[i];
    return NULL;
}

static Produit *chercherProduit(Boutique *b, const char *ref, int actifSeulement)
{
    int i;
    if (b == NULL || ref == NULL) return NULL;
    for (i = 0; i < b->nbProduits; i++)
        if (strcmp(b->produits[i].reference, ref) == 0 &&
            (!actifSeulement || b->produits[i].actif))
            return &b->produits[i];
    return NULL;
}

static int calculerPointsCommande(const Commande *c)
{
    if (c == NULL || c->montantTTC <= 0.0f) return 0;
    return (int)((c->montantTTC + 0.001f) / 10.0f);
}

int creerCommande(Boutique *b, int idClient, int *numeroCommande)
{
    int ic, capacite;
    Client *client;
    Commande *agrandies, nouvelle = {0};
    time_t maintenant;
    struct tm *date;
    if (b == NULL || numeroCommande == NULL) return ERR_INVALIDE;
    ic = trouverClient(b, idClient);
    if (ic < 0) return ERR_INTROUVABLE;
    client = &b->clients[ic];
    if (client->nbCommandes >= client->capaciteCommandes) {
        capacite = client->capaciteCommandes == 0
            ? CAPACITE_INITIALE : client->capaciteCommandes * 2;
        agrandies = realloc(client->commandes, capacite * sizeof *agrandies);
        if (agrandies == NULL) return ERR_MEMOIRE;
        client->commandes = agrandies;
        client->capaciteCommandes = capacite;
    }
    maintenant = time(NULL);
    date = localtime(&maintenant);
    if (date == NULL) return ERR_INVALIDE;
    if (b->numeroSuivant < 1) b->numeroSuivant = 1;
    nouvelle.numero = b->numeroSuivant++;
    nouvelle.idClient = idClient;
    nouvelle.dateCommande.jour = date->tm_mday;
    nouvelle.dateCommande.mois = date->tm_mon + 1;
    nouvelle.dateCommande.annee = date->tm_year + 1900;
    nouvelle.adresseLivraison = client->adresse;
    nouvelle.statut = EN_COURS;
    client->commandes[client->nbCommandes++] = nouvelle;
    *numeroCommande = nouvelle.numero;
    return OK;
}

void recalculerMontantsCommande(Commande *c)
{
    int i;
    float brut = 0.0f;
    if (c == NULL) return;
    for (i = 0; c->lignes != NULL && i < c->nbLignes; i++) {
        c->lignes[i].sousTotalHT = c->lignes[i].prixUnitaireHT * c->lignes[i].quantite;
        brut += c->lignes[i].sousTotalHT;
    }
    c->montantBrutHT = brut;
    c->tauxRemise = brut >= 600.0f ? 15.0f : brut >= 300.0f ? 10.0f : brut >= 100.0f ? 5.0f : 0.0f;
    c->montantRemise = brut * c->tauxRemise / 100.0f;
    c->montantHT = brut - c->montantRemise;
    c->montantTVA = c->montantHT * 0.20f;
    c->fraisPort = brut >= 60.0f ? 0.0f : 5.90f;
    c->montantTTC = c->montantHT + c->montantTVA + c->fraisPort;
}

int ajouterLigneCommande(Boutique *b, int idClient, int numeroCommande,
                         const char *refProduit, int quantite)
{
    int ic, i, capacite, anciensPoints;
    Client *client;
    Commande *c;
    Produit *p;
    LigneCommande *agrandies, *ligne;
    if (b == NULL || refProduit == NULL || refProduit[0] == '\0' || quantite <= 0)
        return ERR_INVALIDE;
    ic = trouverClient(b, idClient);
    if (ic < 0) return ERR_INTROUVABLE;
    client = &b->clients[ic];
    c = chercherCommande(b, idClient, numeroCommande);
    if (c == NULL) return ERR_INTROUVABLE;
    if (c->statut != EN_COURS) return ERR_INTERDIT;
    p = chercherProduit(b, refProduit, 1);
    if (p == NULL) return ERR_INTROUVABLE;
    if (p->quantiteStock < quantite) return ERR_INTERDIT;
    anciensPoints = calculerPointsCommande(c);
    for (i = 0; i < c->nbLignes; i++) {
        if (strcmp(c->lignes[i].refProduit, refProduit) == 0) {
            c->lignes[i].quantite += quantite;
            p->quantiteStock -= quantite;
            recalculerMontantsCommande(c);
            client->pointsFidelite += calculerPointsCommande(c) - anciensPoints;
            return OK;
        }
    }
    if (c->nbLignes >= c->capaciteLignes) {
        capacite = c->capaciteLignes == 0 ? CAPACITE_INITIALE : c->capaciteLignes * 2;
        agrandies = realloc(c->lignes, capacite * sizeof *agrandies);
        if (agrandies == NULL) return ERR_MEMOIRE;
        c->lignes = agrandies;
        c->capaciteLignes = capacite;
    }
    ligne = &c->lignes[c->nbLignes];
    strncpy(ligne->refProduit, p->reference, TAILLE_REFERENCE - 1);
    ligne->refProduit[TAILLE_REFERENCE - 1] = '\0';
    strncpy(ligne->designation, p->designation, TAILLE_DESIGNATION - 1);
    ligne->designation[TAILLE_DESIGNATION - 1] = '\0';
    ligne->prixUnitaireHT = p->prixUnitaireHT;
    ligne->quantite = quantite;
    c->nbLignes++;
    p->quantiteStock -= quantite;
    recalculerMontantsCommande(c);
    client->pointsFidelite += calculerPointsCommande(c) - anciensPoints;
    return OK;
}

int supprimerLigneCommande(Boutique *b, int idClient, int numeroCommande,
                           const char *refProduit)
{
    int ic, i, index = -1, anciensPoints;
    Client *client;
    Commande *c;
    Produit *p;
    if (b == NULL || refProduit == NULL || refProduit[0] == '\0') return ERR_INVALIDE;
    ic = trouverClient(b, idClient);
    if (ic < 0) return ERR_INTROUVABLE;
    client = &b->clients[ic];
    c = chercherCommande(b, idClient, numeroCommande);
    if (c == NULL) return ERR_INTROUVABLE;
    if (c->statut != EN_COURS) return ERR_INTERDIT;
    for (i = 0; i < c->nbLignes; i++)
        if (strcmp(c->lignes[i].refProduit, refProduit) == 0) { index = i; break; }
    if (index < 0) return ERR_INTROUVABLE;
    p = chercherProduit(b, refProduit, 0);
    if (p == NULL) return ERR_INTROUVABLE;
    anciensPoints = calculerPointsCommande(c);
    p->quantiteStock += c->lignes[index].quantite;
    for (i = index; i < c->nbLignes - 1; i++) c->lignes[i] = c->lignes[i + 1];
    c->nbLignes--;
    recalculerMontantsCommande(c);
    client->pointsFidelite += calculerPointsCommande(c) - anciensPoints;
    return OK;
}

int annulerCommande(Boutique *b, int idClient, int numeroCommande)
{
    int ic, i, points;
    Client *client;
    Commande *c;
    Produit *p;
    if (b == NULL) return ERR_INVALIDE;
    ic = trouverClient(b, idClient);
    if (ic < 0) return ERR_INTROUVABLE;
    client = &b->clients[ic];
    c = chercherCommande(b, idClient, numeroCommande);
    if (c == NULL) return ERR_INTROUVABLE;
    if (c->statut != EN_COURS) return ERR_INTERDIT;
    for (i = 0; i < c->nbLignes; i++)
        if (chercherProduit(b, c->lignes[i].refProduit, 0) == NULL)
            return ERR_INTROUVABLE;
    points = calculerPointsCommande(c);
    for (i = 0; i < c->nbLignes; i++) {
        p = chercherProduit(b, c->lignes[i].refProduit, 0);
        p->quantiteStock += c->lignes[i].quantite;
    }
    client->pointsFidelite = client->pointsFidelite >= points
        ? client->pointsFidelite - points : 0;
    c->statut = ANNULEE;
    return OK;
}

int changerStatutCommande(Boutique *b, int idClient, int numeroCommande,
                          StatutCommande nouveauStatut)
{
    Commande *c;
    if (b == NULL) return ERR_INVALIDE;
    if (nouveauStatut != EN_COURS && nouveauStatut != EXPEDIEE &&
        nouveauStatut != LIVREE && nouveauStatut != ANNULEE) return ERR_INVALIDE;
    if (nouveauStatut == ANNULEE) return annulerCommande(b, idClient, numeroCommande);
    c = chercherCommande(b, idClient, numeroCommande);
    if (c == NULL) return ERR_INTROUVABLE;
    if (c->statut == nouveauStatut) return OK;
    if (c->statut == EN_COURS && nouveauStatut == EXPEDIEE) {
        c->statut = EXPEDIEE;
        return OK;
    }
    if (c->statut == EXPEDIEE && nouveauStatut == LIVREE) {
        c->statut = LIVREE;
        return OK;
    }
    return ERR_INTERDIT;
}

void afficherFacture(const Commande *c)
{
    int i;
    if (c == NULL) { printf("Commande invalide.\n"); return; }
    printf("\n--- FACTURE ---\nCommande numero : %d\n", c->numero);
    for (i = 0; c->lignes != NULL && i < c->nbLignes; i++)
        printf("%s | %s | %d x %.2f EUR = %.2f EUR HT\n",
               c->lignes[i].refProduit, c->lignes[i].designation,
               c->lignes[i].quantite, c->lignes[i].prixUnitaireHT,
               c->lignes[i].sousTotalHT);
    printf("Montant brut HT : %.2f EUR\nRemise          : -%.2f EUR\n"
           "Total HT        : %.2f EUR\nTVA             : %.2f EUR\n"
           "Frais de port   : %.2f EUR\nTotal TTC       : %.2f EUR\n",
           c->montantBrutHT, c->montantRemise, c->montantHT,
           c->montantTVA, c->fraisPort, c->montantTTC);
}
