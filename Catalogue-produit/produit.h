/* produit.h - V1 du catalogue */
#ifndef PRODUIT_H
#define PRODUIT_H

#include "../types.h"

/* Boutique doit etre initialisee a zero avant le premier ajout.
 * Le tableau produits sera libere avec free dans detruireBoutique.
 * OK signifie reussite ; ERR_... indique une erreur. */
int trouverProduit(const Boutique *boutique, const char *reference);
CodeRetour ajouterProduit(Boutique *boutique, const Produit *produit);
void afficherCatalogue(const Boutique *boutique);

/* A faire : saisie au clavier, modification, suppression et gestion du stock. */
#endif /* PRODUIT_H */
