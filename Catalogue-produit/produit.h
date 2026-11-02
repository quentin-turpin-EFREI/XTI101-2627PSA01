/* produit.h - V1 du catalogue */
#ifndef PRODUIT_H
#define PRODUIT_H

#include "../types.h"

/* Boutique doit etre initialisee a zero avant le premier ajout.
 * Le tableau produits sera libere avec free dans detruireBoutique.
 * OK signifie reussite ; ERR_... indique une erreur. */
int trouverProduit(Boutique *boutique, const char *reference);
CodeRetour ajouterProduit(Boutique *boutique, const Produit *produit);
CodeRetour modifierProduit(Boutique *boutique, const char *reference,
                          float prix, int stock, int seuil);
CodeRetour supprimerProduit(Boutique *boutique, const char *reference);
void afficherCatalogue(Boutique *boutique);
void rechercherProduits(Boutique *boutique, const char *recherche);

/* A faire : gestion du stock liee aux commandes et alertes. */
#endif /* PRODUIT_H */
