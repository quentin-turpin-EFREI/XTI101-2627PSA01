#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Catalogue-produit/produit.h"

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
    puts("Tous les tests produits V1 sont passes.");
    return 0;
}
