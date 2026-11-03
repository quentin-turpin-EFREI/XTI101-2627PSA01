#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "Commandes/commande.h"

static void verifierProche(float obtenu, float attendu)
{
    float ecart = obtenu - attendu;

    if (ecart < 0.0f)
        ecart = -ecart;
    assert(ecart < 0.001f);
}

static void verifierFacture(float brut, float tauxRemise,
                            float remise, float totalHT,
                            float tva, float port, float ttc)
{
    LigneCommande ligne = {0};
    Commande commande = {0};

    if (brut > 0.0f) {
        ligne.prixUnitaireHT = brut;
        ligne.quantite = 1;
        commande.lignes = &ligne;
        commande.nbLignes = 1;
    }

    recalculerMontantsCommande(&commande);

    verifierProche(commande.montantBrutHT, brut);
    verifierProche(commande.tauxRemise, tauxRemise);
    verifierProche(commande.montantRemise, remise);
    verifierProche(commande.montantHT, totalHT);
    verifierProche(commande.montantTVA, tva);
    verifierProche(commande.fraisPort, port);
    verifierProche(commande.montantTTC, ttc);
    if (brut > 0.0f)
        verifierProche(ligne.sousTotalHT, brut);
}

static void testCommandeVideSansFraisDePort(void)
{
    verifierFacture(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    puts("OK commande vide : aucun frais de port");
}

static void testFraisDePortEtSeuilGratuit(void)
{
    verifierFacture(59.99f, 0.0f, 0.0f, 59.99f,
                    11.998f, 5.90f, 77.888f);
    verifierFacture(60.0f, 0.0f, 0.0f, 60.0f,
                    12.0f, 0.0f, 72.0f);
    puts("OK frais de port : payants sous 60 EUR, gratuits a partir de 60 EUR");
}

static void testRemisesProgressivesEtTVA(void)
{
    verifierFacture(99.99f, 0.0f, 0.0f, 99.99f,
                    19.998f, 0.0f, 119.988f);
    verifierFacture(100.0f, 5.0f, 5.0f, 95.0f,
                    19.0f, 0.0f, 114.0f);
    verifierFacture(299.99f, 5.0f, 14.9995f, 284.9905f,
                    56.9981f, 0.0f, 341.9886f);
    verifierFacture(300.0f, 10.0f, 30.0f, 270.0f,
                    54.0f, 0.0f, 324.0f);
    verifierFacture(599.99f, 10.0f, 59.999f, 539.991f,
                    107.9982f, 0.0f, 647.9892f);
    verifierFacture(600.0f, 15.0f, 90.0f, 510.0f,
                    102.0f, 0.0f, 612.0f);
    puts("OK remises progressives et TVA a 20 % apres remise");
}

static void preparerBoutique(Boutique *boutique)
{
    memset(boutique, 0, sizeof *boutique);
    boutique->clients = calloc(1, sizeof *boutique->clients);
    boutique->produits = calloc(1, sizeof *boutique->produits);
    assert(boutique->clients != NULL);
    assert(boutique->produits != NULL);

    boutique->nbClients = 1;
    boutique->idClientSuivant = 2;
    boutique->numeroSuivant = 100;
    boutique->clients[0].idClient = 1;
    boutique->clients[0].actif = 1;

    boutique->nbProduits = 1;
    strcpy(boutique->produits[0].reference, "SKU1");
    strcpy(boutique->produits[0].designation, "Article test");
    boutique->produits[0].prixUnitaireHT = 50.0f;
    boutique->produits[0].quantiteStock = 5;
    boutique->produits[0].actif = 1;
}

static void libererBoutiqueTest(Boutique *boutique)
{
    int i, j;

    for (i = 0; i < boutique->nbClients; i++) {
        for (j = 0; j < boutique->clients[i].nbCommandes; j++)
            free(boutique->clients[i].commandes[j].lignes);
        free(boutique->clients[i].commandes);
    }
    free(boutique->clients);
    free(boutique->produits);
}

static void testAjoutSuppressionStockEtPoints(void)
{
    Boutique boutique;
    Client *client;
    Commande *commande;
    int numero;

    preparerBoutique(&boutique);
    client = &boutique.clients[0];
    assert(creerCommande(&boutique, 1, &numero) == OK);
    assert(numero == 100);

    assert(ajouterLigneCommande(&boutique, 1, numero, "SKU1", 1) == OK);
    commande = &client->commandes[0];
    assert(boutique.produits[0].quantiteStock == 4);
    assert(commande->nbLignes == 1);
    assert(commande->lignes[0].quantite == 1);
    assert(client->pointsFidelite == 6);

    /* Le deuxieme ajout augmente la ligne existante et recalcule les points. */
    assert(ajouterLigneCommande(&boutique, 1, numero, "SKU1", 1) == OK);
    assert(boutique.produits[0].quantiteStock == 3);
    assert(commande->nbLignes == 1);
    assert(commande->lignes[0].quantite == 2);
    assert(client->pointsFidelite == 11);

    /* La derniere ligne ne peut etre retiree sans annuler la commande. */
    assert(supprimerLigneCommande(&boutique, 1, numero, "SKU1")
           == ERR_INTERDIT);
    assert(boutique.produits[0].quantiteStock == 3);
    assert(commande->nbLignes == 1);
    assert(client->pointsFidelite == 11);
    assert(annulerCommande(&boutique, 1, numero) == OK);
    assert(boutique.produits[0].quantiteStock == 5);
    assert(client->pointsFidelite == 0);

    libererBoutiqueTest(&boutique);
    puts("OK ajout/suppression : stock et points de fidelite ajustes");
}

static void testStockInsuffisantEtAnnulation(void)
{
    Boutique boutique;
    Client *client;
    Commande *commande;
    int numero;

    preparerBoutique(&boutique);
    client = &boutique.clients[0];
    assert(creerCommande(&boutique, 1, &numero) == OK);
    assert(ajouterLigneCommande(&boutique, 1, numero, "SKU1", 6)
           == ERR_INTERDIT);
    assert(boutique.produits[0].quantiteStock == 5);
    assert(client->commandes[0].nbLignes == 0);

    assert(ajouterLigneCommande(&boutique, 1, numero, "SKU1", 2) == OK);
    assert(client->pointsFidelite == 11);
    assert(annulerCommande(&boutique, 1, numero) == OK);

    commande = &client->commandes[0];
    assert(commande->statut == ANNULEE);
    assert(boutique.produits[0].quantiteStock == 5);
    assert(client->pointsFidelite == 0);
    assert(annulerCommande(&boutique, 1, numero) == ERR_INTERDIT);

    libererBoutiqueTest(&boutique);
    puts("OK annulation : stock restitue et points retires");
}

static void testTransitionsDeStatut(void)
{
    Boutique boutique;
    int numero;

    preparerBoutique(&boutique);
    assert(creerCommande(&boutique, 1, &numero) == OK);
    assert(changerStatutCommande(&boutique, 1, numero, EXPEDIEE)
           == ERR_INTERDIT);
    assert(ajouterLigneCommande(&boutique, 1, numero, "SKU1", 1) == OK);
    assert(changerStatutCommande(&boutique, 1, numero, EXPEDIEE) == OK);
    assert(changerStatutCommande(&boutique, 1, numero, LIVREE) == OK);
    assert(changerStatutCommande(&boutique, 1, numero, EN_COURS)
           == ERR_INTERDIT);

    libererBoutiqueTest(&boutique);
    puts("OK statuts : expedition puis livraison, retour refuse");
}

static void testFideliteArrondiEtCumul(void)
{
    Boutique boutique;
    Client *client;
    int premiere, seconde;

    preparerBoutique(&boutique);
    client = &boutique.clients[0];
    /* 20,08 HT + TVA + port = 29,996 TTC, facture arrondie a 30,00. */
    boutique.produits[0].prixUnitaireHT = 20.08f;
    assert(creerCommande(&boutique, 1, &premiere) == OK);
    assert(client->pointsFidelite == 0);
    assert(ajouterLigneCommande(&boutique, 1, premiere, "SKU1", 1) == OK);
    assert(client->pointsFidelite == 3);

    boutique.produits[0].prixUnitaireHT = 20.07f;
    assert(creerCommande(&boutique, 1, &seconde) == OK);
    assert(ajouterLigneCommande(&boutique, 1, seconde, "SKU1", 1) == OK);
    assert(client->pointsFidelite == 5);
    assert(annulerCommande(&boutique, 1, premiere) == OK);
    assert(client->pointsFidelite == 2);
    assert(annulerCommande(&boutique, 1, premiere) == ERR_INTERDIT);
    assert(client->pointsFidelite == 2);
    assert(changerStatutCommande(&boutique, 1, seconde, EXPEDIEE) == OK);
    assert(changerStatutCommande(&boutique, 1, seconde, LIVREE) == OK);
    assert(client->pointsFidelite == 2);

    libererBoutiqueTest(&boutique);
    puts("OK fidelite : arrondi au centime, cumul et absence de double credit");
}

int main(void)
{
    testFideliteArrondiEtCumul();
    testCommandeVideSansFraisDePort();
    testFraisDePortEtSeuilGratuit();
    testRemisesProgressivesEtTVA();
    testAjoutSuppressionStockEtPoints();
    testStockInsuffisantEtAnnulation();
    testTransitionsDeStatut();
    puts("Tous les tests de commandes et facturation sont passes.");
    return 0;
}
