#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Catalogue-produit/produit.h"

static Produit produitValide(const char *reference)
{
    Produit produit = {0};

    strcpy(produit.reference, reference);
    strcpy(produit.designation, "Clavier mecanique");
    strcpy(produit.categorie, "Informatique");
    produit.prixUnitaireHT = 79.90f;
    produit.quantiteStock = 42;
    produit.seuilAlerte = 10;
    return produit;
}

static void liberer(Boutique *boutique)
{
    free(boutique->produits);
    free(boutique->clients);
}

static void testAjoutRechercheEtAgrandissement(void)
{
    Boutique boutique = {0};
    Produit produit;
    char reference[TAILLE_REFERENCE];
    int i;

    produit = produitValide("PRD-0001");
    produit.actif = 0;
    assert(ajouterProduit(&boutique, &produit) == OK);
    assert(boutique.produits[0].actif == 1);
    assert(trouverProduit(&boutique, "PRD-0001") == 0);
    assert(ajouterProduit(&boutique, &produit) == ERR_DOUBLON);
    for (i = 1; i <= CAPACITE_INITIALE + 2; i++) {
        snprintf(reference, sizeof reference, "PRD-%04d", i + 1);
        produit = produitValide(reference);
        assert(ajouterProduit(&boutique, &produit) == OK);
    }
    assert(boutique.nbProduits == CAPACITE_INITIALE + 3);
    assert(boutique.capaciteProduits >= boutique.nbProduits);
    assert(trouverProduit(&boutique, "ABSENT") == -1);
    liberer(&boutique);
    puts("OK testAjoutRechercheEtAgrandissement");
}

static void testValidationEtModification(void)
{
    Boutique boutique = {0};
    Produit produit = produitValide("PRD-0010");

    assert(ajouterProduit(&boutique, &produit) == OK);
    produit = produitValide("   ");
    assert(ajouterProduit(&boutique, &produit) == ERR_INVALIDE);
    produit = produitValide("PRD-0011");
    produit.prixUnitaireHT = 0.0f;
    assert(ajouterProduit(&boutique, &produit) == ERR_INVALIDE);
    produit = produitValide("PRD-0011");
    produit.quantiteStock = -1;
    assert(ajouterProduit(&boutique, &produit) == ERR_INVALIDE);
    produit = produitValide("PRD-0011");
    produit.seuilAlerte = -1;
    assert(ajouterProduit(&boutique, &produit) == ERR_INVALIDE);

    assert(modifierProduit(&boutique, "PRD-0010", 12.5f, 7, 3) == OK);
    assert(boutique.produits[0].prixUnitaireHT == 12.5f);
    assert(boutique.produits[0].quantiteStock == 7);
    assert(boutique.produits[0].seuilAlerte == 3);
    assert(modifierProduit(&boutique, "PRD-0010", -1.0f, 7, 3) == ERR_INVALIDE);
    assert(modifierProduit(&boutique, "ABSENT", 1.0f, 1, 1) == ERR_INTROUVABLE);
    liberer(&boutique);
    puts("OK testValidationEtModification");
}

static void testSuppressionSelonCommandes(void)
{
    Boutique boutique = {0};
    Produit produit = produitValide("PRD-0020");
    Client client = {0};
    Commande commandes[2] = {{0}};
    LigneCommande ligne = {0};

    assert(ajouterProduit(&boutique, &produit) == OK);
    boutique.clients = calloc(1, sizeof *boutique.clients);
    assert(boutique.clients != NULL);
    boutique.nbClients = 1;
    boutique.capaciteClients = 1;
    client.commandes = commandes;
    boutique.clients[0] = client;
    boutique.clients[0].nbCommandes = 1;
    boutique.clients[0].capaciteCommandes = 2;
    strcpy(ligne.refProduit, "PRD-0020");
    commandes[0].lignes = &ligne;
    commandes[0].nbLignes = 1;
    commandes[0].statut = EN_COURS;

    assert(supprimerProduit(&boutique, "PRD-0020") == ERR_INTERDIT);
    commandes[0].statut = LIVREE;
    assert(supprimerProduit(&boutique, "PRD-0020") == ERR_INTERDIT);
    commandes[0].statut = ANNULEE;
    assert(supprimerProduit(&boutique, "PRD-0020") == OK);
    assert(boutique.produits[0].actif == 0);
    assert(trouverProduit(&boutique, "PRD-0020") == 0);
    assert(ajouterProduit(&boutique, &produit) == ERR_DOUBLON);
    assert(supprimerProduit(&boutique, "PRD-0020") == ERR_INTROUVABLE);
    boutique.clients[0].commandes = NULL;
    liberer(&boutique);
    puts("OK testSuppressionSelonCommandes");
}

static void testStockEtValorisation(void)
{
    Boutique boutique = {0};
    Produit clavier = produitValide("PRD-0030");
    Produit souris = produitValide("PRD-0031");

    clavier.quantiteStock = 2;
    souris.prixUnitaireHT = 10.0f;
    souris.quantiteStock = 3;
    souris.actif = 0;
    assert(ajouterProduit(&boutique, &clavier) == OK);
    assert(ajouterProduit(&boutique, &souris) == OK);
    boutique.produits[1].actif = 0;
    assert(valeurStock(&boutique) > 159.7f &&
           valeurStock(&boutique) < 159.9f);
    assert(retirerStock(&boutique, "PRD-0030", 1) == OK);
    assert(boutique.produits[0].quantiteStock == 1);
    assert(retirerStock(&boutique, "PRD-0030", 2) == ERR_INTERDIT);
    assert(retirerStock(&boutique, "PRD-0030", 0) == ERR_INVALIDE);
    assert(remettreStock(&boutique, "PRD-0030", 4) == OK);
    assert(boutique.produits[0].quantiteStock == 5);
    assert(remettreStock(&boutique, "ABSENT", 1) == ERR_INTROUVABLE);
    assert(remettreStock(&boutique, "PRD-0030", -1) == ERR_INVALIDE);
    assert(retirerStock(&boutique, "PRD-0031", 1) == ERR_INTROUVABLE);
    liberer(&boutique);
    puts("OK testStockEtValorisation");
}

int main(void)
{
    testAjoutRechercheEtAgrandissement();
    testValidationEtModification();
    testSuppressionSelonCommandes();
    testStockEtValorisation();
    puts("Tous les tests produits sont passes.");
    return 0;
}
