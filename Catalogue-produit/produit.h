/* produit.h - catalogue, stock et alertes de reapprovisionnement */
#ifndef PRODUIT_H
#define PRODUIT_H

#include "../types.h"

/* Produit est partage avec les autres modules : sa structure est dans types.h.
 * Initialiser Boutique a zero avant le premier ajout ; liberer b->produits
 * a la destruction de la boutique. Un ajout peut deplacer le tableau. */
int trouverProduit(const Boutique *b, const char *reference);
CodeRetour ajouterProduit(Boutique *b, const Produit *produit);
CodeRetour modifierProduit(Boutique *b, const char *reference,
                          float prix, int stock, int seuil);
CodeRetour supprimerProduit(Boutique *b, const char *reference);
CodeRetour retirerStock(Boutique *b, const char *reference, int quantite);
CodeRetour remettreStock(Boutique *b, const char *reference, int quantite);
float valeurStock(const Boutique *b);

void afficherProduit(const Produit *produit);
void afficherCatalogue(const Boutique *b);
void afficherAlertesStock(const Boutique *b);
void menuAjouterProduit(Boutique *b);
void menuModifierProduit(Boutique *b);
void menuSupprimerProduit(Boutique *b);
void menuRechercherProduit(const Boutique *b);

#endif /* PRODUIT_H */
