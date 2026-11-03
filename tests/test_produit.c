#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Catalogue-produit/produit.h"

static void testModificationEtSuppression(void)
{
    Boutique boutique = {0};
    Produit produit = {"TEST-01", "Souris", "Informatique", 20, 3, 1, 1};
    LigneCommande ligne = {"TEST-01", 20, 1};
    Commande commande = {0};
    Client client = {0};

    assert(ajouterProduit(&boutique, &produit) == OK);
    assert(modifierProduit(&boutique, "TEST-01", 15, 0, 0) == OK);
    assert(boutique.produits[0].prixUnitaireHT == 15);
    assert(boutique.produits[0].quantiteStock == 0);
    assert(boutique.produits[0].seuilAlerte == 0);
    assert(strcmp(boutique.produits[0].designation, "Souris") == 0);
    assert(modifierProduit(&boutique, "TEST-01", 0, 4, 2) == ERR_INVALIDE);
    assert(modifierProduit(&boutique, "TEST-01", NAN, 4, 2) == ERR_INVALIDE);
    assert(modifierProduit(&boutique, "TEST-01", INFINITY, 4, 2) == ERR_INVALIDE);
    assert(modifierProduit(&boutique, "TEST-01", 10, -1, 2) == ERR_INVALIDE);
    assert(modifierProduit(&boutique, "TEST-01", 10, 4, -1) == ERR_INVALIDE);
    assert(boutique.produits[0].prixUnitaireHT == 15);
    assert(boutique.produits[0].quantiteStock == 0);
    assert(modifierProduit(&boutique, "ABSENT", 15, 3, 1) == ERR_INTROUVABLE);

    /* Une commande conserve le prix de la vente apres modification du catalogue. */
    assert(ligne.prixUnitaireHT == 20);
    commande.lignes = &ligne;
    commande.nbLignes = 1;
    client.commandes = &commande;
    client.nbCommandes = 1;
    boutique.clients = &client;
    boutique.nbClients = 1;
    commande.statut = EN_COURS;
    assert(supprimerProduit(&boutique, "TEST-01") == ERR_INTERDIT);
    commande.statut = LIVREE;
    assert(supprimerProduit(&boutique, "TEST-01") == ERR_INTERDIT);
    commande.statut = EXPEDIEE;
    assert(supprimerProduit(&boutique, "TEST-01") == ERR_INTERDIT);

    commande.statut = ANNULEE;
    assert(supprimerProduit(&boutique, "TEST-01") == OK);
    assert(boutique.nbProduits == 1 && boutique.produits[0].actif == 0);
    assert(trouverProduit(&boutique, "TEST-01") == 0);
    assert(ajouterProduit(&boutique, &produit) == ERR_DOUBLON);
    assert(modifierProduit(&boutique, "TEST-01", 10, 4, 2) == ERR_INTROUVABLE);
    assert(supprimerProduit(&boutique, "TEST-01") == ERR_INTROUVABLE);
    assert(supprimerProduit(&boutique, "ABSENT") == ERR_INTROUVABLE);

    strcpy(produit.reference, "TEST-02");
    assert(ajouterProduit(&boutique, &produit) == OK);
    assert(supprimerProduit(&boutique, "TEST-02") == OK);
    free(boutique.produits);
    puts("Modification et suppression : OK");
}

int main(void)
{
    Boutique boutique = {0};
    Produit produit = {"PRD-0001", "Clavier", "Informatique", 20, 3, 1, 0};
    int i;

    /* Ajout, activation du produit et doublons. */
    assert(trouverProduit(&boutique, "PRD-0001") == -1);
    assert(ajouterProduit(&boutique, &produit) == OK);
    assert(boutique.produits[0].actif == 1);
    assert(trouverProduit(&boutique, "PRD-0001") == 0);
    assert(ajouterProduit(&boutique, &produit) == ERR_DOUBLON);
    assert(boutique.nbProduits == 1);

    /* Champs obligatoires et valeurs numeriques. */
    strcpy(produit.reference, "PRD-0002");
    produit.prixUnitaireHT = 0;
    assert(ajouterProduit(&boutique, &produit) == ERR_INVALIDE);
    produit.prixUnitaireHT = -1.0f;
    assert(ajouterProduit(&boutique, &produit) == ERR_INVALIDE);
    produit.prixUnitaireHT = NAN;
    assert(ajouterProduit(&boutique, &produit) == ERR_INVALIDE);
    produit.prixUnitaireHT = INFINITY;
    assert(ajouterProduit(&boutique, &produit) == ERR_INVALIDE);
    produit.prixUnitaireHT = 20;
    produit.reference[0] = '\0';
    assert(ajouterProduit(&boutique, &produit) == ERR_INVALIDE);
    strcpy(produit.reference, "PRD-0002");
    produit.categorie[0] = '\0';
    assert(ajouterProduit(&boutique, &produit) == ERR_INVALIDE);
    strcpy(produit.categorie, "Informatique");
    produit.quantiteStock = -1;
    assert(ajouterProduit(&boutique, &produit) == ERR_INVALIDE);
    produit.quantiteStock = 3;
    produit.seuilAlerte = -1;
    assert(ajouterProduit(&boutique, &produit) == ERR_INVALIDE);
    produit.seuilAlerte = 1;
    produit.designation[0] = '\0';
    assert(ajouterProduit(&boutique, &produit) == ERR_INVALIDE);
    strcpy(produit.designation, "Clavier");

    /* Force plusieurs agrandissements du tableau. */
    for (i = 2; i <= 10; i++) {
        snprintf(produit.reference, sizeof produit.reference, "PRD-%04d", i);
        assert(ajouterProduit(&boutique, &produit) == OK);
    }
    assert(boutique.nbProduits == 10);
    assert(boutique.capaciteProduits >= 10);
    assert(trouverProduit(&boutique, "PRD-0010") == 9);
    assert(trouverProduit(&boutique, "ABSENT") == -1);
    afficherCatalogue(&boutique);
    free(boutique.produits);
    testModificationEtSuppression();
    puts("Tous les tests produits sont passes.");
    return 0;
}
