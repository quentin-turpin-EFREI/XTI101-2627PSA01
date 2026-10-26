/* produit.h - premiere partie du catalogue produits */
#ifndef PRODUIT_H
#define PRODUIT_H

#include "../types.h"

/* La structure Produit est deja declaree dans types.h. */
int trouverProduit(const Boutique *boutique, const char *reference);

/* Renvoie 1 si l'ajout reussit, 0 sinon.
 * La boutique doit etre initialisee a zero avant le premier ajout.
 * Son tableau produits devra etre libere avec free en fin de programme. */
int ajouterProduit(Boutique *boutique, const Produit *produit);
void afficherCatalogue(const Boutique *boutique);

/* A faire ensuite : modification, suppression, saisie et gestion du stock. */

#endif /* PRODUIT_H */
